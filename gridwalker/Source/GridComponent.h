#pragma once

#include <JuceHeader.h>
#include "GridEngine.h"
#include "GridWalkerLookAndFeel.h"

class GridComponent : public juce::Component, private juce::Timer {
public:
    GridComponent(gridwalker::GridEngine& engine, GridWalkerLookAndFeel& laf);

    std::function<void(int x, int y)> onCellToggled;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void refresh();

private:
    void timerCallback() override;
    void nudgeAt(juce::Point<int> pos, int delta);
    void paintCellPitch(juce::Graphics& g, juce::Rectangle<float> bounds, int layer, int x, int y) const;
    juce::Rectangle<int> cellBounds(int x, int y) const;
    std::optional<std::pair<int, int>> cellAt(juce::Point<int> p) const;
    juce::String noteLabel(int midiNote) const;

    gridwalker::GridEngine& engine_;
    GridWalkerLookAndFeel& laf_;
    int lastX_ = -1;
    int lastY_ = -1;
};
