#include "HeroCanvas.h"
#include "../MatildaFonts.h"
#include "../MatildaImages.h"
#include "../HeroBackdropDrawing.h"
#include "../ReactShellLayout.h"

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
#include "../Rive/RiveHeroConfig.h"
#if defined(MATILDA_RIVE_BACKEND_METAL)
#include "../Rive/RiveHeroMetalView.h"
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

} // namespace

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)

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
        g.drawText("Cartesia - v1.0", labelLeft, subTop, labelW, subH, juce::Justification::right);
    }
}

#endif

HeroCanvas::HeroCanvas() {
    setInterceptsMouseClicks(false, false);
#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
    setPaintingIsUnclipped(true);
    juce::MessageManager::callAsync([this] { ensureRiveLoaded(); });
#endif
}

void HeroCanvas::setPlaying(bool playing) {
#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
    if (playing_ == playing)
        return;
    playing_ = playing;
    rive_.setPlaying(playing_);
#if !defined(MATILDA_RIVE_BACKEND_METAL)
    syncRiveTimer();
#endif
    repaintPortraitArea();
#endif
    juce::ignoreUnused(playing);
}

void HeroCanvas::setActiveLayerCount(int count) {
#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
    if (activeLayerCount_ == count)
        return;
    activeLayerCount_ = count;
    rive_.setActiveLayerCount(activeLayerCount_);
    repaintPortraitArea();
#endif
    juce::ignoreUnused(count);
}

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)

juce::Component* HeroCanvas::riveOverlayComponent() {
    return riveLoaded_ ? rive_.overlayComponent() : nullptr;
}

#endif

void HeroCanvas::resized() {
#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
    updatePortraitLayout();
#endif
}

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)

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
        if (auto* metalView = dynamic_cast<matilda::rive::RiveHeroMetalView*>(overlay))
            metalView->refreshDisplay();
    }

    if (onRiveOverlayChanged)
        onRiveOverlayChanged();

#if !defined(MATILDA_RIVE_BACKEND_METAL)
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

#if defined(MATILDA_RIVE_BACKEND_METAL)
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
#if defined(MATILDA_RIVE_BACKEND_METAL)
        overlay->toFront(false);
        if (auto* metalView = dynamic_cast<matilda::rive::RiveHeroMetalView*>(overlay))
            metalView->refreshDisplay();
#endif
    }

    if (onRiveOverlayChanged && riveLoaded_)
        onRiveOverlayChanged();
}

#if !defined(MATILDA_RIVE_BACKEND_METAL)

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

#if !defined(MATILDA_RIVE_BACKEND_METAL)

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

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
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

#if defined(MATILDA_RIVE_HERO) && defined(__APPLE__)
#if defined(MATILDA_RIVE_BACKEND_METAL)
    // No static under Metal — overlay appears on first Rive frame only.
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

#if !(defined(MATILDA_RIVE_HERO) && defined(__APPLE__))
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
