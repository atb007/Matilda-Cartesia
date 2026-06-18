#pragma once

#include <JuceHeader.h>

class GridWalkerLookAndFeel : public juce::LookAndFeel_V4 {
public:
    GridWalkerLookAndFeel();

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    juce::Colour backgroundDark;
    juce::Colour panelColour;
    juce::Colour accentColour;
    juce::Colour gateOnColour;
    juce::Colour playheadColour;
    juce::Colour textPrimary;
    juce::Colour textMuted;
};
