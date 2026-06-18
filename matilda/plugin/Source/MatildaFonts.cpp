#include "MatildaFonts.h"
#include "BinaryData.h"

namespace matilda::fonts {

namespace {

juce::Typeface::Ptr loadTypeface(const char* data, int size) {
    return juce::Typeface::createSystemTypefaceFor(data, static_cast<size_t>(size));
}

juce::Typeface::Ptr asimovianFace() {
    static auto face = loadTypeface(BinaryData::AsimovianRegular_ttf, BinaryData::AsimovianRegular_ttfSize);
    return face;
}

juce::Typeface::Ptr kodeMonoBoldFace() {
    static auto face = loadTypeface(BinaryData::KodeMonoBold_ttf, BinaryData::KodeMonoBold_ttfSize);
    return face;
}

juce::Typeface::Ptr supermercadoOneFace() {
    static auto face =
        loadTypeface(BinaryData::SupermercadoOneRegular_ttf, BinaryData::SupermercadoOneRegular_ttfSize);
    return face;
}

juce::Typeface::Ptr jacquard24Face() {
    static auto face =
        loadTypeface(BinaryData::Jacquard24Regular_ttf, BinaryData::Jacquard24Regular_ttfSize);
    return face;
}

} // namespace

juce::Font asimovian(float heightPx) {
    juce::Font font(juce::FontOptions(asimovianFace()).withHeight(heightPx));
    return font;
}

juce::Font kodeMonoBold(float heightPx) {
    juce::Font font(juce::FontOptions(kodeMonoBoldFace()).withHeight(heightPx));
    return font;
}

juce::Font supermercadoOne(float heightPx) {
    juce::Font font(juce::FontOptions(supermercadoOneFace()).withHeight(heightPx));
    return font;
}

juce::Font jacquard24(float heightPx) {
    juce::Font font(juce::FontOptions(jacquard24Face()).withHeight(heightPx));
    return font;
}

} // namespace matilda::fonts
