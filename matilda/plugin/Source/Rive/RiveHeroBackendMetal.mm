#include <JuceHeader.h>

#include "RiveHeroBackendMetal.h"
#include "RiveHeroMetalCore.h"
#include "RiveHeroMetalView.h"

namespace matilda::rive {

namespace {
juce::Image gEmptyImage;
}

struct RiveHeroBackendMetal::Impl {
    metal::MetalRiveCore core;
};

RiveHeroBackendMetal::RiveHeroBackendMetal()
    : impl_(std::make_unique<Impl>()), view_(std::make_unique<RiveHeroMetalView>(*this)) {}

RiveHeroBackendMetal::~RiveHeroBackendMetal() = default;

bool RiveHeroBackendMetal::loadFromMemory(const void* data, size_t numBytes) {
    const bool ok = impl_->core.loadFromMemory(data, numBytes);
    if (ok)
        view_->attachRiveBytes(data, numBytes);
    return ok;
}

void RiveHeroBackendMetal::setPlaying(bool playing) {
    impl_->core.setPlaying(playing);
    view_->setPlaying(playing);
}

void RiveHeroBackendMetal::setActiveLayerCount(int count) {
    impl_->core.setActiveLayerCount(count);
}

void RiveHeroBackendMetal::setPolyphony(bool enabled) {
    impl_->core.setPolyphony(enabled);
}

void RiveHeroBackendMetal::setDisplayRect(juce::Rectangle<int> rect) {
    if (view_ != nullptr)
        view_->setBounds(rect);
}

void RiveHeroBackendMetal::setContentAlignRect(juce::Rectangle<int> rect) {
    contentAlignW_ = rect.getWidth();
    contentAlignH_ = rect.getHeight();
    if (rect.isEmpty()) {
        contentAlignW_ = 0;
        contentAlignH_ = 0;
    }
}

bool RiveHeroBackendMetal::tick(float) { return impl_->core.hasRenderedFrame(); }

bool RiveHeroBackendMetal::isLoaded() const { return impl_ != nullptr && impl_->core.isLoaded(); }

bool RiveHeroBackendMetal::hasVisibleOutput() const {
    return view_ != nullptr && view_->hasRenderedFrame();
}

juce::Component* RiveHeroBackendMetal::overlayComponent() { return view_.get(); }

const juce::Image& RiveHeroBackendMetal::frameImage() const { return gEmptyImage; }

bool RiveHeroBackendMetal::renderMetalLayer(void* cametalLayer, float deltaSeconds) {
    if (impl_ == nullptr)
        return false;

    if (view_ != nullptr && contentAlignW_ > 0 && contentAlignH_ > 0) {
        const float scale = static_cast<float>(view_->getDesktopScaleFactor());
        impl_->core.setContentAlignSize(static_cast<uint32_t>(contentAlignW_ * scale),
                                        static_cast<uint32_t>(contentAlignH_ * scale));
    } else {
        impl_->core.setContentAlignSize(0, 0);
    }

    return impl_->core.render(cametalLayer, deltaSeconds);
}

} // namespace matilda::rive
