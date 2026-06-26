#include "MatildaImages.h"
#include "KnobDrawing.h"
#include "BinaryData.h"

namespace matilda::images {

juce::Image load(const char* data, int size) {
    return juce::ImageCache::getFromMemory(data, static_cast<size_t>(size));
}

juce::Image shellFrameVines() {
    return load(BinaryData::shellframevinesonly2x_png, BinaryData::shellframevinesonly2x_pngSize);
}

juce::Image shellGlassBedding() {
    return load(BinaryData::shellglassbedding_png, BinaryData::shellglassbedding_pngSize);
}

juce::Image heroBackground() {
    return load(BinaryData::herobgm8b_png, BinaryData::herobgm8b_pngSize);
}

juce::Image heroPortrait() {
    return load(BinaryData::matildaportraitmasked_png, BinaryData::matildaportraitmasked_pngSize);
}

juce::Image heroMaskAlpha() {
    return load(BinaryData::matildamaskalpha_png, BinaryData::matildamaskalpha_pngSize);
}

juce::Image collapseToggleExpanded() {
    return load(BinaryData::collapsetoggleexpanded2x_png, BinaryData::collapsetoggleexpanded2x_pngSize);
}

juce::Image collapseToggleCollapsed() {
    return load(BinaryData::collapsetogglecollapsed2x_png, BinaryData::collapsetogglecollapsed2x_pngSize);
}

juce::Image transportPlayGem() {
    return load(BinaryData::transportplaygem_png, BinaryData::transportplaygem_pngSize);
}

juce::Image transportPlayFrame() {
    return load(BinaryData::transportplayframe_png, BinaryData::transportplayframe_pngSize);
}

juce::Image transportGlassBg() {
    return load(BinaryData::transportglassbg_png, BinaryData::transportglassbg_pngSize);
}

juce::Image transportPlayIcon() {
    return load(BinaryData::transportplayicon_png, BinaryData::transportplayicon_pngSize);
}

juce::Image transportStopIcon() {
    return load(BinaryData::transportstopicon_png, BinaryData::transportstopicon_pngSize);
}

juce::Image transportPlayLinkIcon() {
    static const juce::Image cached = [] {
        const auto xml = juce::parseXML(juce::String::fromUTF8(
            BinaryData::transportplaylinkicon_svg, static_cast<size_t>(BinaryData::transportplaylinkicon_svgSize)));
        if (!xml)
            return juce::Image{};
        const auto drawable = juce::Drawable::createFromSVG(*xml);
        if (!drawable)
            return juce::Image{};
        juce::Image img(juce::Image::ARGB, 150, 90, true);
        juce::Graphics g(img);
        g.fillAll(juce::Colours::transparentBlack);
        drawable->drawWithin(g, juce::Rectangle<float>(0.f, 0.f, 150.f, 90.f),
                             juce::RectanglePlacement::stretchToFit, 1.f);
        return img;
    }();
    return cached;
}

juce::Image dawSyncOn() {
    return load(BinaryData::DawSyncOn_png, BinaryData::DawSyncOn_pngSize);
}

juce::Image dawSyncOff() {
    return load(BinaryData::DawSyncOff_png, BinaryData::DawSyncOff_pngSize);
}

juce::Image movementBgTexture() {
    return load(BinaryData::movementbgtexture2xpng_png, BinaryData::movementbgtexture2xpng_pngSize);
}

juce::Image miniGridFrame() {
    return load(BinaryData::minigridframe_png, BinaryData::minigridframe_pngSize);
}

juce::Image miniGridInactive() {
    return load(BinaryData::minigridinactive_png, BinaryData::minigridinactive_pngSize);
}

static juce::Image layerImage(const char* onData, int onSize, const char* offData, int offSize, bool on) {
    return load(on ? onData : offData, on ? onSize : offSize);
}

juce::Image miniGridOn(int layer) {
    switch (layer & 3) {
        case 1: return layerImage(BinaryData::minigridredon_png, BinaryData::minigridredon_pngSize,
                                  BinaryData::minigridredoff_png, BinaryData::minigridredoff_pngSize, true);
        case 2: return layerImage(BinaryData::minigridgreenon_png, BinaryData::minigridgreenon_pngSize,
                                  BinaryData::minigridgreenoff_png, BinaryData::minigridgreenoff_pngSize, true);
        case 3: return layerImage(BinaryData::minigridblueon_png, BinaryData::minigridblueon_pngSize,
                                  BinaryData::minigridblueoff_png, BinaryData::minigridblueoff_pngSize, true);
        default: return layerImage(BinaryData::minigridorangeon_png, BinaryData::minigridorangeon_pngSize,
                                   BinaryData::minigridorangeoff_png, BinaryData::minigridorangeoff_pngSize, true);
    }
}

juce::Image miniGridOff(int layer) {
    switch (layer & 3) {
        case 1: return load(BinaryData::minigridredoff_png, BinaryData::minigridredoff_pngSize);
        case 2: return load(BinaryData::minigridgreenoff_png, BinaryData::minigridgreenoff_pngSize);
        case 3: return load(BinaryData::minigridblueoff_png, BinaryData::minigridblueoff_pngSize);
        default: return load(BinaryData::minigridorangeoff_png, BinaryData::minigridorangeoff_pngSize);
    }
}

struct ScaleGemAsset {
    const char* modeId;
    const char* data;
    int size;
};

static const ScaleGemAsset kScaleGemAssets[] = {
    {"chromatic", BinaryData::scalegemchromatic_png, BinaryData::scalegemchromatic_pngSize},
    {"major", BinaryData::scalegemmajor_png, BinaryData::scalegemmajor_pngSize},
    {"minor", BinaryData::scalegemminor_png, BinaryData::scalegemminor_pngSize},
    {"dorian", BinaryData::scalegemdorian_png, BinaryData::scalegemdorian_pngSize},
    {"phrygian", BinaryData::scalegemphrygian_png, BinaryData::scalegemphrygian_pngSize},
    {"lydian", BinaryData::scalegemlydian_png, BinaryData::scalegemlydian_pngSize},
    {"mixolydian", BinaryData::scalegemmixolydian_png, BinaryData::scalegemmixolydian_pngSize},
    {"locrian", BinaryData::scalegemlocrian_png, BinaryData::scalegemlocrian_pngSize},
    {"harmonic_minor", BinaryData::scalegemharmonic_minor_png, BinaryData::scalegemharmonic_minor_pngSize},
    {"melodic_minor", BinaryData::scalegemmelodic_minor_png, BinaryData::scalegemmelodic_minor_pngSize},
    {"pentatonic", BinaryData::scalegempentatonic_png, BinaryData::scalegempentatonic_pngSize},
    {"pentatonic_minor", BinaryData::scalegempentatonic_minor_png, BinaryData::scalegempentatonic_minor_pngSize},
    {"blues", BinaryData::scalegemblues_png, BinaryData::scalegemblues_pngSize},
};

juce::Image scaleGemForMode(const juce::String& modeId) {
    const auto id = modeId.toLowerCase();
    for (const auto& entry : kScaleGemAssets)
        if (id == entry.modeId)
            return load(entry.data, entry.size);
    return load(BinaryData::scalegemchromatic_png, BinaryData::scalegemchromatic_pngSize);
}

juce::Image knobOuterRing() {
    return load(BinaryData::OuterRing2x_png, BinaryData::OuterRing2x_pngSize);
}

juce::Image knobSphere() {
    return load(BinaryData::DarkSpehere2x_png, BinaryData::DarkSpehere2x_pngSize);
}

juce::Image knobColoredGloss(knob::Variant variant, bool gateOn) {
    const bool on = gateOn;
    switch (variant) {
        case knob::Variant::Red:
            return load(on ? BinaryData::ColoredGlossON2x_png2 : BinaryData::ColoredGlossOFF2x_png2,
                        on ? BinaryData::ColoredGlossON2x_png2Size : BinaryData::ColoredGlossOFF2x_png2Size);
        case knob::Variant::Green:
            return load(on ? BinaryData::ColoredGlossON2x_png3 : BinaryData::ColoredGlossOFF2x_png3,
                        on ? BinaryData::ColoredGlossON2x_png3Size : BinaryData::ColoredGlossOFF2x_png3Size);
        case knob::Variant::Blue:
            return load(on ? BinaryData::ColoredGlossON2x_png4 : BinaryData::ColoredGlossOFF2x_png4,
                        on ? BinaryData::ColoredGlossON2x_png4Size : BinaryData::ColoredGlossOFF2x_png4Size);
        case knob::Variant::Orange:
        default:
            return load(on ? BinaryData::ColoredGlossON2x_png : BinaryData::ColoredGlossOFF2x_png,
                        on ? BinaryData::ColoredGlossON2x_pngSize : BinaryData::ColoredGlossOFF2x_pngSize);
    }
}

juce::Image knobStageGlow() {
    return load(BinaryData::gemknobglow_png, BinaryData::gemknobglow_pngSize);
}

} // namespace matilda::images
