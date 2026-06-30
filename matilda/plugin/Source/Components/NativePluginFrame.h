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
            addAndMakeVisible(shell_);
        }

        void layoutContent(float previewScale, bool showHero) {
            using namespace matilda::react;
            hero_.setVisible(showHero);
            if (!showHero) {
                shell_.setBounds(getLocalBounds());
                return;
            }

            hero_.setBounds(0, 0, sx(kExpandedW, previewScale), sx(kFrameH, previewScale));
            shell_.setBounds(sx(kShellLeft, previewScale), sx(kShellTop, previewScale),
                             sx(kShellW, previewScale), sx(kShellH, previewScale));
        }

    private:
        HeroCanvas& hero_;
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
