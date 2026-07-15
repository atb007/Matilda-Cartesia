#pragma once

#include "RiveHeroBackend.h"

namespace matilda::rive {

class RiveHeroMetalView;

/** GPU Metal PLS backend — renders into a child JUCE component overlay. */
class RiveHeroBackendMetal : public RiveHeroBackend {
public:
    RiveHeroBackendMetal();
    ~RiveHeroBackendMetal() override;

    bool loadFromMemory(const void* data, size_t numBytes) override;
    void setPlaying(bool playing) override;
    void setActiveLayerCount(int count) override;
    void setPolyphony(bool enabled) override;
    void setDisplayRect(juce::Rectangle<int> rect) override;
    void setContentAlignRect(juce::Rectangle<int> rect) override;
    bool tick(float deltaSeconds) override;

    [[nodiscard]] bool isLoaded() const override;
    [[nodiscard]] bool hasVisibleOutput() const override;
    [[nodiscard]] juce::Component* overlayComponent() override;
    [[nodiscard]] const juce::Image& frameImage() const override;

    /** Called from RiveHeroMetalView each frame. */
    bool renderMetalLayer(void* cametalLayer, float deltaSeconds);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::unique_ptr<RiveHeroMetalView> view_;
    int contentAlignW_ = 0;
    int contentAlignH_ = 0;
};

} // namespace matilda::rive
