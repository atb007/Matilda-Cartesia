#pragma once

#include <JuceHeader.h>

// ─── VCV Cartesia coordinate system ────────────────────────────────────────
// Panel: 162.56mm × 128.5mm (32HP Eurorack, approx.)
// Widget positions in Cartesia.cpp use VCV pixels at 75dpi:
//   1 VCV px = 25.4/75 mm = 0.33867 mm
// A scale=1.04 compensation factor is applied in the module widget.
// All raw constants below are the original Vec() x/y arguments (before ×1.04).
//
// To convert a VCV Vec(x, y) to a normalised [0,1] fraction of the panel:
//   norm_x = x * kVcvScale / kVcvPxPerMm / kPanelMmW
//   norm_y = y * kVcvScale / kVcvPxPerMm / kPanelMmH
//
// Then to a JUCE point in an editor of (w, h) pixels:
//   juce_x = norm_x * w
//   juce_y = norm_y * h
// ────────────────────────────────────────────────────────────────────────────

namespace cartesia::grid {

inline constexpr float kVcvScale   = 1.04f;          // Cartesia.cpp compensation
inline constexpr float kVcvPxPerMm = 75.f / 25.4f;   // = 2.9527 px/mm at 75dpi
inline constexpr float kPanelMmW   = 162.56f;
inline constexpr float kPanelMmH   = 128.5f;

// Convert a raw VCV Vec() value to a normalised panel fraction [0,1]
inline float normX(float vcvX) { return vcvX * kVcvScale / kVcvPxPerMm / kPanelMmW; }
inline float normY(float vcvY) { return vcvY * kVcvScale / kVcvPxPerMm / kPanelMmH; }

// Convert normalised coords to JUCE point given editor bounds
inline juce::Point<float> toJuce(float nx, float ny, juce::Rectangle<float> area) {
    return { area.getX() + nx * area.getWidth(), area.getY() + ny * area.getHeight() };
}

// ─── LED mini-grid positions (raw VCV Vec args, x=col, y=row, z=layer) ─────
// LED{x}{y}{z}:  x = col (0-3), y = row (0-3), z = z-slice/layer (0-3)
// Raw origin: (29.04, 53.52); col adds (+10.86x, +6.48y); row adds +17.392y; layer adds +45.547x

inline constexpr float kLedOriginX  = 29.04f;
inline constexpr float kLedOriginY  = 53.52f;
inline constexpr float kLedColDx    = 10.86f;   // x offset per x-step
inline constexpr float kLedColDy    = 6.48f;    // y offset per x-step (isometric)
inline constexpr float kLedRowDy    = 17.392f;  // y offset per y-step
inline constexpr float kLedLayerDx  = 45.547f;  // x offset per z-layer

inline juce::Point<float> ledVcvPx(int x, int y, int z) {
    return {
        kLedOriginX + static_cast<float>(x) * kLedColDx + static_cast<float>(z) * kLedLayerDx,
        kLedOriginY + static_cast<float>(x) * kLedColDy + static_cast<float>(y) * kLedRowDy
    };
}

inline juce::Point<float> ledNorm(int x, int y, int z) {
    const auto p = ledVcvPx(x, y, z);
    return { normX(p.x), normY(p.y) };
}

inline juce::Point<float> ledInArea(int x, int y, int z, juce::Rectangle<float> area) {
    const auto n = ledNorm(x, y, z);
    return toJuce(n.x, n.y, area);
}

// ─── SLICE button positions (raw VCV Vec args) ──────────────────────────────
// z = 0..3 → Vec(47.28, 40.08) / (94.292, 40.08) / (141.304, 40.08) / (188.316, 40.08)
inline constexpr float kSliceRawX[4] = { 47.28f, 94.292f, 141.304f, 188.316f };
inline constexpr float kSliceRawY    = 40.08f;

inline juce::Point<float> sliceNorm(int z) {
    return { normX(kSliceRawX[z & 3]), normY(kSliceRawY) };
}

inline juce::Point<float> sliceInArea(int z, juce::Rectangle<float> area) {
    const auto n = sliceNorm(z);
    return toJuce(n.x, n.y, area);
}

// ─── STAGE light positions (4×4 grid behind main knobs) ─────────────────────
// STAGE{x}{y}: x = col (0-3), y = row (0-3)
// STAGE00: (266.304, 82.444); col dx=58.299; row dy varies (~63)
inline constexpr float kStageRawX[4] = { 266.304f, 324.603f, 382.902f, 441.202f };
inline constexpr float kStageRawY[4] = {  82.444f, 145.468f, 207.389f, 271.501f };

inline juce::Point<float> stageNorm(int x, int y) {
    return { normX(kStageRawX[x & 3]), normY(kStageRawY[y & 3]) };
}

inline juce::Point<float> stageInArea(int x, int y, juce::Rectangle<float> area) {
    const auto n = stageNorm(x, y);
    return toJuce(n.x, n.y, area);
}

// ─── KNOB positions (main 4×4 knob grid) ────────────────────────────────────
// From Cartesia.cpp (exact Vec values before ×scale):
//   KNOB00:(249.463,67.39) KNOB10:(307.762,67.39) KNOB20:(366.062,67.39) KNOB30:(424.361,67.39)
//   KNOB01..KNOB03 row y: 130.414 / 192.335 / 256.447
inline constexpr float kKnobRawX[4] = { 249.463f, 307.762f, 366.062f, 424.361f };
inline constexpr float kKnobRawY[4] = {  67.39f,  130.414f, 192.335f, 256.447f };

inline juce::Point<float> knobNorm(int x, int y) {
    return { normX(kKnobRawX[x & 3]), normY(kKnobRawY[y & 3]) };
}

inline juce::Point<float> knobInArea(int x, int y, juce::Rectangle<float> area) {
    const auto n = knobNorm(x, y);
    return toJuce(n.x, n.y, area);
}

// ─── Convenience: full-panel LED position (for absolute drawing in editor) ──
inline juce::Point<float> ledInEditor(int x, int y, int z, float editorW, float editorH) {
    const auto n = ledNorm(x, y, z);
    return { n.x * editorW, n.y * editorH };
}

inline juce::Point<float> stageInEditor(int x, int y, float editorW, float editorH) {
    const auto n = stageNorm(x, y);
    return { n.x * editorW, n.y * editorH };
}

} // namespace cartesia::grid
