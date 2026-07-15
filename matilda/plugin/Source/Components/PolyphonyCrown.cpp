#include "PolyphonyCrown.h"
#include "../KnobDrawing.h"
#include <cmath>

namespace {

/** bgGlow — viewBox 109×160, fill #73FFFF, blur 6.5px */
juce::Path makeBgGlowPath() {
    juce::Path p;
    p.startNewSubPath(13.3291f, 73.8831f);
    p.lineTo(52.7798f, 13.9031f);
    p.cubicTo(53.5731f, 12.6969f, 55.344f, 12.7029f, 56.1291f, 13.9145f);
    p.lineTo(94.9992f, 73.8937f);
    p.cubicTo(95.403f, 74.5167f, 95.4284f, 75.3122f, 95.0651f, 75.9598f);
    p.lineTo(56.1946f, 145.257f);
    p.cubicTo(55.4333f, 146.614f, 53.482f, 146.62f, 52.7122f, 145.268f);
    p.lineTo(13.262f, 75.9716f);
    p.cubicTo(12.89f, 75.3182f, 12.9159f, 74.5113f, 13.3291f, 73.8831f);
    p.closeSubPath();
    return p;
}

/** frontGlow — local 61×82 art at Figma offset (24, 39). */
juce::Path makeFrontGlowPath() {
    juce::Path p;
    p.startNewSubPath(12.3297f, 37.621f);
    p.lineTo(28.6093f, 12.8992f);
    p.cubicTo(29.4028f, 11.6942f, 31.1722f, 11.7002f, 31.9575f, 12.9105f);
    p.lineTo(47.9971f, 37.6316f);
    p.cubicTo(48.4017f, 38.255f, 48.4271f, 39.0514f, 48.0632f, 39.6995f);
    p.lineTo(32.023f, 68.2615f);
    p.cubicTo(31.2615f, 69.6175f, 29.3117f, 69.6237f, 28.5416f, 68.2726f);
    p.lineTo(12.2625f, 39.7113f);
    p.cubicTo(11.8898f, 39.0574f, 11.9158f, 38.2496f, 12.3297f, 37.621f);
    p.closeSubPath();
    p.applyTransform(juce::AffineTransform::translation(24.f, 39.f));
    return p;
}

} // namespace

PolyphonyCrown::PolyphonyCrown(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    juce::ignoreUnused(laf_);
    setOpaque(false);
    setPaintingIsUnclipped(true);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
    buildPaths();
    startTimerHz(30);
    syncFromPatch();
}

void PolyphonyCrown::buildPaths() {
    bgGlowPath_ = makeBgGlowPath();
    frontGlowPath_ = makeFrontGlowPath();
}

void PolyphonyCrown::syncFromPatch() {
    repaint();
}

int PolyphonyCrown::activeLayerCount() const {
    int n = 0;
    for (const auto& layer : patch_.layers)
        if (layer.active)
            ++n;
    return n;
}

bool PolyphonyCrown::wantsVisible() const {
    return activeLayerCount() > 1;
}

juce::Colour PolyphonyCrown::morphColour(float phase) const {
    static const juce::Colour stops[] = {
        matilda::knob::ledColour(matilda::knob::Variant::Orange),
        matilda::knob::ledColour(matilda::knob::Variant::Red),
        matilda::knob::ledColour(matilda::knob::Variant::Green),
        matilda::knob::ledColour(matilda::knob::Variant::Blue),
    };
    constexpr int n = 4;
    const float t = std::fmod(juce::jmax(0.f, phase), 1.f) * static_cast<float>(n);
    const int i0 = juce::jlimit(0, n - 1, static_cast<int>(std::floor(t)));
    const int i1 = (i0 + 1) % n;
    const float u = t - static_cast<float>(i0);
    const float s = u * u * (3.f - 2.f * u);
    return stops[i0].interpolatedWith(stops[i1], s);
}

float PolyphonyCrown::localScale() const {
    return juce::jmin(static_cast<float>(getWidth()) / kDesignW,
                      static_cast<float>(getHeight()) / kDesignH);
}

juce::AffineTransform PolyphonyCrown::designToLocal() const {
    const float s = localScale();
    const float drawnW = kDesignW * s;
    const float drawnH = kDesignH * s;
    const float ox = (static_cast<float>(getWidth()) - drawnW) * 0.5f;
    const float oy = static_cast<float>(getHeight()) - drawnH;
    return juce::AffineTransform::scale(s).translated(ox, oy);
}

