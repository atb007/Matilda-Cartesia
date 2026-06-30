#pragma once

#include <JuceHeader.h>

namespace matilda::ui::shell {

/** Figma 5064:109807 / shellGlassStyle.ts — radial highlight + vertical fade. */
inline void paintGlassBedding(juce::Graphics& g, juce::Rectangle<float> rect) {
    juce::ColourGradient linear(juce::Colour(0xff1b1b1b), rect.getX(), rect.getY(),
                                juce::Colours::transparentBlack, rect.getX(), rect.getBottom(), false);
    linear.addColour(0.37943f, juce::Colour(0xff1b1b1b));
    linear.addColour(0.63166f, juce::Colour::fromFloatRGBA(50.f / 255.f, 50.f / 255.f, 50.f / 255.f, 0.744f));
    linear.addColour(1.f, juce::Colours::transparentBlack);
    g.setGradientFill(linear);
    g.fillRect(rect);
}

void paintGlassBeddingRadial(juce::Graphics& g, juce::Rectangle<float> rect);

} // namespace matilda::ui::shell
