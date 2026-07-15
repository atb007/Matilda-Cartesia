#include "ShellChrome.h"
#include "../ShellGlassDrawing.h"
#include "../HeroBackdropDrawing.h"
#include "../GlassDropdownDrawing.h"

namespace {

juce::Rectangle<float> shellDesignRect(float x, float y, float w, float h, float scale) {
    return {x * scale, y * scale, w * scale, h * scale};
}

} // namespace

juce::Rectangle<float> ShellChrome::glassRect() const {
    using namespace matilda::react;
    return shellDesignRect(kGlassLeft, kGlassTop, kGlassW, kGlassH, previewScale_);
}

juce::Rectangle<int> ShellChrome::glassRectInFrame() const {
    const auto glass = glassRect().toNearestInt();
    auto* shell = getParentComponent();
    auto* content = shell != nullptr ? shell->getParentComponent() : nullptr;
    if (shell == nullptr || content == nullptr)
        return glass;
    return glass + shell->getBounds().getPosition() + content->getBounds().getPosition();
}

void ShellChrome::resized() {
    frostDirty_ = true;
}

void ShellChrome::rebuildFrostedGlass() {
    const auto glass = glassRect().toNearestInt();
    if (glass.isEmpty()) {
        frostedGlass_ = {};
        frostBounds_ = {};
        frostSourceInFrame_ = {};
        frostDirty_ = false;
        return;
    }

    auto* shell = getParentComponent();
    auto* content = shell != nullptr ? shell->getParentComponent() : nullptr;
    auto* frame = content != nullptr ? content->getParentComponent() : nullptr;
    if (frame == nullptr)
        return; // keep dirty — hierarchy not ready yet

    const auto glassInFrame = glassRectInFrame();

    juce::Image snap(juce::Image::ARGB, glass.getWidth(), glass.getHeight(), true);
    {
        juce::Graphics g(snap);
        g.addTransform(juce::AffineTransform::translation(
            static_cast<float>(-glassInFrame.getX()),
            static_cast<float>(-glassInFrame.getY())));
        matilda::ui::paintHeroBackdropCover(g, frame->getLocalBounds());
    }

    // Stronger blur than dropdowns — large control glass should read clearly frosted.
    frostedGlass_ = matilda::ui::glass::blurSnapshot(std::move(snap), 28);
    frostBounds_ = glass;
    frostSourceInFrame_ = glassInFrame;
    frostDirty_ = false;
}

void ShellChrome::paint(juce::Graphics& g) {
    using namespace matilda::react;
    const float s = previewScale_;
    const auto glass = glassRect();
    const auto glassI = glass.toNearestInt();
    const auto glassInFrame = glassRectInFrame();

    if (frostDirty_ || frostBounds_ != glassI || frostSourceInFrame_ != glassInFrame
        || !frostedGlass_.isValid())
        rebuildFrostedGlass();

    // Frost only in the glass panel — no full-shell dark fill (that showed as
    // black edges through the transparent vine frame).
    if (frostedGlass_.isValid())
        g.drawImage(frostedGlass_, glass, juce::RectanglePlacement::stretchToFit);

    matilda::ui::shell::paintGlassBedding(g, glass);
    matilda::ui::shell::paintGlassBeddingRadial(g, glass);

    const auto frameRect =
        shellDesignRect(kFrameOverlayLeft, kFrameOverlayTop, kFrameOverlayW, kFrameOverlayH, s);
    const auto frameImg = matilda::images::shellFrameVines();
    if (frameImg.isValid())
        g.drawImage(frameImg, frameRect, juce::RectanglePlacement::stretchToFit);
}
