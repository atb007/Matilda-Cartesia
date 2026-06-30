#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerState.h"
#include "../MatildaLookAndFeel.h"

class LayerPathView : public juce::Component {
public:
    LayerPathView(matilda::PatchState& patch, MatildaLookAndFeel& laf);

    void setPlayingLayer(int layer, int playX, int playY);
    void refresh();

private:
    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;
    int playingLayer_ = 0;
    int playX_ = 0;
    int playY_ = 0;

    // all cell rendering is procedural – no image members needed

    void paint(juce::Graphics& g) override;
};
