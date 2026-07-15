#pragma once

#include <JuceHeader.h>
#include "../MatildaImages.h"
#include "../ReactShellLayout.h"

/** Glass bedding + vine frame PNG — React ShellGlassBedding + ShellFrameOverlay. */
class ShellChrome : public juce::Component {
public:
    ShellChrome() {
        setInterceptsMouseClicks(false, false);
        setPaintingIsUnclipped(true);
    }

    void setPreviewScale(float scale) {
        if (previewScale_ != scale) {
            previewScale_ = scale;
            frostDirty_ = true;
            repaint();
        }
    }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    float previewScale_ = matilda::react::kPreviewScale;
    juce::Image frostedGlass_;
    juce::Rectangle<int> frostBounds_;
    juce::Rectangle<int> frostSourceInFrame_;
    bool frostDirty_ = true;

    void rebuildFrostedGlass();
    [[nodiscard]] juce::Rectangle<float> glassRect() const;
    [[nodiscard]] juce::Rectangle<int> glassRectInFrame() const;
};
