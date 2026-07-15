#pragma once

#include "RiveHeroConfig.h"

namespace matilda::rive {

/** Glow/streak booleans — only applied while transport is playing. */
struct LayerGlowState {
    bool bodyStreak = false;
    bool bodyGlow = false;
    bool faceGlowVis = false;
    bool faceStreakVis = false;
};

inline LayerGlowState layerGlowFromInputs(int activeLayerCount, bool polyphony) {
    LayerGlowState s;
    s.bodyGlow = activeLayerCount >= 2;
    s.faceGlowVis = activeLayerCount >= 2;
    s.bodyStreak = activeLayerCount >= 3;
    // faceStreakVis — polyphony only meaningful with ≥2 active layers (same as crown).
    // Cleared when paused via layerGlowForTransport.
    s.faceStreakVis = polyphony && activeLayerCount >= 2;
    return s;
}

inline LayerGlowState layerGlowForTransport(bool playing, int activeLayerCount, bool polyphony) {
    return playing ? layerGlowFromInputs(activeLayerCount, polyphony) : LayerGlowState{};
}

} // namespace matilda::rive
