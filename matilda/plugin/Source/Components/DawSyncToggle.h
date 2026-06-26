#pragma once

#include <JuceHeader.h>
#include <functional>

/** Top-right canvas toggle — Figma Sync (5108:109841) · DawSyncOn/Off @2x PNG. */
class DawSyncToggle : public juce::Component,
                      public juce::SettableTooltipClient {
public:
    DawSyncToggle();

    void setSyncOn(bool on);
    [[nodiscard]] bool isSyncOn() const { return syncOn_; }

    std::function<void(bool)> onToggle;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:
    void updateTooltip();

    bool syncOn_ = true;
    bool pressed_ = false;
};
