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
    engine_.requantizeAllCells();
}

void MatildaAudioProcessor::applyPatchJson(const juce::String& json) {
    matilda::PatchState next;
    if (!matilda::PatchStore::patchFromJson(json, next))
        return;
    patch_ = next;
    engine_.reset();
    engine_.requantizeAllCells();
    sendChangeMessage();
}

void MatildaAudioProcessor::setFollowExternalTransport(bool enabled) {
    followExternalTransport_.store(enabled);
    notifyTransportChanged();
}

void MatildaAudioProcessor::setSyncHostTransport(bool enabled) {
    syncHostTransport_.store(enabled);
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
    sequencerArmed_.store(running);
    if (!running) {
        sequencerStepping_.store(false);
        cancelBeatQuantizedStart();
        sampleClock_ = 0.0;
        useExternalMidiClock_ = false;
        midiClockAccumulator_ = 0;
        panicRequested_.store(true);
    } else {
        activeNote_ = -1;
        sequencerWasRunning_ = false;
        useExternalMidiClock_ = false;
        midiClockAccumulator_ = 0;
        requestBeatQuantizedStart();
    }
    notifyTransportChanged();
}

double MatildaAudioProcessor::samplesPerBeat() const {
    if (sampleRate_ <= 0.0)
        return 0.0;
    return sampleRate_ * 60.0 / juce::jmax(20.0, bpm_);
}

double MatildaAudioProcessor::samplesUntilNextBeat() const {
    const double spb = samplesPerBeat();
    if (spb <= 0.0)
        return 0.0;

    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            if (auto ppq = pos->getPpqPosition()) {
                double frac = std::fmod(*ppq, 1.0);
                if (frac < 0.0)
                    frac += 1.0;
                if (frac < 1e-5)
                    return 0.0;
                return (1.0 - frac) * spb;
            }
        }
    }

    const double phase = std::fmod(static_cast<double>(standaloneTransportSamples_), spb);
    if (phase < 1e-5)
        return 0.0;
    return spb - phase;
}

void MatildaAudioProcessor::requestBeatQuantizedStart() {
    pendingBeatStart_ = true;
    pendingMidiBeatStart_ = followExternalTransport_.load() && isStandaloneWrapper();
    pendingStartSampleCountdown_ = samplesUntilNextBeat();
    midiClockAccumulator_ = 0;
    if (pendingStartSampleCountdown_ <= 0.0)
        pendingStartSampleCountdown_ = 0.0;
}

void MatildaAudioProcessor::cancelBeatQuantizedStart() {
    pendingBeatStart_ = false;
    pendingMidiBeatStart_ = false;
    pendingStartSampleCountdown_ = 0.0;
}

void MatildaAudioProcessor::beginSequencerOnBeat(juce::MidiBuffer& midi, int samplePos) {
    pendingBeatStart_ = false;
    pendingMidiBeatStart_ = false;
    pendingStartSampleCountdown_ = 0.0;
    sampleClock_ = 0.0;
    midiClockAccumulator_ = 0;
    activeNote_ = -1;
    engine_.reset();
    sequencerStepping_.store(true);
    sequencerWasRunning_ = true;
    advanceSequencerStep(midi, samplePos);
    notifyTransportChanged();
}

void MatildaAudioProcessor::prepareToPlay(double sampleRate, int) {
    sampleRate_ = sampleRate;
    sampleClock_ = 0.0;
}

void MatildaAudioProcessor::releaseResources() {}

bool MatildaAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
    const auto& in = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::stereo())
        return false;
    return in == juce::AudioChannelSet::stereo() || in == juce::AudioChannelSet::disabled();
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

void MatildaAudioProcessor::setUserBpm(double bpm) {
    userBpm_ = juce::jlimit(20.0, 300.0, bpm);
    if (externalTempoHoldBlocks_ <= 0)
        bpm_ = userBpm_;
    notifyTransportChanged();
}

