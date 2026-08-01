#include "RiveHeroBackendD3D.h"
#include "RiveHeroConfig.h"
#include "RiveHeroD3DCore.h"
#include "RiveHeroD3DLog.h"

#include <JuceHeader.h>

#include <vector>

namespace matilda::rive {

namespace {

// GPU render + CPU readback — keep resolution bounded like the macOS CG path.
inline constexpr float kRenderScale = 0.75f;
inline constexpr int kMaxRenderW = 720;
inline constexpr int kMaxRenderH = 820;

bool imageHasVisiblePixels(const juce::Image& image) {
    if (!image.isValid())
        return false;

    juce::Image::BitmapData data(image, juce::Image::BitmapData::readOnly);
    const int w = image.getWidth();
    const int h = image.getHeight();
    const int step = juce::jmax(1, juce::jmin(w, h) / 24);
    int hits = 0;
    for (int y = h / 4; y < h - h / 4; y += step) {
        for (int x = w / 4; x < w - w / 4; x += step) {
            if (data.getPixelColour(x, y).getAlpha() > 64)
                ++hits;
        }
    }
    return hits >= 8;
}

void blitRgbaToJuceImage(const std::vector<uint8_t>& rgba, int width, int height, juce::Image& image) {
    if (width <= 0 || height <= 0 || rgba.size() < static_cast<size_t>(width) * static_cast<size_t>(height) * 4u)
        return;

    if (!image.isValid() || image.getWidth() != width || image.getHeight() != height)
        image = juce::Image(juce::Image::ARGB, width, height, true);

    juce::Image::BitmapData dest(image, juce::Image::BitmapData::writeOnly);
    for (int y = 0; y < height; ++y) {
        const auto* row = rgba.data() + static_cast<size_t>(y) * static_cast<size_t>(width) * 4u;
        for (int x = 0; x < width; ++x) {
            const uint8_t r = row[x * 4 + 0];
            const uint8_t g = row[x * 4 + 1];
            const uint8_t b = row[x * 4 + 2];
            const uint8_t a = row[x * 4 + 3];
            dest.setPixelColour(x, y, juce::Colour::fromRGBA(r, g, b, a));
        }
    }
}

} // namespace

struct RiveHeroBackendD3D::FrameStorage {
    juce::Image image;
};

struct RiveHeroBackendD3D::Impl {
    d3d::D3DRiveCore core;
    std::vector<uint8_t> rgba;
    bool renderFailureLogged = false;
    bool firstFrameLogged = false;
};

RiveHeroBackendD3D::RiveHeroBackendD3D()
    : impl_(std::make_unique<Impl>()), frameStorage_(std::make_unique<FrameStorage>()) {}

RiveHeroBackendD3D::~RiveHeroBackendD3D() = default;

bool RiveHeroBackendD3D::loadFromMemory(const void* data, size_t numBytes) {
    hasVisibleFrame_ = false;
    frameStorage_->image = juce::Image();
    return impl_->core.loadFromMemory(data, numBytes);
}

void RiveHeroBackendD3D::setPlaying(bool playing) { impl_->core.setPlaying(playing); }

void RiveHeroBackendD3D::setActiveLayerCount(int count) { impl_->core.setActiveLayerCount(count); }

void RiveHeroBackendD3D::setPolyphony(bool enabled) { impl_->core.setPolyphony(enabled); }

void RiveHeroBackendD3D::setDisplayRect(juce::Rectangle<int> rect) {
    if (rect.isEmpty())
        return;

    const int displayW = juce::jmax(1, rect.getWidth());
    const int displayH = juce::jmax(1, rect.getHeight());
    const int renderW =
        juce::jmin(kMaxRenderW, juce::jmax(1, juce::roundToInt(static_cast<float>(displayW) * kRenderScale)));
    const int renderH =
        juce::jmin(kMaxRenderH, juce::jmax(1, juce::roundToInt(static_cast<float>(displayH) * kRenderScale)));
    if (displayW == displayW_ && displayH == displayH_ && renderW == renderW_ && renderH == renderH_)
        return;

    displayW_ = displayW;
    displayH_ = displayH;
    renderW_ = renderW;
    renderH_ = renderH;
    hasVisibleFrame_ = false;
}

void RiveHeroBackendD3D::setContentAlignRect(juce::Rectangle<int> rect) {
    // Metal parity: Cover+CenterLeft uses the portrait content box while the
    // drawable extends further right into the hero mask (hair/streak spill).
    contentAlignW_ = rect.isEmpty() ? 0 : juce::jmax(1, rect.getWidth());
    contentAlignH_ = rect.isEmpty() ? 0 : juce::jmax(1, rect.getHeight());
}

bool RiveHeroBackendD3D::tick(float deltaSeconds) {
    if (impl_ == nullptr || !impl_->core.isLoaded() || renderW_ <= 0 || renderH_ <= 0)
        return false;

    uint32_t alignW = static_cast<uint32_t>(renderW_);
    uint32_t alignH = static_cast<uint32_t>(renderH_);
    if (contentAlignW_ > 0 && contentAlignH_ > 0 && displayW_ > 0 && displayH_ > 0) {
        const float scaleX = static_cast<float>(renderW_) / static_cast<float>(displayW_);
        const float scaleY = static_cast<float>(renderH_) / static_cast<float>(displayH_);
        alignW = static_cast<uint32_t>(juce::jmax(1, juce::roundToInt(static_cast<float>(contentAlignW_) * scaleX)));
        alignH = static_cast<uint32_t>(juce::jmax(1, juce::roundToInt(static_cast<float>(contentAlignH_) * scaleY)));
    }
    impl_->core.setContentAlignSize(alignW, alignH);

    if (!impl_->core.renderToPixels(static_cast<uint32_t>(renderW_),
                                    static_cast<uint32_t>(renderH_),
                                    deltaSeconds,
                                    impl_->rgba)) {
        if (!impl_->renderFailureLogged) {
            d3dLog("offscreen renderToPixels failed");
            impl_->renderFailureLogged = true;
        }
        return false;
    }

    blitRgbaToJuceImage(impl_->rgba, renderW_, renderH_, frameStorage_->image);
    hasVisibleFrame_ = imageHasVisiblePixels(frameStorage_->image);

    if (hasVisibleFrame_ && !impl_->firstFrameLogged) {
        d3dLog("first D3D offscreen frame blitted to juce::Image");
        impl_->firstFrameLogged = true;
    }

    return hasVisibleFrame_;
}

bool RiveHeroBackendD3D::isLoaded() const { return impl_ != nullptr && impl_->core.isLoaded(); }

bool RiveHeroBackendD3D::hasVisibleOutput() const { return hasVisibleFrame_; }

const juce::Image& RiveHeroBackendD3D::frameImage() const {
    static juce::Image empty;
    return frameStorage_ != nullptr && frameStorage_->image.isValid() ? frameStorage_->image : empty;
}

} // namespace matilda::rive
