#include "PluginProcessor.h"
#include "PluginEditor.h"

MatildaAudioProcessor::MatildaAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      engine_(patch_) {
    loadStartupPreset();
}

void MatildaAudioProcessor::loadStartupPreset() {
    matilda::PatchStore::loadDefaultPreset(patch_);
    engine_.reset();
}

void MatildaAudioProcessor::applyPatchJson(const juce::String& json) {
    matilda::PatchState next;
    if (!matilda::PatchStore::patchFromJson(json, next))
        return;
    patch_ = next;
    engine_.reset();
    sendChangeMessage();
}

void MatildaAudioProcessor::setFollowExternalTransport(bool enabled) {
    followExternalTransport_.store(enabled);
    notifyTransportChanged();
}

void MatildaAudioProcessor::notifyTransportChanged() {
    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        sendChangeMessage();
    else
        triggerAsyncUpdate();
}

void MatildaAudioProcessor::handleAsyncUpdate() {
    sendChangeMessage();
}

void MatildaAudioProcessor::setSequencerRunning(bool running) {
    sequencerRunning_.store(running);
    if (!running) {
        sampleClock_ = 0.0;
        useExternalMidiClock_ = false;
        midiClockAccumulator_ = 0;
        panicRequested_.store(true);
    } else {
        sampleClock_ = 0.0;
        activeNote_ = -1;
        sequencerWasRunning_ = false;
        useExternalMidiClock_ = false;
        midiClockAccumulator_ = 0;
        engine_.reset();
    }
    notifyTransportChanged();
}

void MatildaAudioProcessor::prepareToPlay(double sampleRate, int) {
    sampleRate_ = sampleRate;
    sampleClock_ = 0.0;
}

void MatildaAudioProcessor::releaseResources() {}

bool MatildaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

int MatildaAudioProcessor::midiClocksPerStep() const {
    const double beatsPerStep = juce::jmax(0.001, patch_.masterDivision * 4.0);
    return juce::jmax(1, static_cast<int>(std::round(beatsPerStep * 24.0)));
}

double MatildaAudioProcessor::samplesPerStep(int numSamples) const {
    const double beatsPerStep = juce::jmax(0.001, patch_.masterDivision * 4.0);
    const double stepsPerSecond = juce::jmax(0.001, (bpm_ / 60.0) / beatsPerStep);
    return juce::jmax(static_cast<double>(numSamples), sampleRate_ / stepsPerSecond);
}

void MatildaAudioProcessor::sendNoteOff(juce::MidiBuffer& midi, int note, int samplePos) {
    if (note < 0 || note > 127)
        return;
    midi.addEvent(juce::MidiMessage::noteOff(kMidiChannel, note), samplePos);
    // Some hosts (including GarageBand via IAC) respond more reliably to vel-0 noteOn.
    midi.addEvent(juce::MidiMessage::noteOn(kMidiChannel, note, static_cast<juce::uint8>(0)), samplePos);
}

void MatildaAudioProcessor::panicNotes(juce::MidiBuffer& midi, int samplePos) {
    if (activeNote_ >= 0) {
        sendNoteOff(midi, activeNote_, samplePos);
        activeNote_ = -1;
    }

    midi.addEvent(juce::MidiMessage::allNotesOff(kMidiChannel), samplePos);
}

void MatildaAudioProcessor::emitStepNote(juce::MidiBuffer& midi, int samplePos) {
    const int layer = engine_.lastPlayingLayer();
    const int step = engine_.lastStepIndex();
    const int x = step % matilda::kGridSize;
    const int y = step / matilda::kGridSize;
    const auto& cell = engine_.cell(layer, x, y);

    // GridWalker-style mono legato: one note at a time, noteOff before each new noteOn.
    if (activeNote_ >= 0)
        sendNoteOff(midi, activeNote_, samplePos);

    activeNote_ = engine_.resolveFiredMidiNote(cell);
    midi.addEvent(juce::MidiMessage::noteOn(kMidiChannel, activeNote_,
                                            static_cast<juce::uint8>(cell.velocity)),
                  samplePos);
}

void MatildaAudioProcessor::advanceSequencerStep(juce::MidiBuffer& midi, int samplePos) {
    engine_.tick();
    if (engine_.lastStepFired())
        emitStepNote(midi, samplePos);
}

