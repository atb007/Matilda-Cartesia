#include "MatildaBackground.h"
#include "BinaryData.h"

MatildaBackground::MatildaBackground() {
    setOpaque(true);

    // Load the VCV Cartesia dark SVG panel
    const auto xml = juce::parseXML(
        juce::String::fromUTF8(BinaryData::Cartesiadark_svg,
                                BinaryData::Cartesiadark_svgSize));
    if (xml)
        panel_ = juce::Drawable::createFromSVG(*xml);
}

void MatildaBackground::paint(juce::Graphics& g) {
    const auto bounds = getLocalBounds().toFloat();

    if (panel_) {
        panel_->drawWithin(g, bounds, juce::RectanglePlacement::fillDestination, 1.0f);
    } else {
        // Fallback if SVG failed to load
        g.fillAll(juce::Colour(0xff1a1428));
        g.setColour(juce::Colours::white.withAlpha(0.1f));
        g.drawRect(bounds, 1.f);
    }

    // Matilda title overlay (top-left, within the panel chrome)
    {
        const float titleX = bounds.getWidth()  * 0.02f;
        const float titleY = bounds.getHeight() * 0.01f;
        const float fH     = bounds.getHeight() * 0.032f;

        g.setColour(juce::Colours::white.withAlpha(0.92f));
        g.setFont(juce::FontOptions(fH * 1.4f).withStyle("Bold"));
        g.drawText("Matilda", titleX, titleY, bounds.getWidth() * 0.3f, fH * 1.6f,
                   juce::Justification::centredLeft);
        g.setFont(juce::FontOptions(fH * 0.85f));
        g.setColour(juce::Colour(0xffd08c59));  // Cartesia gold colour
        g.drawText("Cartesia  v1.0", titleX, titleY + fH * 1.65f,
                   bounds.getWidth() * 0.3f, fH,
                   juce::Justification::centredLeft);
    }
}
