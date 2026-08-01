#pragma once

#include "RiveHeroBackend.h"

namespace matilda::rive {

/**
 * Windows D3D11 PLS backend — renders offscreen and blits into a juce::Image.
 * Avoids HWND child swap chains (unreliable in standalone + many VST hosts).
 */
class RiveHeroBackendD3D : public RiveHeroBackend {
public:
    RiveHeroBackendD3D();
    ~RiveHeroBackendD3D() override;

    bool loadFromMemory(const void* data, size_t numBytes) override;
    void setPlaying(bool playing) override;
    void setActiveLayerCount(int count) override;
    void setPolyphony(bool enabled) override;
    void setDisplayRect(juce::Rectangle<int> rect) override;
    void setContentAlignRect(juce::Rectangle<int> rect) override;
    bool tick(float deltaSeconds) override;

    [[nodiscard]] bool isLoaded() const override;
    [[nodiscard]] bool hasVisibleOutput() const override;
    [[nodiscard]] const juce::Image& frameImage() const override;

private:
    struct Impl;
    struct FrameStorage;

    std::unique_ptr<Impl> impl_;
    std::unique_ptr<FrameStorage> frameStorage_;
    bool hasVisibleFrame_ = false;
    int displayW_ = 0;
    int displayH_ = 0;
    int contentAlignW_ = 0;
    int contentAlignH_ = 0;
    int renderW_ = 0;
    int renderH_ = 0;
};

} // namespace matilda::rive
