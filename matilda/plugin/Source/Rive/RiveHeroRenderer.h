#pragma once

#include <memory>

#include <JuceHeader.h>

#include "RiveHeroBackend.h"

#if defined(MATILDA_RIVE_HERO)

/** Facade used by HeroCanvas — picks backend at compile time. */
class RiveHeroRenderer {
public:
    RiveHeroRenderer();
    ~RiveHeroRenderer();

    bool loadFromMemory(const void* data, size_t numBytes);
    void setPlaying(bool playing);
    void setActiveLayerCount(int count);
    void setDisplayRect(juce::Rectangle<int> rect);
    /** Cover+CenterLeft viewport inside the overlay (GPU backends only). Empty = use overlay size. */
    void setContentAlignRect(juce::Rectangle<int> rect);
    bool tick(float deltaSeconds);

    [[nodiscard]] juce::Component* overlayComponent();
    [[nodiscard]] const juce::Image& frameImage() const;
    [[nodiscard]] bool isLoaded() const;
    [[nodiscard]] bool hasVisibleFrame() const;

private:
    std::unique_ptr<matilda::rive::RiveHeroBackend> backend_;
};

#else

class RiveHeroRenderer {
public:
    bool loadFromMemory(const void*, size_t) { return false; }
    void setPlaying(bool) {}
    void setActiveLayerCount(int) {}
    void setDisplayRect(juce::Rectangle<int>) {}
    void setContentAlignRect(juce::Rectangle<int>) {}
    bool tick(float) { return false; }
    juce::Component* overlayComponent() { return nullptr; }
    [[nodiscard]] const juce::Image& frameImage() const;
    [[nodiscard]] bool isLoaded() const { return false; }
    [[nodiscard]] bool hasVisibleFrame() const { return false; }
};

#endif
