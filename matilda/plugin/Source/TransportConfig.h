#pragma once

#include <JuceHeader.h>
#include "Engine/ClockDivisions.h"
#include "Engine/SequencerState.h"

namespace matilda::transport {

inline constexpr int kDdMaxVisibleClockItems = 6;

inline juce::String playModeLabel(PlayMode mode) {
    return mode == PlayMode::Transport ? "Transport" : "Note";
}

inline int clockIndexForDivision(double division) {
    return matilda::indexForMasterDivision(division);
}

inline double clockDivisionAtIndex(int index) {
    return matilda::kClockDivisions[static_cast<size_t>(juce::jlimit(0, matilda::kClockDivisionCount - 1, index))]
        .masterDivision;
}

inline juce::String clockLabelAtIndex(int index) {
    return matilda::kClockDivisions[static_cast<size_t>(juce::jlimit(0, matilda::kClockDivisionCount - 1, index))]
        .label;
}

} // namespace matilda::transport
