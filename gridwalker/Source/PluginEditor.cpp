#include "PluginEditor.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

GridWalkerAudioProcessorEditor::GridWalkerAudioProcessorEditor(GridWalkerAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor_(p),
      grid_(p.engine(), laf_) {
    setLookAndFeel(&laf_);
    setSize(900, 620);

    for (auto* c : {&titleLabel_, &rootLabel_, &statusLabel_, &gridHint_, &xLabel_, &yLabel_, &zLabel_}) {
        c->setColour(juce::Label::textColourId, laf_.textPrimary);
        addAndMakeVisible(c);
    }
    gridHint_.setColour(juce::Label::textColourId, laf_.textMuted);
    gridHint_.setFont(juce::FontOptions(11.f));
    titleLabel_.setFont(juce::FontOptions(22.f).withStyle("Bold"));
    setupTransportButtons();
    monoBadge_.setColour(juce::Label::textColourId, laf_.accentColour);
    monoBadge_.setVisible(false);
    addAndMakeVisible(monoBadge_);

    scaleBox_.addItemList({"Major", "Minor", "Dorian", "Pentatonic", "Chromatic"}, 1);
    masterDivBox_.addItemList({"1/4", "1/8", "1/16", "1/32"}, 1);
    xDivBox_.addItemList({"/1", "/2", "/4", "/8", "/16"}, 1);
    yDivBox_.addItemList({"/1", "/2", "/4", "/8", "/16"}, 1);

    for (auto* box : {&scaleBox_, &masterDivBox_, &xDivBox_, &yDivBox_})
        addAndMakeVisible(box);

    addAndMakeVisible(quantizeToggle_);
    for (auto* s : {&minSlider_, &rangeSlider_}) {
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 18);
        addAndMakeVisible(s);
    }
    minSlider_.setRange(0, 11, 1);
    rangeSlider_.setRange(1, 24, 1);

    for (auto* b : {&xFwd_, &yFwd_}) {
        b->setClickingTogglesState(true);
        b->setToggleState(true, juce::dontSendNotification);
        b->setEnabled(true);
        addAndMakeVisible(b);
    }
    for (auto* b : {&xRev_, &yRev_, &zOff_}) {
        b->setEnabled(false);
        addAndMakeVisible(b);
    }
    zOff_.setToggleState(true, juce::dontSendNotification);

    setupLayerTabs();
    addAndMakeVisible(grid_);

    scaleAtt_ = std::make_unique<AttC>(processor_.apvts, "scale", scaleBox_);
    quantizeAtt_ = std::make_unique<AttB>(processor_.apvts, "quantize", quantizeToggle_);
    minAtt_ = std::make_unique<Att>(processor_.apvts, "minDegree", minSlider_);
    rangeAtt_ = std::make_unique<Att>(processor_.apvts, "range", rangeSlider_);
    masterAtt_ = std::make_unique<AttC>(processor_.apvts, "masterDiv", masterDivBox_);
    xDivAtt_ = std::make_unique<AttC>(processor_.apvts, "xDiv", xDivBox_);
    yDivAtt_ = std::make_unique<AttC>(processor_.apvts, "yDiv", yDivBox_);

    startTimerHz(15);
}

GridWalkerAudioProcessorEditor::~GridWalkerAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

void GridWalkerAudioProcessorEditor::setupTransportButtons() {
    for (auto* b : {&playButton_, &stopButton_}) {
        b->setClickingTogglesState(false);
        addAndMakeVisible(b);
    }

    playButton_.setColour(juce::TextButton::buttonOnColourId, laf_.gateOnColour);
    playButton_.onClick = [this] {
        processor_.setSequencerRunning(true);
        if (auto* holder = juce::StandalonePluginHolder::getInstance())
            holder->startPlaying();
        syncTransportButtons();
    };
    stopButton_.onClick = [this] {
        processor_.setSequencerRunning(false);
        syncTransportButtons();
    };

    syncTransportButtons();
}

void GridWalkerAudioProcessorEditor::syncTransportButtons() {
    const bool running = processor_.isSequencerRunning();
    playButton_.setToggleState(running, juce::dontSendNotification);
    playButton_.setButtonText(running ? "Playing" : "Play");
}

