#pragma once

#include "RiveHeroBackend.h"

namespace matilda::rive {

class RiveHeroD3DView;

/** GPU D3D11 PLS backend — renders into a child JUCE HWND overlay. */
class RiveHeroBackendD3D : public RiveHeroBackend {
public:
    RiveHeroBackendD3D();
    ~RiveHeroBackendD3D() override;

    bool loadFromMemory(const void* data, size_t numBytes) override;
    void setPlaying(bool playing) override;
    void setActiveLayerCount(int count) override;
    void setDisplayRect(juce::Rectangle<int> rect) override;
    void setContentAlignRect(juce::Rectangle<int> rect) override;
    bool tick(float deltaSeconds) override;

    [[nodiscard]] bool isLoaded() const override;
    [[nodiscard]] bool hasVisibleOutput() const override;
    [[nodiscard]] juce::Component* overlayComponent() override;
    [[nodiscard]] const juce::Image& frameImage() const override;

    /** Called from RiveHeroD3DView each frame. */
    bool renderSwapChain(void* hostHwnd, float deltaSeconds);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::unique_ptr<RiveHeroD3DView> view_;
    int contentAlignW_ = 0;
    int contentAlignH_ = 0;
};

} // namespace matilda::rive
