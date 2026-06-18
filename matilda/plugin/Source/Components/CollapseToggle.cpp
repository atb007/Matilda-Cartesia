#include "CollapseToggle.h"
#include "../MatildaImages.h"

CollapseToggle::CollapseToggle() {
    setInterceptsMouseClicks(true, false);
}

void CollapseToggle::setCollapsed(bool collapsed) {
    if (collapsed_ != collapsed) {
        collapsed_ = collapsed;
        repaint();
    }
}

void CollapseToggle::paint(juce::Graphics& g) {
    const auto img = collapsed_ ? matilda::images::collapseToggleCollapsed()
                                : matilda::images::collapseToggleExpanded();
    if (img.isValid())
        g.drawImage(img, getLocalBounds().toFloat(), juce::RectanglePlacement::centred);
}

void CollapseToggle::mouseUp(const juce::MouseEvent& e) {
    if (e.mouseWasClicked() && onToggle)
        onToggle();
}
