#include "LayerPathView.h"
#include "../CartesiaGridLayout.h"
#include "../CartesiaCellLighting.h"
#include "../MatildaLookAndFeel.h"

LayerPathView::LayerPathView(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    setOpaque(false);
}

void LayerPathView::setPlayingLayer(int layer, int playX, int playY) {
    playingLayer_ = layer;  playX_ = playX;  playY_ = playY;
    repaint();
}

void LayerPathView::refresh() { repaint(); }

void LayerPathView::paint(juce::Graphics& g) {
    // Draw directly into the full editor (not a local bounds).
    // The component's setBounds covers the full panel area so
    // the normalised LED positions map directly to our local coords.
    const auto area = getLocalBounds().toFloat();

    for (int z = 0; z < matilda::kLayerCount; ++z) {
        const auto& layer   = patch_.layers[static_cast<size_t>(z)];
        const juce::Colour col = MatildaLookAndFeel::layerColour(z);

        for (int y = 0; y < matilda::kGridSize; ++y) {
            for (int x = 0; x < matilda::kGridSize; ++x) {
                const auto& cell   = layer.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
                const bool isHead  = (z == playingLayer_ && x == playX_ && y == playY_);

                cartesia::lighting::MiniCellState state;
                state.layerActive    = layer.active;
                state.onPlayingLayer = (z == playingLayer_);
                state.isPlayhead     = isHead;
                state.gate           = cell.gate;

                const float brightness = cartesia::lighting::miniLedBrightness(state);
                if (brightness <= 0.f)
                    continue;

                // Map normalised LED position to local component pixels
                const auto n      = cartesia::grid::ledNorm(x, y, z);
                const auto centre = juce::Point<float>{ n.x * area.getWidth(),
                                                        n.y * area.getHeight() };
                const float dotSz = isHead ? 13.f : 8.f;
                const auto  db    = juce::Rectangle<float>(dotSz, dotSz).withCentre(centre);

                cartesia::lighting::drawLedDot(g, db, brightness, col);
            }
        }
    }
}
