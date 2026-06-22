#include "GemCell.h"
#include "../CartesiaCellLighting.h"
#include "../KnobDrawing.h"
#include <cmath>

GemCell::GemCell(MatildaLookAndFeel& laf, matilda::SequencerEngine& engine)
    : laf_(laf), engine_(engine) {
    setOpaque(false);
    setPaintingIsUnclipped(true);
    setRepaintsOnMouseActivity(true);
}

void GemCell::bind(matilda::CellState* cell, int layer, int x, int y) {
    cell_ = cell;
    layer_ = layer;
    cellX_ = x;
    cellY_ = y;
    repaint();
}

void GemCell::setPlayhead(bool v)            { if (playhead_ != v)      { playhead_ = v;      repaint(); } }
void GemCell::setStepFired(bool v)           { if (stepFired_ != v)     { stepFired_ = v;     repaint(); } }
void GemCell::setOnActiveLayer(bool v)       { if (onActiveLayer_ != v) { onActiveLayer_ = v; repaint(); } }
void GemCell::setLayerColour(juce::Colour c) { if (layerColour_ != c)   { layerColour_ = c;   repaint(); } }
void GemCell::setMiniMode(bool v)            { if (mini_ != v)          { mini_ = v;           repaint(); } }

juce::Rectangle<float> GemCell::knobBounds() const {
    const auto b = getLocalBounds().toFloat();
    const float kw = b.getWidth() * (74.f / 120.f);
    const float kh = b.getHeight() * (74.f / 130.f);
    const float kx = b.getWidth() * (28.f / 120.f);
    const float ky = b.getHeight() * (28.f / 130.f);
    return { kx, ky, kw, kh };
}

juce::Rectangle<float> GemCell::ledBounds() const {
    const auto b = getLocalBounds().toFloat();
    const float w = b.getWidth() * (18.f / 120.f);
    const float h = b.getHeight() * (10.f / 130.f);
    const float x = b.getRight() - b.getWidth() * (6.f / 120.f) - w;
    const float y = b.getBottom() - b.getHeight() * (20.f / 130.f) - h;
    return { x, y, w, h };
}

juce::Rectangle<float> GemCell::triggerIconBounds() const {
    const auto k = knobBounds();
    const float s = getLocalBounds().getWidth() * (24.f / 120.f);
    const float gap = getLocalBounds().getWidth() * (10.f / 120.f);
    return { 0.f, k.getCentreY() - s - gap * 0.5f, s, s };
}

juce::Rectangle<float> GemCell::jitterIconBounds() const {
    const auto k = knobBounds();
    const float s = getLocalBounds().getWidth() * (24.f / 120.f);
    const float gap = getLocalBounds().getWidth() * (10.f / 120.f);
    return { 0.f, k.getCentreY() + gap * 0.5f, s, s };
}

bool GemCell::hitTrigger(juce::Point<float> p) const    { return triggerIconBounds().expanded(6.f).contains(p); }
bool GemCell::hitJitter(juce::Point<float> p) const     { return jitterIconBounds().expanded(6.f).contains(p); }
bool GemCell::hitGemCentre(juce::Point<float> p) const  { return knobBounds().contains(p); }

void GemCell::notifyChanged() {
    if (onChanged) onChanged(0, 0);
    repaint();
}