void GridWalkerAudioProcessorEditor::setupLayerTabs() {
    for (int i = 0; i < 4; ++i) {
        layerTabs_[static_cast<size_t>(i)].setButtonText("Z" + juce::String(i + 1));
        layerTabs_[static_cast<size_t>(i)].setClickingTogglesState(true);
        layerTabs_[static_cast<size_t>(i)].onClick = [this, i] {
            if (i > 0)
                return;
            processor_.apvts.getParameterAsValue("activeLayer").setValue(i);
            for (int j = 0; j < 4; ++j)
                layerTabs_[static_cast<size_t>(j)].setToggleState(j == i, juce::dontSendNotification);
            grid_.refresh();
        };
        layerTabs_[static_cast<size_t>(i)].setEnabled(i == 0);
        layerTabs_[static_cast<size_t>(i)].setToggleState(i == 0, juce::dontSendNotification);
        addAndMakeVisible(layerTabs_[static_cast<size_t>(i)]);
    }
}

juce::String GridWalkerAudioProcessorEditor::rootLabelText() const {
    static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    if (!processor_.engine().rootActive() && !processor_.usingDefaultRoot())
        return "Root: waiting for MIDI";
    const int n = processor_.engine().rootMidi();
    const juce::String note = juce::String(names[n % 12]) + juce::String(n / 12 - 1);
    if (processor_.usingDefaultRoot())
        return "Root: " + note + " (sandbox)";
    return "Root: " + note;
}

void GridWalkerAudioProcessorEditor::timerCallback() {
    processor_.syncEngineParams();
    syncTransportButtons();
    rootLabel_.setText(rootLabelText(), juce::dontSendNotification);
    statusLabel_.setText(
        "x=" + juce::String(processor_.engine().playheadX())
            + " y=" + juce::String(processor_.engine().playheadY())
            + " tick=" + juce::String(processor_.engine().masterTick()),
        juce::dontSendNotification);
}

void GridWalkerAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(laf_.backgroundDark);
    g.setColour(laf_.panelColour);
    g.fillRect(getLocalBounds().removeFromTop(52));
    g.fillRect(getLocalBounds().removeFromBottom(88));
}

void GridWalkerAudioProcessorEditor::resized() {
    auto area = getLocalBounds();
    auto header = area.removeFromTop(52).reduced(16, 10);
    titleLabel_.setBounds(header.removeFromLeft(140));
    playButton_.setBounds(header.removeFromLeft(72).reduced(0, 2));
    header.removeFromLeft(6);
    stopButton_.setBounds(header.removeFromLeft(56).reduced(0, 2));
    header.removeFromLeft(12);
    monoBadge_.setBounds(header.removeFromLeft(70));
    rootLabel_.setBounds(header);

    auto pitchRow = area.removeFromTop(56).reduced(16, 8);
    scaleBox_.setBounds(pitchRow.removeFromLeft(140).reduced(0, 4));
    pitchRow.removeFromLeft(8);
    quantizeToggle_.setBounds(pitchRow.removeFromLeft(90));
    pitchRow.removeFromLeft(16);
    minSlider_.setBounds(pitchRow.removeFromLeft(72));
    rangeSlider_.setBounds(pitchRow.removeFromLeft(72));

    auto layerRow = area.removeFromTop(36).reduced(16, 4);
    for (auto& tab : layerTabs_) {
        tab.setBounds(layerRow.removeFromLeft(48).reduced(2));
        layerRow.removeFromLeft(4);
    }
    gridHint_.setBounds(area.removeFromTop(18).reduced(16, 0));

    auto footer = area.removeFromBottom(88).reduced(16, 10);
    statusLabel_.setBounds(footer.removeFromRight(180));
    masterDivBox_.setBounds(footer.removeFromLeft(100).reduced(0, 4));
    footer.removeFromLeft(16);

    auto axisRow = footer;
    xLabel_.setBounds(axisRow.removeFromLeft(16));
    xDivBox_.setBounds(axisRow.removeFromLeft(72).reduced(0, 4));
    xFwd_.setBounds(axisRow.removeFromLeft(40).reduced(2));
    xRev_.setBounds(axisRow.removeFromLeft(40).reduced(2));
    axisRow.removeFromLeft(12);
    yLabel_.setBounds(axisRow.removeFromLeft(16));
    yDivBox_.setBounds(axisRow.removeFromLeft(72).reduced(0, 4));
    yFwd_.setBounds(axisRow.removeFromLeft(40).reduced(2));
    yRev_.setBounds(axisRow.removeFromLeft(40).reduced(2));
    axisRow.removeFromLeft(12);
    zLabel_.setBounds(axisRow.removeFromLeft(48));
    zOff_.setBounds(axisRow.removeFromLeft(48).reduced(2));

    grid_.setBounds(area.reduced(16, 8));
}