void MatildaAudioProcessor::markExternalTempo(double bpm) {
    bpm_ = juce::jlimit(20.0, 300.0, bpm);
    externalTempoHoldBlocks_ = kExternalTempoHoldBlocks;
}

void MatildaAudioProcessor::finalizeTempoForBlock() {
    if (externalTempoHoldBlocks_ > 0) {
        --externalTempoHoldBlocks_;
        return;
    }
    bpm_ = userBpm_;
}

void MatildaAudioProcessor::updateTempoFromPlayHead() {
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition()) {
            if (auto hostBpm = pos->getBpm()) {
                const double bpm = *hostBpm;
                if (bpm >= 20.0 && bpm <= 300.0)
                    markExternalTempo(bpm);
            }
        }
    }
}

void MatildaAudioProcessor::updateTempoFromMidiClockInterval(int samplesSinceLastClock) {
    if (samplesSinceLastClock <= 0 || sampleRate_ <= 0.0)
        return;

    const double secondsPerClock = static_cast<double>(samplesSinceLastClock) / sampleRate_;
    if (secondsPerClock <= 0.0)
        return;

    const double estimated = 60.0 / (secondsPerClock * 24.0);
    if (estimated >= 20.0 && estimated <= 300.0)
        markExternalTempo(bpm_ * 0.65 + estimated * 0.35);
}

void MatildaAudioProcessor::scanIncomingMidiForTempo(const juce::MidiBuffer& incoming) {
    for (const auto metadata : incoming) {
        if (metadata.getMessage().isMidiClock()) {
            updateTempoFromMidiClockInterval(midiClockSampleCounter_);
            midiClockSampleCounter_ = 0;
        }
    }
}

void MatildaAudioProcessor::handleIncomingMidi(const juce::MidiBuffer& incoming, juce::MidiBuffer& outgoing) {
    if (!followExternalTransport_.load())
        return;

    const bool isStandalone = isStandaloneWrapper();
    if (!isStandalone)
        return;

    for (const auto metadata : incoming) {
        const auto msg = metadata.getMessage();

        if (msg.isMidiClock()) {
            if (!sequencerArmed_.load())
                continue;

            useExternalMidiClock_ = true;

            if (pendingMidiBeatStart_) {
                ++midiClockAccumulator_;
                if (midiClockAccumulator_ < 24)
                    continue;
                midiClockAccumulator_ = 0;
                beginSequencerOnBeat(outgoing, 0);
                continue;
            }

            if (!sequencerStepping_.load())
                continue;

            ++midiClockAccumulator_;
            if (midiClockAccumulator_ < midiClocksPerStep())
                continue;

            midiClockAccumulator_ = 0;
            advanceSequencerStep(outgoing, 0);
            continue;
        }

        if (msg.isMidiStart() || msg.isMidiContinue()) {
            midiClockSampleCounter_ = 0;
            if (!sequencerArmed_.load()) {
                sampleClock_ = 0.0;
                activeNote_ = -1;
                useExternalMidiClock_ = true;
                sequencerArmed_.store(true);
                requestBeatQuantizedStart();
                notifyTransportChanged();
            }
            continue;
        }

        if (msg.isMidiStop()) {
            if (sequencerArmed_.load() || sequencerStepping_.load()) {
                sequencerArmed_.store(false);
                sequencerStepping_.store(false);
                cancelBeatQuantizedStart();
                useExternalMidiClock_ = false;
                midiClockAccumulator_ = 0;
                panicRequested_.store(true);
                notifyTransportChanged();
            }
        }
    }
}

