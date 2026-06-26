#pragma once

#include <JuceHeader.h>

namespace matilda::knob { enum class Variant; }

namespace matilda::images {

juce::Image load(const char* data, int size);
juce::Image shellFrameVines();
juce::Image shellGlassBedding();
juce::Image heroBackground();
juce::Image heroPortrait();
juce::Image heroMaskAlpha();
juce::Image collapseToggleExpanded();
juce::Image collapseToggleCollapsed();
juce::Image transportPlayGem();
juce::Image transportPlayFrame();
juce::Image transportGlassBg();
juce::Image transportPlayIcon();
juce::Image transportStopIcon();
juce::Image transportPlayLinkIcon();
juce::Image dawSyncOn();
juce::Image dawSyncOff();
juce::Image movementBgTexture();
juce::Image miniGridFrame();
juce::Image miniGridInactive();
juce::Image miniGridOn(int layer);
juce::Image miniGridOff(int layer);
juce::Image scaleGemForMode(const juce::String& modeId);

juce::Image knobOuterRing();
juce::Image knobSphere();
juce::Image knobColoredGloss(knob::Variant variant, bool gateOn);
juce::Image knobStageGlow();

} // namespace matilda::images
