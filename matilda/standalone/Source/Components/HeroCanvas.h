#pragma once

#include <JuceHeader.h>

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
#include "../Rive/RiveHeroRenderer.h"
#endif

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
/** Wordmark overlay — sibling above HeroCanvas so it wins over native Metal NSView. */
class HeroWordmark : public juce::Component {
public:
    HeroWordmark() { setInterceptsMouseClicks(false, false); }
    void paint(juce::Graphics& g) override;
};
#endif

/** Starfield + masked portrait + wordmark — React HeroCanvas.tsx parity. */
class HeroCanvas : public juce::Component
#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__) && !defined(MATILDA_RIVE_BACKEND_METAL)
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

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
    /** Metal NSView overlay — re-front after shell/wordmark stack sync. */
    juce::Component* riveOverlayComponent();
    /** ContentPanel uses this to keep shell above native Metal after overlay attach. */
    std::function<void()> onRiveOverlayChanged;
#endif

private:
#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
    void ensureRiveLoaded();
    void updatePortraitLayout();
    void repaintPortraitArea();
#if !defined(MATILDA_RIVE_BACKEND_METAL)
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
