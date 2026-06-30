#pragma once

#include <JuceHeader.h>

/** Starfield + masked portrait + wordmark — React HeroCanvas.tsx parity. */
class HeroCanvas : public juce::Component {
public:
    HeroCanvas() { setInterceptsMouseClicks(false, false); }
    void paint(juce::Graphics& g) override;
};
