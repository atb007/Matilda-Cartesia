#pragma once

#include <JuceHeader.h>

namespace matilda::ui::shell {

/** Figma 5064:109807 / shellGlassStyle.ts — frosted translucent fill over blurred wallpaper. */
inline void paintGlassBedding(juce::Graphics& g, juce::Rectangle<float> rect) {
    // Keep the Figma vertical fade shape, but translucent so the blurred wallpaper reads through.
    juce::ColourGradient linear(juce::Colour(0xb81b1b1b), rect.getX(), rect.getY(),
                                juce::Colours::transparentBlack, rect.getX(), rect.getBottom(), false);
    linear.addColour(0.37943f, juce::Colour(0xb01b1b1b));
    linear.addColour(0.63166f, juce::Colour::fromFloatRGBA(50.f / 255.f, 50.f / 255.f, 50.f / 255.f, 0.52f));
    linear.addColour(1.f, juce::Colours::transparentBlack);
    g.setGradientFill(linear);
    g.fillRect(rect);

    // Extra frost veil — cools/desaturates the blur without fully hiding it.
    juce::ColourGradient frost(juce::Colour(0x40242830), rect.getTopLeft(),
                               juce::Colour(0x18101418), rect.getBottomRight(), false);
    frost.addColour(0.45f, juce::Colour(0x28303840));
    g.setGradientFill(frost);
    g.fillRect(rect);
}

void paintGlassBeddingRadial(juce::Graphics& g, juce::Rectangle<float> rect);

} // namespace matilda::ui::shell
