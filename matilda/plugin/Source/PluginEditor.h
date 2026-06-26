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
#include "Components/UiResizeGrip.h"
#include "UiScale.h"
#include <array>

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
    juce::ToggleButton syncToggle_{"Sync external transport"};
    juce::Label statusLabel_{"", ""};
    juce::TooltipWindow tooltipWindow_{this, 400};

    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void bindCallbacks();
    void refreshAll();
    void updateStatusLine();
    void applyBpmFromLabel();
    void layoutChromeOverlays();
    void applyUiScale();
    void beginGripResize(matilda::ui::UiResizeGripId grip);
    void continueGripResize(juce::Point<int> screenMouse);
    void layoutResizeGrips();
    void updateResizeLimits();
    void syncEditorToViewport();
    [[nodiscard]] juce::Point<int> intendedEditorSize() const;

    bool suppressHostResizeSync_ = false;

    std::array<UiResizeGrip, 8> resizeGrips_ {
        UiResizeGrip(matilda::ui::UiResizeGripId::topLeft),
        UiResizeGrip(matilda::ui::UiResizeGripId::top),
        UiResizeGrip(matilda::ui::UiResizeGripId::topRight),
        UiResizeGrip(matilda::ui::UiResizeGripId::left),
        UiResizeGrip(matilda::ui::UiResizeGripId::right),
        UiResizeGrip(matilda::ui::UiResizeGripId::bottomLeft),
        UiResizeGrip(matilda::ui::UiResizeGripId::bottom),
        UiResizeGrip(matilda::ui::UiResizeGripId::bottomRight),
    };
    matilda::ui::UiResizeGripId activeResizeGrip_ = matilda::ui::UiResizeGripId::bottomRight;
    float uiScaleFactor_ = matilda::ui::kUiScaleDefault;
    bool gripResizeActive_ = false;
    juce::Point<int> gripResizeStartMouse_;
    int gripResizeStartWidth_ = 0;
    int gripResizeStartHeight_ = 0;
    int gripResizeRefWidth100_ = 0;
    int gripResizeRefHeight100_ = 0;
    bool lastTransportRunning_ = false;
};
