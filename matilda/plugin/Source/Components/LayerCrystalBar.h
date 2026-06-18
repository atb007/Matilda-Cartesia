#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerState.h"
#include "../MatildaLookAndFeel.h"

class LayerCrystalBar : public juce::Component {
public:
    LayerCrystalBar(matilda::PatchState& patch, MatildaLookAndFeel& laf);

    std::function<void(int layer)> onLayerSelected;

    void refresh();

private:
    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;

    juce::Image crystalImg_;
    std::array<juce::Rectangle<int>, matilda::kLayerCount> crystalHits_{};

    [[nodiscard]] std::vector<int> activeLayerIndices() const;
    [[nodiscard]] int hitTestCrystal(juce::Point<int> p) const;

    void paint(juce::Graphics& g) override;
    void mouseDown(const juce::MouseEvent& e) override;
};