void MatildaAudioProcessor::processSequencer(juce::MidiBuffer& midi, int numSamples) {
    const bool isStandalone = isStandaloneWrapper();

    bool hostPlaying = false;
    if (auto* playHead = getPlayHead()) {
        if (auto pos = playHead->getPosition())
            hostPlaying = pos->getIsPlaying();
    }

    const bool armed = sequencerArmed_.load();
    const bool hostSync = !isStandalone && syncHostTransport_.load();

    if (hostSync) {
        if (hostPlaying && !hostWasPlaying_) {
            if (!armed)
                setSequencerRunning(true);
            else {
                cancelBeatQuantizedStart();
                requestBeatQuantizedStart();
            }
        } else if (!hostPlaying && hostWasPlaying_ && armed) {
            setSequencerRunning(false);
        }
    } else if (!isStandalone && patch_.playMode == matilda::PlayMode::Transport && armed && hostPlaying
               && !hostWasPlaying_) {
        cancelBeatQuantizedStart();
        requestBeatQuantizedStart();
    }
    hostWasPlaying_ = hostPlaying;

    // GridWalker parity — Standalone+IAC: when sync is on, MIDI clock drives steps.
    if (isStandalone && followExternalTransport_.load() && armed)
        return;

    bool shouldRun = false;
    if (hostSync) {
        shouldRun = hostPlaying;
    } else if (isStandalone || patch_.playMode == matilda::PlayMode::Note) {
        shouldRun = armed;
    } else {
        shouldRun = armed && hostPlaying;
    }

    if (!shouldRun) {
        if (sequencerWasRunning_ || sequencerStepping_.load() || activeNote_ >= 0) {
            sequencerStepping_.store(false);
            cancelBeatQuantizedStart();
            panicNotes(midi, 0);
            sequencerWasRunning_ = false;
        }
        return;
    }

    if (pendingBeatStart_) {
        if (pendingStartSampleCountdown_ >= static_cast<double>(numSamples)) {
            pendingStartSampleCountdown_ -= static_cast<double>(numSamples);
            return;
        }

        const int offset = juce::jlimit(0, numSamples - 1,
                                        static_cast<int>(std::lround(pendingStartSampleCountdown_)));
        beginSequencerOnBeat(midi, offset);
        sampleClock_ = static_cast<double>(numSamples - offset);
        return;
    }

    if (!sequencerStepping_.load())
        return;

    sequencerWasRunning_ = true;

    if (sampleRate_ <= 0.0 || numSamples <= 0)
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
    const int numInput = getTotalNumInputChannels();
    const int numOutput = getTotalNumOutputChannels();
    if (numInput <= 0) {
        buffer.clear();
    } else {
        for (int ch = numInput; ch < numOutput; ++ch)
            buffer.clear(ch, 0, buffer.getNumSamples());
    }

    juce::MidiBuffer incoming;
    incoming.swapWith(midi);

    midiClockSampleCounter_ += buffer.getNumSamples();
    scanIncomingMidiForTempo(incoming);
    updateTempoFromPlayHead();

    handleIncomingMidi(incoming, midi);

    for (const auto metadata : incoming)
        midi.addEvent(metadata.getMessage(), metadata.samplePosition);

    standaloneTransportSamples_ += buffer.getNumSamples();

    if (panicRequested_.exchange(false))
        panicNotes(midi, 0);

    processSequencer(midi, buffer.getNumSamples());
    finalizeTempoForBlock();
}

juce::AudioProcessorEditor* MatildaAudioProcessor::createEditor() {
    return new MatildaAudioProcessorEditor(*this);
}

void MatildaAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    juce::ValueTree state("MatildaState");
    state.setProperty("patchJson", matilda::PatchStore::patchToJson(patch_), nullptr);
    state.setProperty("followExternalTransport", followExternalTransport_.load(), nullptr);
    state.setProperty("syncHostTransport", syncHostTransport_.load(), nullptr);
    state.setProperty("userBpm", userBpm_, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void MatildaAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        const auto vt = juce::ValueTree::fromXml(*xml);
        const auto json = vt.getProperty("patchJson", {}).toString();
        followExternalTransport_.store(static_cast<bool>(vt.getProperty("followExternalTransport", false)));
        syncHostTransport_.store(static_cast<bool>(vt.getProperty("syncHostTransport", true)));
        setUserBpm(static_cast<double>(vt.getProperty("userBpm", kFallbackBpm)));
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
