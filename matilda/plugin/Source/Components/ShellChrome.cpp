#include "ShellChrome.h"
#include "../ShellGlassDrawing.h"

namespace {

juce::Rectangle<float> shellDesignRect(float x, float y, float w, float h, float scale) {
    return {x * scale, y * scale, w * scale, h * scale};
}

} // namespace

void ShellChrome::paint(juce::Graphics& g) {
    using namespace matilda::react;
    const float s = previewScale_;

    const auto glassRect = shellDesignRect(kGlassLeft, kGlassTop, kGlassW, kGlassH, s);
    matilda::ui::shell::paintGlassBedding(g, glassRect);
    matilda::ui::shell::paintGlassBeddingRadial(g, glassRect);

    const auto frameRect =
        shellDesignRect(kFrameOverlayLeft, kFrameOverlayTop, kFrameOverlayW, kFrameOverlayH, s);
    const auto frameImg = matilda::images::shellFrameVines();
    if (frameImg.isValid())
        g.drawImage(frameImg, frameRect, juce::RectanglePlacement::stretchToFit);
}
