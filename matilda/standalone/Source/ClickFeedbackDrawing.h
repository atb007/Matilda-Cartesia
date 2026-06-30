#pragma once

#include <JuceHeader.h>

namespace matilda::ui {

/** Brief scale-down while pressed — for circular icon buttons. */
inline void paintWithPressScale(juce::Graphics& g,
                              juce::Rectangle<float> bounds,
                              bool pressed,
                              float scale = 0.92f) {
    if (pressed) {
        const auto cx = bounds.getCentreX();
        const auto cy = bounds.getCentreY();
        g.addTransform(juce::AffineTransform::scale(scale, scale, cx, cy));
    }
}

} // namespace matilda::ui
