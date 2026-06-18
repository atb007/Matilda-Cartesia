#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerEngine.h"
#include "../MatildaLookAndFeel.h"
#include "GemCell.h"

/** Centers a GemCell (120×130 design px) inside a Figma grid slot. */
class GemCellSlot : public juce::Component {
public:
    explicit GemCellSlot(GemCell& cell);

    void resized() override;

private:
    GemCell& cell_;
    static constexpr float kDesignW = 120.f;
    static constexpr float kDesignH = 130.f;
};

class GemGrid : public juce::Component {
public:
    GemGrid(matilda::SequencerEngine& engine, MatildaLookAndFeel& laf);

    void setLayer(int layer);
    void refresh();
    void setPlayhead(int step, int playingLayer, bool noteFired);
    void setGridMetrics(float cellW, float cellH, float colGap, float rowGap, float scale = 1.f);

    GemCell& cellAt(int x, int y);
    /** M1 dev — show only the top-left cell, filling the grid bounds. */
    void setSingleCellDevPreview(bool enabled);

    std::function<void()> onCellChanged;

private:
    matilda::SequencerEngine& engine_;
    MatildaLookAndFeel& laf_;
    int layer_ = 0;
    float cellW_ = 0.f;
    float cellH_ = 0.f;
    float colGap_ = 0.f;
    float rowGap_ = 0.f;
    float layoutScale_ = 1.f;
    bool singleCellDev_ = false;
    std::array<std::array<std::unique_ptr<GemCell>, matilda::kGridSize>, matilda::kGridSize> cells_{};
    std::array<std::array<std::unique_ptr<GemCellSlot>, matilda::kGridSize>, matilda::kGridSize> slots_{};

    [[nodiscard]] juce::Rectangle<int> slotBounds(int x, int y) const;
    void resized() override;
    void paint(juce::Graphics& g) override;
};