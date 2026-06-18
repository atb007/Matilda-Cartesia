#pragma once

#include <JuceHeader.h>

namespace matilda::scale {

/** Spark / glow colours per scale — mirrors `scaleConfig.ts` SCALE_GEM_COLORS. */
struct GemPalette {
    juce::Colour core;
    juce::Colour glow;
    juce::Colour hot;
};

inline GemPalette gemPaletteForModeId(const juce::String& modeId) {
    const auto id = modeId.toLowerCase();
    if (id == "major")
        return {juce::Colour(0xffffc860), juce::Colour(0xffe87820), juce::Colour(0xfffff4d8)};
    if (id == "minor")
        return {juce::Colour(0xff88b8ff), juce::Colour(0xff3858c8), juce::Colour(0xffd8e8ff)};
    if (id == "dorian")
        return {juce::Colour(0xff78d8c8), juce::Colour(0xff289878), juce::Colour(0xffd8fff0)};
    if (id == "phrygian")
        return {juce::Colour(0xffff8868), juce::Colour(0xffc83828), juce::Colour(0xffffe0d8)};
    if (id == "lydian")
        return {juce::Colour(0xffc8a0ff), juce::Colour(0xff7040d8), juce::Colour(0xfff0e0ff)};
    if (id == "mixolydian")
        return {juce::Colour(0xffffb870), juce::Colour(0xffd87820), juce::Colour(0xfffff0d8)};
    if (id == "locrian")
        return {juce::Colour(0xffa898b8), juce::Colour(0xff584868), juce::Colour(0xffe8e0f0)};
    if (id == "harmonic_minor")
        return {juce::Colour(0xff9090ff), juce::Colour(0xff4040c0), juce::Colour(0xffe0e0ff)};
    if (id == "melodic_minor")
        return {juce::Colour(0xff80a0ff), juce::Colour(0xff3060d0), juce::Colour(0xffd8e8ff)};
    if (id == "pentatonic")
        return {juce::Colour(0xff70e8b0), juce::Colour(0xff28a868), juce::Colour(0xffd8ffe8)};
    if (id == "pentatonic_minor")
        return {juce::Colour(0xff68c890), juce::Colour(0xff208858), juce::Colour(0xffd0ffe0)};
    if (id == "blues")
        return {juce::Colour(0xff6898d0), juce::Colour(0xff2858a0), juce::Colour(0xffd8e8ff)};
    return {juce::Colour(0xff7ec8ff), juce::Colour(0xff5a48e8), juce::Colour(0xffe8f4ff)};
}

} // namespace matilda::scale
