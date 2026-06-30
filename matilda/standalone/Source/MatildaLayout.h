#pragma once

#include <JuceHeader.h>
#include "CartesiaGridLayout.h"

// ─── Matilda editor layout ──────────────────────────────────────────────────
// Editor renders the Cartesia-dark.svg panel as its background.
// All component regions are expressed as normalised fractions of the panel
// then scaled to actual pixel bounds by the editor.
//
// Panel proportions: 162.56mm × 128.5mm (Cartesia)
// Native VCV resolution: 480×379 px (at 75dpi)
// JUCE editor: 960×758 px (2× VCV native, i.e. kUiScale = 2.0)
// ────────────────────────────────────────────────────────────────────────────

namespace matilda::layout {

inline constexpr float kDesignW = 960.f;
inline constexpr float kDesignH = 758.f;

// Normalised panel fractions derived from Cartesia coordinate analysis.
// See CartesiaGridLayout.h for the derivation.
//
// Mini LED grid (LED000..LED330 bounding box, all 4 layers):
//   x: norm(29.04) = 0.063  →  norm(29.04 + 3*10.86 + 3*45.547) = norm(198.26) = 0.431
//   y: norm(53.52) = 0.147  →  norm(53.52 + 3*6.48 + 3*17.392)  = norm(125.14) = 0.344
//
// SLICE buttons (z-layer tabs):
//   x: 0.102 → 0.408,  y: 0.110
//
// STAGE/KNOB grid (main 4×4 cell area):
//   x: norm(249.463) = 0.541  →  norm(441.202) = 0.956
//   y: norm(82.444)  = 0.226  →  norm(271.501) = 0.744

inline juce::Rectangle<int> mapR(float nx, float ny, float nw, float nh,
                                  float edW, float edH) {
    return {
        juce::roundToInt(nx * edW), juce::roundToInt(ny * edH),
        juce::roundToInt(nw * edW), juce::roundToInt(nh * edH)
    };
}

struct Regions {
    float edW = kDesignW;
    float edH = kDesignH;

    // Mini LED grid + SLICE tabs — LayerOverview component hosts both
    juce::Rectangle<int> layerOverview() const {
        // x: starts at SLICE1 (0.102), extends to just past LED330 (0.431) with margin
        // y: SLICE row top (0.085), down to LED bottom (0.355) with margin
        return mapR(0.06f, 0.05f, 0.40f, 0.32f, edW, edH);
    }

    // Main 4×4 gate/note grid — GemGrid
    // KNOB00 at norm(0.541,0.185) → KNOB33 at norm(0.920,0.703)
    // Add headroom for note labels above and LED glow below
    juce::Rectangle<int> gemGrid() const {
        return mapR(0.50f, 0.10f, 0.48f, 0.64f, edW, edH);
    }

    // Quantise scale / root / octave panel (left strip, lower half)
    juce::Rectangle<int> quantisePanel() const {
        return mapR(0.02f, 0.36f, 0.48f, 0.38f, edW, edH);
    }

    // Movement mode selector (pill + dropdown, above quantise)
    juce::Rectangle<int> movement() const {
        return mapR(0.02f, 0.76f, 0.46f, 0.10f, edW, edH);
    }

    // Layer select crystal bar (bottom-right, below main grid)
    juce::Rectangle<int> crystalBar() const {
        return mapR(0.52f, 0.80f, 0.46f, 0.12f, edW, edH);
    }

    // Play / clock / mode transport (bottom-left)
    juce::Rectangle<int> transport() const {
        return mapR(0.02f, 0.87f, 0.18f, 0.11f, edW, edH);
    }
};

} // namespace matilda::layout
