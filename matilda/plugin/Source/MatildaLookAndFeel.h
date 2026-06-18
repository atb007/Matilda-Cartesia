#pragma once

#include <JuceHeader.h>

class MatildaLookAndFeel : public juce::LookAndFeel_V4 {
public:
    MatildaLookAndFeel();

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getTextButtonFont(juce::TextButton&, int) override;

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;

    juce::Colour textPrimary;
    juce::Colour textMuted;
    juce::Colour pillFill;
    juce::Colour pillBorder;
    juce::Colour accentOrange;
    juce::Colour accentGreen;
    juce::Colour playheadColour;

    // Per-layer LED colours – matching Cartesia's multi-slice visual identity
    static juce::Colour layerColour(int z) {
        switch (z & 3) {
            case 0: return juce::Colour(0xff00e060);  // Cartesia green
            case 1: return juce::Colour(0xff00c8e0);  // cyan
            case 2: return juce::Colour(0xffe87a3a);  // amber (Matilda accent)
            case 3: return juce::Colour(0xffaa55ee);  // purple
            default: return juce::Colour(0xff00e060);
        }
    }
};
