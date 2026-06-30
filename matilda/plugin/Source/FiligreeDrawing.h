#pragma once

#include <JuceHeader.h>

namespace matilda::ui::filigree {

struct Layout {
    float filigreeW = 383.f;
    float filigreeLeft = 0.f;
    float textureW = 354.88f;
    float textureLeft = 0.f;
    float alphaScale = 0.40f;
    uint8_t grey = 186;
};

/** Matches SVG horizontal gradient (edges fade, centre bright). */
inline float horizontalEnvelope(float t) {
    t = juce::jlimit(0.f, 1.f, t);
    if (t <= 0.f || t >= 1.f)
        return 0.f;
    if (t < 0.155051f)
        return t / 0.155051f;
    if (t > 0.852137f)
        return (1.f - t) / (1.f - 0.852137f);
    return 1.f;
}

inline juce::String prepareSvgForRaster(juce::String svg) {
    for (;;) {
        const int defsStart = svg.indexOf("<defs>");
        if (defsStart < 0)
            break;
        const int defsEnd = svg.indexOf(defsStart, "</defs>");
        if (defsEnd < 0)
            break;
        svg = svg.replaceSection(defsStart, defsEnd - defsStart + 7, "");
    }

    for (;;) {
        const int start = svg.indexOf("fill=\"url(#");
        if (start < 0)
            break;
        const int end = svg.indexOf(start, "\")");
        if (end < 0)
            break;
        svg = svg.replaceSection(start, end - start + 2, "fill=\"#FFFFFF\"");
    }

    svg = svg.replace(" fill-opacity=\"0.2\"", "");
    svg = svg.replace("fill-opacity=\"0.2\" ", "");
    return svg;
}

inline juce::Colour sampleTextureAtDesignX(const juce::Image& texture, float designX, const Layout& layout) {
    if (!texture.isValid())
        return juce::Colours::transparentBlack;

    const float textureT = juce::jlimit(0.f, 1.f, (designX - layout.textureLeft) / layout.textureW);
    const int x = juce::roundToInt(textureT * static_cast<float>(texture.getWidth() - 1));
    const int y = texture.getHeight() / 2;
    return texture.getPixelAt(x, y);
}

inline juce::Colour tintFromTexture(const juce::Image& texture, float filigreeT, const Layout& layout) {
    const float designX = layout.filigreeLeft + filigreeT * layout.filigreeW;
    const float textureT = (designX - layout.textureLeft) / layout.textureW;
    const float textureRight = layout.textureLeft + layout.textureW;
    const float wingT = (layout.filigreeW - layout.textureW) * 0.5f / layout.filigreeW;

    if (textureT >= 0.f && textureT <= 1.f)
        return sampleTextureAtDesignX(texture, designX, layout);

    const auto edge = sampleTextureAtDesignX(texture, textureT < 0.f ? layout.textureLeft : textureRight, layout);
    if (textureT < 0.f) {
        const float wing = juce::jlimit(0.f, 1.f, filigreeT / wingT);
        return edge.withAlpha(edge.getFloatAlpha() * wing);
    }

    const float wing = juce::jlimit(0.f, 1.f, (1.f - filigreeT) / wingT);
    return edge.withAlpha(edge.getFloatAlpha() * wing);
}

inline float envelopeAlphaAt(const juce::Image& texture, float filigreeT, const Layout& layout) {
    const auto sample = tintFromTexture(texture, filigreeT, layout);
    const float grad = horizontalEnvelope(filigreeT) * 0.2f;
    const float tex = juce::jmax(sample.getFloatAlpha(), sample.getBrightness() * 0.75f);
    return juce::jmax(grad, tex) * layout.alphaScale;
}

inline void applyTextureTint(juce::Image& img, const juce::Image& texture, const Layout& layout) {
    if (!img.isValid() || img.getWidth() <= 1)
        return;

    const int w = img.getWidth();
    const int h = img.getHeight();
    const float invW = 1.f / static_cast<float>(w - 1);
    const uint8_t greyByte = layout.grey;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const auto src = img.getPixelAt(x, y);
            const float shapeA =
                juce::jmax(src.getFloatRed(), src.getFloatGreen(), src.getFloatBlue(), src.getFloatAlpha());
            if (shapeA <= 0.f)
                continue;

            const float envelopeA = envelopeAlphaAt(texture, static_cast<float>(x) * invW, layout);
            const float alpha = envelopeA * shapeA;
            img.setPixelAt(x, y, juce::Colour::fromFloatRGBA(
                                        static_cast<float>(greyByte) / 255.f,
                                        static_cast<float>(greyByte) / 255.f,
                                        static_cast<float>(greyByte) / 255.f, alpha));
        }
    }
}