void PolyphonyCrown::paintSoftGlow(juce::Graphics& g,
                                   const juce::Path& path,
                                   juce::Colour colour,
                                   float intensity,
                                   float blurStdDesign,
                                   bool hotCore) {
    if (intensity <= 0.01f)
        return;

    juce::Graphics::ScopedSaveState ss(g);
    g.addTransform(designToLocal());

    // Wide → narrow stroke rings approximate Gaussian bloom without a rectangular image edge.
    constexpr int kRings = 7;
    for (int i = kRings; i >= 1; --i) {
        const float u = static_cast<float>(i) / static_cast<float>(kRings);
        const float width = blurStdDesign * (0.55f + 1.65f * u);
        const float alpha = intensity * (0.045f + 0.09f * (1.f - u));
        g.setColour(colour.withAlpha(juce::jlimit(0.f, 1.f, alpha)));
        g.strokePath(path,
                     juce::PathStrokeType(width,
                                         juce::PathStrokeType::curved,
                                         juce::PathStrokeType::rounded));
    }

    g.setColour(colour.withAlpha(juce::jlimit(0.f, 1.f, intensity * (hotCore ? 0.55f : 0.28f))));
    g.fillPath(path);

    if (hotCore) {
        g.setColour(colour.brighter(0.55f).withAlpha(juce::jlimit(0.f, 1.f, intensity * 0.4f)));
        g.fillPath(path);
    }
}

void PolyphonyCrown::paint(juce::Graphics& g) {
    if (getLocalBounds().isEmpty() || displayIntensity_ <= 0.01f)
        return;

    const float i = displayIntensity_;
    const bool on = polyOn() && activeLayerCount() > 1;

    if (on) {
        // Glow-only stack (union removed): outer bloom + inner hot glow, morphing hues.
        const auto c = morphColour(morphPhase_);
        const auto bg = c.interpolatedWith(juce::Colour(0xff73ffff), 0.4f);
        const auto front = c.interpolatedWith(juce::Colour(0xff59fff7), 0.45f);
        paintSoftGlow(g, bgGlowPath_, bg, i * 0.9f, kBgBlurStd, false);
        paintSoftGlow(g, frontGlowPath_, front, i, kFrontBlurStd, true);
    } else {
        // Idle discoverability: soft white bloom that clearly pulses.
        const float pulse =
            0.55f + 0.45f * (0.5f + 0.5f * std::sin(morphPhase_ * juce::MathConstants<float>::twoPi));
        const float glow = i * pulse;
        paintSoftGlow(g, bgGlowPath_, juce::Colours::white, glow * 0.55f, kBgBlurStd * 1.1f, false);
        paintSoftGlow(g, frontGlowPath_, juce::Colours::white, glow, kFrontBlurStd, true);
    }
}

void PolyphonyCrown::resized() { repaint(); }

void PolyphonyCrown::mouseEnter(const juce::MouseEvent&) {
    hovered_ = true;
    repaint();
}

void PolyphonyCrown::mouseExit(const juce::MouseEvent&) {
    hovered_ = false;
    repaint();
}

void PolyphonyCrown::mouseDown(const juce::MouseEvent&) {
    if (activeLayerCount() < 2 && !polyOn())
        return;
    patch_.polyphony = !patch_.polyphony;
    if (onChanged)
        onChanged();
    repaint();
}

void PolyphonyCrown::timerCallback() {
    const float target = wantsVisible() ? (hovered_ && !polyOn() ? 0.85f : 1.f) : 0.f;
    const float speed = target > displayIntensity_ ? 0.08f : 0.05f;
    if (std::abs(target - displayIntensity_) > 0.002f) {
        displayIntensity_ += (target - displayIntensity_) * speed;
        if (std::abs(target - displayIntensity_) < 0.01f)
            displayIntensity_ = target;
        repaint();
    } else {
        displayIntensity_ = target;
    }

    if (displayIntensity_ > 0.02f) {
        morphPhase_ += polyOn() ? 0.013f : 0.006f;
        if (morphPhase_ >= 1.f)
            morphPhase_ -= 1.f;
        repaint();
    }
}
