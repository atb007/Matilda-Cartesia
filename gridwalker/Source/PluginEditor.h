#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GridComponent.h"
#include "GridWalkerLookAndFeel.h"

class GridWalkerAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit GridWalkerAudioProcessorEditor(GridWalkerAudioProcessor&);
    ~GridWalkerAudioProcessorEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;
    void setupLayerTabs();
    void setupTransportButtons();
    juce::String rootLabelText() const;
    void syncTransportButtons();

    GridWalkerAudioProcessor& processor_;
    GridWalkerLookAndFeel laf_;

    juce::Label titleLabel_{"", "GridWalker"};
    juce::Label rootLabel_{"", "Root: —"};
    juce::Label monoBadge_{"", "MONO IN"};
    juce::Label statusLabel_{"", ""};
    juce::Label gridHint_{"", "Click = gate  ·  scroll / double-click = pitch  ·  right-click = next scale step"};
    juce::TextButton playButton_{"Play"};
    juce::TextButton stopButton_{"Stop"};

    juce::ComboBox scaleBox_;
    juce::ToggleButton quantizeToggle_{"Quantize"};
    juce::Slider minSlider_;
    juce::Slider rangeSlider_;
    juce::ComboBox masterDivBox_;
    juce::ComboBox xDivBox_;
    juce::ComboBox yDivBox_;
    juce::Label xLabel_{"", "X"};
    juce::Label yLabel_{"", "Y"};
    juce::Label zLabel_{"", "Z (off)"};
    juce::TextButton xFwd_{"Fwd"};
    juce::TextButton xRev_{"Rev"};
    juce::TextButton yFwd_{"Fwd"};
    juce::TextButton yRev_{"Rev"};
    juce::TextButton zOff_{"Off"};

    std::array<juce::TextButton, 4> layerTabs_{};

    GridComponent grid_;

    using Att = juce::AudioProcessorValueTreeState::SliderAttachment;
    using AttB = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using AttC = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    std::unique_ptr<AttC> scaleAtt_, masterAtt_, xDivAtt_, yDivAtt_;
    std::unique_ptr<AttB> quantizeAtt_;
    std::unique_ptr<Att> minAtt_, rangeAtt_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridWalkerAudioProcessorEditor)
};
