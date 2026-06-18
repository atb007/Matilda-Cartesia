#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace gridwalker {

struct Cell {
    int degree = 0;
    bool gate = true;
    int velocity = 100;
};

struct AxisState {
    int pos = 0;
    int direction = 1;
    float counter = 0.f;
    int division = 1;
    bool reverseForced = false;
};

struct StepResult {
    std::optional<int> midiNote;
    int velocity = 100;
    int x = 0;
    int y = 0;
    int z = 0;
    bool fired = false;
};

class GridEngine {
public:
    static constexpr int kGridSize = 4;
    static constexpr int kLayerCount = 4;

    GridEngine();

    void reset();
    StepResult advanceTick();

    [[nodiscard]] int playheadX() const { return x_.pos; }
    [[nodiscard]] int playheadY() const { return y_.pos; }
    [[nodiscard]] int playheadZ() const { return z_.pos; }
    [[nodiscard]] int masterTick() const { return masterTick_; }

    Cell& cell(int layer, int x, int y);
    [[nodiscard]] const Cell& cell(int layer, int x, int y) const;

    void nudgeCellDegree(int layer, int x, int y, int delta);
    [[nodiscard]] int resolveMidi(int layer, int x, int y) const;
    [[nodiscard]] int scaleSpan() const;

    void setRootMidi(int note) { rootMidi_ = note; }
    [[nodiscard]] int rootMidi() const { return rootMidi_; }
    void setRootActive(bool active) { rootActive_ = active; }
    [[nodiscard]] bool rootActive() const { return rootActive_; }

    int masterDivision = 16;
    bool quantize = true;
    int scaleIndex = 0;
    int minDegree = 0;
    int rangeSemitones = 12;
    bool zMovementEnabled = false;
    int activeLayer = 0;

    AxisState x_;
    AxisState y_;
    AxisState z_;

private:
    int mapDegree(int raw) const;
    bool axisDue(AxisState& axis);
    void advanceAxis(AxisState& axis);
    void movePlayhead();

    std::array<std::array<std::array<Cell, kGridSize>, kGridSize>, kLayerCount> layers_{};
    int masterTick_ = 0;
    int rootMidi_ = 60;
    bool rootActive_ = false;
    std::optional<int> lastDegree_;
};

} // namespace gridwalker
