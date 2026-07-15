#include "StepScroll.h"
#include "../MatildaImages.h"
#include "../KnobDrawing.h"

#include <cmath>

namespace {

void drawPitStop(juce::Graphics& g, juce::Rectangle<float> bounds, int layer) {
    const auto variant = matilda::knob::variantForLayer(layer);
    juce::Colour fill, stroke;
    switch (variant) {
        case matilda::knob::Variant::Red:
            fill = juce::Colour(0xff7b080f);
            stroke = juce::Colour(0xffffaab2);
            break;
        case matilda::knob::Variant::Green:
            fill = juce::Colour(0xff4eaa28);
            stroke = juce::Colour(0xffd1ffda);
            break;
        case matilda::knob::Variant::Blue:
            fill = juce::Colour(0xff1a8881);
            stroke = juce::Colour(0xffd1fffc);
            break;
        default:
            fill = juce::Colour(0xffb32b07);
            stroke = juce::Colour(0xffffd2b7);
            break;
    }

    const auto c = bounds.getCentre();
    const float rOuter = bounds.getWidth() * 0.475f;
    const float rInner = bounds.getWidth() * 0.25f;
    g.setColour(fill);
    g.fillEllipse(c.x - rOuter, c.y - rOuter, rOuter * 2.f, rOuter * 2.f);
    g.setColour(stroke);
    g.drawEllipse(c.x - rOuter, c.y - rOuter, rOuter * 2.f, rOuter * 2.f, 1.f);
    g.setColour(juce::Colour(0xfffddeff));
    g.fillEllipse(c.x - rInner, c.y - rInner, rInner * 2.f, rInner * 2.f);
}

} // namespace

StepScroll::StepScroll(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    juce::ignoreUnused(laf_);
    setOpaque(false);
    setPaintingIsUnclipped(true);
    setMouseCursor(juce::MouseCursor::PointingHandCursor);

    vineImg_ = matilda::images::stepScrollVine();
    for (int i = 0; i < matilda::kLayerCount; ++i)
        crystalImg_[static_cast<size_t>(i)] = matilda::images::stepScrollCrystal(i);

    syncFromPatch();
}

void StepScroll::setLayer(int layer) {
    layer_ = juce::jlimit(0, matilda::kLayerCount - 1, layer);
    syncFromPatch();
}

void StepScroll::syncFromPatch() {
    const int count = matilda::clampActiveStepCount(
        patch_.layers[static_cast<size_t>(layer_)].activeStepCount);
    patch_.layers[static_cast<size_t>(layer_)].activeStepCount = count;
    if (!dragging_)
        visualCount_ = static_cast<float>(count);
    repaint();
}

float StepScroll::designScale() const {
    return juce::jmin(static_cast<float>(getWidth()) / kDesignW,
                      static_cast<float>(getHeight()) / kDesignH);
}

juce::Rectangle<float> StepScroll::designBounds() const {
    const float s = designScale();
    const float w = kDesignW * s;
    const float h = kDesignH * s;
    return {(static_cast<float>(getWidth()) - w) * 0.5f,
            (static_cast<float>(getHeight()) - h) * 0.5f,
            w, h};
}

float StepScroll::countToCentreX(float count, juce::Rectangle<float> bounds) const {
    const float t = juce::jlimit(1.f, 16.f, count) / 16.f;
    const float crystalW = kCrystalDesignW * designScale();
    const float left = bounds.getX() + crystalW * 0.35f;
    const float right = bounds.getRight() - crystalW * 0.35f;
    return juce::jmap(t, 1.f / 16.f, 1.f, left, right);
}

float StepScroll::xToCount(float x, juce::Rectangle<float> bounds) const {
    const float crystalW = kCrystalDesignW * designScale();
    const float left = bounds.getX() + crystalW * 0.35f;
    const float right = bounds.getRight() - crystalW * 0.35f;
    if (right <= left)
        return 16.f;
    const float t = juce::jlimit(0.f, 1.f, (x - left) / (right - left));
    return juce::jmap(t, 0.f, 1.f, 1.f, 16.f);
}

float StepScroll::applyMajorSnap(float count) const {
    static constexpr float majors[] = {4.f, 8.f, 12.f, 16.f};
    for (float m : majors) {
        if (std::abs(count - m) <= kMajorSnapRadius)
            return m;
    }
    return count;
}

