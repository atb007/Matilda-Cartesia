#pragma once

#include "RiveHeroConfig.h"

namespace matilda::rive {

/** Layer-driven glow/streak booleans — only applied while transport is playing. */
struct LayerGlowState {
    bool bodyStreak = false;
    bool bodyGlow = false;
    bool faceGlowVis = false;
    bool faceStreakVis = false;
};

inline LayerGlowState layerGlowFromActiveCount(int activeLayerCount) {
    LayerGlowState s;
    s.bodyGlow = activeLayerCount >= 2;
    s.faceGlowVis = activeLayerCount >= 2;
    s.bodyStreak = activeLayerCount >= 3;
    s.faceStreakVis = activeLayerCount >= 4;
    return s;
}

inline LayerGlowState layerGlowForTransport(bool playing, int activeLayerCount) {
    return playing ? layerGlowFromActiveCount(activeLayerCount) : LayerGlowState{};
}

} // namespace matilda::rive
