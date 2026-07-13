#include "RiveHeroRenderer.h"

#if defined(MATILDA_RIVE_HERO)

#if defined(MATILDA_RIVE_BACKEND_METAL)
#include "RiveHeroBackendMetal.h"
#elif defined(MATILDA_RIVE_BACKEND_D3D)
#include "RiveHeroBackendD3D.h"
#elif defined(__APPLE__)
#include "RiveHeroBackendCG.h"
#endif

namespace matilda::rive {

std::unique_ptr<RiveHeroBackend> createRiveHeroBackend() {
#if defined(MATILDA_RIVE_BACKEND_METAL)
    return std::make_unique<RiveHeroBackendMetal>();
#elif defined(MATILDA_RIVE_BACKEND_D3D)
    return std::make_unique<RiveHeroBackendD3D>();
#elif defined(__APPLE__)
    return std::make_unique<RiveHeroBackendCG>();
#else
    return nullptr;
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
