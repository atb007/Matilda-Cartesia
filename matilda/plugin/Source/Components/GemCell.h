#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerEngine.h"
#include "../MatildaLookAndFeel.h"

class GemCell : public juce::Component {
public:
    GemCell(MatildaLookAndFeel& laf, matilda::SequencerEngine& engine);

    void bind(matilda::CellState* cell, int layer, int x, int y);

    void setPlayhead(bool isPlayhead);
    void setOnActiveLayer(bool onActiveLayer);
    void setLayerColour(juce::Colour c);
    void setMiniMode(bool mini);
    void setStepFired(bool fired);

    std::function<void(int x, int y)> onChanged;

private:
    MatildaLookAndFeel& laf_;
    matilda::SequencerEngine& engine_;
    matilda::CellState* cell_ = nullptr;
    int layer_ = 0;
    int cellX_ = 0;
    int cellY_ = 0;
    bool playhead_ = false;
    bool stepFired_ = true;
    bool onActiveLayer_ = false;
    bool mini_ = false;
    juce::Colour layerColour_{ 0xff00e060 };
    bool hovered_ = false;
    enum class DragTarget { None, Trigger, Jitter, Degree } dragTarget_ = DragTarget::None;
    int dragStartY_ = 0;
    int dragStartQuantisedIndex_ = 0;
    bool dragMoved_ = false;

    juce::Rectangle<float> knobBounds() const;
    juce::Rectangle<float> ledBounds() const;
    juce::Rectangle<float> triggerIconBounds() const;
    juce::Rectangle<float> jitterIconBounds() const;
    bool hitTrigger(juce::Point<float> p) const;
    bool hitJitter(juce::Point<float> p) const;
    bool hitGemCentre(juce::Point<float> p) const;
    void updateMouseCursor(juce::Point<float> p);

    void paint(juce::Graphics& g) override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseMove(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent&) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void notifyChanged();
};
