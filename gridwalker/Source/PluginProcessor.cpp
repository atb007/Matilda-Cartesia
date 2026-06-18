#include "PluginProcessor.h"
#include "PluginEditor.h"

GridWalkerAudioProcessor::GridWalkerAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameterLayout()) {}

void GridWalkerAudioProcessor::setSequencerRunning(bool running) {
    sequencerRunning_.store(running);
    if (!running) {
        sampleClock_ = 0.0;
        if (activeNote_ >= 0)
            activeNote_ = -1;
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout GridWalkerAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add(std::make_unique<juce::AudioParameterChoice>("scale", "Scale",
        juce::StringArray{"Major", "Minor", "Dorian", "Pentatonic", "Chromatic"}, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>("quantize", "Quantize", true));
    layout.add(std::make_unique<juce::AudioParameterInt>("minDegree", "Min", 0, 11, 0));
    layout.add(std::make_unique<juce::AudioParameterInt>("range", "Range", 1, 24, 12));
    layout.add(std::make_unique<juce::AudioParameterChoice>("masterDiv", "Master",
        juce::StringArray{"1/4", "1/8", "1/16", "1/32"}, 2));
    layout.add(std::make_unique<juce::AudioParameterChoice>("xDiv", "X Div",
        juce::StringArray{"1", "2", "4", "8", "16"}, 0));
    layout.add(std::make_unique<juce::AudioParameterChoice>("yDiv", "Y Div",
        juce::StringArray{"1", "2", "4", "8", "16"}, 2));
    layout.add(std::make_unique<juce::AudioParameterInt>("activeLayer", "Layer", 0, 3, 0));
    return layout;
}

void GridWalkerAudioProcessor::prepareToPlay(double sampleRate, int) {
    sampleRate_ = sampleRate;
    sampleClock_ = 0.0;
}

void GridWalkerAudioProcessor::releaseResources() {}

bool GridWalkerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void GridWalkerAudioProcessor::syncEngineParams() {
    engine_.quantize = apvts.getRawParameterValue("quantize")->load() > 0.5f;
    engine_.scaleIndex = static_cast<int>(apvts.getRawParameterValue("scale")->load());
    engine_.minDegree = static_cast<int>(apvts.getRawParameterValue("minDegree")->load());
    engine_.rangeSemitones = static_cast<int>(apvts.getRawParameterValue("range")->load());
    engine_.activeLayer = static_cast<int>(apvts.getRawParameterValue("activeLayer")->load());
}

void GridWalkerAudioProcessor::updateHeldRoot(const juce::MidiBuffer& midi) {
    for (const auto metadata : midi) {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            heldNotes_[static_cast<size_t>(msg.getNoteNumber())] = true;
        else if (msg.isNoteOff())
            heldNotes_[static_cast<size_t>(msg.getNoteNumber())] = false;
    }

    int lowest = -1;
    for (int n = 0; n < 128; ++n)
        if (heldNotes_[static_cast<size_t>(n)] && (lowest < 0 || n < lowest))
            lowest = n;

    rootActive_ = lowest >= 0;
    if (rootActive_)
        heldRoot_ = lowest;
}

void GridWalkerAudioProcessor::processSequencer(juce::MidiBuffer& midi, int numSamples) {
    const bool isStandalone = (wrapperType == juce::AudioProcessor::wrapperType_Standalone);

    bool hostPlaying = false;
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            hostPlaying = pos->getIsPlaying();
            if (pos->getBpm())
                bpm_ = *pos->getBpm();
        }
    }

    const bool midiGate = rootActive_;
    usingDefaultRoot_ = isStandalone && sequencerRunning_.load() && !midiGate;

    engine_.setRootMidi(midiGate ? heldRoot_ : 60);
    engine_.setRootActive(midiGate || usingDefaultRoot_);

    const bool shouldRun = isStandalone
        ? sequencerRunning_.load() && (midiGate || usingDefaultRoot_)
        : hostPlaying && midiGate;

    if (!shouldRun) {
        if (activeNote_ >= 0) {
            midi.addEvent(juce::MidiMessage::noteOff(1, activeNote_), 0);
            activeNote_ = -1;
        }
        return;
    }

    const int masterDiv = static_cast<int>(apvts.getRawParameterValue("masterDiv")->load());
    static const int divSteps[] = {4, 8, 16, 32};
    engine_.masterDivision = divSteps[juce::jlimit(0, 3, masterDiv)];

    static const int axisDivs[] = {1, 2, 4, 8, 16};
    engine_.x_.division = axisDivs[juce::jlimit(0, 4, static_cast<int>(apvts.getRawParameterValue("xDiv")->load()))];
    engine_.y_.division = axisDivs[juce::jlimit(0, 4, static_cast<int>(apvts.getRawParameterValue("yDiv")->load()))];
    engine_.quantize = apvts.getRawParameterValue("quantize")->load() > 0.5f;
    engine_.scaleIndex = static_cast<int>(apvts.getRawParameterValue("scale")->load());
    engine_.minDegree = static_cast<int>(apvts.getRawParameterValue("minDegree")->load());
    engine_.rangeSemitones = static_cast<int>(apvts.getRawParameterValue("range")->load());
    engine_.activeLayer = static_cast<int>(apvts.getRawParameterValue("activeLayer")->load());

    const double stepsPerSecond = (bpm_ / 60.0) * (static_cast<double>(engine_.masterDivision) / 4.0);
    const double samplesPerStep = sampleRate_ / stepsPerSecond;

    sampleClock_ += static_cast<double>(numSamples);
    while (sampleClock_ >= samplesPerStep) {
        sampleClock_ -= samplesPerStep;

        const auto step = engine_.advanceTick();
        if (step.fired && step.midiNote) {
            if (activeNote_ >= 0)
                midi.addEvent(juce::MidiMessage::noteOff(1, activeNote_), 0);
            activeNote_ = *step.midiNote;
            midi.addEvent(juce::MidiMessage::noteOn(1, activeNote_, static_cast<juce::uint8>(step.velocity)), 0);
        }
    }
}

void GridWalkerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    buffer.clear();
    juce::MidiBuffer incoming;
    incoming.swapWith(midi);
    updateHeldRoot(incoming);
    processSequencer(midi, buffer.getNumSamples());
}

juce::AudioProcessorEditor* GridWalkerAudioProcessor::createEditor() {
    return new GridWalkerAudioProcessorEditor(*this);
}

void GridWalkerAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void GridWalkerAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new GridWalkerAudioProcessor();
}
