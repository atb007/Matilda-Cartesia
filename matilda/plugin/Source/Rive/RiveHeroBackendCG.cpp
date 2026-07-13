#include "RiveHeroBackendCG.h"

#include "RiveHeroBindings.h"
#include "RiveHeroConfig.h"

#include "cg_factory.hpp"
#include "cg_renderer.hpp"
#include "rive/artboard.hpp"
#include "rive/file.hpp"
#include "rive/layout.hpp"
#include "rive/math/aabb.hpp"
#include "rive/refcnt.hpp"
#include "rive/scene.hpp"
#include "rive/viewmodel/viewmodel_instance_boolean.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

namespace matilda::rive {

namespace {

inline constexpr float kRenderScale = 0.55f;
inline constexpr int kMaxRenderW = 720;
inline constexpr int kMaxRenderH = 820;

} // namespace

struct RiveHeroBackendCG::FrameStorage {
    juce::Image image;
};

struct RiveHeroBackendCG::Impl {
    rive::CGFactory factory;
    rive::rcp<rive::File> file;
    std::unique_ptr<rive::ArtboardInstance> artboard;
    std::unique_ptr<rive::Scene> scene;
    rive::rcp<rive::ViewModelInstance> viewModelInstance;
    rive::ViewModelInstanceBoolean* streakVisible = nullptr;
    rive::ViewModelInstanceBoolean* bodyStreak = nullptr;
    rive::ViewModelInstanceBoolean* bodyGlow = nullptr;
    rive::ViewModelInstanceBoolean* faceGlowVis = nullptr;
    rive::ViewModelInstanceBoolean* faceStreakVis = nullptr;

    std::vector<uint32_t> pixels;
    AutoCF<CGColorSpaceRef> colorSpace;
    AutoCF<CGContextRef> ctx;
    int width = 0;
    int height = 0;

    void ensureContext(int w, int h) {
        if (w <= 0 || h <= 0)
            return;
        if (w == width && h == height && ctx != nullptr)
            return;

        width = w;
        height = h;
        pixels.assign(static_cast<size_t>(w) * static_cast<size_t>(h), 0);

        if (colorSpace == nullptr)
            colorSpace.reset(CGColorSpaceCreateDeviceRGB());

        const auto info = static_cast<uint32_t>(kCGBitmapByteOrder32Big)
                          | static_cast<uint32_t>(kCGImageAlphaLast);
        ctx.reset(CGBitmapContextCreate(pixels.data(),
                                        static_cast<size_t>(w),
                                        static_cast<size_t>(h),
                                        8,
                                        static_cast<size_t>(w) * 4,
                                        colorSpace,
                                        info));
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
            streakVisible = prop->as<rive::ViewModelInstanceBoolean>();
        if (auto* prop = viewModelInstance->propertyValue(kBodyStreakBoolean))
            bodyStreak = prop->as<rive::ViewModelInstanceBoolean>();
        if (auto* prop = viewModelInstance->propertyValue(kBodyGlowBoolean))
            bodyGlow = prop->as<rive::ViewModelInstanceBoolean>();
        if (auto* prop = viewModelInstance->propertyValue(kFaceGlowVisBoolean))
            faceGlowVis = prop->as<rive::ViewModelInstanceBoolean>();
        if (auto* prop = viewModelInstance->propertyValue(kFaceStreakVisBoolean))
            faceStreakVis = prop->as<rive::ViewModelInstanceBoolean>();

        return true;
    }

    void applyDataBindings(bool playing, int activeLayerCount) {
        if (streakVisible != nullptr)
            streakVisible->propertyValue(playing);

        const auto glow = layerGlowForTransport(playing, activeLayerCount);
        if (bodyStreak != nullptr)
            bodyStreak->propertyValue(glow.bodyStreak);
        if (bodyGlow != nullptr)
            bodyGlow->propertyValue(glow.bodyGlow);
        if (faceGlowVis != nullptr)
            faceGlowVis->propertyValue(glow.faceGlowVis);
        if (faceStreakVis != nullptr)
            faceStreakVis->propertyValue(glow.faceStreakVis);
    }

    void applyPlaying(bool playing, int activeLayerCount) {
        applyDataBindings(playing, activeLayerCount);
    }

    void renderFrame() {
        if (ctx == nullptr || artboard == nullptr)
            return;

        std::fill(pixels.begin(), pixels.end(), 0u);

        if (artboard->width() <= 0.f || artboard->height() <= 0.f)
            return;

        auto renderer = std::make_unique<rive::CGRenderer>(ctx, width, height);
        renderer->save();
        renderer->align(rive::Fit::cover,
                        rive::Alignment::centerLeft,
                        rive::AABB(0.f, 0.f, static_cast<float>(width), static_cast<float>(height)),
                        artboard->bounds());
        artboard->draw(renderer.get());
        renderer->restore();
        CGContextFlush(ctx);
    }