inline juce::Image rasterizeSvg(const char* data, int size, int width, int height, const juce::Image& texture,
                                const Layout& layout) {
    const auto flat = prepareSvgForRaster(juce::String::fromUTF8(data, static_cast<size_t>(size)));
    const auto xml = juce::parseXML(flat);
    if (!xml)
        return {};
    const auto drawable = juce::Drawable::createFromSVG(*xml);
    if (!drawable)
        return {};

    juce::Image img(juce::Image::ARGB, width, height, true);
    juce::Graphics g(img);
    g.fillAll(juce::Colours::transparentBlack);
    drawable->drawWithin(g, juce::Rectangle<float>(0.f, 0.f, static_cast<float>(width), static_cast<float>(height)),
                         juce::RectanglePlacement::stretchToFit, 1.f);
    applyTextureTint(img, texture, layout);
    return img;
}

/** Bottom title filigree — Figma rotate(180) scaleX(-1) ≡ vertical mirror of top. */
inline juce::Image flipImageVertically(const juce::Image& src) {
    if (!src.isValid())
        return {};

    const int w = src.getWidth();
    const int h = src.getHeight();
    juce::Image dst(juce::Image::ARGB, w, h, true);
    juce::Graphics g(dst);
    g.drawImageTransformed(src, juce::AffineTransform::scale(1.f, -1.f).translated(0.f, static_cast<float>(h)));
    return dst;
}

inline void drawImage(juce::Graphics& g, const juce::Image& img, juce::Rectangle<float> dest) {
    if (img.isValid())
        g.drawImage(img, dest, juce::RectanglePlacement::stretchToFit);
}

inline void drawImageFlippedY(juce::Graphics& g, const juce::Image& img, juce::Rectangle<float> dest) {
    if (!img.isValid())
        return;
    g.saveState();
    g.addTransform(juce::AffineTransform::scale(1.f, -1.f, dest.getCentreX(), dest.getCentreY()));
    g.drawImage(img, dest, juce::RectanglePlacement::stretchToFit);
    g.restoreState();
}

inline void drawImageFlipped180ScaleX(juce::Graphics& g, const juce::Image& img, juce::Rectangle<float> dest) {
    if (!img.isValid())
        return;
    g.saveState();
    g.addTransform(juce::AffineTransform::scale(-1.f, -1.f, dest.getCentreX(), dest.getCentreY()));
    g.drawImage(img, dest, juce::RectanglePlacement::stretchToFit);
    g.restoreState();
}

/** React / standalone parity — render SVG with native gradients (no raster tint pass). */
inline std::unique_ptr<juce::Drawable> loadSvgDrawable(const char* data, int size) {
    const auto xml = juce::parseXML(juce::String::fromUTF8(data, static_cast<size_t>(size)));
    if (!xml)
        return {};
    return juce::Drawable::createFromSVG(*xml);
}

inline void drawDrawableInRect(juce::Graphics& g, const juce::Drawable& drawable, juce::Rectangle<float> dest) {
    drawable.drawWithin(g, dest, juce::RectanglePlacement::stretchToFit, 1.f);
}

/** Figma bottom filigree — rotate(180deg) scaleX(-1). */
inline void drawDrawableFlipped180ScaleX(juce::Graphics& g, const juce::Drawable& drawable,
                                         juce::Rectangle<float> dest) {
    g.saveState();
    g.addTransform(juce::AffineTransform::scale(-1.f, -1.f, dest.getCentreX(), dest.getCentreY()));
    drawable.drawWithin(g, dest, juce::RectanglePlacement::stretchToFit, 1.f);
    g.restoreState();
}

} // namespace matilda::ui::filigree
