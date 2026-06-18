#pragma once

#include <JuceHeader.h>

// Brightness rules ported from CVfunk Cartesia.cpp (LED map + STAGE lights).
// Draw helpers are fully procedural – no image assets used for cells.
namespace cartesia::lighting {

// ─── Brightness computation ─────────────────────────────────────────────────

struct MiniCellState {
    bool layerActive    = true;
    bool onPlayingLayer = false;
    bool isPlayhead     = false;
    bool gate           = false;
};

struct MainCellState {
    bool onActiveLayer = false;
    bool isPlayhead    = false;
    bool gate          = false;
};

// Cartesia LED000..LED333 brightness rules from Cartesia.cpp step():
//   active z-layer, playhead cell  → 1.0
//   active z-layer, other gated    → 0.12
//   inactive layer, gated          → 0.04
//   everything else                → 0.0
inline float miniLedBrightness(const MiniCellState& s) {
    if (s.onPlayingLayer) {
        if (s.isPlayhead)            return 1.f;
        return s.gate ? 0.12f : 0.f;
    }
    if (!s.layerActive) return s.gate ? 0.04f : 0.f;
    return s.gate ? 0.06f : 0.f;
}

// STAGE light brightness (main grid indicator ring pulse).
inline float stageBrightness(const MainCellState& s) {
    return (s.onActiveLayer && s.isPlayhead) ? 1.f : 0.f;
}

// Knob LED brightness: gate on for the selected layer → 1.0, off layer dim.
inline float knobBrightness(const MainCellState& s) {
    if (s.onActiveLayer) return s.gate ? 1.f : 0.f;
    return s.gate ? 0.3f : 0.f;
}

// ─── Procedural draw helpers ─────────────────────────────────────────────────

// Glow helper: concentric rings diminishing outward (additive-ish)
inline void drawGlow(juce::Graphics& g,
                     juce::Point<float> centre,
                     float coreRadius,
                     float peakAlpha,
                     juce::Colour colour) {
    for (int ring = 4; ring >= 1; --ring) {
        const float r = coreRadius * (1.f + static_cast<float>(ring) * 0.55f);
        const float a = peakAlpha * 0.22f / static_cast<float>(ring);
        g.setColour(colour.withAlpha(juce::jlimit(0.f, 1.f, a)));
        g.fillEllipse(centre.x - r, centre.y - r, r * 2.f, r * 2.f);
    }
}

// ── Mini-grid LED dot (Cartesia LED000-style) ─────────────────────────────────
// bounds   : square area to fill
// brightness: 0..1 from miniLedBrightness()
// colour   : per-layer colour (e.g. green for L1, cyan for L2, etc.)
inline void drawLedDot(juce::Graphics& g,
                       juce::Rectangle<float> bounds,
                       float brightness,
                       juce::Colour colour) {
    if (brightness <= 0.f) return;

    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float r  = bounds.getWidth() * 0.5f;

    // glow
    drawGlow(g, { cx, cy }, r, brightness, colour);

    // core fill
    g.setColour(colour.withAlpha(juce::jmin(1.f, brightness)));
    g.fillEllipse(bounds);

    // specular highlight when bright
    if (brightness >= 0.7f) {
        const float hs = r * 0.32f;
        g.setColour(juce::Colours::white.withAlpha(brightness * 0.55f));
        g.fillEllipse(cx - hs * 0.5f, cy - hs * 1.1f, hs, hs);
    }
}

// ── Main-grid knob LED (Cartesia STAGE-style) ─────────────────────────────────
// bounds       : square area for the full knob cell
// knobBright   : 0..1 for gate/active state
// stageBright  : 0..1 for playhead pulse ring
// colour       : layer accent colour
// bgColour     : panel dark background
inline void drawKnobLed(juce::Graphics& g,
                        juce::Rectangle<float> bounds,
                        float knobBright,
                        float stageBright,
                        juce::Colour colour,
                        juce::Colour bgColour = juce::Colour(0xff0d0a18)) {
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float r  = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // ── background circle ──────────────────────────────────────────────────
    g.setColour(bgColour);
    g.fillEllipse(cx - r, cy - r, r * 2.f, r * 2.f);

    // ── outer border ring (always visible, dims when gate off) ─────────────
    const float ringAlpha = knobBright > 0.f ? 0.6f : 0.18f;
    g.setColour(colour.withAlpha(ringAlpha));
    g.drawEllipse(cx - r + 1.f, cy - r + 1.f, (r - 1.f) * 2.f, (r - 1.f) * 2.f, 1.5f);

    // ── glow + inner fill when gate is on ─────────────────────────────────
    if (knobBright > 0.f) {
        drawGlow(g, { cx, cy }, r * 0.55f, knobBright * 0.9f, colour);

        const float ir = r * 0.58f;
        g.setColour(colour.withAlpha(knobBright * 0.82f));
        g.fillEllipse(cx - ir, cy - ir, ir * 2.f, ir * 2.f);

        // specular
        if (knobBright >= 0.7f) {
            const float hs = ir * 0.32f;
            g.setColour(juce::Colours::white.withAlpha(knobBright * 0.5f));
            g.fillEllipse(cx - hs * 0.4f, cy - hs * 1.15f, hs, hs);
        }
    }

    // ── playhead stage ring (white pulse) ─────────────────────────────────
    if (stageBright > 0.f) {
        // inner bright ring
        g.setColour(juce::Colours::white.withAlpha(stageBright * 0.95f));
        g.drawEllipse(cx - r - 2.f, cy - r - 2.f, (r + 2.f) * 2.f, (r + 2.f) * 2.f, 2.2f);
        // outer soft ring
        g.setColour(juce::Colours::white.withAlpha(stageBright * 0.28f));
        g.drawEllipse(cx - r - 5.f, cy - r - 5.f, (r + 5.f) * 2.f, (r + 5.f) * 2.f, 1.f);
    }
}

} // namespace cartesia::lighting
