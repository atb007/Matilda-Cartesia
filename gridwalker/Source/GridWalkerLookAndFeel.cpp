#include "GridWalkerLookAndFeel.h"

GridWalkerLookAndFeel::GridWalkerLookAndFeel() {
    backgroundDark = juce::Colour(0xff0b1020);
    panelColour = juce::Colour(0xff1a2238);
    accentColour = juce::Colour(0xff7b9fd4);
    gateOnColour = juce::Colour(0xff8fb8a8);
    playheadColour = juce::Colour(0xffb8e0ff);
    textPrimary = juce::Colour(0xffe8edf5);
    textMuted = juce::Colour(0xff8b95a8);

    setColour(juce::ResizableWindow::backgroundColourId, backgroundDark);
    setColour(juce::Label::textColourId, textPrimary);
    setColour(juce::ComboBox::backgroundColourId, panelColour);
    setColour(juce::ComboBox::textColourId, textPrimary);
    setColour(juce::PopupMenu::backgroundColourId, panelColour);
    setColour(juce::PopupMenu::textColourId, textPrimary);
    setColour(juce::ToggleButton::textColourId, textPrimary);
}

void GridWalkerLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                             float sliderPos, float startAngle, float endAngle,
                                             juce::Slider&) {
    const auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                               static_cast<float>(width), static_cast<float>(height)).reduced(4.f);
    const float radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const float cx = bounds.getCentreX();
    const float cy = bounds.getCentreY();
    const float angle = startAngle + sliderPos * (endAngle - startAngle);

    g.setColour(panelColour.brighter(0.15f));
    g.fillEllipse(cx - radius, cy - radius, radius * 2.f, radius * 2.f);

    juce::Path arc;
    arc.addCentredArc(cx, cy, radius * 0.85f, radius * 0.85f, 0.f, startAngle, angle, true);
    g.setColour(accentColour);
    g.strokePath(arc, juce::PathStrokeType(2.5f));
}

void GridWalkerLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                 const juce::Colour&,
                                                 bool highlighted, bool down) {
    auto bounds = button.getLocalBounds().toFloat().reduced(1.f);
    const bool on = button.getToggleState();
    auto fill = on ? gateOnColour.withAlpha(0.45f) : panelColour;
    if (down)
        fill = fill.brighter(0.1f);
    else if (highlighted)
        fill = fill.brighter(0.05f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, 6.f);
    g.setColour(on ? gateOnColour : textMuted.withAlpha(0.35f));
    g.drawRoundedRectangle(bounds, 6.f, 1.f);
}
