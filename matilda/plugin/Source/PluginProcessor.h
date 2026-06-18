#pragma once

#include <JuceHeader.h>
#include "Engine/SequencerEngine.h"
#include "Engine/PatchStore.h"

class MatildaAudioProcessor : public juce::AudioProcessor,
                              public juce::ChangeBroadcaster,
                              private juce::AsyncUpdater {
public:
    MatildaAudioProcessor();
    ~MatildaAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    matilda::PatchState& patch() { return patch_; }
    const matilda::PatchState& patch() const { return patch_; }
    matilda::SequencerEngine& engine() { return engine_; }

    void setSequencerRunning(bool running);
    [[nodiscard]] bool isSequencerRunning() const { return sequencerRunning_.load(); }
    [[nodiscard]] double getBpm() const { return bpm_; }
    [[nodiscard]] bool followExternalTransport() const { return followExternalTransport_.load(); }
    void setFollowExternalTransport(bool enabled);

    /** Reload patch from JSON string (preset import / host restore). */
    void applyPatchJson(const juce::String& json);

private:
    matilda::PatchState patch_;
    matilda::SequencerEngine engine_;

    std::atomic<bool> sequencerRunning_{false};
    std::atomic<bool> panicRequested_{false};
    std::atomic<bool> followExternalTransport_{true};

    double sampleRate_ = 44100.0;
    double sampleClock_ = 0.0;
    double bpm_ = 120.0;
    int activeNote_ = -1;
    bool sequencerWasRunning_ = false;

    bool useExternalMidiClock_ = false;
    int midiClockAccumulator_ = 0;

    static constexpr int kMidiChannel = 1;
    static constexpr int kMaxTicksPerBlock = 1;
    static constexpr double kStandaloneBpm = 120.0;

    void handleAsyncUpdate() override;

    [[nodiscard]] int midiClocksPerStep() const;
    [[nodiscard]] double samplesPerStep(int numSamples) const;

    void panicNotes(juce::MidiBuffer& midi, int samplePos);
    void sendNoteOff(juce::MidiBuffer& midi, int note, int samplePos);
    void emitStepNote(juce::MidiBuffer& midi, int samplePos);
    void advanceSequencerStep(juce::MidiBuffer& midi, int samplePos);
    void handleIncomingMidi(const juce::MidiBuffer& incoming, juce::MidiBuffer& outgoing);
    void processSequencer(juce::MidiBuffer& midi, int numSamples);
    void loadStartupPreset();
    void notifyTransportChanged();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatildaAudioProcessor)
};
