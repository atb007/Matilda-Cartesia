#include "GemGrid.h"

GemCellSlot::GemCellSlot(GemCell& cell) : cell_(cell) {
    setOpaque(false);
    addAndMakeVisible(cell_);
}

void GemCellSlot::resized() {
    const auto b = getLocalBounds();
    if (b.isEmpty())
        return;

    const float sx = static_cast<float>(b.getWidth()) / kDesignW;
    const float sy = static_cast<float>(b.getHeight()) / kDesignH;
    const int w = juce::roundToInt(kDesignW * sx);
    const int h = juce::roundToInt(kDesignH * sy);
    cell_.setBounds((b.getWidth() - w) / 2, (b.getHeight() - h) / 2, w, h);
}

GemGrid::GemGrid(matilda::SequencerEngine& engine, MatildaLookAndFeel& laf)
    : engine_(engine), laf_(laf) {
    setOpaque(false);
    for (int y = 0; y < matilda::kGridSize; ++y)
        for (int x = 0; x < matilda::kGridSize; ++x) {
            auto c = std::make_unique<GemCell>(laf_, engine_);
            c->onChanged = [this](int, int) {
                repaint();
                if (onCellChanged)
                    onCellChanged();
            };
            auto slot = std::make_unique<GemCellSlot>(*c);
            addAndMakeVisible(*slot);
            slots_[static_cast<size_t>(y)][static_cast<size_t>(x)] = std::move(slot);
            cells_[static_cast<size_t>(y)][static_cast<size_t>(x)] = std::move(c);
        }
}

GemCell& GemGrid::cellAt(int x, int y) {
    return *cells_[static_cast<size_t>(juce::jlimit(0, matilda::kGridSize - 1, y))]
                [static_cast<size_t>(juce::jlimit(0, matilda::kGridSize - 1, x))];
}

void GemGrid::setLayer(int layer) {
    layer_ = juce::jlimit(0, matilda::kLayerCount - 1, layer);
    const juce::Colour col = MatildaLookAndFeel::layerColour(layer_);
    for (auto& row : cells_)
        for (auto& c : row)
            c->setLayerColour(col);
    refresh();
}

void GemGrid::refresh() {
    for (int y = 0; y < matilda::kGridSize; ++y)
        for (int x = 0; x < matilda::kGridSize; ++x)
            cells_[static_cast<size_t>(y)][static_cast<size_t>(x)]->bind(
                &engine_.cell(layer_, x, y),
                layer_, x, y);
    resized();
    repaint();
}

void GemGrid::setPlayhead(int step, int playingLayer, bool noteFired) {
    const bool onLayer = playingLayer == layer_ && step >= 0;
    const int px = onLayer ? step % matilda::kGridSize : -1;
    const int py = onLayer ? step / matilda::kGridSize : -1;

    for (int y = 0; y < matilda::kGridSize; ++y)
        for (int x = 0; x < matilda::kGridSize; ++x) {
            auto& c = *cells_[static_cast<size_t>(y)][static_cast<size_t>(x)];
            c.setOnActiveLayer(onLayer);
            c.setPlayhead(x == px && y == py);
            c.setStepFired(noteFired && x == px && y == py);
        }
}

void GemGrid::setSingleCellDevPreview(bool enabled) {
    singleCellDev_ = enabled;
    resized();
}

void GemGrid::setGridMetrics(float cellW, float cellH, float colGap, float rowGap, float scale) {
    cellW_ = cellW;
    cellH_ = cellH;
    colGap_ = colGap;
    rowGap_ = rowGap;
    layoutScale_ = scale;
    resized();
}

juce::Rectangle<int> GemGrid::slotBounds(int x, int y) const {
    if (cellW_ > 0.f && cellH_ > 0.f) {
        const int cw = juce::roundToInt(cellW_ * layoutScale_);
        const int ch = juce::roundToInt(cellH_ * layoutScale_);
        const int cg = juce::roundToInt(colGap_ * layoutScale_);
        const int rg = juce::roundToInt(rowGap_ * layoutScale_);
        return {x * (cw + cg), y * (ch + rg), cw, ch};
    }

    const int pad = 6;
    const int cw = (getWidth() - pad * 2) / matilda::kGridSize;
    const int ch = (getHeight() - pad * 2) / matilda::kGridSize;
    const int gridW = cw * matilda::kGridSize;
    const int gridH = ch * matilda::kGridSize;
    const int ox = (getWidth() - gridW) / 2;
    const int oy = (getHeight() - gridH) / 2;
    return {ox + x * cw, oy + y * ch, cw, ch};
}

void GemGrid::resized() {
    if (singleCellDev_) {
        for (int y = 0; y < matilda::kGridSize; ++y)
            for (int x = 0; x < matilda::kGridSize; ++x) {
                auto& slot = *slots_[static_cast<size_t>(y)][static_cast<size_t>(x)];
                const bool show = x == 0 && y == 0;
                slot.setVisible(show);
                if (show)
                    slot.setBounds(getLocalBounds());
            }
        return;
    }

    for (int y = 0; y < matilda::kGridSize; ++y)
        for (int x = 0; x < matilda::kGridSize; ++x)
            slots_[static_cast<size_t>(y)][static_cast<size_t>(x)]->setVisible(true);

    for (int y = 0; y < matilda::kGridSize; ++y)
        for (int x = 0; x < matilda::kGridSize; ++x)
            slots_[static_cast<size_t>(y)][static_cast<size_t>(x)]->setBounds(slotBounds(x, y));
}

void GemGrid::paint(juce::Graphics& g) { juce::ignoreUnused(g); }
