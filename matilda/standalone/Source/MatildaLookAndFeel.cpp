#include "MatildaLookAndFeel.h"

MatildaLookAndFeel::MatildaLookAndFeel() {
    textPrimary = juce::Colours::white;
    textMuted = juce::Colour(0xffb8b0c8);
    pillFill = juce::Colour(0xcc1a1428);
    pillBorder = juce::Colour(0x55ffffff);
    accentOrange = juce::Colour(0xffe87a3a);
    accentGreen = juce::Colour(0xff5fd68a);
    playheadColour = juce::Colour(0xfff0e6ff);

    setColour(juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::ComboBox::textColourId, textPrimary);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xee1a1428));
    setColour(juce::PopupMenu::textColourId, textPrimary);
    setColour(juce::Label::textColourId, textPrimary);
}

juce::Font MatildaLookAndFeel::getComboBoxFont(juce::ComboBox&) {
    return juce::FontOptions(13.f);
}

juce::Font MatildaLookAndFeel::getTextButtonFont(juce::TextButton&, int) {
    return juce::FontOptions(12.f);
}

void MatildaLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawFittedText(label.getText(), label.getLocalBounds(), label.getJustificationType(), 1);
}

void MatildaLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour&,
                                              bool highlighted, bool down) {
    if (button.getProperties().contains("matilda-pill")) {
        auto b = button.getLocalBounds().toFloat().reduced(0.5f);
        auto fill = pillFill;
        if (down)
            fill = fill.brighter(0.12f);
        else if (highlighted)
            fill = fill.brighter(0.06f);
        g.setColour(fill);
        g.fillRoundedRectangle(b, b.getHeight() * 0.45f);
        g.setColour(pillBorder.withAlpha(0.35f));
        g.drawRoundedRectangle(b, b.getHeight() * 0.45f, 1.f);
        return;
    }

    if (button.getProperties().contains("matilda-arrow")) {
        auto b = button.getLocalBounds().toFloat();
        g.setColour(textPrimary.withAlpha(highlighted ? 0.9f : 0.65f));
        const auto dir = button.getProperties()["matilda-arrow"].toString();
        juce::Path tri;
        if (dir == "up")
            tri.addTriangle(b.getCentreX(), b.getY() + 2.f, b.getRight() - 2.f, b.getBottom() - 2.f, b.getX() + 2.f,
                            b.getBottom() - 2.f);
        else if (dir == "left")
            tri.addTriangle(b.getX() + 2.f, b.getCentreY(), b.getRight() - 2.f, b.getY() + 2.f, b.getRight() - 2.f,
                            b.getBottom() - 2.f);
        else if (dir == "right")
            tri.addTriangle(b.getRight() - 2.f, b.getCentreY(), b.getX() + 2.f, b.getY() + 2.f, b.getX() + 2.f,
                            b.getBottom() - 2.f);
        else
            tri.addTriangle(b.getX() + 2.f, b.getY() + 2.f, b.getRight() - 2.f, b.getY() + 2.f, b.getCentreX(),
                            b.getBottom() - 2.f);
        g.fillPath(tri);
        return;
    }

    juce::LookAndFeel_V4::drawButtonBackground(g, button, pillFill, highlighted, down);
}

void MatildaLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                      int, int, int, int, juce::ComboBox& box) {
    auto bounds = juce::Rectangle<float>(0.f, 0.f, static_cast<float>(width), static_cast<float>(height));
    const float radius = juce::jmax(4.f, bounds.getHeight() * 0.22f);
    g.setColour(pillFill.withAlpha(0.85f));
    g.fillRoundedRectangle(bounds.reduced(0.5f), radius);
    g.setColour(pillBorder.withAlpha(0.28f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), radius, 1.f);

    g.setColour(textPrimary);
    g.setFont(juce::FontOptions(juce::jmax(10.f, bounds.getHeight() * 0.38f)));
    g.drawFittedText(box.getText(), bounds.reduced(8.f, 2.f).toNearestInt(), juce::Justification::centredLeft, 1);

    auto arrow = bounds.removeFromRight(juce::jmax(14.f, bounds.getHeight() * 0.55f));
    g.setColour(textPrimary.withAlpha(0.7f));
    juce::Path tri;
    tri.addTriangle(arrow.getCentreX() - 3.f, arrow.getCentreY() - 2.f, arrow.getCentreX() + 3.f,
                    arrow.getCentreY() - 2.f, arrow.getCentreX(), arrow.getCentreY() + 3.f);
    g.fillPath(tri);
}

void MatildaLookAndFeel::positionComboBoxText(juce::ComboBox&, juce::Label& label) {
    label.setBounds(0, 0, 0, 0);
}
