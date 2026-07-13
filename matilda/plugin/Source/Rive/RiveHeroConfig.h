#pragma once

namespace matilda::rive {

/** Shared Rive hero settings — keep in sync with cartesia-vst-ui/src/riveConfig.ts and docs/RIVE_ANIMATION_RULEBOOK.md */
inline constexpr const char* kRiveAssetVersion = "v2";
inline constexpr const char* kArtboard = "Artboard";
inline constexpr const char* kViewModel = "HairStreaksTrimControl";
inline constexpr const char* kPlayBoolean = "streakVisible";
inline constexpr const char* kBodyStreakBoolean = "bodyStreak";
inline constexpr const char* kBodyGlowBoolean = "bodyGlow";
inline constexpr const char* kFaceGlowVisBoolean = "faceGlowVis";
inline constexpr const char* kFaceStreakVisBoolean = "faceStreakVIs";

/** Same box as static PNG / React MatildaRivePortrait (bottom-left anchor in mask space). */
inline constexpr float kPortraitOffsetX = 20.f;
inline constexpr float kPortraitOffsetY = 0.f;
inline constexpr float kPortraitHeightScale = 1.2f;
/** Uniform scale on portrait width/height (1 = design size). */
inline constexpr float kPortraitContentScale = 0.88f;

/** Optional GPU nudge after Cover+CenterLeft align (drawable px). */
inline constexpr float kGpuRenderPanX = 0.f;
inline constexpr float kGpuRenderPanY = 0.f;
inline constexpr float kMetalRenderPanX = kGpuRenderPanX;
inline constexpr float kMetalRenderPanY = kGpuRenderPanY;

inline constexpr int kIdleFps = 24;
inline constexpr int kPlayingFps = 30;
inline constexpr bool kAnimateWhenIdle = true;

} // namespace matilda::rive