void StepScroll::setCountFromInteraction(float countF, bool notify) {
    countF = applyMajorSnap(juce::jlimit(1.f, 16.f, countF));
    visualCount_ = countF;
    const int count = matilda::clampActiveStepCount(juce::roundToInt(countF));
    auto& layer = patch_.layers[static_cast<size_t>(layer_)];
    if (layer.activeStepCount != count) {
        layer.activeStepCount = count;
        if (notify && onChanged)
            onChanged();
    }
    repaint();
}

void StepScroll::commitIntegerCount() {
    const int count = matilda::clampActiveStepCount(juce::roundToInt(visualCount_));
    visualCount_ = static_cast<float>(count);
    auto& layer = patch_.layers[static_cast<size_t>(layer_)];
    if (layer.activeStepCount != count) {
        layer.activeStepCount = count;
        if (onChanged)
            onChanged();
    }
    repaint();
}

void StepScroll::paint(juce::Graphics& g) {
    const auto bounds = designBounds();
    const float s = designScale();
    if (bounds.isEmpty())
        return;

    // Vine lives in the Figma 43px track band, vertically centered in the taller hit box.
    const float trackH = kTrackDesignH * s;
    const auto track = juce::Rectangle<float>(bounds.getX(),
                                              bounds.getCentreY() - trackH * 0.5f,
                                              bounds.getWidth(),
                                              trackH);

    if (vineImg_.isValid()) {
        g.setOpacity(1.f);
        g.drawImage(vineImg_, track, juce::RectanglePlacement::stretchToFit);
    }

    // Major pit stops at ~0 / 4 / 8 / 12 (Figma offsets from component centre).
    static constexpr float kPitOffsets[] = {-279.5f, -134.5f, 2.5f, 139.5f};
    const float pit = kPitStopDesign * s;
    const float cy = track.getCentreY() + 2.5f * s;
    const float handleX = countToCentreX(visualCount_, track);
    for (float ox : kPitOffsets) {
        const float cx = track.getCentreX() + ox * s;
        // Hide markers under / past the crystal (matches Figma row4-zero).
        if (cx >= handleX - pit * 0.25f)
            continue;
        drawPitStop(g, {cx - pit * 0.5f, cy - pit * 0.5f, pit, pit}, layer_);
    }

    const auto& crystal = crystalImg_[static_cast<size_t>(layer_)];
    if (crystal.isValid()) {
        const float cw = kCrystalDesignW * s;
        const float ch = cw * (1246.f / 1188.f);
        // Keep crystal fully inside the component (was clipped by shell when overhanging).
        const float top = juce::jlimit(bounds.getY(),
                                       bounds.getBottom() - ch,
                                       track.getY() - 7.f * s);
        g.setOpacity(1.f);
        g.drawImage(crystal, {handleX - cw * 0.5f, top, cw, ch},
                    juce::RectanglePlacement::centred);
    }
}

void StepScroll::resized() { repaint(); }

void StepScroll::mouseDown(const juce::MouseEvent& e) {
    dragging_ = true;
    setCountFromInteraction(xToCount(static_cast<float>(e.x), designBounds()), true);
}

void StepScroll::mouseDrag(const juce::MouseEvent& e) {
    if (!dragging_)
        return;
    setCountFromInteraction(xToCount(static_cast<float>(e.x), designBounds()), true);
}

void StepScroll::mouseUp(const juce::MouseEvent&) {
    if (!dragging_)
        return;
    dragging_ = false;
    commitIntegerCount();
}

void StepScroll::mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) {
    const float delta = wheel.deltaY != 0.f ? wheel.deltaY : wheel.deltaX;
    if (std::abs(delta) < 1.0e-4f)
        return;

    const int dir = delta > 0.f ? 1 : -1;
    const int current = matilda::clampActiveStepCount(
        patch_.layers[static_cast<size_t>(layer_)].activeStepCount);
    const int next = matilda::clampActiveStepCount(current + dir);
    visualCount_ = static_cast<float>(next);
    auto& layer = patch_.layers[static_cast<size_t>(layer_)];
    if (layer.activeStepCount != next) {
        layer.activeStepCount = next;
        if (onChanged)
            onChanged();
    }
    repaint();
}
