#include "GridEngine.h"

#include <algorithm>

namespace gridwalker {

namespace {
constexpr int kMajor[] = {0, 2, 4, 5, 7, 9, 11};
constexpr int kMinor[] = {0, 2, 3, 5, 7, 8, 10};
constexpr int kDorian[] = {0, 2, 3, 5, 7, 9, 10};
constexpr int kPentMajor[] = {0, 2, 4, 7, 9};
constexpr int kChromatic[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

const int* scaleForIndex(int index, int& length) {
    switch (index) {
        case 1: length = 7; return kMinor;
        case 2: length = 7; return kDorian;
        case 3: length = 5; return kPentMajor;
        case 4: length = 12; return kChromatic;
        default: length = 7; return kMajor;
    }
}
} // namespace

GridEngine::GridEngine() {
    x_.division = 1;
    y_.division = 4;
    z_.division = 16;
    reset();
}

void GridEngine::reset() {
    x_ = {};
    y_ = {};
    z_ = {};
    x_.division = 1;
    y_.division = 4;
    z_.division = 16;
    masterTick_ = 0;
    lastDegree_.reset();
}

Cell& GridEngine::cell(int layer, int x, int y) {
    return layers_.at(static_cast<size_t>(layer)).at(static_cast<size_t>(y)).at(static_cast<size_t>(x));
}

const Cell& GridEngine::cell(int layer, int x, int y) const {
    return layers_.at(static_cast<size_t>(layer)).at(static_cast<size_t>(y)).at(static_cast<size_t>(x));
}

int GridEngine::mapDegree(int raw) const {
    if (quantize) {
        int length = 7;
        const int* scale = scaleForIndex(scaleIndex, length);
        const int idx = ((raw % length) + length) % length;
        return scale[idx];
    }
    const int span = std::max(1, rangeSemitones);
    return minDegree + ((raw % span) + span) % span;
}

int GridEngine::scaleSpan() const {
    if (quantize) {
        int length = 7;
        scaleForIndex(scaleIndex, length);
        return length;
    }
    return std::max(1, rangeSemitones);
}

void GridEngine::nudgeCellDegree(int layer, int x, int y, int delta) {
    auto& c = cell(layer, x, y);
    const int span = scaleSpan();
    c.degree = ((c.degree + delta) % span + span) % span;
}

int GridEngine::resolveMidi(int layer, int x, int y) const {
    const auto& c = cell(layer, x, y);
    return std::clamp(rootMidi_ + mapDegree(c.degree), 0, 127);
}

bool GridEngine::axisDue(AxisState& axis) {
    axis.counter += 1.f;
    if (axis.counter >= static_cast<float>(std::max(1, axis.division))) {
        axis.counter -= static_cast<float>(std::max(1, axis.division));
        return true;
    }
    return false;
}

void GridEngine::advanceAxis(AxisState& axis) {
    const int direction = axis.reverseForced ? -axis.direction : axis.direction;
    int next = axis.pos + direction;
    if (next < 0)
        next = 3;
    else if (next > 3)
        next = 0;
    axis.pos = next;
}

void GridEngine::movePlayhead() {
    if (axisDue(x_))
        advanceAxis(x_);
    if (axisDue(y_))
        advanceAxis(y_);
    if (zMovementEnabled && axisDue(z_))
        advanceAxis(z_);
}

StepResult GridEngine::advanceTick() {
    ++masterTick_;
    movePlayhead();

    StepResult result;
    result.x = x_.pos;
    result.y = y_.pos;
    result.z = zMovementEnabled ? z_.pos : activeLayer;

    const auto& c = cell(activeLayer, x_.pos, y_.pos);
    if (!c.gate || !rootActive_) {
        result.fired = false;
        return result;
    }

    const int degree = mapDegree(c.degree);
    lastDegree_ = degree;
    result.midiNote = resolveMidi(activeLayer, x_.pos, y_.pos);
    result.velocity = c.velocity;
    result.fired = true;
    return result;
}

} // namespace gridwalker