void MatildaAudioProcessor::handleIncomingMidi(const juce::MidiBuffer& incoming, juce::MidiBuffer& outgoing) {
    if (!followExternalTransport_.load())
        return;

    const bool isStandalone = (wrapperType == juce::AudioProcessor::wrapperType_Standalone);
    if (!isStandalone)
        return;

    for (const auto metadata : incoming) {
        const auto msg = metadata.getMessage();

        if (msg.isMidiClock()) {
            if (!sequencerRunning_.load())
                continue;

            useExternalMidiClock_ = true;
            ++midiClockAccumulator_;
            if (midiClockAccumulator_ < midiClocksPerStep())
                continue;

            midiClockAccumulator_ = 0;
            advanceSequencerStep(outgoing, 0);
            continue;
        }

        if (msg.isMidiStart() || msg.isMidiContinue()) {
            if (!sequencerRunning_.load()) {
                midiClockAccumulator_ = 0;
                sampleClock_ = 0.0;
                activeNote_ = -1;
                engine_.reset();
                // Lock to MIDI clock immediately so internal sample clock cannot double-step.
                useExternalMidiClock_ = true;
                sequencerRunning_.store(true);
                sequencerWasRunning_ = true;
                notifyTransportChanged();
            }
            continue;
        }

        if (msg.isMidiStop()) {
            if (sequencerRunning_.load()) {
                sequencerRunning_.store(false);
                useExternalMidiClock_ = false;
                midiClockAccumulator_ = 0;
                panicRequested_.store(true);
                notifyTransportChanged();
            }
        }
    }
}

void MatildaAudioProcessor::processSequencer(juce::MidiBuffer& midi, int numSamples) {
    const bool isStandalone = (wrapperType == juce::AudioProcessor::wrapperType_Standalone);

    bool hostPlaying = false;
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            hostPlaying = pos->getIsPlaying();
            if (!isStandalone && pos->getBpm())
                bpm_ = *pos->getBpm();
        }
    }

    if (isStandalone)
        bpm_ = kStandaloneBpm;

    const bool shouldRun = isStandalone ? sequencerRunning_.load() : hostPlaying;
    if (!shouldRun) {
        if (sequencerWasRunning_ || activeNote_ >= 0) {
            panicNotes(midi, 0);
            sequencerWasRunning_ = false;
        }
        return;
    }

    sequencerWasRunning_ = true;

    if (sampleRate_ <= 0.0 || numSamples <= 0)
        return;

    // When GarageBand is sending MIDI clock, steps are driven in handleIncomingMidi().
    if (useExternalMidiClock_ && isStandalone)
        return;

    const double stepSamples = samplesPerStep(numSamples);
    sampleClock_ += static_cast<double>(numSamples);

    int ticksThisBlock = 0;
    while (sampleClock_ >= stepSamples && ticksThisBlock < kMaxTicksPerBlock) {
        sampleClock_ -= stepSamples;
        ++ticksThisBlock;
        advanceSequencerStep(midi, 0);
    }
}

void MatildaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    buffer.clear();

    juce::MidiBuffer incoming;
    incoming.swapWith(midi);

    handleIncomingMidi(incoming, midi);

    if (panicRequested_.exchange(false))
        panicNotes(midi, 0);

    processSequencer(midi, buffer.getNumSamples());
}

juce::AudioProcessorEditor* MatildaAudioProcessor::createEditor() {
    return new MatildaAudioProcessorEditor(*this);
}

void MatildaAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::ValueTree state("MatildaState");
    state.setProperty("patchJson", matilda::PatchStore::patchToJson(patch_), nullptr);
    state.setProperty("followExternalTransport", followExternalTransport_.load(), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void MatildaAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        const auto vt = juce::ValueTree::fromXml(*xml);
        const auto json = vt.getProperty("patchJson", {}).toString();
        followExternalTransport_.store(static_cast<bool>(vt.getProperty("followExternalTransport", true)));
        if (json.isNotEmpty())
            applyPatchJson(json);
        else {
            patch_.root = vt.getProperty("root", patch_.root).toString();
            patch_.mode = vt.getProperty("mode", patch_.mode).toString();
            patch_.selectedLayer = static_cast<int>(vt.getProperty("selectedLayer", patch_.selectedLayer));
            engine_.reset();
            sendChangeMessage();
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new MatildaAudioProcessor();
}
