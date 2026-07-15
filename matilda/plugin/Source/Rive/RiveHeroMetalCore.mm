#include "RiveHeroMetalCore.h"
#include "RiveHeroBindings.h"
#include "RiveHeroConfig.h"

#include "rive/animation/linear_animation_instance.hpp"
#include "rive/animation/state_machine_instance.hpp"
#include "rive/artboard.hpp"
#include "rive/file.hpp"
#include "rive/layout.hpp"
#include "rive/math/aabb.hpp"
#include "rive/math/mat2d.hpp"
#include "rive/refcnt.hpp"
#include "rive/renderer/metal/render_context_metal_impl.h"
#include "rive/renderer/render_context.hpp"
#include "rive/renderer/rive_renderer.hpp"
#include "rive/scene.hpp"
#include "rive/viewmodel/viewmodel_instance_boolean.hpp"

#include <cstring>
#include <vector>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

namespace matilda::rive::metal {

namespace {

struct MetalRiveState {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    id<MTLCommandQueue> queue = nil;
    std::unique_ptr<::rive::gpu::RenderContext> renderContext;
    ::rive::gpu::RenderContextMetalImpl* metalImpl = nullptr;
    ::rive::rcp<::rive::File> file;
    std::unique_ptr<::rive::ArtboardInstance> artboard;
    std::unique_ptr<::rive::Scene> scene;
    ::rive::rcp<::rive::ViewModelInstance> viewModelInstance;
    ::rive::ViewModelInstanceBoolean* streakVisible = nullptr;
    ::rive::ViewModelInstanceBoolean* bodyStreak = nullptr;
    ::rive::ViewModelInstanceBoolean* bodyGlow = nullptr;
    ::rive::ViewModelInstanceBoolean* faceGlowVis = nullptr;
    ::rive::ViewModelInstanceBoolean* faceStreakVis = nullptr;
    ::rive::rcp<::rive::gpu::RenderTargetMetal> renderTarget;
    bool loaded = false;
    bool playing = false;
    int activeLayerCount = 1;
    bool polyphony = false;
    bool hasFrame = false;
    uint32_t alignWidth = 0;
    uint32_t alignHeight = 0;

    bool initQueue() {
        if (device == nil)
            return false;
        if (queue == nil)
            queue = [device newCommandQueue];

        if (renderContext == nullptr) {
            ::rive::gpu::RenderContextMetalImpl::ContextOptions options;
            renderContext = ::rive::gpu::RenderContextMetalImpl::MakeContext(device, options);
            metalImpl = renderContext->static_impl_cast<::rive::gpu::RenderContextMetalImpl>();
            metalImpl->setCommandQueue(queue);
        }
        return renderContext != nullptr;
    }

    bool bindViewModel() {
        if (file == nullptr || artboard == nullptr)
            return false;

        if (auto* vm = file->viewModel(kViewModel))
            viewModelInstance = file->createDefaultViewModelInstance(vm);
        if (viewModelInstance == nullptr)
            viewModelInstance = file->createDefaultViewModelInstance(artboard.get());
        if (viewModelInstance == nullptr)
            return false;

        file->completeViewModelInstance(viewModelInstance);
        artboard->bindViewModelInstance(viewModelInstance);

        if (auto* prop = viewModelInstance->propertyValue(kPlayBoolean))
            streakVisible = prop->as<::rive::ViewModelInstanceBoolean>();
        if (auto* prop = viewModelInstance->propertyValue(kBodyStreakBoolean))
            bodyStreak = prop->as<::rive::ViewModelInstanceBoolean>();
        if (auto* prop = viewModelInstance->propertyValue(kBodyGlowBoolean))
            bodyGlow = prop->as<::rive::ViewModelInstanceBoolean>();
        if (auto* prop = viewModelInstance->propertyValue(kFaceGlowVisBoolean))
            faceGlowVis = prop->as<::rive::ViewModelInstanceBoolean>();
        if (auto* prop = viewModelInstance->propertyValue(kFaceStreakVisBoolean))
            faceStreakVis = prop->as<::rive::ViewModelInstanceBoolean>();

        return true;
    }

    void applyDataBindings() {
        if (streakVisible != nullptr)
            streakVisible->propertyValue(playing);

        const auto glow = layerGlowForTransport(playing, activeLayerCount, polyphony);
        if (bodyStreak != nullptr)
            bodyStreak->propertyValue(glow.bodyStreak);
        if (bodyGlow != nullptr)
            bodyGlow->propertyValue(glow.bodyGlow);
        if (faceGlowVis != nullptr)
            faceGlowVis->propertyValue(glow.faceGlowVis);
        if (faceStreakVis != nullptr)
            faceStreakVis->propertyValue(glow.faceStreakVis);
    }

    void applyPlaying(bool playingState) {
        playing = playingState;
        applyDataBindings();
    }

    void applyActiveLayerCount(int count) {
        activeLayerCount = count;
        applyDataBindings();
    }

    void applyPolyphony(bool enabled) {
        polyphony = enabled;
        applyDataBindings();
    }

