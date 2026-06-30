#include "StepPathStrip.h"

StepPathStrip::StepPathStrip(MatildaLookAndFeel& laf) : laf_(laf) {
    setOpaque(false);
}

void StepPathStrip::setPlayhead(int step, int playingLayer, bool noteFired) {
    if (playheadStep_ == step && playingLayer_ == playingLayer && noteFired_ == noteFired)
        return;
    playheadStep_ = step;
    playingLayer_ = playingLayer;
    noteFired_ = noteFired;
    repaint();
}

void StepPathStrip::setLayerColour(juce::Colour c) {
    if (layerColour_ == c)
        return;
    layerColour_ = c;
    repaint();
}

void StepPathStrip::paint(juce::Graphics& g) {
    auto area = getLocalBounds().toFloat().reduced(2.f);
    g.setColour(laf_.textMuted);
    g.setFont(juce::FontOptions(11.f));
    g.drawText("Step path (row-major 0-15)", area.removeFromLeft(160.f).toNearestInt(),
               juce::Justification::centredLeft);

    const int count = 16;
    const float gap = 3.f;
    const float pillW = (area.getWidth() - gap * static_cast<float>(count - 1)) / static_cast<float>(count);
    const float pillH = juce::jmin(area.getHeight(), 22.f);
    const float y = area.getCentreY() - pillH * 0.5f;

    for (int i = 0; i < count; ++i) {
        auto pill = juce::Rectangle<float>(area.getX() + static_cast<float>(i) * (pillW + gap), y, pillW, pillH);
        const bool isHead = i == playheadStep_;

        g.setColour(juce::Colour(0xff2a2438));
        g.fillRoundedRectangle(pill, 4.f);

        if (isHead) {
            const auto col = noteFired_ ? layerColour_ : layerColour_.withAlpha(0.35f);
            g.setColour(col.withAlpha(0.55f));
            g.fillRoundedRectangle(pill, 4.f);
            g.setColour(col);
            g.drawRoundedRectangle(pill, 4.f, 1.5f);
        }

        g.setColour(isHead ? laf_.textPrimary : laf_.textMuted.withAlpha(0.7f));
        g.setFont(juce::FontOptions(isHead ? 10.f : 9.f).withStyle(isHead ? "Bold" : "Regular"));
        g.drawText(juce::String(i), pill.toNearestInt(), juce::Justification::centred);
    }
}
