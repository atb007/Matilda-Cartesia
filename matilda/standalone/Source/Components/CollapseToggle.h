#pragma once

#include <JuceHeader.h>
#include <functional>

/** Glass chevron — Figma `5002:6419` (expanded >> · collapsed <<). */
class CollapseToggle : public juce::Component,
                       public juce::SettableTooltipClient {
public:
    CollapseToggle();

    void setCollapsed(bool collapsed);
    [[nodiscard]] bool isCollapsed() const { return collapsed_; }

    std::function<void()> onToggle;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:
    void updateTooltip();

    bool collapsed_ = false;
    bool pressed_ = false;
};
