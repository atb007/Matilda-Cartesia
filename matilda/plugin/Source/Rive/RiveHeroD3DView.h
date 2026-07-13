#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace matilda::rive {

class RiveHeroBackendD3D;

/** Native HWND + DXGI swap chain host embedded via HWNDComponent. */
class RiveHeroD3DView : public juce::HWNDComponent, private juce::Timer {
public:
    explicit RiveHeroD3DView(RiveHeroBackendD3D& backend);
    ~RiveHeroD3DView() override;

    void setPlaying(bool playing);
    void attachRiveBytes(const void* data, size_t numBytes);
    void refreshDisplay();

    [[nodiscard]] bool isRiveReady() const { return riveReady_; }
    [[nodiscard]] bool hasRenderedFrame() const { return hasRenderedFrame_; }

private:
    void resized() override;
    void parentHierarchyChanged() override;
    void visibilityChanged() override;
    void timerCallback() override;
    void syncTimer();
    void ensureHostWindow();
    void updateSwapChainGeometry();
    bool renderSwapChain(float deltaSeconds);

    RiveHeroBackendD3D& backend_;
    bool playing_ = false;
    bool riveReady_ = false;
    bool hasRenderedFrame_ = false;
    juce::MemoryBlock rivBytes_;
    void* hostHwnd_ = nullptr;
};

} // namespace matilda::rive
