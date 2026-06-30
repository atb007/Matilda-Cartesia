#include "UiResizeGrip.h"

namespace {

void paintCornerGrip(juce::Graphics& g, juce::Rectangle<float> b, bool fromRight, bool fromBottom) {
    const float edgeX = fromRight ? b.getRight() - 2.f : b.getX() + 2.f;
    const float edgeY = fromBottom ? b.getBottom() - 2.f : b.getY() + 2.f;
    const float x0 = fromRight ? b.getRight() - 10.f : b.getX() + 2.f;
    const float y0 = fromBottom ? b.getBottom() - 10.f : b.getY() + 2.f;

    for (int i = 0; i < 3; ++i) {
        const float o = static_cast<float>(i) * 4.f;
        if (fromRight && fromBottom) {
            g.drawLine(x0 + o, edgeY, edgeX, y0 + o, 1.2f);
        } else if (!fromRight && fromBottom) {
            g.drawLine(b.getRight() - 2.f - o, edgeY, edgeX, y0 + o, 1.2f);
        } else if (fromRight && !fromBottom) {
            g.drawLine(x0 + o, edgeY, edgeX, b.getBottom() - 2.f - o, 1.2f);
        } else {
            g.drawLine(b.getRight() - 2.f - o, edgeY, edgeX, b.getBottom() - 2.f - o, 1.2f);
        }
    }
}

void paintEdgeGrip(juce::Graphics& g, juce::Rectangle<float> b, bool horizontal) {
    const float cx = b.getCentreX();
    const float cy = b.getCentreY();

    if (horizontal) {
        for (int i = -1; i <= 1; ++i) {
            const float x = cx + static_cast<float>(i) * 6.f;
            g.drawLine(x, cy - 1.f, x, cy + 1.f, 1.2f);
        }
    } else {
        for (int i = -1; i <= 1; ++i) {
            const float y = cy + static_cast<float>(i) * 6.f;
            g.drawLine(cx - 1.f, y, cx + 1.f, y, 1.2f);
        }
    }
}

} // namespace

UiResizeGrip::UiResizeGrip(matilda::ui::UiResizeGripId id) : id_(id) {
    setMouseCursor(gripCursor());
}

juce::MouseCursor UiResizeGrip::gripCursor() const {
    using matilda::ui::UiResizeGripId;
    switch (id_) {
        case UiResizeGripId::topLeft:
            return juce::MouseCursor::TopLeftCornerResizeCursor;
        case UiResizeGripId::topRight:
            return juce::MouseCursor::TopRightCornerResizeCursor;
        case UiResizeGripId::bottomLeft:
            return juce::MouseCursor::BottomLeftCornerResizeCursor;
        case UiResizeGripId::bottomRight:
            return juce::MouseCursor::BottomRightCornerResizeCursor;
        case UiResizeGripId::top:
        case UiResizeGripId::bottom:
            return juce::MouseCursor::UpDownResizeCursor;
        case UiResizeGripId::left:
        case UiResizeGripId::right:
            return juce::MouseCursor::LeftRightResizeCursor;
    }
    return juce::MouseCursor::NormalCursor;
}

void UiResizeGrip::paint(juce::Graphics& g) {
    const auto b = getLocalBounds().toFloat();
    g.setColour(juce::Colours::white.withAlpha(isMouseOver() ? 0.35f : 0.18f));

    using matilda::ui::UiResizeGripId;
    switch (id_) {
        case UiResizeGripId::topLeft:
            paintCornerGrip(g, b.reduced(4.f), false, false);
            break;
        case UiResizeGripId::topRight:
            paintCornerGrip(g, b.reduced(4.f), true, false);
            break;
        case UiResizeGripId::bottomLeft:
            paintCornerGrip(g, b.reduced(4.f), false, true);
            break;
        case UiResizeGripId::bottomRight:
            paintCornerGrip(g, b.reduced(4.f), true, true);
            break;
        case UiResizeGripId::top:
        case UiResizeGripId::bottom:
            paintEdgeGrip(g, b, true);
            break;
        case UiResizeGripId::left:
        case UiResizeGripId::right:
            paintEdgeGrip(g, b, false);
            break;
    }
}

void UiResizeGrip::mouseDown(const juce::MouseEvent&) {
    if (onDragStart)
        onDragStart();
}

void UiResizeGrip::mouseDrag(const juce::MouseEvent& e) {
    if (onDragMove)
        onDragMove(e.getScreenPosition());
}

void UiResizeGrip::mouseUp(const juce::MouseEvent&) {
    if (onDragEnd)
        onDragEnd();
}
