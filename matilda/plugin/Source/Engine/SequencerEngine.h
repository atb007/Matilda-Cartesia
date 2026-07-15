#pragma once

#include "SequencerState.h"

namespace matilda {

struct LayerTickResult {
    int layer = 0;
    int stepIndex = -1;
    bool fired = false;
};

class SequencerEngine {
public:
    static constexpr int kMaxStepsPerLayer = kGridSize * kGridSize;

    explicit SequencerEngine(PatchState& patch);

    void reset();
    void tick();
    /** Clamp step index / random bag after activeStepCount changes. */
    void onActiveStepCountChanged(int layer);

    [[nodiscard]] int playheadX() const;
    [[nodiscard]] int playheadY() const;
    [[nodiscard]] int playingLayer() const;
    [[nodiscard]] int masterTick() const { return masterTick_; }

    [[nodiscard]] CellState& cell(int layer, int x, int y);
    [[nodiscard]] const CellState& cell(int layer, int x, int y) const;

    [[nodiscard]] juce::String noteLabel(int layer, int x, int y) const;
    void bumpCellDegree(CellState& cell, int delta);
    /** Index of the cell's resolved pitch in the active min…max quantised set. */
    [[nodiscard]] int quantisedNoteIndex(const CellState& cell) const;
    /** Set cell pitch from an index into quantisedMidisInWindow (clamped). */
    void setQuantisedNoteIndex(CellState& cell, int index) const;
    [[nodiscard]] int quantisedNoteCount() const;
    [[nodiscard]] float knobVisualPosition(const CellState& cell) const;
    void requantizeAllCells();
    [[nodiscard]] bool lastStepFired() const { return lastFired_; }
    [[nodiscard]] int lastStepIndex() const { return lastStepIndex_; }
    [[nodiscard]] int currentStepIndex() const;
    [[nodiscard]] int lastPlayingLayer() const { return lastPlayingLayer_; }
    [[nodiscard]] int layerActiveStepCount(int layer) const;
    [[nodiscard]] int layerStepIndex(int layer) const;
    [[nodiscard]] const std::vector<LayerTickResult>& lastTickResults() const { return lastTickResults_; }
    [[nodiscard]] int resolveMidiNote(const CellState& cell) const;
    /** Scale pitch + jitter for a fired step (uses engine RNG). */
    [[nodiscard]] int resolveFiredMidiNote(const CellState& cell);

private:
    PatchState& patch_;
    int masterTick_ = 0;
    int playingLayerIdx_ = 0;
    int stepsOnLayer_ = 0;
    bool lastFired_ = false;
    int lastStepIndex_ = -1;
    int lastPlayingLayer_ = 0;
    std::vector<LayerTickResult> lastTickResults_;
    mutable juce::Random rng_{};

    [[nodiscard]] int findNextActiveLayer(int fromLayer) const;
    void reconcilePlayingLayer();
    void advancePath(LayerState& layer);
    void maybeSwitchLayer();
    LayerTickResult tickLayer(int layerIdx);
    [[nodiscard]] int computeCandidateMidi(const CellState& cell) const;
    [[nodiscard]] bool rollTrigger(const CellState& cell) const;
    [[nodiscard]] static std::pair<int, int> indexToXY(int index);
    [[nodiscard]] std::vector<int> quantisedMidisInWindow() const;
    void assignCellFromMidi(CellState& cell, int midi) const;
};

} // namespace matilda
