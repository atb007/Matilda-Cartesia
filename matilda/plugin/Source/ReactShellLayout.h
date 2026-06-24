#pragma once

#include <JuceHeader.h>

/**
 * Pixel layout constants ported from cartesia-vst-ui (Figma-native design px).
 * Sources: shellLayout.ts · heroLayout.ts · MatildaShell.tsx · component BASE_* sizes.
 */
namespace matilda::react {

// ── Plugin frame (MatildaPluginFrame.tsx · heroLayout.ts) ───────────────────
inline constexpr float kFrameH = 1805.f;
inline constexpr float kExpandedW = 2376.f;
inline constexpr float kCollapsedW = 1515.f;
inline constexpr float kHeroPanelW = kExpandedW - kCollapsedW;
inline constexpr float kShellLeft = 886.f;   // EXPANDED_W - SHELL_W - SHELL_RIGHT_GUTTER
inline constexpr float kShellTop = 50.f;
inline constexpr float kShellRightGutter = 85.f;
inline constexpr float kPreviewScale = 0.52f; // 100% user scale (see UiScale.h)
inline constexpr int kCollapseMs = 380;

// Collapse chevron (heroLayout.ts · CollapseToggle.tsx)
inline constexpr float kIconSize = 70.f;
inline constexpr float kIconCollapsedInsetLeft = 10.f;

inline float viewportContentOffset(float viewportW) { return viewportW - kExpandedW; }

// ── Hero (heroLayout.ts · HeroCanvas.tsx) ───────────────────────────────────
inline constexpr float kHeroMainLeft = 66.f;
/** Equal viewport inset — X and Y both HERO_MAIN_LEFT + 17 (= 83 design px). */
inline constexpr float kIconExpandedLeft = kHeroMainLeft + 17.f;
inline constexpr float kIconExpandedTop = kIconExpandedLeft;
inline constexpr float kHeroBgTop = 7.f;
inline constexpr float kHeroBgW = 2310.f;
inline constexpr float kHeroBgH = 1798.f;
inline constexpr float kHeroMaskLeft = -463.39f;
inline constexpr float kHeroMaskTop = -217.05f;
inline constexpr float kHeroMaskW = 1853.47f;
inline constexpr float kHeroMaskH = 2048.29f;
inline constexpr float kHeroPortraitLeft = -895.32f;
inline constexpr float kHeroPortraitTop = -47.48f;
inline constexpr float kHeroPortraitW = 1905.46f;
inline constexpr float kHeroPortraitH = 1716.41f;
inline constexpr float kHeroLabelLeft = 109.f;
inline constexpr float kHeroLabelTop = 1449.f;
inline constexpr float kHeroLabelW = 480.f;
inline constexpr float kHeroTitleFs = 180.f;
inline constexpr float kHeroTitleLineH = 120.7f;
inline constexpr float kHeroSubtitleFs = 60.f;
inline constexpr float kHeroSubtitleGap = 21.f;

// ── Control shell (shellLayout.ts) ──────────────────────────────────────────
inline constexpr float kShellW = 1405.f;
inline constexpr float kShellH = 1737.f;

inline constexpr float kFrameOverlayLeft = 0.f;
inline constexpr float kFrameOverlayTop = -14.f;
inline constexpr float kFrameOverlayW = 1405.f;
inline constexpr float kFrameOverlayH = 1766.f;

inline constexpr float kGlassLeft = 83.2177734375f;
inline constexpr float kGlassTop = 151.f;
inline constexpr float kGlassW = 1205.f;
inline constexpr float kGlassH = 1407.f;

// ── MatildaShell.tsx — module positions (shell-absolute) ────────────────────
struct Point { float x, y; };
struct Size { float w, h; };

inline constexpr Point kScalePanelPos{118.f, 200.f};
inline constexpr Size kScalePanelSize{418.f, 598.f};

inline constexpr Point kLayerOverviewPos{628.879f, 218.21f};
inline constexpr Size kLayerOverviewSize{609.565f, 301.43f};

inline constexpr Point kMovementPos{675.18f, 667.796f};
inline constexpr Size kMovementSize{514.f, 89.f};

inline constexpr Point kGridPos{629.f, 818.945f};
inline constexpr float kGridCellW = 122.35546875f;
inline constexpr float kGridCellH = 140.9765625f;
inline constexpr float kGridColGap = 38.64453125f;
inline constexpr float kGridRowGap = 17.078125f;
inline constexpr float kGridW = kGridCellW * 4.f + kGridColGap * 3.f;
inline constexpr float kGridH = kGridCellH * 4.f + kGridRowGap * 3.f;

inline constexpr Point kTransportPos{103.218f, 887.449f};
inline constexpr Size kTransportSize{439.f, 485.f};

// TransportChrome.tsx — clock row inside transport module
inline constexpr Point kTransportClockPos{0.f, 127.55f + 156.37f + 28.f + 12.f}; // play + gap + playMode + gap
inline constexpr Size kTransportClockSize{251.169f, 28.f};

inline constexpr Point kTransportPlayPos{0.f, 127.55f};
inline constexpr Size kTransportPlaySize{156.37f, 156.37f};

// ── Scale helpers ───────────────────────────────────────────────────────────
inline int sx(float designPx, float scale = kPreviewScale) {
    return juce::roundToInt(designPx * scale);
}

inline juce::Rectangle<float> designRectF(float x, float y, float w, float h, float scale = kPreviewScale) {
    return {x * scale, y * scale, w * scale, h * scale};
}

inline juce::Rectangle<int> designRect(float x, float y, float w, float h, float scale = kPreviewScale) {
    return {sx(x, scale), sx(y, scale), sx(w, scale), sx(h, scale)};
}

inline juce::Point<int> designPoint(float x, float y, float scale = kPreviewScale) {
    return {sx(x, scale), sx(y, scale)};
}

} // namespace matilda::react
