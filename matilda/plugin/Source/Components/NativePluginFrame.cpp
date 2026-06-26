#include "NativePluginFrame.h"
#include "../HeroBackdropDrawing.h"
#include "../UiDevConfig.h"

namespace {

float solveBezierTForX(float x, float x1, float x2) {
    float t = x;
    for (int i = 0; i < 10; ++i) {
        const float t2 = t * t;
        const float t3 = t2 * t;
        const float mt = 1.f - t;
        const float mt2 = mt * mt;
        const float xAtT = 3.f * mt2 * t * x1 + 3.f * mt * t2 * x2 + t3;
        const float dx = 3.f * mt2 * x1 + 6.f * mt * t * (x2 - x1) + 3.f * t2 * (1.f - x2);
        if (std::abs(dx) < 1e-5f)
            break;
        t -= (xAtT - x) / dx;
        t = juce::jlimit(0.f, 1.f, t);
    }
    return t;
}

float bezierY(float t, float y1, float y2) {
    const float mt = 1.f - t;
    return 3.f * mt * mt * t * y1 + 3.f * mt * t * t * y2 + t * t * t;
}

} // namespace

NativePluginFrame::NativePluginFrame(MatildaShellPanel& shell)
    : shell_(shell), content_(hero_, shell) {
    addAndMakeVisible(content_);
    addAndMakeVisible(collapseToggle_);
    addAndMakeVisible(dawSyncToggle_);
    setPaintingIsUnclipped(false);

    collapseToggle_.onToggle = [this] { setCollapsed(!collapsed_, true); };
}

juce::Point<int> NativePluginFrame::currentViewportPixelSize() const {
    using namespace matilda::react;
    const float viewportW = juce::jmap(animProgress_, kExpandedW, kCollapsedW);
    return { sx(viewportW, previewScale_), sx(kFrameH, previewScale_) };
}

void NativePluginFrame::paint(juce::Graphics& g) {
    matilda::ui::paintHeroBackdropCover(g, getLocalBounds());
}

void NativePluginFrame::setPreviewScale(float scale) {
    previewScale_ = scale;
    shell_.setPreviewScale(scale);
    layoutFromProgress(animProgress_);
    notifyViewportSize();
}

void NativePluginFrame::setCollapsed(bool collapsed, bool animate) {
    if (collapsed_ == collapsed && !animating_)
        return;

    collapsed_ = collapsed;
    collapseToggle_.setCollapsed(collapsed);

    const float target = collapsed ? 1.f : 0.f;
    if (!animate || matilda::ui::devIsolatedModule()) {
        stopTimer();
        animating_ = false;
        animProgress_ = target;
        layoutFromProgress(animProgress_);
        notifyViewportSize();
        return;
    }

    animFromProgress_ = animProgress_;
    animToProgress_ = target;
    animStartMs_ = juce::Time::getMillisecondCounterHiRes();
    animating_ = true;
    startTimerHz(60);
}

float NativePluginFrame::collapseEased(float linearT) {
    const float t = solveBezierTForX(linearT, 0.4f, 0.2f);
    return bezierY(t, 0.f, 1.f);
}

void NativePluginFrame::timerCallback() {
    const double elapsed = juce::Time::getMillisecondCounterHiRes() - animStartMs_;
    float linearT = static_cast<float>(elapsed / static_cast<double>(matilda::react::kCollapseMs));
    if (linearT >= 1.f) {
        linearT = 1.f;
        stopTimer();
        animating_ = false;
    }

    const float eased = collapseEased(linearT);
    animProgress_ = juce::jmap(eased, animFromProgress_, animToProgress_);
    layoutFromProgress(animProgress_);
    notifyViewportSize();
}

void NativePluginFrame::layoutFromProgress(float progress) {
    using namespace matilda::react;

    if (!hero_.isVisible()) {
        content_.setBounds(getLocalBounds());
        content_.layoutContent(previewScale_, false);
        collapseToggle_.setVisible(false);
        dawSyncToggle_.setVisible(false);
        return;
    }

    collapseToggle_.setVisible(true);

    const float viewportW = juce::jmap(progress, kExpandedW, kCollapsedW);
    const float contentOffset = viewportContentOffset(viewportW);

    const float iconCanvasX =
        juce::jmap(progress, kIconExpandedLeft, kShellLeft + kIconCollapsedInsetLeft);
    const float iconViewportX = iconCanvasX + contentOffset;

    const float dawSyncViewportX = kDawSyncExpandedLeft + contentOffset;

    const int offsetPx = sx(contentOffset, previewScale_);
    content_.setBounds(offsetPx, 0, sx(kExpandedW, previewScale_), sx(kFrameH, previewScale_));
    content_.layoutContent(previewScale_, hero_.isVisible());

    collapseToggle_.setBounds(sx(iconViewportX, previewScale_), sx(kIconExpandedTop, previewScale_),
                              sx(kIconSize, previewScale_), sx(kIconSize, previewScale_));
    dawSyncToggle_.setBounds(sx(dawSyncViewportX, previewScale_), sx(kIconExpandedTop, previewScale_),
                             sx(kIconSize, previewScale_), sx(kIconSize, previewScale_));
    collapseToggle_.toFront(false);
    dawSyncToggle_.toFront(false);
}

void NativePluginFrame::notifyViewportSize() {
    if (!hero_.isVisible())
        return;

    using namespace matilda::react;

    const float viewportW = juce::jmap(animProgress_, kExpandedW, kCollapsedW);
    const auto size = juce::Point<int>(sx(viewportW, previewScale_), sx(kFrameH, previewScale_));

    setSize(size.x, size.y);

    if (onViewportSizeChanged) {
        onViewportSizeChanged(size);
    } else if (auto* parent = getParentComponent()) {
        if (parent->getWidth() != size.x || parent->getHeight() != size.y)
            parent->setSize(size.x, size.y);
    }
}

void NativePluginFrame::resized() {
    layoutFromProgress(animProgress_);
}