void GemCell::updateMouseCursor(juce::Point<float> p) {
    if (!cell_ || mini_) {
        setMouseCursor(juce::MouseCursor::NormalCursor);
        return;
    }

    if (dragTarget_ == DragTarget::Trigger || dragTarget_ == DragTarget::Jitter
        || dragTarget_ == DragTarget::Degree) {
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
        return;
    }

    const bool showMinis = cell_->gate
        && (hovered_ || cell_->triggerArmed || cell_->jitterArmed);

    if (showMinis && hitTrigger(p)) {
        setMouseCursor(cell_->triggerArmed ? juce::MouseCursor::UpDownResizeCursor
                                           : juce::MouseCursor::PointingHandCursor);
        return;
    }
    if (showMinis && hitJitter(p)) {
        setMouseCursor(cell_->jitterArmed ? juce::MouseCursor::UpDownResizeCursor
                                          : juce::MouseCursor::PointingHandCursor);
        return;
    }
    if (hitGemCentre(p)) {
        setMouseCursor(cell_->gate ? juce::MouseCursor::UpDownResizeCursor
                                   : juce::MouseCursor::PointingHandCursor);
        return;
    }

    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void GemCell::paint(juce::Graphics& g) {
    if (!cell_) return;

    const auto knob = knobBounds();
    const auto variant = matilda::knob::variantForLayer(layer_);
    const bool showMinis = !mini_ && cell_->gate
        && (hovered_ || cell_->triggerArmed || cell_->jitterArmed);

    cartesia::lighting::MainCellState state;
    state.onActiveLayer = onActiveLayer_;
    state.isPlayhead    = playhead_;
    state.gate          = cell_->gate;

    const float stageBright = cartesia::lighting::stageBrightness(state);

    matilda::knob::drawSequencerKnob(g, knob, variant, cell_->gate,
                                     engine_.knobVisualPosition(*cell_), stageBright);

    if (!mini_) {
        const juce::String lbl = engine_.noteLabel(layer_, cellX_, cellY_);
        const auto labelArea = getLocalBounds().toFloat().removeFromTop(juce::jmax(14.f, getHeight() * 0.16f));
        const float fh = juce::jmax(11.f, labelArea.getHeight() * 0.85f);
        g.setFont(juce::FontOptions(fh).withStyle("Bold"));
        const bool outOfRange = lbl == juce::String::fromUTF8("\xe2\x80\x94");
        g.setColour(outOfRange ? laf_.textMuted
                               : (cell_->gate ? juce::Colours::white : juce::Colour(0xff444444)));
        g.drawText(lbl, labelArea.toNearestInt(), juce::Justification::centred);
    }

    if (!mini_) {
        const bool ledLit = playhead_ && cell_->gate;
        matilda::knob::drawCellLed(g, ledBounds(), ledLit, matilda::knob::ledColour(variant));
    }

    if (showMinis) {
        const auto mouse = isMouseOver(true) ? getMouseXYRelative().toFloat() : juce::Point<float>{};
        const bool triggerHov = triggerIconBounds().expanded(4.f).contains(mouse);
        const bool jitterHov = jitterIconBounds().expanded(4.f).contains(mouse);
        const bool triggerDrag = dragTarget_ == DragTarget::Trigger;
        const bool jitterDrag = dragTarget_ == DragTarget::Jitter;

        if (hovered_ || cell_->triggerArmed) {
            const float triggerVal = cell_->triggerArmed ? cell_->triggerProb : -1.f;
            matilda::knob::drawMiniKnob(g, triggerIconBounds(), true,
                                        triggerHov || triggerDrag, triggerDrag,
                                        triggerVal, "Note Trigger");
        }
        if (hovered_ || cell_->jitterArmed) {
            const float jitterVal = cell_->jitterArmed ? cell_->jitterAmount : -1.f;
            matilda::knob::drawMiniKnob(g, jitterIconBounds(), false,
                                        jitterHov || jitterDrag, jitterDrag,
                                        jitterVal, "Jitter");
        }
    }
}

void GemCell::mouseEnter(const juce::MouseEvent& e) {
    hovered_ = true;
    updateMouseCursor(e.position);
    repaint();
}

void GemCell::mouseExit(const juce::MouseEvent&) {
    hovered_ = false;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    repaint();
}

void GemCell::mouseMove(const juce::MouseEvent& e) {
    updateMouseCursor(e.position);
    if (hovered_)
        repaint();
}

void GemCell::mouseDown(const juce::MouseEvent& e) {
    if (!cell_ || mini_) return;
    const auto p = e.position;
    if (hitTrigger(p)) {
        if (!cell_->triggerArmed) {
            cell_->triggerArmed = true;
            cell_->triggerProb = 0.5f;
            notifyChanged();
            return;
        }
        dragTarget_ = DragTarget::Trigger;
        dragMoved_ = false;
    } else if (hitJitter(p)) {
        if (!cell_->jitterArmed) {
            cell_->jitterArmed = true;
            cell_->jitterAmount = 0.5f;
            notifyChanged();
            return;
        }
        dragTarget_ = DragTarget::Jitter;
        dragMoved_ = false;
    } else if (hitGemCentre(p)) {
        dragTarget_ = DragTarget::Degree;
        dragStartY_ = e.getPosition().y;
        dragStartQuantisedIndex_ = engine_.quantisedNoteIndex(*cell_);
        wheelAccumulator_ = 0.f;
        dragMoved_ = false;
    }
    updateMouseCursor(p);
}

void GemCell::mouseDrag(const juce::MouseEvent& e) {
    if (!cell_) return;
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);

    if (dragTarget_ == DragTarget::Trigger) {
        if (std::abs(e.getDistanceFromDragStartY()) > 3)
            dragMoved_ = true;
        const float delta = -static_cast<float>(e.getDistanceFromDragStartY()) * 0.0125f;
        cell_->triggerProb = juce::jlimit(0.f, 1.f, cell_->triggerProb + delta);
        notifyChanged();
    } else if (dragTarget_ == DragTarget::Jitter) {
        if (std::abs(e.getDistanceFromDragStartY()) > 3)
            dragMoved_ = true;
        const float delta = -static_cast<float>(e.getDistanceFromDragStartY()) * 0.0125f;
        cell_->jitterAmount = juce::jlimit(0.f, 1.f, cell_->jitterAmount + delta);
        notifyChanged();
    } else if (dragTarget_ == DragTarget::Degree) {
        const int dy = dragStartY_ - e.getPosition().y;
        if (std::abs(dy) > 2)
            dragMoved_ = true;

        const int noteCount = engine_.quantisedNoteCount();
        if (noteCount <= 1)
            return;

        constexpr float kPixelsPerFullTurn = 11.f * 14.f;
        const int delta = juce::roundToInt(static_cast<float>(dy) * static_cast<float>(noteCount - 1)
                                           / kPixelsPerFullTurn);
        const int before = engine_.quantisedNoteIndex(*cell_);
        engine_.setQuantisedNoteIndex(*cell_, dragStartQuantisedIndex_ + delta);
        if (engine_.quantisedNoteIndex(*cell_) != before)
            notifyChanged();
    }
}

void GemCell::mouseUp(const juce::MouseEvent& e) {
    if (dragTarget_ == DragTarget::Trigger && !dragMoved_) {
        cell_->triggerArmed = false;
        notifyChanged();
    } else if (dragTarget_ == DragTarget::Jitter && !dragMoved_) {
        cell_->jitterArmed = false;
        notifyChanged();
    } else if (dragTarget_ == DragTarget::Degree && !dragMoved_ && cell_) {
        cell_->gate = !cell_->gate;
        notifyChanged();
    }
    dragTarget_ = DragTarget::None;
    dragMoved_ = false;
    updateMouseCursor(e.position);
}

void GemCell::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (!cell_ || mini_ || !cell_->gate)
        return;
    if (!hitGemCentre(e.position))
        return;

    wheelAccumulator_ += wheel.deltaY;
    const int steps = static_cast<int>(std::trunc(wheelAccumulator_));
    if (steps == 0)
        return;

    wheelAccumulator_ -= static_cast<float>(steps);
    engine_.bumpCellDegree(*cell_, -steps);
    notifyChanged();
}
