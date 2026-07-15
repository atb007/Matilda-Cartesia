#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerState.h"
#include "../MatildaLookAndFeel.h"

/** Figma stepScroll vine — per-layer pattern-length control (1…16). */
class StepScroll : public juce::Component {
public:
    StepScroll(matilda::PatchState& patch, MatildaLookAndFeel& laf);

    std::function<void()> onChanged;

    void syncFromPatch();
    void setLayer(int layer);

private:
    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;
    int layer_ = 0;
    float visualCount_ = 16.f;
    bool dragging_ = false;

    juce::Image vineImg_;
    juce::Image crystalImg_[matilda::kLayerCount];

    static constexpr float kDesignW = 585.f;
    /** Includes crystal overhang above/below the 43px vine track. */
    static constexpr float kDesignH = 60.f;
    static constexpr float kTrackDesignH = 43.f;
    static constexpr float kCrystalDesignW = 57.f;
    static constexpr float kPitStopDesign = 20.f;
    static constexpr float kMajorSnapRadius = 0.55f;

    [[nodiscard]] float designScale() const;
    [[nodiscard]] juce::Rectangle<float> designBounds() const;
    [[nodiscard]] float countToCentreX(float count, juce::Rectangle<float> bounds) const;
    [[nodiscard]] float xToCount(float x, juce::Rectangle<float> bounds) const;
    [[nodiscard]] float applyMajorSnap(float count) const;
    void setCountFromInteraction(float countF, bool notify);
    void commitIntegerCount();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
};
