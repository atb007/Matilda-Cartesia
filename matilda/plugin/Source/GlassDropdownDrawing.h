#pragma once

#include <JuceHeader.h>
#include <vector>

namespace matilda::ui::glass {

/** Figma glass dropdown item size — boosted for JUCE legibility (same ratio as Movement title). */
inline constexpr float kDdItemFigmaFs = 16.f;
inline constexpr float kDdItemFs = 20.f;
inline constexpr float kDdItemLineMul = 1.25f;
inline constexpr float kInlineBoxRadius = 8.f;

inline float ddItemBlockHeight(float scale) {
    return kDdItemFs * scale * kDdItemLineMul;
}

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
    const float radius = 24.f * scale;

    // Light charcoal tint — blurred backdrop should remain visible through the glass.
    juce::ColourGradient frost(juce::Colour(0x58282c34), bounds.getTopLeft(),
                               juce::Colour(0x481a1e24), bounds.getBottomRight(), false);
    frost.addColour(0.35, juce::Colour(0x50323840));
    frost.addColour(0.65, juce::Colour(0x40202428));
    g.setGradientFill(frost);
    g.fillRoundedRectangle(bounds, radius);

    juce::ColourGradient sheen(juce::Colours::white.withAlpha(0.14f), bounds.getTopLeft(),
                               juce::Colours::white.withAlpha(0.03f), bounds.getCentre(), false);
    g.setGradientFill(sheen);
    g.fillRoundedRectangle(bounds, radius);

    g.setColour(juce::Colours::white.withAlpha(0.20f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), radius, 1.f);

    const float inset = juce::jmax(1.f, 1.5f * scale);
    g.setColour(juce::Colours::white.withAlpha(0.30f));
    g.drawLine(bounds.getX() + inset, bounds.getY() + inset, bounds.getRight() - inset, bounds.getY() + inset,
               inset);
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawLine(bounds.getX() + inset, bounds.getY() + inset, bounds.getX() + inset, bounds.getBottom() - inset,
               inset);

    g.setColour(juce::Colours::black.withAlpha(0.32f));
    g.drawLine(bounds.getX() + inset, bounds.getBottom() - inset, bounds.getRight() - inset,
               bounds.getBottom() - inset, 2.5f * scale);
    g.setColour(juce::Colours::black.withAlpha(0.18f));
    g.drawLine(bounds.getRight() - inset, bounds.getY() + inset, bounds.getRight() - inset,
               bounds.getBottom() - inset, 1.5f * scale);

    g.setColour(juce::Colour(0x0dffffff));
    g.drawRoundedRectangle(bounds.reduced(inset * 2.f), radius - inset * 2.f, 1.f);

    g.setColour(juce::Colours::black.withAlpha(0.38f));
    g.drawRoundedRectangle(bounds.translated(0.f, 8.f * scale).reduced(4.f, 0.f), radius, 2.f * scale);
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
