#include "HeroCanvas.h"
#include "../MatildaFonts.h"
#include "../MatildaImages.h"
#include "../HeroBackdropDrawing.h"
#include "../ReactShellLayout.h"

#if defined(MATILDA_RIVE_HERO)
#include "../Rive/RiveHeroConfig.h"
#if defined(MATILDA_RIVE_BACKEND_METAL)
#include "../Rive/RiveHeroMetalView.h"
#elif defined(MATILDA_RIVE_BACKEND_D3D)
#include "../Rive/RiveHeroD3DView.h"
#endif
#include "BinaryData.h"
#endif

namespace {

juce::Rectangle<float> portraitRectInHero(float componentWidth, float portraitOffsetX, float portraitOffsetY,
                                            float portraitHeightScale, float portraitContentScale) {
    using namespace matilda::react;

    const float s = componentWidth / kExpandedW;

    const float maskX = (kHeroMainLeft + kHeroMaskLeft) * s;
    const float maskY = kHeroMaskTop * s;

    const float baseW = kHeroPortraitW * s;
    const float baseH = kHeroPortraitH * portraitHeightScale * s;
    const float pw = baseW * portraitContentScale;
    const float ph = baseH * portraitContentScale;

    const float px = maskX + (kHeroPortraitLeft - kHeroMaskLeft + portraitOffsetX) * s;
    const float py = maskY + (kHeroPortraitTop - kHeroMaskTop + portraitOffsetY) * s + (baseH - ph);

    return { px, py, pw, ph };
}

#if defined(MATILDA_RIVE_HERO)

void refreshGpuOverlay(juce::Component* overlay) {
#if defined(MATILDA_RIVE_BACKEND_METAL)
    if (auto* metalView = dynamic_cast<matilda::rive::RiveHeroMetalView*>(overlay))
        metalView->refreshDisplay();
#elif defined(MATILDA_RIVE_BACKEND_D3D)
    if (auto* d3dView = dynamic_cast<matilda::rive::RiveHeroD3DView*>(overlay))
        d3dView->refreshDisplay();
#endif
}

#endif

} // namespace

#if defined(MATILDA_RIVE_HERO)

void HeroWordmark::paint(juce::Graphics& g) {
    using namespace matilda::react;

    if (auto* parent = getParentComponent()) {
        const float s = getWidth() / kExpandedW;

        const int labelLeft = juce::roundToInt(kHeroLabelLeft * s);
        const int labelW = juce::roundToInt(kHeroLabelW * s);
        const int titleTop = juce::roundToInt(kHeroLabelTop * s);
        const int titleH = juce::roundToInt(kHeroTitleLineH * s);

        g.setColour(juce::Colours::white);
        g.setFont(matilda::fonts::jacquard24(kHeroTitleFs * s));
        g.drawText("Matilda", labelLeft, titleTop, labelW, titleH, juce::Justification::right);

        const int subTop = juce::roundToInt((kHeroLabelTop + kHeroTitleLineH + kHeroSubtitleGap) * s);
        const int subH = juce::roundToInt(kHeroSubtitleFs * s);

        g.setColour(juce::Colour(0xffdf90e5));
        g.setFont(matilda::fonts::jacquard24(kHeroSubtitleFs * s));
        g.drawText("Cartesia - v" + juce::String(JucePlugin_VersionString), labelLeft, subTop, labelW, subH,
                   juce::Justification::right);
    }
}

#endif

HeroCanvas::HeroCanvas() {
    setInterceptsMouseClicks(false, false);
#if defined(MATILDA_RIVE_HERO)
    setPaintingIsUnclipped(true);
    juce::MessageManager::callAsync([this] { ensureRiveLoaded(); });
#endif
}

void HeroCanvas::setPlaying(bool playing) {
#if defined(MATILDA_RIVE_HERO)
    if (playing_ == playing)
        return;
    playing_ = playing;
    rive_.setPlaying(playing_);
#if !defined(MATILDA_RIVE_BACKEND_GPU)
    syncRiveTimer();
#endif
    repaintPortraitArea();
#endif
    juce::ignoreUnused(playing);
}

