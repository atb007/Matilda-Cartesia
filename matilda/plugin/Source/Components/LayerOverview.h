#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerState.h"
#include "../MatildaLookAndFeel.h"

class LayerOverview : public juce::Component {
public:
    LayerOverview(matilda::PatchState& patch, MatildaLookAndFeel& laf);

    std::function<void(int layer)> onLayerActivated;
    std::function<void(int layer)> onLayerSelected;

    void setPlayingLayer(int layer, int playheadStep);
    void refresh();

private:
    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;
    int playingLayer_ = 0;
    int playheadStep_ = -1;
    std::array<juce::Rectangle<int>, matilda::kLayerCount> layerHitBounds_{};
    std::array<juce::Rectangle<int>, matilda::kLayerCount> toggleHitBounds_{};

    float designScale() const;
    juce::Point<float> designOrigin() const;
    juce::Rectangle<float> cellSlot(float leftPct, float topPx, float scale, juce::Point<float> origin) const;
    void rebuildHitBounds();

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
};
