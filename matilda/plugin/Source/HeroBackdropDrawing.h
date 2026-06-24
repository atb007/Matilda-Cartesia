#pragma once

#include "MatildaImages.h"
#include <JuceHeader.h>

namespace matilda::ui {

/** Starfield wallpaper — aspect-cover fill (fills host oversize voids in VST3). */
inline void paintHeroBackdropCover(juce::Graphics& g, juce::Rectangle<int> bounds) {
    const auto bg = matilda::images::heroBackground();
    if (!bg.isValid()) {
        g.fillAll(juce::Colour(0xff050510));
        return;
    }

    const float imgAspect = static_cast<float>(bg.getWidth()) / static_cast<float>(bg.getHeight());
    auto dest = bounds.toFloat();
    const float boundsAspect = dest.getWidth() / dest.getHeight();

    if (imgAspect > boundsAspect)
        dest.setWidth(dest.getHeight() * imgAspect);
    else
        dest.setHeight(dest.getWidth() / imgAspect);

    dest.setCentre(bounds.toFloat().getCentre());
    g.drawImage(bg, dest);
}

} // namespace matilda::ui
