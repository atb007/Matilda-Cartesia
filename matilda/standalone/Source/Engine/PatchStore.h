#pragma once

#include "SequencerState.h"

namespace matilda {

/** Patch v2 JSON — matches matilda presets and cartesia/model.py. */
class PatchStore {
public:
    static juce::String patchToJson(const PatchState& patch);
    static bool patchFromJson(const juce::String& json, PatchState& out);
    static bool loadDefaultPreset(PatchState& out);
    static bool loadFromFile(const juce::File& file, PatchState& out);

    static juce::String movementToString(MovementMode mode);
    static MovementMode movementFromString(const juce::String& s);
};

} // namespace matilda
