#pragma once

#include "SequencerState.h"
#include <vector>

namespace matilda {

/** In-memory layer cell clipboard + undo for mini-grid right-click actions. */
class LayerClipboard {
public:
    enum class Mode : std::uint8_t { None, NotesOnly, NotesAndKnobs };

    struct Entry {
        Mode mode = Mode::None;
        int stepCount = 0; // visible steps copied (1…16)
        std::array<CellState, kGridSize * kGridSize> cells{};
    };

    struct UndoSnapshot {
        int layer = 0;
        bool active = false;
        int activeStepCount = kGridSize * kGridSize;
        std::array<std::array<CellState, kGridSize>, kGridSize> cells{};
    };

    [[nodiscard]] bool hasClipboard() const { return entry_.mode != Mode::None && entry_.stepCount > 0; }
    [[nodiscard]] bool hasNotesAndKnobs() const { return entry_.mode == Mode::NotesAndKnobs; }
    [[nodiscard]] bool canUndo() const { return !undoStack_.empty(); }
    [[nodiscard]] const Entry& entry() const { return entry_; }

    void clear() { entry_ = {}; }

    void copyFrom(const LayerState& layer, Mode mode) {
        entry_ = {};
        entry_.mode = mode;
        entry_.stepCount = clampActiveStepCount(layer.activeStepCount);
        for (int step = 0; step < entry_.stepCount; ++step) {
            const int y = step / kGridSize;
            const int x = step % kGridSize;
            entry_.cells[static_cast<size_t>(step)] =
                layer.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
        }
    }

    static void copyNoteFields(CellState& dst, const CellState& src) {
        dst.degree = src.degree;
        dst.gate = src.gate;
        dst.velocity = src.velocity;
        dst.octaveOffset = src.octaveOffset;
    }

    static void copyKnobFields(CellState& dst, const CellState& src) {
        dst.triggerArmed = src.triggerArmed;
        dst.triggerProb = src.triggerProb;
        dst.jitterArmed = src.jitterArmed;
        dst.jitterAmount = src.jitterAmount;
    }

    /** Default cell after RESET VALUES — gated on at lowest degree (not deactivated). */
    static CellState clearedCell() {
        CellState c;
        c.gate = true;
        c.degree = 0;
        c.velocity = 90;
        c.octaveOffset = 0;
        c.triggerArmed = false;
        c.triggerProb = 0.5f;
        c.jitterArmed = false;
        c.jitterAmount = 0.5f;
        return c;
    }

    static UndoSnapshot snapshot(int layerIndex, const LayerState& layer) {
        UndoSnapshot s;
        s.layer = layerIndex;
        s.active = layer.active;
        s.activeStepCount = clampActiveStepCount(layer.activeStepCount);
        s.cells = layer.cells;
        return s;
    }

    void pushUndo(UndoSnapshot snap) {
        undoStack_.push_back(std::move(snap));
        if (undoStack_.size() > 32)
            undoStack_.erase(undoStack_.begin());
    }

    /** Apply clipboard onto target. Returns false if clipboard empty / mode mismatch. */
    bool pasteOnto(LayerState& target, bool includeKnobs) {
        if (!hasClipboard())
            return false;
        if (includeKnobs && entry_.mode != Mode::NotesAndKnobs)
            return false;

        const int n = clampActiveStepCount(entry_.stepCount);
        for (int step = 0; step < n; ++step) {
            const int y = step / kGridSize;
            const int x = step % kGridSize;
            auto& dst = target.cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
            const auto& src = entry_.cells[static_cast<size_t>(step)];
            copyNoteFields(dst, src);
            if (includeKnobs)
                copyKnobFields(dst, src);
        }
        target.activeStepCount = n;
        target.active = true;
        return true;
    }

    void resetLayer(LayerState& target) {
        const auto cleared = clearedCell();
        for (int y = 0; y < kGridSize; ++y)
            for (int x = 0; x < kGridSize; ++x)
                target.cells[static_cast<size_t>(y)][static_cast<size_t>(x)] = cleared;
    }

    /** Restores last snapshot. Returns layer index, or -1 if none. */
    int undoOnto(PatchState& patch) {
        if (undoStack_.empty())
            return -1;
        const auto snap = undoStack_.back();
        undoStack_.pop_back();
        if (snap.layer < 0 || snap.layer >= kLayerCount)
            return -1;
        auto& layer = patch.layers[static_cast<size_t>(snap.layer)];
        layer.cells = snap.cells;
        layer.activeStepCount = clampActiveStepCount(snap.activeStepCount);
        if (snap.layer == 0)
            layer.active = true;
        else
            layer.active = snap.active;
        return snap.layer;
    }

private:
    Entry entry_{};
    std::vector<UndoSnapshot> undoStack_;
};

} // namespace matilda
