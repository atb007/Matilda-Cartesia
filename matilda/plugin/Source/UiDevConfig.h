#pragma once

/**
 * JUCE UI dev harness — mirror the React cartesia-vst-ui milestone workflow.
 * Change kDevView and rebuild to preview one module in isolation (Standalone).
 */
namespace matilda::ui {

enum class DevView {
    FullShell,      // assembled M8b frame
    M1_GemCell,     // single Cell Anatomy States
    M2_Grid4x4,     // 16-cell grid only
    M3_LayerOverview,
    M4_QuantisePanel,
    M5_MovementMenu,
    M6_Transport,
    M7_ShellChrome, // glass + vines, no controls
};

/** Set to the module under test; FullShell for integrated preview. */
inline constexpr DevView kDevView = DevView::FullShell;

/** Preview scale when testing an isolated module (slightly larger than shell 0.52). */
inline constexpr float kDevPreviewScale = 0.78f;

inline bool devIsolatedModule() { return kDevView != DevView::FullShell; }

} // namespace matilda::ui
