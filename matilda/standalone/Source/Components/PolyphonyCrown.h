#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerState.h"
#include "../MatildaLookAndFeel.h"

/**
 * Figma glowPolyphony (AdMaker 5171:102837) — glow-only stack (union omitted; too opaque):
 *   bgGlow   → soft outer diamond bloom
 *   frontGlow → brighter inner glow
 */
class PolyphonyCrown : public juce::Component, private juce::Timer {
public:
    PolyphonyCrown(matilda::PatchState& patch, MatildaLookAndFeel& laf);

    std::function<void()> onChanged;

    void syncFromPatch();

private:
    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;
    bool hovered_ = false;
    float morphPhase_ = 0.f;
    /** 0 = invisible … 1 = full. Smoothly tracks layer/poly state. */
    float displayIntensity_ = 0.f;

    juce::Path bgGlowPath_;
    juce::Path frontGlowPath_;

    /** Full Figma artboard for bgGlow. */
    static constexpr float kDesignW = 108.321f;
    static constexpr float kDesignH = 159.276f;
    static constexpr float kBgBlurStd = 6.5f;
    static constexpr float kFrontBlurStd = 6.f;

    [[nodiscard]] int activeLayerCount() const;
    [[nodiscard]] bool wantsVisible() const;
    [[nodiscard]] bool polyOn() const { return patch_.polyphony; }
    [[nodiscard]] juce::Colour morphColour(float phase) const;
    [[nodiscard]] float localScale() const;
    [[nodiscard]] juce::AffineTransform designToLocal() const;
    void buildPaths();

    /** Soft bloom via stroked rings — avoids rectangular blur-image edges. */
    void paintSoftGlow(juce::Graphics& g,
                       const juce::Path& path,
                       juce::Colour colour,
                       float intensity,
                       float blurStdDesign,
                       bool hotCore);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseEnter(const juce::MouseEvent&) override;
    void mouseExit(const juce::MouseEvent&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void timerCallback() override;
};
