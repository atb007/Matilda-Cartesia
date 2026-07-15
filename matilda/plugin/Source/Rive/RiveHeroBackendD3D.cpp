#include "RiveHeroBackendD3D.h"
#include "RiveHeroConfig.h"
#include "RiveHeroD3DCore.h"
#include "RiveHeroD3DView.h"

#include "rive/renderer/d3d/d3d_utils.hpp"

#define NOMINMAX
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <windows.h>

namespace matilda::rive {

namespace {

bool succeeded(HRESULT hr) { return SUCCEEDED(hr); }

constexpr wchar_t kHostWindowClassName[] = L"MatildaRiveD3DHost";

void ensureHostWindowClassRegistered() {
    static bool registered = false;
    if (registered)
        return;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = DefWindowProcW;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kHostWindowClassName;
    RegisterClassExW(&wc);
    registered = true;
}

} // namespace

struct RiveHeroBackendD3D::Impl {
    d3d::D3DRiveCore core;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    uint32_t drawableWidth = 0;
    uint32_t drawableHeight = 0;
};

RiveHeroBackendD3D::RiveHeroBackendD3D()
    : impl_(std::make_unique<Impl>()), view_(std::make_unique<RiveHeroD3DView>(*this)) {}

RiveHeroBackendD3D::~RiveHeroBackendD3D() = default;

bool RiveHeroBackendD3D::loadFromMemory(const void* data, size_t numBytes) {
    const bool ok = impl_->core.loadFromMemory(data, numBytes);
    if (ok)
        view_->attachRiveBytes(data, numBytes);
    return ok;
}

void RiveHeroBackendD3D::setPlaying(bool playing) {
    impl_->core.setPlaying(playing);
    view_->setPlaying(playing);
}

void RiveHeroBackendD3D::setActiveLayerCount(int count) {
    impl_->core.setActiveLayerCount(count);
}

void RiveHeroBackendD3D::setPolyphony(bool enabled) {
    impl_->core.setPolyphony(enabled);
}

void RiveHeroBackendD3D::setDisplayRect(juce::Rectangle<int> rect) {
    if (view_ != nullptr)
        view_->setBounds(rect);
}

void RiveHeroBackendD3D::setContentAlignRect(juce::Rectangle<int> rect) {
    contentAlignW_ = rect.getWidth();
    contentAlignH_ = rect.getHeight();
    if (rect.isEmpty()) {
        contentAlignW_ = 0;
        contentAlignH_ = 0;
    }
}

bool RiveHeroBackendD3D::tick(float) { return impl_->core.hasRenderedFrame(); }

bool RiveHeroBackendD3D::isLoaded() const { return impl_ != nullptr && impl_->core.isLoaded(); }

bool RiveHeroBackendD3D::hasVisibleOutput() const {
    return view_ != nullptr && view_->hasRenderedFrame();
}

juce::Component* RiveHeroBackendD3D::overlayComponent() { return view_.get(); }

namespace {
juce::Image gEmptyImage;
}

const juce::Image& RiveHeroBackendD3D::frameImage() const { return gEmptyImage; }

bool RiveHeroBackendD3D::renderSwapChain(void* hostHwnd, float deltaSeconds) {
    if (impl_ == nullptr || hostHwnd == nullptr)
        return false;

    auto* hwnd = static_cast<HWND>(hostHwnd);
    if (!IsWindow(hwnd))
        return false;

    RECT clientRect{};
    if (!GetClientRect(hwnd, &clientRect))
        return false;

    const int clientW = juce::jmax(1, static_cast<int>(clientRect.right - clientRect.left));
    const int clientH = juce::jmax(1, static_cast<int>(clientRect.bottom - clientRect.top));
    const float scale = view_ != nullptr ? static_cast<float>(view_->getDesktopScaleFactor()) : 1.f;
    const uint32_t drawableW = static_cast<uint32_t>(clientW * scale);
    const uint32_t drawableH = static_cast<uint32_t>(clientH * scale);

    if (view_ != nullptr && contentAlignW_ > 0 && contentAlignH_ > 0) {
        impl_->core.setContentAlignSize(static_cast<uint32_t>(contentAlignW_ * scale),
                                        static_cast<uint32_t>(contentAlignH_ * scale));
    } else {
        impl_->core.setContentAlignSize(0, 0);
    }

    auto* device = impl_->core.device();
    auto* factory = impl_->core.dxgiFactory();
    if (device == nullptr || factory == nullptr)
        return false;

    if (impl_->swapChain == nullptr || impl_->drawableWidth != drawableW
        || impl_->drawableHeight != drawableH) {
        impl_->swapChain.Reset();

        DXGI_SWAP_CHAIN_DESC1 scd{};
        scd.Width = drawableW;
        scd.Height = drawableH;
        scd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.SampleDesc.Count = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
        scd.BufferCount = 2;
        scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        scd.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        scd.Scaling = DXGI_SCALING_STRETCH;

        if (!succeeded(factory->CreateSwapChainForHwnd(device,
                                                       hwnd,
                                                       &scd,
                                                       nullptr,
                                                       nullptr,
                                                       impl_->swapChain.ReleaseAndGetAddressOf())))
            return false;

        impl_->drawableWidth = drawableW;
        impl_->drawableHeight = drawableH;
    }

    if (!impl_->core.resizeRenderTarget(drawableW, drawableH))
        return false;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
    if (!succeeded(impl_->swapChain->GetBuffer(0, IID_PPV_ARGS(&backbuffer))))
        return false;

    if (!impl_->core.render(backbuffer.Get(), drawableW, drawableH, deltaSeconds))
        return false;

    impl_->swapChain->Present(0, 0);
    return true;
}

RiveHeroD3DView::RiveHeroD3DView(RiveHeroBackendD3D& backend) : backend_(backend) {
    setInterceptsMouseClicks(false, false);
    setOpaque(false);
    ensureHostWindowClassRegistered();
    ensureHostWindow();
}

RiveHeroD3DView::~RiveHeroD3DView() {
    stopTimer();
    if (hostHwnd_ != nullptr) {
        DestroyWindow(static_cast<HWND>(hostHwnd_));
        hostHwnd_ = nullptr;
    }
}

void RiveHeroD3DView::setPlaying(bool playing) {
    if (playing_ == playing)
        return;
    playing_ = playing;
    syncTimer();
}

void RiveHeroD3DView::attachRiveBytes(const void* data, size_t numBytes) {
    if (data == nullptr || numBytes == 0)
        return;
    rivBytes_.setSize(numBytes, true);
    rivBytes_.copyFrom(data, 0, numBytes);
    riveReady_ = true;
    refreshDisplay();
}

void RiveHeroD3DView::refreshDisplay() {
    ensureHostWindow();
    updateSwapChainGeometry();
    syncTimer();
}

void RiveHeroD3DView::ensureHostWindow() {
    if (hostHwnd_ != nullptr)
        return;

    if (auto* peer = getPeer()) {
        const auto parentHwnd = static_cast<HWND>(peer->getNativeHandle());
        hostHwnd_ = CreateWindowExW(0,
                                      kHostWindowClassName,
                                      L"",
                                      WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                      0,
                                      0,
                                      1,
                                      1,
                                      parentHwnd,
                                      nullptr,
                                      GetModuleHandleW(nullptr),
                                      nullptr);
        if (hostHwnd_ != nullptr)
            setHWND(hostHwnd_);
    }
}

void RiveHeroD3DView::updateSwapChainGeometry() {
    if (getWidth() <= 0 || getHeight() <= 0)
        return;

    if (hostHwnd_ != nullptr) {
        SetWindowPos(static_cast<HWND>(hostHwnd_),
                     HWND_TOP,
                     0,
                     0,
                     getWidth(),
                     getHeight(),
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
}

void RiveHeroD3DView::resized() {
    updateSwapChainGeometry();
    syncTimer();
}

void RiveHeroD3DView::parentHierarchyChanged() {
    HWNDComponent::parentHierarchyChanged();
    ensureHostWindow();
    refreshDisplay();
}

void RiveHeroD3DView::visibilityChanged() {
    HWNDComponent::visibilityChanged();
    refreshDisplay();
}

void RiveHeroD3DView::syncTimer() {
    stopTimer();
    const bool shouldRun =
        riveReady_ && getParentComponent() != nullptr && (playing_ || kAnimateWhenIdle);
    if (shouldRun) {
        const int fps = playing_ ? kPlayingFps : kIdleFps;
        startTimerHz(fps);
    }
}

void RiveHeroD3DView::timerCallback() {
    if (hostHwnd_ == nullptr)
        ensureHostWindow();
    if (hostHwnd_ == nullptr)
        return;

    const int fps = playing_ ? kPlayingFps : kIdleFps;
    if (backend_.renderSwapChain(hostHwnd_, 1.f / static_cast<float>(fps))) {
        const bool firstFrame = !hasRenderedFrame_;
        hasRenderedFrame_ = true;
        if (firstFrame)
            if (auto* parent = getParentComponent())
                parent->repaint();
    }
}

} // namespace matilda::rive
