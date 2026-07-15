#include "RiveHeroD3DCore.h"
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
#include "rive/renderer/d3d/d3d_utils.hpp"
#include "rive/renderer/d3d11/render_context_d3d_impl.hpp"
#include "rive/renderer/render_context.hpp"
#include "rive/renderer/rive_renderer.hpp"
#include "rive/renderer/texture.hpp"
#include "rive/scene.hpp"
#include "rive/viewmodel/viewmodel_instance_boolean.hpp"

#include <cstring>
#include <vector>

#define NOMINMAX
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

namespace matilda::rive::d3d {

namespace {

bool succeeded(HRESULT hr) { return SUCCEEDED(hr); }

struct D3DRiveState {
    Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
    Microsoft::WRL::ComPtr<ID3D11Device> device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;
    std::unique_ptr<::rive::gpu::RenderContext> renderContext;
    ::rive::gpu::RenderContextD3DImpl* d3dImpl = nullptr;
    ::rive::rcp<::rive::File> file;
    std::unique_ptr<::rive::ArtboardInstance> artboard;
    std::unique_ptr<::rive::Scene> scene;
    ::rive::rcp<::rive::ViewModelInstance> viewModelInstance;
    ::rive::ViewModelInstanceBoolean* streakVisible = nullptr;
    ::rive::ViewModelInstanceBoolean* bodyStreak = nullptr;
    ::rive::ViewModelInstanceBoolean* bodyGlow = nullptr;
    ::rive::ViewModelInstanceBoolean* faceGlowVis = nullptr;
    ::rive::ViewModelInstanceBoolean* faceStreakVis = nullptr;
    ::rive::rcp<::rive::gpu::RenderTargetD3D> renderTarget;
    bool loaded = false;
    bool playing = false;
    int activeLayerCount = 1;
    bool polyphony = false;
    bool hasFrame = false;
    uint32_t alignWidth = 0;
    uint32_t alignHeight = 0;
    uint32_t targetWidth = 0;
    uint32_t targetHeight = 0;

    bool initDevice() {
        if (device != nullptr && context != nullptr && renderContext != nullptr)
            return true;

        if (factory == nullptr) {
            Microsoft::WRL::ComPtr<IDXGIFactory1> factory1;
            if (!succeeded(CreateDXGIFactory(IID_PPV_ARGS(&factory1))))
                return false;
            if (!succeeded(factory1.As(&factory)))
                return false;
        }

        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        DXGI_ADAPTER_DESC adapterDesc{};
        ::rive::gpu::D3DContextOptions contextOptions;

        for (UINT i = 0; factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            if (!succeeded(adapter->GetDesc(&adapterDesc)))
                continue;
            contextOptions.isIntel = adapterDesc.VendorId == 0x163C || adapterDesc.VendorId == 0x8086
                                     || adapterDesc.VendorId == 0x8087;
            break;
        }

        const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1};
        const HRESULT hr = D3D11CreateDevice(adapter.Get(),
                                             adapter != nullptr ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE,
                                             nullptr,
                                             0,
                                             featureLevels,
                                             static_cast<UINT>(std::size(featureLevels)),
                                             D3D11_SDK_VERSION,
                                             &device,
                                             nullptr,
                                             &context);
        if (!succeeded(hr) || device == nullptr || context == nullptr)
            return false;

        renderContext = ::rive::gpu::RenderContextD3DImpl::MakeContext(device, context, contextOptions);
        if (renderContext == nullptr)
            return false;

        d3dImpl = renderContext->static_impl_cast<::rive::gpu::RenderContextD3DImpl>();
        return d3dImpl != nullptr;
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
        targetWidth = 0;
        targetHeight = 0;

        if (!initDevice() || data == nullptr || numBytes == 0)
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

    bool ensureRenderTarget(uint32_t width, uint32_t height) {
        if (d3dImpl == nullptr || width == 0 || height == 0)
            return false;

        if (renderTarget == nullptr || targetWidth != width || targetHeight != height) {
            renderTarget = d3dImpl->makeRenderTarget(width, height);
            targetWidth = width;
            targetHeight = height;
        }
        return renderTarget != nullptr;
    }

    bool renderBackbuffer(ID3D11Texture2D* backbuffer, uint32_t width, uint32_t height, float deltaSeconds) {
        if (!loaded || backbuffer == nullptr || d3dImpl == nullptr || artboard == nullptr)
            return false;

        if (!ensureRenderTarget(width, height))
            return false;

        renderTarget->setTargetTexture(backbuffer);

        applyDataBindings();
        artboard->advance(0.f);
        if (scene != nullptr)
            scene->advanceAndApply(deltaSeconds);
        else
            artboard->advance(deltaSeconds);

        ::rive::gpu::RenderContext::FrameDescriptor frameDesc;
        frameDesc.renderTargetWidth = width;
        frameDesc.renderTargetHeight = height;
        frameDesc.loadAction = ::rive::gpu::LoadAction::clear;
        frameDesc.clearColor = 0;
        renderContext->beginFrame(frameDesc);

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

        ::rive::gpu::RenderContext::FlushResources flushResources;
        flushResources.renderTarget = renderTarget.get();
        renderContext->flush(flushResources);
        renderTarget->setTargetTexture(nullptr);

        hasFrame = true;
        return true;
    }
};

} // namespace

struct D3DRiveCore::Impl {
    D3DRiveState state;
};

D3DRiveCore::D3DRiveCore() : impl_(std::make_unique<Impl>()) {}

D3DRiveCore::~D3DRiveCore() = default;

bool D3DRiveCore::loadFromMemory(const void* data, size_t numBytes) {
    return impl_ != nullptr && impl_->state.loadBytes(data, numBytes);
}

void D3DRiveCore::setPlaying(bool playing) {
    if (impl_ != nullptr)
        impl_->state.applyPlaying(playing);
}

void D3DRiveCore::setActiveLayerCount(int count) {
    if (impl_ != nullptr)
        impl_->state.applyActiveLayerCount(count);
}

void D3DRiveCore::setPolyphony(bool enabled) {
    if (impl_ != nullptr)
        impl_->state.applyPolyphony(enabled);
}

void D3DRiveCore::setContentAlignSize(uint32_t width, uint32_t height) {
    if (impl_ != nullptr) {
        impl_->state.alignWidth = width;
        impl_->state.alignHeight = height;
    }
}

bool D3DRiveCore::resizeRenderTarget(uint32_t width, uint32_t height) {
    return impl_ != nullptr && impl_->state.ensureRenderTarget(width, height);
}

bool D3DRiveCore::isLoaded() const { return impl_ != nullptr && impl_->state.loaded; }

bool D3DRiveCore::hasRenderedFrame() const { return impl_ != nullptr && impl_->state.hasFrame; }

ID3D11Device* D3DRiveCore::device() const {
    return impl_ != nullptr ? impl_->state.device.Get() : nullptr;
}

IDXGIFactory2* D3DRiveCore::dxgiFactory() const {
    return impl_ != nullptr ? impl_->state.factory.Get() : nullptr;
}

bool D3DRiveCore::render(ID3D11Texture2D* backbuffer,
                         uint32_t width,
                         uint32_t height,
                         float deltaSeconds) {
    return impl_ != nullptr
           && impl_->state.renderBackbuffer(backbuffer, width, height, deltaSeconds);
}

} // namespace matilda::rive::d3d
