#include "GridComponent.h"

GridComponent::GridComponent(gridwalker::GridEngine& engine, GridWalkerLookAndFeel& laf)
    : engine_(engine), laf_(laf) {
    startTimerHz(30);
}

void GridComponent::refresh() {
    repaint();
}

void GridComponent::timerCallback() {
    if (engine_.playheadX() != lastX_ || engine_.playheadY() != lastY_) {
        lastX_ = engine_.playheadX();
        lastY_ = engine_.playheadY();
        repaint();
    }
}

juce::Rectangle<int> GridComponent::cellBounds(int x, int y) const {
    const int pad = 8;
    const int size = (juce::jmin(getWidth(), getHeight()) - pad * 2) / gridwalker::GridEngine::kGridSize;
    const int gridW = size * gridwalker::GridEngine::kGridSize;
    const int ox = (getWidth() - gridW) / 2;
    const int oy = (getHeight() - gridW) / 2;
    return {ox + x * size + 2, oy + y * size + 2, size - 4, size - 4};
}

std::optional<std::pair<int, int>> GridComponent::cellAt(juce::Point<int> p) const {
    for (int y = 0; y < gridwalker::GridEngine::kGridSize; ++y)
        for (int x = 0; x < gridwalker::GridEngine::kGridSize; ++x)
            if (cellBounds(x, y).contains(p))
                return std::make_pair(x, y);
    return std::nullopt;
}

juce::String GridComponent::noteLabel(int midiNote) const {
    static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const int n = juce::jlimit(0, 127, midiNote);
    return juce::String(names[n % 12]) + juce::String(n / 12 - 1);
}

void GridComponent::paintCellPitch(juce::Graphics& g, juce::Rectangle<float> bounds, int layer, int x, int y) const {
    const auto& cell = engine_.cell(layer, x, y);
    if (!cell.gate)
        return;

    const int midi = engine_.resolveMidi(layer, x, y);
    auto degreeArea = bounds.removeFromBottom(14.f);

    g.setColour(laf_.textPrimary);
    g.setFont(13.f);
    g.drawText(noteLabel(midi), bounds, juce::Justification::centred);

    g.setColour(laf_.textMuted);
    g.setFont(10.f);
    g.drawText("#" + juce::String(cell.degree + 1), degreeArea, juce::Justification::centred);
}

void GridComponent::paint(juce::Graphics& g) {
    g.fillAll(laf_.backgroundDark.darker(0.2f));

    const int px = engine_.playheadX();
    const int py = engine_.playheadY();
    const int layer = engine_.activeLayer;

    for (int y = 0; y < gridwalker::GridEngine::kGridSize; ++y) {
        for (int x = 0; x < gridwalker::GridEngine::kGridSize; ++x) {
            const auto bounds = cellBounds(x, y).toFloat();
            const auto& cell = engine_.cell(layer, x, y);
            const bool isPlayhead = x == px && y == py;

            if (cell.gate)
                g.setColour(laf_.gateOnColour.withAlpha(0.35f));
            else
                g.setColour(laf_.panelColour);
            g.fillRoundedRectangle(bounds, 8.f);

            if (isPlayhead) {
                g.setColour(laf_.playheadColour.withAlpha(0.25f));
                g.fillRoundedRectangle(bounds.expanded(3.f), 10.f);
                g.setColour(laf_.playheadColour);
                g.drawRoundedRectangle(bounds.expanded(3.f), 10.f, 2.f);
            } else {
                g.setColour(laf_.textMuted.withAlpha(0.3f));
                g.drawRoundedRectangle(bounds, 8.f, 1.f);
            }

            paintCellPitch(g, bounds, layer, x, y);
        }
    }
}

void GridComponent::nudgeAt(juce::Point<int> pos, int delta) {
    if (auto cell = cellAt(pos)) {
        engine_.nudgeCellDegree(engine_.activeLayer, cell->first, cell->second, delta);
        repaint();
    }
}

void GridComponent::mouseDown(const juce::MouseEvent& e) {
    if (auto cell = cellAt(e.getPosition())) {
        auto& c = engine_.cell(engine_.activeLayer, cell->first, cell->second);
        if (e.mods.isPopupMenu()) {
            engine_.nudgeCellDegree(engine_.activeLayer, cell->first, cell->second, 1);
        } else {
            c.gate = !c.gate;
            if (onCellToggled)
                onCellToggled(cell->first, cell->second);
        }
        repaint();
    }
}

void GridComponent::mouseDoubleClick(const juce::MouseEvent& e) {
    nudgeAt(e.getPosition(), 1);
}

void GridComponent::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (std::abs(wheel.deltaY) < 0.01f)
        return;
    nudgeAt(e.getPosition(), wheel.deltaY > 0.f ? -1 : 1);
}
