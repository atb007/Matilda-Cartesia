#include "RiveHeroD3DCore.h"
#include "RiveHeroBindings.h"
#include "RiveHeroConfig.h"
#include "RiveHeroD3DLog.h"

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
    Microsoft::WRL::ComPtr<ID3D11Texture2D> colorTexture;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> stagingTexture;
    bool loaded = false;
    bool playing = false;
    int activeLayerCount = 1;
    bool polyphony = false;
    bool hasFrame = false;
    uint32_t alignWidth = 0;
    uint32_t alignHeight = 0;
    uint32_t targetWidth = 0;
    uint32_t targetHeight = 0;

    bool createDevice(IDXGIAdapter* adapter, D3D_DRIVER_TYPE driverType, D3D_FEATURE_LEVEL& createdLevel) {
        const D3D_FEATURE_LEVEL featureLevels[] = {D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0};
        HRESULT hr = D3D11CreateDevice(adapter,
                                       driverType,
                                       nullptr,
                                       0,
                                       featureLevels,
                                       static_cast<UINT>(std::size(featureLevels)),
                                       D3D11_SDK_VERSION,
                                       &device,
                                       &createdLevel,
                                       &context);
        if (hr == E_INVALIDARG) {
            // Pre-11.1 D3D runtimes reject arrays that mention 11_1 — retry with 11_0 only.
            hr = D3D11CreateDevice(adapter,
                                   driverType,
                                   nullptr,
                                   0,
                                   &featureLevels[1],
                                   1,
                                   D3D11_SDK_VERSION,
                                   &device,
                                   &createdLevel,
                                   &context);
        }
        return succeeded(hr) && device != nullptr && context != nullptr;
    }

    bool initDevice() {
        if (device != nullptr && context != nullptr && renderContext != nullptr)
            return true;

        if (factory == nullptr) {
            Microsoft::WRL::ComPtr<IDXGIFactory1> factory1;
            const HRESULT factoryHr = CreateDXGIFactory(IID_PPV_ARGS(&factory1));
            if (!succeeded(factoryHr)) {
                d3dLogHr("CreateDXGIFactory failed", static_cast<long>(factoryHr));
                return false;
            }
            if (!succeeded(factory1.As(&factory))) {
                d3dLog("IDXGIFactory1 -> IDXGIFactory2 cast failed (DXGI 1.2 unavailable)");
                return false;
            }
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

        D3D_FEATURE_LEVEL createdLevel{};
        const D3D_DRIVER_TYPE hwType =
            adapter != nullptr ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
        if (!createDevice(adapter.Get(), hwType, createdLevel)) {
            d3dLog("D3D11CreateDevice hardware failed — trying WARP");
            device.Reset();
            context.Reset();
            if (!createDevice(nullptr, D3D_DRIVER_TYPE_WARP, createdLevel)) {
                d3dLog("D3D11CreateDevice WARP failed");
                return false;
            }
            d3dLog("D3D11 WARP device created");
        }

        {
            char buf[96];
            std::snprintf(buf, sizeof(buf), "D3D11 device created, feature level 0x%04X",
                          static_cast<unsigned>(createdLevel));
            d3dLog(buf);
        }

        renderContext = ::rive::gpu::RenderContextD3DImpl::MakeContext(device, context, contextOptions);
        if (renderContext == nullptr) {
            d3dLog("RenderContextD3DImpl::MakeContext returned null");
            return false;
        }

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
        colorTexture.Reset();
        stagingTexture.Reset();
        targetWidth = 0;
        targetHeight = 0;

        {
            char buf[96];
            std::snprintf(buf, sizeof(buf), "loadBytes: %zu bytes", numBytes);
            d3dLog(buf);
        }

        if (!initDevice() || data == nullptr || numBytes == 0) {
            d3dLog("loadBytes: initDevice failed or empty data");
            return false;
        }

        std::vector<uint8_t> bytes(numBytes);
        std::memcpy(bytes.data(), data, numBytes);

        ::rive::ImportResult result = ::rive::ImportResult::malformed;
        file = ::rive::File::import(bytes, renderContext.get(), &result);
        if (file == nullptr) {
            char buf[96];
            std::snprintf(buf, sizeof(buf), "File::import failed, ImportResult=%d",
                          static_cast<int>(result));
            d3dLog(buf);
            return false;
        }

        artboard = file->artboardNamed(kArtboard);
        if (artboard == nullptr)
            artboard = file->artboardDefault();
        if (artboard == nullptr) {
            d3dLog("loadBytes: no artboard found");
            return false;
        }

        if (!bindViewModel()) {
            d3dLog("loadBytes: bindViewModel failed");
            return false;
        }

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

        d3dLog(scene != nullptr ? "loadBytes: ok (state machine bound)"
                                : "loadBytes: ok (no scene — static artboard)");
        loaded = true;
        return true;
    }

    bool ensureOffscreenTargets(uint32_t width, uint32_t height) {
        if (device == nullptr || d3dImpl == nullptr || width == 0 || height == 0)
            return false;

        if (colorTexture != nullptr && stagingTexture != nullptr && targetWidth == width
            && targetHeight == height && renderTarget != nullptr)
            return true;

        colorTexture.Reset();
        stagingTexture.Reset();
        renderTarget = nullptr;
        targetWidth = 0;
        targetHeight = 0;

        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;

        HRESULT hr = device->CreateTexture2D(&desc, nullptr, colorTexture.ReleaseAndGetAddressOf());
        if (!succeeded(hr) || colorTexture == nullptr) {
            d3dLogHr("CreateTexture2D color RT/UAV failed", static_cast<long>(hr));
            return false;
        }

        desc.BindFlags = 0;
        desc.Usage = D3D11_USAGE_STAGING;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        hr = device->CreateTexture2D(&desc, nullptr, stagingTexture.ReleaseAndGetAddressOf());
        if (!succeeded(hr) || stagingTexture == nullptr) {
            d3dLogHr("CreateTexture2D staging failed", static_cast<long>(hr));
            colorTexture.Reset();
            return false;
        }

        renderTarget = d3dImpl->makeRenderTarget(width, height);
        if (renderTarget == nullptr) {
            d3dLog("makeRenderTarget returned null");
            colorTexture.Reset();
            stagingTexture.Reset();
            return false;
        }

        targetWidth = width;
        targetHeight = height;
        return true;
    }

    bool renderAndReadback(uint32_t width, uint32_t height, float deltaSeconds, std::vector<uint8_t>& rgbaOut) {
        if (!loaded || d3dImpl == nullptr || artboard == nullptr || context == nullptr)
            return false;

        if (!ensureOffscreenTargets(width, height))
            return false;

        renderTarget->setTargetTexture(colorTexture);

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

        context->CopyResource(stagingTexture.Get(), colorTexture.Get());

        D3D11_MAPPED_SUBRESOURCE mapped{};
        const HRESULT mapHr = context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mapped);
        if (!succeeded(mapHr) || mapped.pData == nullptr) {
            d3dLogHr("Map staging texture failed", static_cast<long>(mapHr));
            return false;
        }

        const size_t tightPitch = static_cast<size_t>(width) * 4u;
        rgbaOut.resize(tightPitch * static_cast<size_t>(height));
        auto* dst = rgbaOut.data();
        auto* src = static_cast<const uint8_t*>(mapped.pData);
        for (uint32_t y = 0; y < height; ++y) {
            std::memcpy(dst + y * tightPitch, src + y * mapped.RowPitch, tightPitch);
        }
        context->Unmap(stagingTexture.Get(), 0);

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

bool D3DRiveCore::renderToPixels(uint32_t width,
                                 uint32_t height,
                                 float deltaSeconds,
                                 std::vector<uint8_t>& rgbaOut) {
    return impl_ != nullptr && impl_->state.renderAndReadback(width, height, deltaSeconds, rgbaOut);
}

bool D3DRiveCore::isLoaded() const { return impl_ != nullptr && impl_->state.loaded; }

bool D3DRiveCore::hasRenderedFrame() const { return impl_ != nullptr && impl_->state.hasFrame; }

} // namespace matilda::rive::d3d
