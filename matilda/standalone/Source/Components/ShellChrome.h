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
            repaint();
        }
    }

    void paint(juce::Graphics& g) override;

private:
    float previewScale_ = matilda::react::kPreviewScale;
};
