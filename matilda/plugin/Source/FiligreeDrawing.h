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

inline void applyTextureTint(juce::Image& img, const juce::Image& texture, const Layout& layout) {
    if (!img.isValid() || img.getWidth() <= 1)
        return;

    juce::Image::BitmapData data(img, juce::Image::BitmapData::readWrite);
    const int w = img.getWidth();
    const int h = img.getHeight();
    const float invW = 1.f / static_cast<float>(w - 1);
    const float grey = static_cast<float>(layout.grey) / 255.f;

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            auto* px = data.getPixelPointer(x, y);
            const float shapeA = static_cast<float>(px[3]) / 255.f;
            if (shapeA <= 0.f)
                continue;

            const float envelopeA =
                tintFromTexture(texture, static_cast<float>(x) * invW, layout).getFloatAlpha() * layout.alphaScale;
            const float alpha = envelopeA * shapeA;
            px[0] = static_cast<uint8_t>(grey * 255.f);
            px[1] = px[0];
            px[2] = px[0];
            px[3] = static_cast<uint8_t>(alpha * 255.f);
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

} // namespace matilda::ui::filigree
