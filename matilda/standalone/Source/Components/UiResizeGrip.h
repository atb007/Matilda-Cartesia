#pragma once

#include "../UiScale.h"
#include <JuceHeader.h>

/** Corner or edge grip — drag to scale the whole plugin UI. */
class UiResizeGrip : public juce::Component {
public:
    explicit UiResizeGrip(matilda::ui::UiResizeGripId id);

    [[nodiscard]] matilda::ui::UiResizeGripId gripId() const { return id_; }

    std::function<void()> onDragStart;
    std::function<void(juce::Point<int> screenPosition)> onDragMove;
    std::function<void()> onDragEnd;

    void paint(juce::Graphics& g) override;

private:
    matilda::ui::UiResizeGripId id_;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    [[nodiscard]] juce::MouseCursor gripCursor() const;
};
