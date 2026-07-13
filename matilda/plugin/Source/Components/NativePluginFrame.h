#pragma once

#include <JuceHeader.h>
#include "../ReactShellLayout.h"
#include "CollapseToggle.h"
#include "DawSyncToggle.h"
#include "HeroCanvas.h"
#include "MatildaShellPanel.h"

/**
 * Full plugin canvas — React MatildaPluginFrame (collapse clips hero from the left).
 */
class NativePluginFrame : public juce::Component,
                          private juce::Timer {
public:
    NativePluginFrame(MatildaShellPanel& shell);

    void setPreviewScale(float scale);
    [[nodiscard]] float previewScale() const { return previewScale_; }
    [[nodiscard]] bool isCollapsed() const { return collapsed_; }
    [[nodiscard]] bool isAnimating() const { return animating_; }
    [[nodiscard]] juce::Point<int> currentViewportPixelSize() const;

    void setCollapsed(bool collapsed, bool animate = true);
    std::function<void(juce::Point<int> viewportSize)> onViewportSizeChanged;
    std::function<void(bool collapsed)> onCollapsedChanged;

    HeroCanvas& hero() { return hero_; }
    MatildaShellPanel& shell() { return shell_; }
    DawSyncToggle& dawSyncToggle() { return dawSyncToggle_; }

    void setDawSyncVisible(bool visible) { dawSyncToggle_.setVisible(visible); }

private:
    class ContentPanel : public juce::Component {
    public:
        ContentPanel(HeroCanvas& hero, MatildaShellPanel& shell) : hero_(hero), shell_(shell) {
            addAndMakeVisible(hero_);
#if defined(MATILDA_RIVE_HERO)
            addAndMakeVisible(heroWordmark_);
            heroWordmark_.setInterceptsMouseClicks(false, false);
            heroWordmark_.setOpaque(false);
            hero_.onRiveOverlayChanged = [this] { syncContentStackOrder(); };
#endif
            addAndMakeVisible(shell_);
            shell_.setOpaque(true);
        }

        void layoutContent(float previewScale, bool showHero) {
            using namespace matilda::react;
            hero_.setVisible(showHero);
#if defined(MATILDA_RIVE_HERO)
            heroWordmark_.setVisible(showHero);
#endif
            if (!showHero) {
                shell_.setBounds(getLocalBounds());
                return;
            }

            const int heroW = sx(kExpandedW, previewScale);
            const int heroH = sx(kFrameH, previewScale);
            hero_.setBounds(0, 0, heroW, heroH);
#if defined(MATILDA_RIVE_HERO)
            heroWordmark_.setBounds(0, 0, heroW, heroH);
#endif
            shell_.setBounds(sx(kShellLeft, previewScale), sx(kShellTop, previewScale),
                             sx(kShellW, previewScale), sx(kShellH, previewScale));
            syncContentStackOrder();
        }

        void syncContentStackOrder() {
#if defined(MATILDA_RIVE_HERO)
            if (auto* overlay = hero_.riveOverlayComponent())
                overlay->toFront(false);
            heroWordmark_.toFront(false);
#endif
            shell_.toFront(false);
        }

    private:
        HeroCanvas& hero_;
#if defined(MATILDA_RIVE_HERO)
        HeroWordmark heroWordmark_;
#endif
        MatildaShellPanel& shell_;
    };

    float previewScale_ = matilda::react::kPreviewScale;
    bool collapsed_ = false;
    bool animating_ = false;
    float animProgress_ = 0.f;
    float animFromProgress_ = 0.f;
    float animToProgress_ = 0.f;
    double animStartMs_ = 0.;

    HeroCanvas hero_;
    MatildaShellPanel& shell_;
    ContentPanel content_;
    CollapseToggle collapseToggle_;
    DawSyncToggle dawSyncToggle_;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void timerCallback() override;
    void layoutFromProgress(float progress);
    [[nodiscard]] static float collapseEased(float linearT);
    void notifyViewportSize();
};
