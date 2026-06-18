#pragma once

#include <JuceHeader.h>

namespace matilda {

inline constexpr int kChromatic[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
inline constexpr int kMajor[] = {0, 2, 4, 5, 7, 9, 11};
inline constexpr int kMinor[] = {0, 2, 3, 5, 7, 8, 10};
inline constexpr int kDorian[] = {0, 2, 3, 5, 7, 9, 10};
inline constexpr int kPhrygian[] = {0, 1, 3, 5, 7, 8, 10};
inline constexpr int kLydian[] = {0, 2, 4, 6, 7, 9, 11};
inline constexpr int kMixolydian[] = {0, 2, 4, 5, 7, 9, 10};
inline constexpr int kLocrian[] = {0, 1, 3, 5, 6, 8, 10};
inline constexpr int kHarmonicMinor[] = {0, 2, 3, 5, 7, 8, 11};
inline constexpr int kMelodicMinor[] = {0, 2, 3, 5, 7, 9, 11};
inline constexpr int kPentMajor[] = {0, 2, 4, 7, 9};
inline constexpr int kPentMinor[] = {0, 3, 5, 7, 10};
inline constexpr int kBlues[] = {0, 3, 5, 6, 7, 10};

struct ScaleMode {
    const char* label;
    const char* id;
    const int* intervals;
    int count;
};

inline constexpr ScaleMode kScaleModes[] = {
    {"Chromatic", "chromatic", kChromatic, 12},
    {"Major", "major", kMajor, 7},
    {"Minor", "minor", kMinor, 7},
    {"Dorian", "dorian", kDorian, 7},
    {"Phrygian", "phrygian", kPhrygian, 7},
    {"Lydian", "lydian", kLydian, 7},
    {"Mixolydian", "mixolydian", kMixolydian, 7},
    {"Locrian", "locrian", kLocrian, 7},
    {"Harmonic minor", "harmonic_minor", kHarmonicMinor, 7},
    {"Melodic minor", "melodic_minor", kMelodicMinor, 7},
    {"Pentatonic", "pentatonic", kPentMajor, 5},
    {"Pentatonic minor", "pentatonic_minor", kPentMinor, 5},
    {"Blues", "blues", kBlues, 6},
};

inline constexpr int kScaleModeCount = static_cast<int>(sizeof(kScaleModes) / sizeof(kScaleModes[0]));

inline const ScaleMode* findScaleMode(const juce::String& modeId) {
    const auto id = modeId.toLowerCase();
    for (int i = 0; i < kScaleModeCount; ++i) {
        if (id == kScaleModes[static_cast<size_t>(i)].id)
            return &kScaleModes[static_cast<size_t>(i)];
    }
    // Legacy aliases
    if (id == "ionian")
        return &kScaleModes[1];
    if (id == "aeolian")
        return &kScaleModes[2];
    return nullptr;
}

inline const int* scaleIntervalsForMode(const juce::String& mode, int& outCount) {
    if (const auto* sm = findScaleMode(mode)) {
        outCount = sm->count;
        return sm->intervals;
    }
    outCount = 7;
    return kMajor;
}

inline juce::String scaleLabelForMode(const juce::String& mode) {
    if (const auto* sm = findScaleMode(mode))
        return sm->label;
    const auto id = mode.toLowerCase();
    if (id == "ionian")
        return "Major";
    if (id == "aeolian")
        return "Minor";
    return mode;
}

inline int scaleModeIndex(const juce::String& mode) {
    const auto id = mode.toLowerCase();
    for (int i = 0; i < kScaleModeCount; ++i) {
        if (id == kScaleModes[static_cast<size_t>(i)].id)
            return i;
    }
    if (id == "ionian")
        return 1;
    if (id == "aeolian")
        return 2;
    return 1;
}

/** Tonic-relative note label for min/max window (Figma / React parity). */
inline juce::String formatTonicRelativeNote(const juce::String& root, int octaveIndex) {
    return root + juce::String(octaveIndex);
}

} // namespace matilda
