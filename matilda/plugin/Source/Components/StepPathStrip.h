#pragma once

#include <JuceHeader.h>
#include "../MatildaLookAndFeel.h"

/** Row of 16 step pills — stepic playhead (row-major 0…15), not XYZ. */
class StepPathStrip : public juce::Component {
public:
    explicit StepPathStrip(MatildaLookAndFeel& laf);

    void setPlayhead(int step, int playingLayer, bool noteFired);
    void setLayerColour(juce::Colour c);

private:
    MatildaLookAndFeel& laf_;
    int playheadStep_ = -1;
    int playingLayer_ = 0;
    bool noteFired_ = true;
    juce::Colour layerColour_{ 0xff00e060 };

    void paint(juce::Graphics& g) override;
};
