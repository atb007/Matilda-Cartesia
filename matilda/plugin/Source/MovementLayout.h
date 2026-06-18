#pragma once

/**
 * Movement menu geometry — ported from cartesia-vst-ui MovementMenu.tsx (Figma 4957:103346).
 */
namespace matilda::movement {

inline constexpr float kBaseW = 514.f;
inline constexpr float kBaseH = 89.f;
inline constexpr float kFiligreeW = 383.2f;
inline constexpr float kFiligreeH = 26.73f;
/** Filigree sits behind the label — muted silver, peak ~12% alpha (Figma was 20% on sparse SVG). */
inline constexpr float kFiligreeAlphaScale = 0.40f;
inline constexpr uint8_t kFiligreeGrey = 186;
inline constexpr float kTextureW = 354.88f;
inline constexpr float kTextureH = 31.31f;
inline constexpr float kTextureY = 29.3f;
inline constexpr float kArrowW = 27.5f;
inline constexpr float kArrowH = 17.78f;
inline constexpr float kArrowRowY = 33.96f;
inline constexpr float kArrowGapFromTexture = 6.f;
/** Figma spec 18.875 — boosted for JUCE legibility (matches React visual weight). */
inline constexpr float kFigmaFontSize = 18.875f;
inline constexpr float kFontSize = 24.f;
inline constexpr float kFontBoost = kFontSize / kFigmaFontSize;
inline constexpr float kTracking = 0.755f * kFontBoost;
inline constexpr float kLabelPadX = 10.f * kFontBoost;
inline constexpr float kLabelPadY = 2.5f * kFontBoost;
inline constexpr float kNeonShadowY = 2.5f * kFontBoost;

inline constexpr float kTextureLeft = (kBaseW - kTextureW) * 0.5f;
inline constexpr float kTextureRight = kTextureLeft + kTextureW;
inline constexpr float kLeftArrowX = kTextureLeft - kArrowW - kArrowGapFromTexture;
inline constexpr float kRightArrowX = kTextureRight + kArrowGapFromTexture;
inline constexpr float kLabelCenterY = kTextureY + kTextureH * 0.5f;

} // namespace matilda::movement
