#include "ShellGlassDrawing.h"
#include "BinaryData.h"

namespace matilda::ui::shell {

namespace {

juce::Drawable* glassRadialDrawable() {
    static std::unique_ptr<juce::Drawable> drawable = []() -> std::unique_ptr<juce::Drawable> {
        const auto xml =
            juce::parseXML(juce::String::fromUTF8(BinaryData::shellglassradial_svg,
                                                  static_cast<size_t>(BinaryData::shellglassradial_svgSize)));
        return xml ? juce::Drawable::createFromSVG(*xml) : nullptr;
    }();
    return drawable.get();
}

} // namespace

void paintGlassBeddingRadial(juce::Graphics& g, juce::Rectangle<float> rect) {
    if (auto* radial = glassRadialDrawable()) {
        g.saveState();
        g.setOpacity(0.2f);
        radial->drawWithin(g, rect, juce::RectanglePlacement::stretchToFit, 1.f);
        g.restoreState();
    }
}

} // namespace matilda::ui::shell