void HeroCanvas::setActiveLayerCount(int count) {
#if defined(MATILDA_RIVE_HERO)
    if (activeLayerCount_ == count)
        return;
    activeLayerCount_ = count;
    rive_.setActiveLayerCount(activeLayerCount_);
    repaintPortraitArea();
#endif
    juce::ignoreUnused(count);
}

#if defined(MATILDA_RIVE_HERO)

juce::Component* HeroCanvas::riveOverlayComponent() {
    return riveLoaded_ ? rive_.overlayComponent() : nullptr;
}

#endif

void HeroCanvas::resized() {
#if defined(MATILDA_RIVE_HERO)
    updatePortraitLayout();
#endif
}

#if defined(MATILDA_RIVE_HERO)

void HeroCanvas::ensureRiveLoaded() {
    if (riveLoaded_)
        return;

    updatePortraitLayout();
    riveLoaded_ = rive_.loadFromMemory(BinaryData::matildacartesiav2_riv,
                                       BinaryData::matildacartesiav2_rivSize);
    if (!riveLoaded_)
        return;

    rive_.setPlaying(playing_);
    rive_.setActiveLayerCount(activeLayerCount_);

    if (auto* overlay = rive_.overlayComponent()) {
        addAndMakeVisible(overlay);
        overlay->setBounds(portraitOverlayRect_);
        overlay->setInterceptsMouseClicks(false, false);
        overlay->toFront(false);
        refreshGpuOverlay(overlay);
    }

    if (onRiveOverlayChanged)
        onRiveOverlayChanged();

#if !defined(MATILDA_RIVE_BACKEND_GPU)
    syncRiveTimer();
    rive_.tick(1.f / static_cast<float>(matilda::rive::kIdleFps));
#endif

    repaintPortraitArea();
}

void HeroCanvas::updatePortraitLayout() {
    using namespace matilda::react;
    using namespace matilda::rive;

    const auto bounds = getLocalBounds().toFloat();
    if (bounds.isEmpty())
        return;

    const float s = bounds.getWidth() / kExpandedW;

    const float maskX = (kHeroMainLeft + kHeroMaskLeft) * s;
    const float maskY = kHeroMaskTop * s;
    const float maskW = kHeroMaskW * s;
    const float maskH = kHeroMaskH * s;
    portraitClipRect_ = juce::Rectangle<int>(juce::roundToInt(maskX), juce::roundToInt(maskY),
                                             juce::roundToInt(maskW), juce::roundToInt(maskH));
    portraitClipRect_ = portraitClipRect_.getIntersection(getLocalBounds());

    const float portraitOffsetX = kPortraitOffsetX;
    const float portraitOffsetY = kPortraitOffsetY;
    const float portraitHeightScale = kPortraitHeightScale;
    const float portraitContentScale = kPortraitContentScale;

    const auto portraitRect = portraitRectInHero(bounds.getWidth(), portraitOffsetX, portraitOffsetY,
                                                 portraitHeightScale, portraitContentScale);
    portraitLocalRect_ = portraitRect.toNearestInt();

#if defined(MATILDA_RIVE_BACKEND_GPU)
    const float maskRight = maskX + maskW;
    const float rightExtend = juce::jmax(0.f, maskRight - portraitRect.getRight());
    portraitOverlayRect_ = portraitLocalRect_;
    portraitOverlayRect_.setWidth(portraitOverlayRect_.getWidth() + juce::roundToInt(rightExtend));
#else
    portraitOverlayRect_ = portraitLocalRect_;
#endif

    rive_.setDisplayRect(portraitOverlayRect_);
    rive_.setContentAlignRect(portraitLocalRect_);

    if (auto* overlay = rive_.overlayComponent()) {
        overlay->setBounds(portraitOverlayRect_);
#if defined(MATILDA_RIVE_BACKEND_GPU)
        overlay->toFront(false);
        refreshGpuOverlay(overlay);
#endif
    }

    if (onRiveOverlayChanged && riveLoaded_)
        onRiveOverlayChanged();
}

#if !defined(MATILDA_RIVE_BACKEND_GPU)

