#pragma once

#include <JuceHeader.h>

#if defined(MATILDA_RIVE_HERO)
#include "../Rive/RiveHeroRenderer.h"
#endif

#if defined(MATILDA_RIVE_HERO)
/** Wordmark overlay — sibling above HeroCanvas so it wins over native GPU overlay. */
class HeroWordmark : public juce::Component {
public:
    HeroWordmark() { setInterceptsMouseClicks(false, false); }
    void paint(juce::Graphics& g) override;
};
#endif

/** Starfield + masked portrait + wordmark — React HeroCanvas.tsx parity. */
class HeroCanvas : public juce::Component
#if defined(MATILDA_RIVE_HERO) && !defined(MATILDA_RIVE_BACKEND_GPU)
    ,
                  private juce::Timer
#endif
{
public:
    HeroCanvas();
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setPlaying(bool playing);
    void setActiveLayerCount(int count);

#if defined(MATILDA_RIVE_HERO)
    /** Native GPU overlay — re-front after shell/wordmark stack sync. */
    juce::Component* riveOverlayComponent();
    /** ContentPanel uses this to keep shell above native GPU after overlay attach. */
    std::function<void()> onRiveOverlayChanged;
#endif

private:
#if defined(MATILDA_RIVE_HERO)
    void ensureRiveLoaded();
    void updatePortraitLayout();
    void repaintPortraitArea();
#if !defined(MATILDA_RIVE_BACKEND_GPU)
    void timerCallback() override;
    void syncRiveTimer();
    [[nodiscard]] bool shouldDrawCgRiveFrame() const;
#endif

    RiveHeroRenderer rive_;
    bool playing_ = false;
    int activeLayerCount_ = 1;
    bool riveLoaded_ = false;
    juce::Rectangle<int> portraitClipRect_;
    juce::Rectangle<int> portraitLocalRect_;
    juce::Rectangle<int> portraitOverlayRect_;
#endif
};
