#include "DawSyncToggle.h"
#include "../ClickFeedbackDrawing.h"
#include "../MatildaImages.h"

DawSyncToggle::DawSyncToggle() {
    setInterceptsMouseClicks(true, false);
    setOpaque(false);
    updateTooltip();
}

void DawSyncToggle::setSyncOn(bool on) {
    if (syncOn_ != on) {
        syncOn_ = on;
        updateTooltip();
        repaint();
    }
}

void DawSyncToggle::updateTooltip() {
    setTooltip(syncOn_ ? "DAW sync is on" : "DAW sync is off");
}

void DawSyncToggle::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds().toFloat();
    matilda::ui::paintWithPressScale(g, bounds, pressed_);

    g.saveState();
    juce::Path clip;
    clip.addEllipse(bounds);
    g.reduceClipRegion(clip);

    const auto img = syncOn_ ? matilda::images::dawSyncOn() : matilda::images::dawSyncOff();
    if (img.isValid())
        g.drawImage(img, bounds, juce::RectanglePlacement::stretchToFit);

    g.restoreState();
}

void DawSyncToggle::mouseDown(const juce::MouseEvent&) {
    pressed_ = true;
    repaint();
}

void DawSyncToggle::mouseUp(const juce::MouseEvent& e) {
    const bool wasPressed = pressed_;
    pressed_ = false;
    repaint();

    if (!wasPressed || !e.mouseWasClicked() || !onToggle)
        return;

    syncOn_ = !syncOn_;
    updateTooltip();
    onToggle(syncOn_);
    repaint();
}

void DawSyncToggle::mouseEnter(const juce::MouseEvent&) {
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void DawSyncToggle::mouseExit(const juce::MouseEvent&) {
    if (pressed_) {
        pressed_ = false;
        repaint();
    }
    setMouseCursor(juce::MouseCursor::NormalCursor);
}
