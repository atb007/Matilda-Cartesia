#include "RiveHeroRenderer.h"

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)

#include "RiveHeroBackendCG.h"
#if defined(MATILDA_RIVE_BACKEND_METAL)
#include "RiveHeroBackendMetal.h"
#endif

namespace matilda::rive {

std::unique_ptr<RiveHeroBackend> createRiveHeroBackend() {
#if defined(MATILDA_RIVE_BACKEND_METAL)
    return std::make_unique<RiveHeroBackendMetal>();
#else
    return std::make_unique<RiveHeroBackendCG>();
#endif
}

} // namespace matilda::rive

namespace {
juce::Image gEmptyRiveFrame;
}

RiveHeroRenderer::RiveHeroRenderer() : backend_(matilda::rive::createRiveHeroBackend()) {}

RiveHeroRenderer::~RiveHeroRenderer() = default;

bool RiveHeroRenderer::loadFromMemory(const void* data, size_t numBytes) {
    return backend_ != nullptr && backend_->loadFromMemory(data, numBytes);
}

void RiveHeroRenderer::setPlaying(bool playing) {
    if (backend_ != nullptr)
        backend_->setPlaying(playing);
}

void RiveHeroRenderer::setActiveLayerCount(int count) {
    if (backend_ != nullptr)
        backend_->setActiveLayerCount(count);
}

void RiveHeroRenderer::setDisplayRect(juce::Rectangle<int> rect) {
    if (backend_ != nullptr)
        backend_->setDisplayRect(rect);
}

void RiveHeroRenderer::setContentAlignRect(juce::Rectangle<int> rect) {
    if (backend_ != nullptr)
        backend_->setContentAlignRect(rect);
}

bool RiveHeroRenderer::tick(float deltaSeconds) {
    return backend_ != nullptr && backend_->tick(deltaSeconds);
}

juce::Component* RiveHeroRenderer::overlayComponent() {
    return backend_ != nullptr ? backend_->overlayComponent() : nullptr;
}

const juce::Image& RiveHeroRenderer::frameImage() const {
    return backend_ != nullptr ? backend_->frameImage() : gEmptyRiveFrame;
}

bool RiveHeroRenderer::isLoaded() const { return backend_ != nullptr && backend_->isLoaded(); }

bool RiveHeroRenderer::hasVisibleFrame() const {
    return backend_ != nullptr && backend_->hasVisibleOutput();
}

#else

const juce::Image& RiveHeroRenderer::frameImage() const {
    static juce::Image empty;
    return empty;
}

#endif
