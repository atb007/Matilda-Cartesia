#pragma once

#include <JuceHeader.h>

#include <array>
#include <cstdint>
#include <vector>

namespace matilda {

inline constexpr int kGridSize = 4;
inline constexpr int kLayerCount = 4;

enum class MovementMode : std::uint8_t {
    Forward,
    Reverse,
    PingPong,
    Pendulum,
    Random,
    RandomSkip
};

enum class PlayMode : std::uint8_t { Transport, Note };

struct CellState {
    int degree = 0;
    bool gate = true;
    int velocity = 90;
    int octaveOffset = 0;
    bool triggerArmed = false;
    float triggerProb = 0.5f;
    bool jitterArmed = false;
    float jitterAmount = 0.5f;
};

struct LayerState {
    bool active = false;
    MovementMode movement = MovementMode::Forward;
    float randomSkipProb = 0.0f;
    int stepIndex = 0;
    int stepDir = 1;
    int stepsOnLayer = 0;
    int pathHold = 0;
    std::vector<int> randomBag;
    int randomBagPos = 0;
    std::array<std::array<CellState, kGridSize>, kGridSize> cells{};
};

struct PatchState {
    juce::String root = "C";
    juce::String mode = "lydian";
    bool quantize = true;
    int minOctave = 1;
    int maxOctave = 9;
    double masterDivision = 1.0 / 16.0;
    int selectedLayer = 0;
    PlayMode playMode = PlayMode::Transport;
    std::array<LayerState, kLayerCount> layers{};
};

} // namespace matilda