    void blitToJuceImage(juce::Image& image) {
        if (width <= 0 || height <= 0)
            return;

        if (!image.isValid() || image.getWidth() != width || image.getHeight() != height)
            image = juce::Image(juce::Image::ARGB, width, height, true);

        juce::Image::BitmapData dest(image, juce::Image::BitmapData::writeOnly);
        for (int y = 0; y < height; ++y) {
            const auto* row =
                reinterpret_cast<const uint8_t*>(pixels.data() + static_cast<size_t>(y) * static_cast<size_t>(width));
            auto* dst = dest.getLinePointer(y);
            for (int x = 0; x < width; ++x) {
                const uint8_t r = row[x * 4 + 0];
                const uint8_t g = row[x * 4 + 1];
                const uint8_t b = row[x * 4 + 2];
                const uint8_t a = row[x * 4 + 3];
                dst[x] = juce::PixelARGB(a, r, g, b).getInARGBMaskOrder();
            }
        }
    }

    static bool imageHasVisiblePixels(const juce::Image& image) {
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
};

RiveHeroBackendCG::RiveHeroBackendCG()
    : impl_(std::make_unique<Impl>()), frameStorage_(std::make_unique<FrameStorage>()) {}

RiveHeroBackendCG::~RiveHeroBackendCG() = default;

const juce::Image& RiveHeroBackendCG::frameImage() const {
    static juce::Image empty;
    return frameStorage_ != nullptr && frameStorage_->image.isValid() ? frameStorage_->image : empty;
}

bool RiveHeroBackendCG::loadFromMemory(const void* data, size_t numBytes) {
    loaded_ = false;
    hasVisibleFrame_ = false;
    frameStorage_->image = juce::Image();

    if (data == nullptr || numBytes == 0)
        return false;

    std::vector<uint8_t> bytes(numBytes);
    std::memcpy(bytes.data(), data, numBytes);

    rive::ImportResult result = rive::ImportResult::malformed;
    impl_->file = rive::File::import(bytes, &impl_->factory, &result);
    if (impl_->file == nullptr)
        return false;

    impl_->artboard = impl_->file->artboardNamed(kArtboard);
    if (impl_->artboard == nullptr)
        impl_->artboard = impl_->file->artboardDefault();
    if (impl_->artboard == nullptr)
        return false;

    if (!impl_->bindViewModel())
        return false;

    impl_->scene = impl_->artboard->defaultStateMachine();
    if (impl_->scene == nullptr)
        impl_->scene = impl_->artboard->animationAt(0);
    if (impl_->scene != nullptr && impl_->viewModelInstance != nullptr)
        impl_->scene->bindViewModelInstance(impl_->viewModelInstance);

    impl_->applyPlaying(false, activeLayerCount_);
    if (impl_->scene != nullptr)
        impl_->scene->advanceAndApply(0.f);
    else
        impl_->artboard->advance(0.f);

    loaded_ = true;
    sizeDirty_ = true;
    return true;
}

void RiveHeroBackendCG::setPlaying(bool playing) {
    playing_ = playing;
    impl_->applyPlaying(playing_, activeLayerCount_);
}

void RiveHeroBackendCG::setActiveLayerCount(int count) {
    activeLayerCount_ = count;
    impl_->applyPlaying(playing_, activeLayerCount_);
}

void RiveHeroBackendCG::setDisplayRect(juce::Rectangle<int> rect) {
    if (rect.isEmpty())
        return;

    const int renderW = juce::jmin(kMaxRenderW, juce::jmax(1, juce::roundToInt(static_cast<float>(rect.getWidth()) * kRenderScale)));
    const int renderH = juce::jmin(kMaxRenderH, juce::jmax(1, juce::roundToInt(static_cast<float>(rect.getHeight()) * kRenderScale)));
    if (renderW == renderW_ && renderH == renderH_)
        return;
    renderW_ = renderW;
    renderH_ = renderH;
    sizeDirty_ = true;
    hasVisibleFrame_ = false;
}

bool RiveHeroBackendCG::tick(float deltaSeconds) {
    if (!loaded_ || impl_ == nullptr)
        return false;

    if (sizeDirty_ && renderW_ > 0 && renderH_ > 0) {
        impl_->ensureContext(renderW_, renderH_);
        sizeDirty_ = false;
    }

    if (impl_->ctx == nullptr)
        return false;

    impl_->applyPlaying(playing_, activeLayerCount_);
    impl_->artboard->advance(0.f);

    if (impl_->scene != nullptr)
        impl_->scene->advanceAndApply(deltaSeconds);
    else
        impl_->artboard->advance(deltaSeconds);

    impl_->renderFrame();
    impl_->blitToJuceImage(frameStorage_->image);
    hasVisibleFrame_ = Impl::imageHasVisiblePixels(frameStorage_->image);
    return hasVisibleFrame_;
}

} // namespace matilda::rive
