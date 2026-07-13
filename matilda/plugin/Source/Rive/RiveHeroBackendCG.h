#pragma once

#include "RiveHeroBackend.h"

namespace matilda::rive {

/** CPU CoreGraphics backend — bitmap blit in HeroCanvas::paint. */
class RiveHeroBackendCG : public RiveHeroBackend {
public:
    RiveHeroBackendCG();
    ~RiveHeroBackendCG() override;

    bool loadFromMemory(const void* data, size_t numBytes) override;
    void setPlaying(bool playing) override;
    void setActiveLayerCount(int count) override;
    void setDisplayRect(juce::Rectangle<int> rect) override;
    void setContentAlignRect(juce::Rectangle<int> rect) override {}
    bool tick(float deltaSeconds) override;

    [[nodiscard]] bool isLoaded() const override { return loaded_; }
    [[nodiscard]] bool hasVisibleOutput() const override { return hasVisibleFrame_; }
    [[nodiscard]] const juce::Image& frameImage() const override;

private:
    struct Impl;
    struct FrameStorage;

    std::unique_ptr<Impl> impl_;
    std::unique_ptr<FrameStorage> frameStorage_;
    bool loaded_ = false;
    bool hasVisibleFrame_ = false;
    bool playing_ = false;
    int activeLayerCount_ = 1;
    int renderW_ = 0;
    int renderH_ = 0;
    bool sizeDirty_ = true;
};

} // namespace matilda::rive
