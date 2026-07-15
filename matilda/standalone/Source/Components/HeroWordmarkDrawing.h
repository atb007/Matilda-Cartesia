#pragma once

#include <JuceHeader.h>
#include "../MatildaFonts.h"
#include "../ReactShellLayout.h"

namespace matilda::ui {

inline juce::String cartesiaVersionLabel() {
#if defined(JucePlugin_VersionString)
    return juce::String("Cartesia - v") + JucePlugin_VersionString;
#else
    return "Cartesia - v1.0.11";
#endif
}

/** Paint Matilda / Cartesia labels into a label-sized component (origin at top-left). */
inline void paintHeroWordmark(juce::Graphics& g, float componentWidth) {
    using namespace matilda::react;

    if (componentWidth <= 0.f)
        return;

    const float s = componentWidth / kHeroLabelW;
    const float topPad = kHeroLabelTopPad * s;
    const int labelW = juce::roundToInt(componentWidth);
    const int titleTop = juce::roundToInt(topPad);
    const int titleH = juce::roundToInt(kHeroTitleLineH * s);

    g.setColour(juce::Colours::white);
    g.setFont(matilda::fonts::jacquard24(kHeroTitleFs * s));
    g.drawText("Matilda", 0, titleTop, labelW, titleH, juce::Justification::right);

    const int subTop = juce::roundToInt(topPad + (kHeroTitleLineH + kHeroSubtitleGap) * s);
    const int subH = juce::roundToInt(kHeroSubtitleFs * s);

    g.setColour(juce::Colour(0xffdf90e5));
    g.setFont(matilda::fonts::jacquard24(kHeroSubtitleFs * s));
    g.drawText(cartesiaVersionLabel(), 0, subTop, labelW, subH, juce::Justification::right);
}

} // namespace matilda::ui