    bool loadBytes(const void* data, size_t numBytes) {
        loaded = false;
        hasFrame = false;
        file = nullptr;
        artboard.reset();
        scene.reset();
        viewModelInstance = nullptr;
        streakVisible = nullptr;
        bodyStreak = nullptr;
        bodyGlow = nullptr;
        faceGlowVis = nullptr;
        faceStreakVis = nullptr;
        renderTarget = nullptr;

        if (!initQueue() || data == nullptr || numBytes == 0)
            return false;

        std::vector<uint8_t> bytes(numBytes);
        std::memcpy(bytes.data(), data, numBytes);

        ::rive::ImportResult result = ::rive::ImportResult::malformed;
        file = ::rive::File::import(bytes, renderContext.get(), &result);
        if (file == nullptr)
            return false;

        artboard = file->artboardNamed(kArtboard);
        if (artboard == nullptr)
            artboard = file->artboardDefault();
        if (artboard == nullptr)
            return false;

        if (!bindViewModel())
            return false;

        scene = artboard->defaultStateMachine();
        if (scene == nullptr)
            scene = artboard->animationAt(0);
        if (scene != nullptr && viewModelInstance != nullptr)
            scene->bindViewModelInstance(viewModelInstance);

        applyPlaying(playing);
        if (scene != nullptr)
            scene->advanceAndApply(0.f);
        else
            artboard->advance(0.f);

        loaded = true;
        return true;
    }

    bool render(CAMetalLayer* layer, float deltaSeconds) {
        if (!loaded || layer == nil || metalImpl == nullptr || artboard == nullptr)
            return false;

        layer.device = device;
        layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
        layer.opaque = NO;
        layer.backgroundColor = CGColorGetConstantColor(kCGColorClear);

        id<CAMetalDrawable> drawable = [layer nextDrawable];
        if (drawable == nil)
            return false;

        const uint32_t width = static_cast<uint32_t>(layer.drawableSize.width);
        const uint32_t height = static_cast<uint32_t>(layer.drawableSize.height);
        if (width == 0 || height == 0)
            return false;

        if (renderTarget == nullptr || renderTarget->width() != width || renderTarget->height() != height)
            renderTarget = metalImpl->makeRenderTarget(MTLPixelFormatBGRA8Unorm, width, height);
        renderTarget->setTargetTexture(drawable.texture);

        applyDataBindings();
        artboard->advance(0.f);
        if (scene != nullptr)
            scene->advanceAndApply(deltaSeconds);
        else
            artboard->advance(deltaSeconds);

        renderContext->beginFrame({
            .renderTargetWidth = width,
            .renderTargetHeight = height,
            .loadAction = ::rive::gpu::LoadAction::clear,
            .clearColor = 0,
        });

        auto renderer = std::make_unique<::rive::RiveRenderer>(renderContext.get());
        renderer->save();
        const float alignW =
            alignWidth > 0 ? static_cast<float>(alignWidth) : static_cast<float>(width);
        const float alignH =
            alignHeight > 0 ? static_cast<float>(alignHeight) : static_cast<float>(height);
        renderer->align(::rive::Fit::cover,
                        ::rive::Alignment::centerLeft,
                        ::rive::AABB(0.f, 0.f, alignW, alignH),
                        artboard->bounds());
        if (kGpuRenderPanX != 0.f || kGpuRenderPanY != 0.f)
            renderer->transform(::rive::Mat2D::fromTranslate(kGpuRenderPanX, kGpuRenderPanY));
        artboard->draw(renderer.get());
        renderer->restore();

        id<MTLCommandBuffer> commandBuffer = [queue commandBuffer];
        renderContext->flush({
            .renderTarget = renderTarget.get(),
            .externalCommandBuffer = (__bridge void*) commandBuffer,
        });
        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];

        hasFrame = true;
        return true;
    }
};

} // namespace

struct MetalRiveCore::Impl {
    MetalRiveState state;
};

MetalRiveCore::MetalRiveCore() : impl_(std::make_unique<Impl>()) {}

MetalRiveCore::~MetalRiveCore() = default;

bool MetalRiveCore::loadFromMemory(const void* data, size_t numBytes) {
    return impl_ != nullptr && impl_->state.loadBytes(data, numBytes);
}

void MetalRiveCore::setPlaying(bool playing) {
    if (impl_ != nullptr)
        impl_->state.applyPlaying(playing);
}

void MetalRiveCore::setActiveLayerCount(int count) {
    if (impl_ != nullptr)
        impl_->state.applyActiveLayerCount(count);
}

void MetalRiveCore::setPolyphony(bool enabled) {
    if (impl_ != nullptr)
        impl_->state.applyPolyphony(enabled);
}

void MetalRiveCore::setContentAlignSize(uint32_t width, uint32_t height) {
    if (impl_ != nullptr) {
        impl_->state.alignWidth = width;
        impl_->state.alignHeight = height;
    }
}

bool MetalRiveCore::isLoaded() const { return impl_ != nullptr && impl_->state.loaded; }

bool MetalRiveCore::hasRenderedFrame() const { return impl_ != nullptr && impl_->state.hasFrame; }

bool MetalRiveCore::render(void* cametalLayer, float deltaSeconds) {
    return impl_ != nullptr && cametalLayer != nullptr
           && impl_->state.render((__bridge CAMetalLayer*) cametalLayer, deltaSeconds);
}

} // namespace matilda::rive::metal
