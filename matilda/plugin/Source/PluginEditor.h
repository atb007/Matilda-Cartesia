#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MatildaLookAndFeel.h"
#include "Components/GemGrid.h"
#include "Components/LayerOverview.h"
#include "Components/QuantisePanel.h"
#include "Components/TransportBar.h"
#include "Components/MovementSelector.h"
#include "Components/MatildaShellPanel.h"
#include "Components/NativePluginFrame.h"

class MatildaAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer,
                                    private juce::ChangeListener {
public:
    explicit MatildaAudioProcessorEditor(MatildaAudioProcessor&);
    ~MatildaAudioProcessorEditor() override;

private:
    MatildaAudioProcessor& processor_;
    MatildaLookAndFeel laf_;

    TransportBar      transport_{ processor_.patch(), laf_ };
    QuantisePanel     quantise_{ processor_.patch(), laf_ };
    LayerOverview     overview_{ processor_.patch(), laf_ };
    MovementSelector  movement_{ processor_.patch(), laf_ };
    GemGrid           grid_{ processor_.engine(), laf_ };
    MatildaShellPanel shell_{ processor_.patch(), processor_.engine(), laf_, transport_, quantise_,
                              overview_, movement_, grid_ };
    NativePluginFrame frame_{ shell_ };

    juce::Label bpmLabel_{"", "120 BPM"};
    juce::ToggleButton syncToggle_{"Sync GB transport"};
    juce::Label statusLabel_{"", ""};

    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void bindCallbacks();
    void refreshAll();
    void updateStatusLine();
    void layoutChromeOverlays();
    bool lastTransportRunning_ = false;
};
