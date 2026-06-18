#pragma once

#include <JuceHeader.h>
#include "MatildaImages.h"

namespace matilda::knob {

enum class Variant { Orange = 0, Red, Green, Blue };

inline Variant variantForLayer(int layer) {
    switch (layer & 3) {
        case 1:  return Variant::Red;
        case 2:  return Variant::Green;
        case 3:  return Variant::Blue;
        default: return Variant::Orange;
    }
}

inline juce::Colour ledColour(Variant v) {
    switch (v) {
        case Variant::Red:   return juce::Colour(0xffFF5545);
        case Variant::Green: return juce::Colour(0xff50E080);
        case Variant::Blue:  return juce::Colour(0xff50B0FF);
        case Variant::Orange:
        default:             return juce::Colour(0xffFFA040);
    }
}

inline juce::Rectangle<float> insetRect(juce::Rectangle<float> b, float pct) {
    const float m = b.getWidth() * pct;
    return b.reduced(m);
}

inline void drawImageCentred(juce::Graphics& g,
                             const juce::Image& img,
                             juce::Rectangle<float> dest,
                             float opacity = 1.f) {
    if (!img.isValid() || opacity <= 0.f)
        return;
    g.setOpacity(opacity);
    g.drawImage(img, dest, juce::RectanglePlacement::centred);
    g.setOpacity(1.f);
}

inline void drawIndicatorTick(juce::Graphics& g,
                              juce::Rectangle<float> knobBounds,
                              int noteIndex,
                              bool gateOn) {
    if (!gateOn)
        return;

    const float cx = knobBounds.getCentreX();
    const float cy = knobBounds.getCentreY();
    const float sphereR = knobBounds.getWidth() * 0.5f;
    const float innerR = sphereR * 0.55f;
    const float outerR = sphereR * 0.90f;
    const float idx = juce::jlimit(0, 11, noteIndex);
    const float angleDeg = -135.f + (idx / 11.f) * 270.f;
    const float rad = juce::degreesToRadians(angleDeg);
    const float sx = cx + std::sin(rad) * innerR;
    const float sy = cy - std::cos(rad) * innerR;
    const float ex = cx + std::sin(rad) * outerR;
    const float ey = cy - std::cos(rad) * outerR;

    juce::ColourGradient grad(juce::Colour(0xffB75600), { sx, sy },
                              juce::Colour(0xff512600), { ex, ey }, false);
    g.setGradientFill(grad);
    g.drawLine(sx, sy, ex, ey, juce::jmax(2.f, knobBounds.getWidth() * 0.08f));
}

inline void drawSequencerKnob(juce::Graphics& g,
                            juce::Rectangle<float> bounds,
                            Variant variant,
                            bool gateOn,
                            int noteIndex,
                            float stageBrightness) {
    if (stageBrightness > 0.f) {
        const auto glow = matilda::images::knobStageGlow();
        if (glow.isValid()) {
            g.setOpacity(stageBrightness * 0.85f);
            g.drawImage(glow, bounds.expanded(bounds.getWidth() * 0.22f),
                        juce::RectanglePlacement::centred);
            g.setOpacity(1.f);
        }
    }

    const auto ringBounds = bounds;
    const auto sphereBounds = insetRect(bounds, 0.076f);
    const auto glossBounds = insetRect(bounds, 0.25f);

    drawImageCentred(g, matilda::images::knobOuterRing(), ringBounds);
    if (!gateOn) {
        g.setColour(juce::Colours::black.withAlpha(0.38f));
        g.fillEllipse(ringBounds);
        g.setColour(juce::Colours::white.withAlpha(0.08f));
        g.drawEllipse(ringBounds, 1.f);
    }

    drawImageCentred(g, matilda::images::knobSphere(), sphereBounds);
    drawImageCentred(g, matilda::images::knobColoredGloss(variant, false), glossBounds, gateOn ? 0.f : 1.f);
    drawImageCentred(g, matilda::images::knobColoredGloss(variant, true), glossBounds, gateOn ? 1.f : 0.f);

    if (!gateOn) {
        juce::ColourGradient inset(juce::Colours::black.withAlpha(0.85f),
                                   { sphereBounds.getCentreX(), sphereBounds.getY() },
                                   juce::Colours::transparentBlack,
                                   sphereBounds.getCentre(),
                                   true);
        g.setGradientFill(inset);
        g.fillEllipse(sphereBounds);
    }

    drawIndicatorTick(g, bounds, noteIndex, gateOn);
}

inline juce::Point<float> miniPointAt(float cx, float cy, float r, float degFrom12) {
    const float rad = juce::degreesToRadians(degFrom12);
    return { cx + std::sin(rad) * r, cy - std::cos(rad) * r };
}

inline juce::Path miniArcPath(float cx, float cy, float r, float fromDeg, float toDeg) {
    juce::Path path;
    if (toDeg <= fromDeg + 0.001f)
        return path;

    const int segments = juce::jmax(4, juce::roundToInt((toDeg - fromDeg) / 8.f));
    path.startNewSubPath(miniPointAt(cx, cy, r, fromDeg));
    for (int i = 1; i <= segments; ++i) {
        const float t = fromDeg + (toDeg - fromDeg) * (static_cast<float>(i) / static_cast<float>(segments));
        path.lineTo(miniPointAt(cx, cy, r, t));
    }
    return path;
}

inline void drawMiniKnobTooltip(juce::Graphics& g,
                                juce::Rectangle<float> knobBounds,
                                const juce::String& text,
                                float fontSize) {
    if (text.isEmpty())
        return;

    const float padX = fontSize * 0.55f;
    const float padY = fontSize * 0.25f;
    g.setFont(juce::FontOptions(fontSize).withStyle("Bold"));

    const float tw = g.getCurrentFont().getStringWidthFloat(text);
    const float th = fontSize * 1.15f;
    const float x = knobBounds.getRight() + knobBounds.getWidth() * 0.25f;
    const float y = knobBounds.getCentreY() - th * 0.5f;
    const juce::Rectangle<float> box(x, y, tw + padX * 2.f, th + padY * 2.f);

    g.setColour(juce::Colour(0xeb0c0e12));
    g.fillRoundedRectangle(box, 4.f);
    g.setColour(juce::Colour(0x40cfeff3));
    g.drawRoundedRectangle(box, 4.f, 1.f);
    g.setColour(juce::Colours::white);
    g.drawText(text, box, juce::Justification::centred);
}

inline void drawMiniChevrons(juce::Graphics& g, float cx, float cy, float size) {
    const float w = size * 0.34f;
    const float h = size * 0.18f;
    const float offset = size * 0.52f;
    g.setColour(juce::Colour(0xff888888));

    juce::Path up;
    up.addTriangle(cx, cy - offset, cx - w * 0.5f, cy - offset + h, cx + w * 0.5f, cy - offset + h);
    g.fillPath(up);

    juce::Path down;
    down.addTriangle(cx, cy + offset, cx - w * 0.5f, cy + offset - h, cx + w * 0.5f, cy + offset - h);
    g.fillPath(down);
}

struct MiniKnobPalette {
    juce::Colour track;
    juce::Colour fill;
    juce::Colour dot;
    juce::Colour ring;
};

inline MiniKnobPalette miniPalette(bool amber) {
    if (amber)
        return { juce::Colour(0xff5c3a0a), juce::Colour(0xfff0920a),
                 juce::Colour(0xffffc04a), juce::Colour(0xffc0720a) };
    return { juce::Colour(0xff1a5c42), juce::Colour(0xff2dca8c),
             juce::Colour(0xff58f0b8), juce::Colour(0xff1e8c62) };
}

inline void drawMiniKnob(juce::Graphics& g,
                         juce::Rectangle<float> bounds,
                         bool amber,
                         bool hovered,
                         bool dragging,
                         float valueOrMinusOne,
                         const juce::String& label) {
    const bool active = valueOrMinusOne >= 0.f;
    const float value = active ? juce::jlimit(0.f, 1.f, valueOrMinusOne) : 0.f;
    const auto pal = miniPalette(amber);

    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float trackR = bounds.getWidth() * 0.5f - bounds.getWidth() * (2.5f / 24.f);
    const float dotR = bounds.getWidth() * 0.15f;
    const float tipR = bounds.getWidth() * (2.f / 24.f);
    const float fontSize = juce::jmax(9.f, bounds.getHeight() * 0.46f);

    const auto face = bounds.reduced(bounds.getWidth() * 0.04f);
    juce::ColourGradient bg(juce::Colour(active ? 0xff2a2a2a : 0xff303030),
                            { cx, cy - trackR * 0.35f },
                            juce::Colour(0xff111111),
                            { cx, cy + trackR },
                            true);
    g.setGradientFill(bg);
    g.fillEllipse(face);
    g.setColour(active ? pal.ring : juce::Colour(0xff404040));
    g.drawEllipse(face, 1.5f);

    g.setColour(active ? pal.track : juce::Colour(0xff2a2a2a));
    g.strokePath(miniArcPath(cx, cy, trackR, -135.f, 135.f),
                 juce::PathStrokeType(juce::jmax(1.5f, bounds.getWidth() * (2.f / 24.f)),
                                      juce::PathStrokeType::curved,
                                      juce::PathStrokeType::rounded));

    const float currentAngle = active ? (-135.f + value * 270.f) : -135.f;

    if (active && value > 0.01f) {
        g.setColour(pal.fill);
        g.strokePath(miniArcPath(cx, cy, trackR, -135.f, currentAngle),
                     juce::PathStrokeType(juce::jmax(2.f, bounds.getWidth() * (2.5f / 24.f)),
                                          juce::PathStrokeType::curved,
                                          juce::PathStrokeType::rounded));
    }

    g.setColour(active ? pal.dot : juce::Colour(0xff555555));
    g.fillEllipse(cx - dotR, cy - dotR, dotR * 2.f, dotR * 2.f);

    if (active) {
        const float rad = juce::degreesToRadians(currentAngle);
        const float tx = cx + std::sin(rad) * trackR;
        const float ty = cy - std::cos(rad) * trackR;
        g.setColour(pal.dot);
        g.fillEllipse(tx - tipR, ty - tipR, tipR * 2.f, tipR * 2.f);
    }

    if (active && (hovered || dragging))
        drawMiniChevrons(g, cx, cy, bounds.getWidth());

    if (hovered || dragging) {
        const juce::String tip = active ? juce::String(juce::roundToInt(value * 100.f)) + "%" : label;
        drawMiniKnobTooltip(g, bounds, tip, fontSize);
    }
}

inline void drawCellLed(juce::Graphics& g, juce::Rectangle<float> bounds, bool lit, juce::Colour litColour) {
    const float radius = bounds.getHeight() * 0.533f;
    g.setColour(juce::Colour(0xff8f8f8f));
    g.fillRoundedRectangle(bounds, radius);

    g.setColour(juce::Colours::black.withAlpha(0.25f));
    g.drawRoundedRectangle(bounds, radius, 0.5f);

    if (lit) {
        juce::ColourGradient grad(juce::Colours::white,
                                { bounds.getCentreX(), bounds.getY() + bounds.getHeight() * 0.35f },
                                litColour,
                                bounds.getCentre(),
                                true);
        g.setGradientFill(grad);
        g.setOpacity(1.f);
        g.fillRoundedRectangle(bounds, radius);

        for (int i = 2; i >= 1; --i) {
            g.setColour(litColour.withAlpha(0.35f / static_cast<float>(i)));
            g.drawRoundedRectangle(bounds.expanded(static_cast<float>(i)), radius, 1.f);
        }
    }
}

} // namespace matilda::knob
