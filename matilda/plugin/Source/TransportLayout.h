#pragma once

#include <JuceHeader.h>
#include "GlassDropdownDrawing.h"

/**
 * Global Settings / transport chrome geometry — ported from TransportChrome.tsx (Figma 4991:4644).
 */
namespace matilda::transport {

inline constexpr float kBaseW = 439.f;
inline constexpr float kBaseH = 485.f;

inline constexpr float kTitleFigmaFs = 18.875f;
/** Boosted for JUCE legibility — matches Movement title. */
inline constexpr float kTitleFs = 24.f;
inline constexpr float kTitleFontBoost = kTitleFs / kTitleFigmaFs;
inline constexpr float kTitleTrack = 0.755f * kTitleFontBoost;
inline constexpr float kLabelFs = 24.f;
inline constexpr float kValueFs = 20.f;

inline constexpr float kFiligreeW = 383.087f;
inline constexpr float kFiligreeH = 26.287f;
inline constexpr float kFiligreeTopLeft = (kBaseW - kFiligreeW) * 0.5f;
inline constexpr float kFiligreeBotTop = 61.05f;

/** Movement-style title texture strip (no chevrons). */
inline constexpr float kTitleTextureW = 354.88f;
inline constexpr float kTitleTextureH = 31.31f;
inline constexpr float kTitleTextureY = 28.84f;
inline constexpr float kTitleTextureLeft = (kBaseW - kTitleTextureW) * 0.5f;
inline constexpr float kTitleFiligreeAlphaScale = 0.40f;
inline constexpr uint8_t kTitleFiligreeGrey = 186;
inline constexpr float kNeonShadowY = 2.5f * kTitleFontBoost;
inline constexpr float kTitleCenterY = kTitleTextureY + kTitleTextureH * 0.5f;

inline constexpr float kColTop = 127.55f;
inline constexpr float kColGap = 28.f;
inline constexpr float kRowGap = 12.f;
inline constexpr float kPlaySize = 156.37f;
/** Ornamental frame extends slightly past the Figma slot — avoid clipping glow/spikes. */
inline constexpr float kPlayFrameBleed = 5.f;
inline constexpr float kPlayModeW = 277.5f;
inline constexpr float kPlayModeTitleW = 264.18f;
inline constexpr float kClockW = 251.169f;

inline constexpr float kGlassInsetTop = 0.0766f;
inline constexpr float kGlassInsetRight = 0.0735f;
inline constexpr float kGlassInsetBottom = 0.0782f;
inline constexpr float kGlassInsetLeft = 0.061f;

inline constexpr float kDropdownPadX = 15.f;
inline constexpr float kDropdownPadY = 6.f;
inline constexpr float kDropdownRadius = 12.f;
inline constexpr float kDropdownBorder = 1.5f;
inline constexpr float kDropdownValuePadLeft = 39.f;
inline constexpr float kChevronW = 9.f;
inline constexpr float kChevronGap = 4.f;
inline constexpr float kDdW = 220.f;
inline constexpr float kDdPadY = 14.f;
inline constexpr float kDdItemFs = matilda::ui::glass::kDdItemFs;
inline constexpr float kDdItemGap = 11.f;
inline constexpr float kDdLineGap = 15.f;
inline constexpr float kDdClose = 18.f;

inline constexpr float kColLeft = (kBaseW - kPlayModeW) * 0.5f;

} // namespace matilda::transport
