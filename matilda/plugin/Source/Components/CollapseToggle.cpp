#include "CollapseToggle.h"
#include "../MatildaImages.h"

CollapseToggle::CollapseToggle() {
    setInterceptsMouseClicks(true, false);
    setOpaque(false);
}

void CollapseToggle::setCollapsed(bool collapsed) {
    if (collapsed_ != collapsed) {
        collapsed_ = collapsed;
        repaint();
    }
}

void CollapseToggle::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds().toFloat();

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

void CollapseToggle::mouseUp(const juce::MouseEvent& e) {
    if (e.mouseWasClicked() && onToggle)
        onToggle();
}
