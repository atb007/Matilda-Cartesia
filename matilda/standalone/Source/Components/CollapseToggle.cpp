#include "CollapseToggle.h"
#include "../ClickFeedbackDrawing.h"
#include "../MatildaImages.h"

CollapseToggle::CollapseToggle() {
    setInterceptsMouseClicks(true, false);
    setOpaque(false);
    updateTooltip();
}

void CollapseToggle::setCollapsed(bool collapsed) {
    if (collapsed_ != collapsed) {
        collapsed_ = collapsed;
        updateTooltip();
        repaint();
    }
}

void CollapseToggle::updateTooltip() {
    setTooltip(collapsed_ ? "Expand hero panel" : "Collapse hero panel");
}

void CollapseToggle::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds().toFloat();
    matilda::ui::paintWithPressScale(g, bounds, pressed_);

    g.saveState();
    juce::Path clip;
    clip.addEllipse(bounds);
    g.reduceClipRegion(clip);

    const auto img = collapsed_ ? matilda::images::collapseToggleCollapsed()
                                : matilda::images::collapseToggleExpanded();
    if (img.isValid())
        g.drawImage(img, bounds, juce::RectanglePlacement::stretchToFit);

    g.restoreState();
}

void CollapseToggle::mouseDown(const juce::MouseEvent&) {
    pressed_ = true;
    repaint();
}

void CollapseToggle::mouseUp(const juce::MouseEvent& e) {
    const bool wasPressed = pressed_;
    pressed_ = false;
    repaint();

    if (wasPressed && e.mouseWasClicked() && onToggle)
        onToggle();
}

void CollapseToggle::mouseEnter(const juce::MouseEvent&) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void CollapseToggle::mouseExit(const juce::MouseEvent&) {
    if (pressed_) {
        pressed_ = false;
        repaint();
    }
    setMouseCursor(juce::MouseCursor::NormalCursor);
}
