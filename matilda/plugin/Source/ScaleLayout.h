#pragma once

#include <JuceHeader.h>
#include "GlassDropdownDrawing.h"

/**
 * Quantise Scale panel geometry — ported from ScalePanel.tsx (Figma 4976:3937).
 */
namespace matilda::scale {

inline constexpr float kBaseW = 418.f;
inline constexpr float kBaseH = 598.f;

inline constexpr float kFiligreeW = 383.2f;
inline constexpr float kFiligreeH = 26.73f;
inline constexpr float kFiligreeCentreLeft = (kBaseW - kFiligreeW) * 0.5f;
inline constexpr float kFiligreeTop = -9.99f;
inline constexpr float kRuleTop = 19.28f;
inline constexpr float kRuleH = 31.31f;
inline constexpr float kFiligreeBotTop = kRuleTop + kRuleH + 2.f;
/** Movement-style title texture strip (same recipe as Global Settings). */
inline constexpr float kTitleTextureW = 354.88f;
inline constexpr float kTitleTextureH = 31.31f;
inline constexpr float kTitleTextureY = kRuleTop;
inline constexpr float kTitleTextureLeft = (kBaseW - kTitleTextureW) * 0.5f;
inline constexpr float kTitleFiligreeAlphaScale = 0.40f;
inline constexpr uint8_t kTitleFiligreeGrey = 186;
inline constexpr float kTitleFigmaFs = 18.875f;
/** Boosted for JUCE legibility — matches Movement title. */
inline constexpr float kTitleFs = 24.f;
inline constexpr float kTitleFontBoost = kTitleFs / kTitleFigmaFs;
inline constexpr float kTitleTrack = 0.755f * kTitleFontBoost;
inline constexpr float kNeonShadowY = 2.5f * kTitleFontBoost;
inline constexpr float kTitleCenterY = kTitleTextureY + kTitleTextureH * 0.5f;

inline constexpr float kMinMaxTop = 95.f;
inline constexpr float kLabelFs = 24.f;
/** Decorative font needs wider slots than string metrics (avoid clipping last glyph). */
inline constexpr float kMinLabelSlotW = 44.f;
inline constexpr float kTonicLabelSlotW = 64.f;
inline constexpr float kMaxLabelSlotW = 44.f;
inline constexpr float kLabelRowGap = 11.f;
inline constexpr float kMinMaxRowLeft = 4.f;
inline constexpr float kMinMaxRowW = 394.f;
inline constexpr float kPickerGap = 11.f;
inline constexpr float kConnectorW = 23.59f;
inline constexpr float kConnectorH = 2.f;
inline constexpr float kTonicPillW = 75.f;
inline constexpr float kTonicPillMinH = 37.f;
inline constexpr float kTonicPillPadY = 7.f;
inline constexpr float kTonicPillPadX = 18.f;
inline constexpr float kNoteFs = 18.f;
inline constexpr float kTonicFs = 24.f;
inline constexpr float kMinMaxBoxW =
    (kMinMaxRowW - 2.f * kConnectorW - kTonicPillW - 4.f * kPickerGap) * 0.5f;
inline constexpr float kMinMaxBoxH = 31.f;
inline constexpr float kBoxPadX = 8.f;
inline constexpr float kBoxPadY = 4.f;
inline constexpr float kDividerW = 39.f;
inline constexpr float kDividerH = 2.f;
inline constexpr float kOrnamentW = 61.22f;
inline constexpr float kOrnamentH = 19.94f;

inline constexpr float kGemW = 370.057f;
inline constexpr float kGemSlotH = 321.92f;
inline constexpr float kGemH = kGemW * (390.f / 418.f);
inline constexpr float kGemLeft = 23.13f;
inline constexpr float kGemTop = 220.03f + (kGemSlotH - kGemH) * 0.5f;

inline constexpr float kScaleBarLeft = 19.78f;
inline constexpr float kScaleBarTop = 561.f;
inline constexpr float kScaleBarW = 378.f;
inline constexpr float kScaleBarH = 28.f;
inline constexpr float kScaleBarFs = 20.f;
inline constexpr float kScaleBarTrack = 0.8f;
inline constexpr float kArrowW = 27.5f;
inline constexpr float kArrowH = 17.78f;

inline constexpr float kDdW = 220.f;
inline constexpr float kDdPadY = 14.f;
inline constexpr float kDdItemFs = matilda::ui::glass::kDdItemFs;
inline constexpr float kDdItemGap = 11.f;
inline constexpr float kDdLineGap = 15.f;
inline constexpr float kDdClose = 18.f;
inline constexpr int kDdMaxVisibleItems = 6;

inline constexpr float kBoxRadius = 8.f;
inline constexpr float kPillRadius = 31.f;
inline constexpr float kChevronW = 6.f;
inline constexpr float kChevronH = 3.f;
inline constexpr float kChevronGap = 5.f;

} // namespace matilda::scale