void HeroCanvas::syncRiveTimer() {
    if (!riveLoaded_)
        return;
    const int fps = playing_ ? matilda::rive::kPlayingFps : matilda::rive::kIdleFps;
    startTimerHz(fps);
}

void HeroCanvas::timerCallback() {
    if (!riveLoaded_)
        ensureRiveLoaded();
    if (!riveLoaded_)
        return;

    const float fps = playing_ ? static_cast<float>(matilda::rive::kPlayingFps)
                               : static_cast<float>(matilda::rive::kIdleFps);
    rive_.tick(1.f / fps);
    repaintPortraitArea();
}

#endif

void HeroCanvas::repaintPortraitArea() {
    const auto area = portraitClipRect_.isEmpty() ? getLocalBounds() : portraitClipRect_;
    repaint(area);
}

#if !defined(MATILDA_RIVE_BACKEND_GPU)

bool HeroCanvas::shouldDrawCgRiveFrame() const {
    return rive_.isLoaded() && rive_.hasVisibleFrame() && rive_.frameImage().isValid();
}

#endif

#endif

void HeroCanvas::paint(juce::Graphics& g) {
    using namespace matilda::react;

    const auto bounds = getLocalBounds().toFloat();
    const float s = bounds.getWidth() / kExpandedW;

    matilda::ui::paintHeroCanvasBackground(g, bounds);

    const auto staticPortrait = matilda::images::heroPortrait();
    const float maskX = (kHeroMainLeft + kHeroMaskLeft) * s;
    const float maskY = kHeroMaskTop * s;
    const float maskW = kHeroMaskW * s;
    const float maskH = kHeroMaskH * s;

    g.saveState();
    g.reduceClipRegion(juce::Rectangle<int>(juce::roundToInt(maskX), juce::roundToInt(maskY),
                                            juce::roundToInt(maskW), juce::roundToInt(maskH)));

#if defined(MATILDA_RIVE_HERO)
    using namespace matilda::rive;
    const auto portraitRect =
        portraitRectInHero(bounds.getWidth(), kPortraitOffsetX, kPortraitOffsetY, kPortraitHeightScale,
                           kPortraitContentScale);
#else
    const float portraitOffsetX = 0.f;
    const float portraitOffsetY = 0.f;
    const float portraitHeightScale = 1.1634f;
    const float portraitContentScale = 1.f;
    const auto portraitRect =
        portraitRectInHero(bounds.getWidth(), portraitOffsetX, portraitOffsetY, portraitHeightScale,
                           portraitContentScale);
#endif

#if defined(MATILDA_RIVE_HERO)
#if defined(MATILDA_RIVE_BACKEND_GPU)
    if (!riveLoaded_ && staticPortrait.isValid())
        g.drawImage(staticPortrait, portraitRect);
#else
    if (!shouldDrawCgRiveFrame() && staticPortrait.isValid())
        g.drawImage(staticPortrait, portraitRect);
    else if (shouldDrawCgRiveFrame())
        g.drawImage(rive_.frameImage(), portraitRect);
#endif
#else
    if (staticPortrait.isValid())
        g.drawImage(staticPortrait, portraitRect);
#endif

    g.restoreState();

#if !defined(MATILDA_RIVE_HERO)
    const int labelLeft = juce::roundToInt(kHeroLabelLeft * s);
    const int labelW = juce::roundToInt(kHeroLabelW * s);
    const int titleTop = juce::roundToInt(kHeroLabelTop * s);
    const int titleH = juce::roundToInt(kHeroTitleLineH * s);

    g.setColour(juce::Colours::white);
    g.setFont(matilda::fonts::jacquard24(kHeroTitleFs * s));
    g.drawText("Matilda", labelLeft, titleTop, labelW, titleH, juce::Justification::right);

    const int subTop = juce::roundToInt((kHeroLabelTop + kHeroTitleLineH + kHeroSubtitleGap) * s);
    const int subH = juce::roundToInt(kHeroSubtitleFs * s);

    g.setColour(juce::Colour(0xffdf90e5));
    g.setFont(matilda::fonts::jacquard24(kHeroSubtitleFs * s));
    g.drawText("Cartesia - v1.0", labelLeft, subTop, labelW, subH, juce::Justification::right);
#endif
}
