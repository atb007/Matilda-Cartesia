#pragma once

#include <JuceHeader.h>

// Renders the VCV Cartesia dark panel SVG as the full editor background,
// then overlays the "Matilda / Cartesia v1.0" title text.
class MatildaBackground : public juce::Component {
public:
    MatildaBackground();
    void paint(juce::Graphics& g) override;

private:
    std::unique_ptr<juce::Drawable> panel_;
};
