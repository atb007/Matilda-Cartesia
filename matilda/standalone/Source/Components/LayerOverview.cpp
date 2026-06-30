#include "LayerOverview.h"
#include "../KnobDrawing.h"
#include "../MatildaImages.h"
#include "../MiniGridLayout.h"

namespace {

using namespace matilda::minigrid;

void drawInactiveSocket(juce::Graphics& g, juce::Rectangle<float> slot, float scale, float opacity) {
    const auto img = matilda::images::miniGridInactive();
    if (!img.isValid() || opacity <= 0.f)
        return;

    auto dest = slot.withCentre(slot.getCentre() + juce::Point<float>(0.f, kInactiveYNudge * scale));
    g.setOpacity(opacity);
    g.drawImage(img, dest, juce::RectanglePlacement::centred);
    g.setOpacity(1.f);
}

void drawColoredGem(juce::Graphics& g,
                    juce::Rectangle<float> slot,
                    int layer,
                    bool on,
                    bool glow,
                    float scale,
                    float opacity) {
    const auto img = on ? matilda::images::miniGridOn(layer) : matilda::images::miniGridOff(layer);
    if (!img.isValid() || opacity <= 0.f)
        return;

    if (glow && on) {
        const auto led = matilda::knob::ledColour(matilda::knob::variantForLayer(layer));
        const auto c = slot.getCentre();
        const auto radius = slot.getWidth() * 0.5f + 7.f * scale;
        juce::ColourGradient softGlow(led.withAlpha(0.34f), c.x, c.y, led.withAlpha(0.f), c.x, c.y + radius, true);
        softGlow.addColour(0.45, led.withAlpha(0.15f));
        softGlow.addColour(0.80, led.withAlpha(0.04f));
        g.setGradientFill(softGlow);
        g.fillEllipse(slot.expanded(8.f * scale));
    }

    g.setOpacity(opacity);
    g.drawImage(img, slot, juce::RectanglePlacement::stretchToFit);
    g.setOpacity(1.f);
}

bool cellGateOn(const matilda::LayerState& layer, int miniIndex) {
    const int step = rowMajorFromMiniGridIndex(miniIndex);
    const int y = step / matilda::kGridSize;
    const int x = step % matilda::kGridSize;
    return layer.cells[static_cast<size_t>(y)][static_cast<size_t>(x)].gate;
}

} // namespace

LayerOverview::LayerOverview(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    setOpaque(false);
    refresh();
}

void LayerOverview::setPlayingLayer(int layer, int playheadStep) {
    playingLayer_ = layer;
    playheadStep_ = playheadStep;
    repaint();
}

void LayerOverview::refresh() {
    patch_.layers[0].active = true;
    rebuildHitBounds();
    repaint();
}

float LayerOverview::designScale() const {
    return juce::jmin(static_cast<float>(getWidth()) / kBaseW, static_cast<float>(getHeight()) / kBaseH);
}

juce::Point<float> LayerOverview::designOrigin() const {
    const float s = designScale();
    return {(static_cast<float>(getWidth()) - kBaseW * s) * 0.5f,
            (static_cast<float>(getHeight()) - kBaseH * s) * 0.5f};
}

juce::Rectangle<float> LayerOverview::cellSlot(float leftPct, float topPx, float scale, juce::Point<float> origin) const {
    const float slot = kCell * scale;
    return {origin.x + (leftPct / 100.f) * kBaseW * scale, origin.y + topPx * scale, slot, slot};
}

void LayerOverview::rebuildHitBounds() {
    const float s = designScale();
    const auto origin = designOrigin();
    const float pad = 4.f * s;
    const float slot = kCell * s;

    for (int layer = 0; layer < kLayerCount; ++layer) {
        const auto* cells = layerCells(layer);
        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        for (int i = 0; i < kCellsPerLayer; ++i) {
            const auto slotRect = cellSlot(cells[i].leftPct, cells[i].topPx, s, origin);
            minX = juce::jmin(minX, slotRect.getX());
            minY = juce::jmin(minY, slotRect.getY());
            maxX = juce::jmax(maxX, slotRect.getRight());
            maxY = juce::jmax(maxY, slotRect.getBottom());
        }
        layerHitBounds_[static_cast<size_t>(layer)] =
            juce::Rectangle<float>(minX - pad, minY - pad, maxX - minX + slot + pad * 2.f, maxY - minY + slot + pad * 2.f)
                .toNearestInt();

        const auto& toggle = kToggles[layer];
        toggleHitBounds_[static_cast<size_t>(layer)] =
            cellSlot(toggle.leftPct, toggle.topPx, s, origin).toNearestInt();
    }
}

