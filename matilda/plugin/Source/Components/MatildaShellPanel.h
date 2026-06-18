#pragma once

#include <JuceHeader.h>
#include "../ReactShellLayout.h"
#include "../UiDevConfig.h"
#include "ShellChrome.h"
#include "QuantisePanel.h"
#include "LayerOverview.h"
#include "MovementSelector.h"
#include "GemGrid.h"
#include "TransportBar.h"

class MatildaLookAndFeel;

/**
 * React MatildaShell — positions functional modules at Figma-native coordinates.
 * Editor scales the whole panel via kPreviewScale (default 0.52, matching React preview).
 */
class MatildaShellPanel : public juce::Component {
public:
    MatildaShellPanel(matilda::PatchState& patch,
                      matilda::SequencerEngine& engine,
                      MatildaLookAndFeel& laf,
                      TransportBar& transport,
                      QuantisePanel& quantise,
                      LayerOverview& overview,
                      MovementSelector& movement,
                      GemGrid& grid);

    void setPreviewScale(float scale);
    [[nodiscard]] float previewScale() const { return previewScale_; }
    void applyDevView(matilda::ui::DevView view);

    QuantisePanel& quantisePanel() { return quantise_; }
    LayerOverview& layerOverview() { return overview_; }
    MovementSelector& movementSelector() { return movement_; }
    GemGrid& gemGrid() { return grid_; }
    TransportBar& transportBar() { return transport_; }

private:
    matilda::PatchState& patch_;
    matilda::SequencerEngine& engine_;
    MatildaLookAndFeel& laf_;
    float previewScale_ = matilda::react::kPreviewScale;

    ShellChrome chrome_;
    TransportBar& transport_;
    QuantisePanel& quantise_;
    LayerOverview& overview_;
    MovementSelector& movement_;
    GemGrid& grid_;

    void resized() override;
};
