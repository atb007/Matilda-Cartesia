#pragma once

#include <JuceHeader.h>
#include "SequencerState.h"

namespace matilda {

/** User-folder preset library — seed Init on first launch; unlimited saves. */
class PresetLibrary {
public:
    static constexpr const char* kInitName = "Init";
    static constexpr int kMaxVisibleDropdownItems = 10;

    [[nodiscard]] static juce::File presetsDirectory();
    /** Create folder + seed Init.json from default patch if missing. */
    static void ensureSeeded();

    [[nodiscard]] static juce::StringArray listPresetNames();
    [[nodiscard]] static juce::File fileForName(const juce::String& name);
    [[nodiscard]] static bool loadNamed(const juce::String& name, PatchState& out);
    [[nodiscard]] static bool saveNamed(const juce::String& name, const PatchState& patch);
    [[nodiscard]] static bool saveToFile(const juce::File& file, const PatchState& patch);

    [[nodiscard]] static juce::String sanitizeName(const juce::String& name);
    [[nodiscard]] static juce::String displayNameFromFile(const juce::File& file);
};

} // namespace matilda
