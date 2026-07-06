#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

namespace matilda::rive {

class RiveHeroBackendMetal;

/** Native CAMetalLayer host embedded via NSViewComponent (avoids JUCE CG layer conflicts). */
class RiveHeroMetalView : public juce::NSViewComponent, private juce::Timer {
public:
    explicit RiveHeroMetalView(RiveHeroBackendMetal& backend);
    ~RiveHeroMetalView() override;

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
    void updateMetalLayerGeometry();
    void* metalLayerHandle() const;

    RiveHeroBackendMetal& backend_;
    bool playing_ = false;
    bool riveReady_ = false;
    bool hasRenderedFrame_ = false;
    juce::MemoryBlock rivBytes_;
};

} // namespace matilda::rive
