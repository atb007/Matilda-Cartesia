#include "HeroCanvas.h"
#include "../MatildaFonts.h"
#include "../MatildaImages.h"
#include "../ReactShellLayout.h"

void HeroCanvas::paint(juce::Graphics& g) {
    using namespace matilda::react;

    const float s = static_cast<float>(getWidth()) / kExpandedW;

    const auto bg = matilda::images::heroBackground();
    if (bg.isValid()) {
        g.drawImage(bg, juce::Rectangle<float>(kHeroMainLeft * s, kHeroBgTop * s, kHeroBgW * 1.0758f * s,
                                               kHeroBgH * 1.032f * s));
    } else {
        g.fillAll(juce::Colour(0xff050510));
    }

    const auto portrait = matilda::images::heroPortrait();
    if (portrait.isValid()) {
        const float maskX = (kHeroMainLeft + kHeroMaskLeft) * s;
        const float maskY = kHeroMaskTop * s;
        const float maskW = kHeroMaskW * s;
        const float maskH = kHeroMaskH * s;
        g.saveState();
        g.reduceClipRegion(juce::Rectangle<int>(juce::roundToInt(maskX), juce::roundToInt(maskY),
                                                juce::roundToInt(maskW), juce::roundToInt(maskH)));
        const float px = (kHeroMainLeft + kHeroPortraitLeft) * s;
        const float py = kHeroPortraitTop * s;
        const float pw = kHeroPortraitW * s;
        const float ph = kHeroPortraitH * 1.1634f * s;
        g.drawImage(portrait, juce::Rectangle<float>(px, py, pw, ph));
        g.restoreState();
    }

    const int labelLeft = juce::roundToInt(kHeroLabelLeft * s);
    const int labelW = juce::roundToInt(kHeroLabelW * s);
    const int titleTop = juce::roundToInt(kHeroLabelTop * s);
    const int titleH = juce::roundToInt(kHeroTitleLineH * s);

    g.setColour(juce::Colours::white);
    g.setFont(matilda::fonts::jacquard24(kHeroTitleFs * s));
    g.drawText("Matilda", labelLeft, titleTop, labelW, titleH, juce::Justification::right);

    const int subTop = juce::roundToInt((kHeroLabelTop + kHeroTitleLineH + kHeroSubtitleGap) * s);
    const int subH = juce::roundToInt(kHeroSubtitleFs * s);

    g.setColour(juce::Colour(0xffdf90e5));
    g.setFont(matilda::fonts::jacquard24(kHeroSubtitleFs * s));
    g.drawText("Cartesia - v1.0", labelLeft, subTop, labelW, subH, juce::Justification::right);
}