void LayerOverview::paint(juce::Graphics& g) {
    const float s = designScale();
    const auto origin = designOrigin();
    const int miniPlayheadIndex =
        playheadStep_ >= 0 ? miniGridIndexFromRowMajorStep(playheadStep_) : -1;

    const auto frame = matilda::images::miniGridFrame();
    if (frame.isValid()) {
        const auto frameDest = juce::Rectangle<float>(origin.x, origin.y + kFrameTop * s, kBaseW * s, kFrameH * s);
        g.drawImage(frame, frameDest, juce::RectanglePlacement::stretchToFit);
    }

    for (int layer = 0; layer < kLayerCount; ++layer) {
        const auto& layerState = patch_.layers[static_cast<size_t>(layer)];
        const bool active = layerState.active;
        const bool isPlayingLayer = layer == playingLayer_ && miniPlayheadIndex >= 0;
        const auto* cells = layerCells(layer);

        for (int i = 0; i < kCellsPerLayer; ++i) {
            const auto slot = cellSlot(cells[i].leftPct, cells[i].topPx, s, origin);
            const bool isPlayhead = isPlayingLayer && active && i == miniPlayheadIndex;
            const bool gateOn = cellGateOn(layerState, i);
            const bool showPlayheadLit = isPlayhead && gateOn;
            const bool showInactiveGem = !active || !gateOn;

            drawInactiveSocket(g, slot, s, showInactiveGem ? 1.f : 0.f);
            drawColoredGem(g, slot, layer, showPlayheadLit, showPlayheadLit, s, showInactiveGem ? 0.f : 1.f);
        }
    }

    const bool isPlaying = playheadStep_ >= 0;

    for (int layer = 0; layer < kLayerCount; ++layer) {
        const auto& layerState = patch_.layers[static_cast<size_t>(layer)];
        const bool active = layerState.active;
        const bool selected = patch_.selectedLayer == layer;
        const auto& toggle = kToggles[layer];
        const auto slot = cellSlot(toggle.leftPct, toggle.topPx, s, origin);

        drawInactiveSocket(g, slot, s, active ? 0.f : 1.f);
        const bool toggleOn = active && (isPlaying ? layer == playingLayer_ : selected);
        drawColoredGem(g, slot, layer, toggleOn, toggleOn, s, active ? 1.f : 0.f);
    }
}

void LayerOverview::resized() {
    rebuildHitBounds();
}

void LayerOverview::mouseDown(const juce::MouseEvent& e) {
    const auto pos = e.getPosition();

    for (int layer = 0; layer < kLayerCount; ++layer) {
        if (toggleHitBounds_[static_cast<size_t>(layer)].contains(pos)) {
            if (layer == 0)
                return;
            auto& layerState = patch_.layers[static_cast<size_t>(layer)];
            layerState.active = !layerState.active;
            if (!layerState.active && patch_.selectedLayer == layer) {
                patch_.selectedLayer = 0;
                if (onLayerSelected)
                    onLayerSelected(0);
            }
            if (onLayerActivated)
                onLayerActivated(layer);
            refresh();
            return;
        }
    }

    for (int layer = 0; layer < kLayerCount; ++layer) {
        if (!patch_.layers[static_cast<size_t>(layer)].active)
            continue;
        if (!layerHitBounds_[static_cast<size_t>(layer)].contains(pos))
            continue;
        patch_.selectedLayer = layer;
        if (onLayerSelected)
            onLayerSelected(layer);
        refresh();
        return;
    }
}
