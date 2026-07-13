#pragma once

#include <JuceHeader.h>
#include "MatildaFonts.h"
#include <vector>

namespace matilda::ui::glass {

/** Figma glass dropdown item size — boosted for JUCE legibility (same ratio as Movement title). */
inline constexpr float kDdItemFigmaFs = 16.f;
inline constexpr float kDdItemFs = 20.f;

/**
 * Screen-space dropdown list item metrics — option labels stay at a fixed pixel size
 * regardless of app UI resize scale (Movement, Quantise, Transport glass menus).
 *
 * Figma / React original: 16px. JUCE nominal before screen-space fix: 20px × UI scale
 * (~9–14px on screen at default zoom). Screen-space target: 18px (legible, not oversized).
 */
inline constexpr float kDdItemScreenFs = 18.f;
inline constexpr float kDdItemScreenLineGap = 8.f;
inline constexpr float kDdItemScreenItemGap = 5.f;
inline constexpr float kDdItemLineMul = 1.15f;
/** Tightest menu uses 83% of panel width for item text (Movement). */
inline constexpr float kDdItemWidthFraction = 0.83f;
inline constexpr float kDdMenuHorizPadPx = 44.f;

inline juce::Font ddItemMenuFont() {
    return matilda::fonts::kodeMonoBold(kDdItemScreenFs);
}

/** Panel width in screen px — at least {@p minWidthPx}, expanded to fit widest label. */
inline int ddMenuWidthForItems(const juce::StringArray& items, int minWidthPx) {
    const auto font = ddItemMenuFont();
    int maxText = 0;
    for (const auto& item : items)
        maxText = juce::jmax(maxText, font.getStringWidth(item));

    const int needed =
        juce::roundToInt(static_cast<float>(maxText) / kDdItemWidthFraction + kDdMenuHorizPadPx);
    return juce::jmax(minWidthPx, needed);
}

inline float ddItemTextHeightScreen() {
    return kDdItemScreenFs * kDdItemLineMul;
}

/** Gap between rows: hairline + spacing (not drawn after the last item). */
inline float ddItemSeparatorScreen() {
    return kDdItemScreenLineGap + 1.f + kDdItemScreenItemGap;
}

/** Total list-area height for {@p itemCount} rows — separators only between items. */
inline int ddMenuListHeightScreen(int itemCount) {
    if (itemCount <= 0)
        return 0;
    return juce::roundToInt(ddItemTextHeightScreen() * static_cast<float>(itemCount)
                            + ddItemSeparatorScreen() * static_cast<float>(itemCount - 1));
}

/** Minimum top/bottom inset so the rounded frame border stays visible with fixed-size items. */
inline constexpr float kDdMinVertPadScreen = 10.f;

inline float ddMenuVertPadScreen(float designPadY, float designScale) {
    return juce::jmax(kDdMinVertPadScreen, designPadY * designScale);
}

/** Full dropdown panel height — scaled vertical chrome + fixed-size list block. */
inline int ddMenuHeightScreen(int visibleItemCount, float designScale, float designPadY) {
    const float pad = ddMenuVertPadScreen(designPadY, designScale);
    if (visibleItemCount <= 0)
        return juce::roundToInt(pad * 2.f);
    return juce::roundToInt(pad * 2.f + static_cast<float>(ddMenuListHeightScreen(visibleItemCount)));
}

inline float ddItemBlockHeight(float scale) {
    juce::ignoreUnused(scale);
    return ddItemTextHeightScreen() + ddItemSeparatorScreen();
}

/** Uniform row step used for scroll offset (each row includes its separator slot). */
inline float ddItemScrollStrideScreen() {
    return ddItemTextHeightScreen() + ddItemSeparatorScreen();
}

/** Advance Y after drawing item {@p index} of {@p itemCount} (separator only between items). */
inline void advanceDropdownItemY(float& y, int index, int itemCount) {
    y += ddItemTextHeightScreen();
    if (index + 1 < itemCount)
        y += ddItemSeparatorScreen();
}

inline constexpr float kInlineBoxRadius = 8.f;

/** Inline picker / setting row — matches ScalePanel PickerDropdown box variant. */
inline void drawInlinePickerBox(juce::Graphics& g, juce::Rectangle<float> bounds, float scale) {
    const float r = kInlineBoxRadius * scale;
    g.setColour(juce::Colour(0x1ab8b8b8));
    g.fillRoundedRectangle(bounds, r);
    g.setColour(juce::Colour(0xffcfeff3));
    g.drawRoundedRectangle(bounds.reduced(0.5f * scale), r, 1.f * scale);
}

namespace detail {

inline void boxBlurPass(juce::Image& img, int radius, bool horizontal) {
    if (!img.isValid() || radius <= 0)
        return;

    juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
    const int w = data.width;
    const int h = data.height;
    const int r = juce::jlimit(1, 24, radius);
    const int window = r * 2 + 1;

    std::vector<uint8_t> scratch(static_cast<size_t>(w * h * 4));

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int sumA = 0, sumR = 0, sumG = 0, sumB = 0;

            if (horizontal) {
                for (int k = -r; k <= r; ++k) {
                    const int sx = juce::jlimit(0, w - 1, x + k);
                    const auto* px = data.getPixelPointer(sx, y);
                    sumA += px[3];
                    sumR += px[0];
                    sumG += px[1];
                    sumB += px[2];
                }
            } else {
                for (int k = -r; k <= r; ++k) {
                    const int sy = juce::jlimit(0, h - 1, y + k);
                    const auto* px = data.getPixelPointer(x, sy);
                    sumA += px[3];
                    sumR += px[0];
                    sumG += px[1];
                    sumB += px[2];
                }
            }

            auto* out = scratch.data() + static_cast<size_t>((y * w + x) * 4);
            out[0] = static_cast<uint8_t>(sumR / window);
            out[1] = static_cast<uint8_t>(sumG / window);
            out[2] = static_cast<uint8_t>(sumB / window);
            out[3] = static_cast<uint8_t>(sumA / window);
        }
    }

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const auto* src = scratch.data() + static_cast<size_t>((y * w + x) * 4);
            auto* px = data.getPixelPointer(x, y);
            px[0] = src[0];
            px[1] = src[1];
            px[2] = src[2];
            px[3] = src[3];
        }
    }
}

} // namespace detail

