#pragma once

#include <JuceHeader.h>
#include "GridEngine.h"

class GridWalkerAudioProcessor : public juce::AudioProcessor {
public:
    GridWalkerAudioProcessor();
    ~GridWalkerAudioProcessor() override = default;

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

    gridwalker::GridEngine& engine() { return engine_; }
    const gridwalker::GridEngine& engine() const { return engine_; }

    juce::AudioProcessorValueTreeState apvts;

    void setSequencerRunning(bool running);
    [[nodiscard]] bool isSequencerRunning() const { return sequencerRunning_.load(); }
    [[nodiscard]] bool usingDefaultRoot() const { return usingDefaultRoot_; }
    void syncEngineParams();

private:
    gridwalker::GridEngine engine_;

    std::atomic<bool> sequencerRunning_{true};

    double sampleRate_ = 44100.0;
    double sampleClock_ = 0.0;
    double bpm_ = 120.0;
    int heldRoot_ = 60;
    bool rootActive_ = false;
    bool usingDefaultRoot_ = true;
    int activeNote_ = -1;
    std::array<bool, 128> heldNotes_{};

    void updateHeldRoot(const juce::MidiBuffer& midi);
    void processSequencer(juce::MidiBuffer& midi, int numSamples);
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridWalkerAudioProcessor)
};
