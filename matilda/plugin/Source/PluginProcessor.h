#pragma once

#include <JuceHeader.h>
#include "Engine/SequencerEngine.h"
#include "Engine/PatchStore.h"
#include "UiScale.h"

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
    bool isSynth() const { return false; }
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
    [[nodiscard]] bool isSequencerRunning() const { return sequencerArmed_.load(); }
    [[nodiscard]] bool isSequencerStepping() const { return sequencerStepping_.load(); }
    [[nodiscard]] bool isStandaloneWrapper() const {
        return wrapperType == juce::AudioProcessor::wrapperType_Standalone;
    }
    [[nodiscard]] double getBpm() const { return bpm_; }
    [[nodiscard]] double getUserBpm() const { return userBpm_; }
    void setUserBpm(double bpm);
    [[nodiscard]] bool hasExternalTempo() const { return externalTempoHoldBlocks_ > 0; }
    [[nodiscard]] bool followExternalTransport() const { return followExternalTransport_.load(); }
    void setFollowExternalTransport(bool enabled);
    /** VST/AU — follow host play/stop (Global Settings → DAW Sync). */
    [[nodiscard]] bool syncHostTransport() const { return syncHostTransport_.load(); }
    void setSyncHostTransport(bool enabled);

    /** Reload patch from JSON string (preset import / host restore). */
    void applyPatchJson(const juce::String& json);

    [[nodiscard]] float editorUiScale() const { return editorUiScale_; }
    void setEditorUiScale(float factor);
    [[nodiscard]] bool editorShellCollapsed() const { return editorShellCollapsed_; }
    void setEditorShellCollapsed(bool collapsed);

private:
    matilda::PatchState patch_;
    matilda::SequencerEngine engine_;

    std::atomic<bool> sequencerArmed_{false};
    std::atomic<bool> sequencerStepping_{false};
    std::atomic<bool> panicRequested_{false};
    std::atomic<bool> followExternalTransport_{false};
    std::atomic<bool> syncHostTransport_{true};
    std::atomic<bool> hostTransportEverDetected_{false};
    std::atomic<bool> hostOpaqueFallback_{false};

    float editorUiScale_ = matilda::ui::kUiScaleDefault;
    bool editorShellCollapsed_ = false;

    double sampleRate_ = 44100.0;
    double sampleClock_ = 0.0;
    double bpm_ = kFallbackBpm;
    double userBpm_ = kFallbackBpm;
    int externalTempoHoldBlocks_ = 0;
    int activeNote_ = -1;
    bool sequencerWasRunning_ = false;
    bool hostWasPlaying_ = false;
    bool pendingBeatStart_ = false;
    bool pendingMidiBeatStart_ = false;
    std::atomic<bool> midiTransportRunning_{false};
    double pendingStartSampleCountdown_ = 0.0;
    int64_t standaloneTransportSamples_ = 0;

    bool useExternalMidiClock_ = false;
    int midiClockAccumulator_ = 0;
    int midiClockSampleCounter_ = 0;

    static constexpr int kMidiChannel = 1;
    static constexpr int kMaxTicksPerBlock = 1;
    static constexpr double kFallbackBpm = 120.0;
    static constexpr int kExternalTempoHoldBlocks = 220;

    void handleAsyncUpdate() override;

    [[nodiscard]] int midiClocksPerStep() const;
    [[nodiscard]] double samplesPerStep(int numSamples) const;

    void updateTempoFromPlayHead();
    void updateTempoFromMidiClockInterval(int samplesSinceLastClock);
    void scanIncomingMidiForTempo(const juce::MidiBuffer& incoming);
    void markExternalTempo(double bpm);
    void finalizeTempoForBlock();

    [[nodiscard]] double samplesPerBeat() const;
    [[nodiscard]] double samplesUntilNextBeat() const;
    void requestBeatQuantizedStart();
    void cancelBeatQuantizedStart();
    void beginSequencerOnBeat(juce::MidiBuffer& midi, int samplePos);
    void panicNotes(juce::MidiBuffer& midi, int samplePos);
    void sendNoteOff(juce::MidiBuffer& midi, int note, int samplePos);
    void emitStepNote(juce::MidiBuffer& midi, int samplePos);
    void advanceSequencerStep(juce::MidiBuffer& midi, int samplePos);
    void handleTransportMidi(const juce::MidiBuffer& incoming, juce::MidiBuffer& outgoing);
    void processSequencer(juce::MidiBuffer& midi, int numSamples);
    void loadStartupPreset();
    void notifyTransportChanged();
    void markHostTransportDetected();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MatildaAudioProcessor)
};