/** Blur a captured backdrop — downscale + multi-pass box blur mimics CSS backdrop-filter. */
inline juce::Image blurSnapshot(juce::Image src, int radius = 14) {
    if (!src.isValid())
        return {};

    constexpr int downscale = 3;
    auto small = src.rescaled(juce::jmax(1, src.getWidth() / downscale),
                              juce::jmax(1, src.getHeight() / downscale),
                              juce::Graphics::lowResamplingQuality);

    const int smallRadius = juce::jmax(2, radius / downscale);
    for (int pass = 0; pass < 3; ++pass) {
        detail::boxBlurPass(small, smallRadius, true);
        detail::boxBlurPass(small, smallRadius, false);
    }

    return small.rescaled(src.getWidth(), src.getHeight(), juce::Graphics::mediumResamplingQuality);
}

inline juce::Image captureBackdrop(juce::Component& root, juce::Rectangle<int> area) {
    if (area.isEmpty())
        return {};

    auto snap = root.createComponentSnapshot(area, false);
    if (!snap.isValid())
        return {};

    return blurSnapshot(std::move(snap), 16);
}

inline void drawFrostOverlay(juce::Graphics& g, juce::Rectangle<float> bounds, float scale) {
    const float inset = 1.5f;
    const auto frame = bounds.reduced(inset);
    const float radius =
        juce::jmin(24.f * scale, frame.getWidth() * 0.5f, frame.getHeight() * 0.5f);

    // Soft shadow beneath the panel (fill only — avoids side "U" stroke artefacts).
    {
        juce::Path shadow;
        shadow.addRoundedRectangle(bounds.translated(0.f, 4.f).reduced(3.f, 0.f), radius);
        g.setColour(juce::Colours::black.withAlpha(0.22f));
        g.fillPath(shadow);
    }

    juce::ColourGradient frost(juce::Colour(0x58282c34), frame.getTopLeft(),
                               juce::Colour(0x481a1e24), frame.getBottomRight(), false);
    frost.addColour(0.35, juce::Colour(0x50323840));
    frost.addColour(0.65, juce::Colour(0x40202428));
    g.setGradientFill(frost);
    g.fillRoundedRectangle(frame, radius);

    juce::ColourGradient sheen(juce::Colours::white.withAlpha(0.14f), frame.getTopLeft(),
                               juce::Colours::white.withAlpha(0.03f), frame.getCentre(), false);
    g.setGradientFill(sheen);
    g.fillRoundedRectangle(frame, radius);

    // Single inset rounded stroke — complete frame on all four sides (not clipped at bottom).
    g.setColour(juce::Colours::white.withAlpha(0.30f));
    g.drawRoundedRectangle(frame, radius, 1.f);

    g.setColour(juce::Colours::white.withAlpha(0.20f));
    g.drawLine(frame.getX() + radius * 0.35f, frame.getY() + 0.5f, frame.getRight() - radius * 0.35f,
               frame.getY() + 0.5f, 1.f);
}

/** Frosted glass panel — blurred backdrop when provided, otherwise heavy frost fill. */
inline void drawPanel(juce::Graphics& g, juce::Rectangle<float> bounds, float scale,
                      const juce::Image& backdrop = {}) {
    const float radius = 24.f * scale;

    juce::Path clip;
    clip.addRoundedRectangle(bounds, radius);
    g.saveState();
    g.reduceClipRegion(clip);

    if (backdrop.isValid())
        g.drawImage(backdrop, bounds, juce::RectanglePlacement::stretchToFit);

    g.restoreState();

    drawFrostOverlay(g, bounds, scale);
}

inline void drawHairline(juce::Graphics& g, juce::Rectangle<float> line) {
    juce::ColourGradient grad(juce::Colours::transparentBlack, line.getCentreX(), line.getY(),
                              juce::Colours::transparentBlack, line.getRight(), line.getY(), false);
    grad.addColour(0.5, juce::Colours::white.withAlpha(0.22f));
    g.setGradientFill(grad);
    g.fillRect(line);
}

inline void drawCloseIcon(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour stroke) {
    g.setColour(stroke);
    const float inset = bounds.getWidth() * 0.22f;
    g.drawLine(bounds.getX() + inset, bounds.getY() + inset, bounds.getRight() - inset, bounds.getBottom() - inset,
               1.6f);
    g.drawLine(bounds.getRight() - inset, bounds.getY() + inset, bounds.getX() + inset, bounds.getBottom() - inset,
               1.6f);
}

} // namespace matilda::ui::glass
