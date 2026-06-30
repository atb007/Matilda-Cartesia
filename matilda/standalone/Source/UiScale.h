#pragma once

#include "ReactShellLayout.h"
#include <JuceHeader.h>

/**
 * User UI scale — kPreviewScale (0.52) × factor; default 0.9 (10% smaller than design 100%).
 * Corner / edge drag: 0.7…1.0 — minimum absolute size unchanged from prior 70%-of-full default.
 */
namespace matilda::ui {

enum class UiResizeGripId {
    topLeft,
    top,
    topRight,
    left,
    right,
    bottomLeft,
    bottom,
    bottomRight,
};

inline constexpr float kUiScaleMin = 0.7f;
inline constexpr float kUiScaleMax = 1.0f;
inline constexpr float kUiScaleDefault = 0.9f;

inline bool isCornerGrip(UiResizeGripId id) {
    return id == UiResizeGripId::topLeft || id == UiResizeGripId::topRight
        || id == UiResizeGripId::bottomLeft || id == UiResizeGripId::bottomRight;
}

inline float effectivePreviewScale(float uiScaleFactor) {
    const float clamped = juce::jlimit(kUiScaleMin, kUiScaleMax, uiScaleFactor);
    return matilda::react::kPreviewScale * clamped;
}

inline juce::Point<int> viewportPixelSize(float viewportDesignW, float uiScaleFactor) {
    const float s = effectivePreviewScale(uiScaleFactor);
    return { matilda::react::sx(viewportDesignW, s), matilda::react::sx(matilda::react::kFrameH, s) };
}

inline juce::Point<int> editorResizeLimitsMin() {
    return viewportPixelSize(matilda::react::kCollapsedW, kUiScaleMin);
}

inline juce::Point<int> editorResizeLimitsMax(float uiScaleFactor = kUiScaleMax) {
    return viewportPixelSize(matilda::react::kExpandedW, uiScaleFactor);
}

inline int referenceViewportWidth100(float viewportDesignW) {
    return matilda::react::sx(viewportDesignW, matilda::react::kPreviewScale);
}

inline int referenceViewportHeight100() {
    return matilda::react::sx(matilda::react::kFrameH, matilda::react::kPreviewScale);
}

inline float uiScaleFactorFromGripDrag(UiResizeGripId grip,
                                       int deltaX,
                                       int deltaY,
                                       int startWidth,
                                       int startHeight,
                                       int refWidth100,
                                       int refHeight100) {
    float fw = 1.f;
    float fh = 1.f;

    switch (grip) {
        case UiResizeGripId::topLeft:
            fw = static_cast<float>(startWidth - deltaX) / static_cast<float>(refWidth100);
            fh = static_cast<float>(startHeight - deltaY) / static_cast<float>(refHeight100);
            return juce::jlimit(kUiScaleMin, kUiScaleMax, (fw + fh) * 0.5f);
        case UiResizeGripId::topRight:
            fw = static_cast<float>(startWidth + deltaX) / static_cast<float>(refWidth100);
            fh = static_cast<float>(startHeight - deltaY) / static_cast<float>(refHeight100);
            return juce::jlimit(kUiScaleMin, kUiScaleMax, (fw + fh) * 0.5f);
        case UiResizeGripId::bottomLeft:
            fw = static_cast<float>(startWidth - deltaX) / static_cast<float>(refWidth100);
            fh = static_cast<float>(startHeight + deltaY) / static_cast<float>(refHeight100);
            return juce::jlimit(kUiScaleMin, kUiScaleMax, (fw + fh) * 0.5f);
        case UiResizeGripId::bottomRight:
            fw = static_cast<float>(startWidth + deltaX) / static_cast<float>(refWidth100);
            fh = static_cast<float>(startHeight + deltaY) / static_cast<float>(refHeight100);
            return juce::jlimit(kUiScaleMin, kUiScaleMax, (fw + fh) * 0.5f);
        case UiResizeGripId::top:
            return juce::jlimit(kUiScaleMin, kUiScaleMax,
                                static_cast<float>(startHeight - deltaY) / static_cast<float>(refHeight100));
        case UiResizeGripId::bottom:
            return juce::jlimit(kUiScaleMin, kUiScaleMax,
                                static_cast<float>(startHeight + deltaY) / static_cast<float>(refHeight100));
        case UiResizeGripId::left:
            return juce::jlimit(kUiScaleMin, kUiScaleMax,
                                static_cast<float>(startWidth - deltaX) / static_cast<float>(refWidth100));
        case UiResizeGripId::right:
            return juce::jlimit(kUiScaleMin, kUiScaleMax,
                                static_cast<float>(startWidth + deltaX) / static_cast<float>(refWidth100));
    }

    return kUiScaleDefault;
}

} // namespace matilda::ui
