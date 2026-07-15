#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerState.h"
#include "../Engine/LayerClipboard.h"
#include "../MatildaLookAndFeel.h"
#include <array>
#include <memory>

class LayerOverview : public juce::Component {
public:
    LayerOverview(matilda::PatchState& patch, MatildaLookAndFeel& laf);
    ~LayerOverview() override;

    std::function<void(int layer)> onLayerActivated;
    std::function<void(int layer)> onLayerSelected;
    /** Fired after copy/paste/reset/undo mutates layer cell data (or activation via paste). */
    std::function<void(int layer)> onLayerDataChanged;

    /** Mono: one playing layer + step. Poly: per-layer steps (-1 = hidden). */
    void setPlayingLayer(int layer, int playheadStep);
    void setPolyPlayheads(const std::array<int, matilda::kLayerCount>& stepsPerLayer, bool running);
    void refresh();

private:
    class ContextMenu;
    class DismissLayer;
    class GlobalClickListener;
    class FeedbackFloater;

    enum class MenuAction : int {
        CopyNotes = 0,
        CopyNotesAndKnobs,
        PasteNotes,
        PasteNotesAndKnobs,
        ResetValues,
        Undo,
        Count
    };

    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;
    matilda::LayerClipboard clipboard_;
    int playingLayer_ = 0;
    int playheadStep_ = -1;
    bool polyMode_ = false;
    bool transportRunning_ = false;
    std::array<int, matilda::kLayerCount> polySteps_{ -1, -1, -1, -1 };
    std::array<juce::Rectangle<int>, matilda::kLayerCount> layerHitBounds_{};
    std::array<juce::Rectangle<int>, matilda::kLayerCount> toggleHitBounds_{};

    int menuLayer_ = -1;
    bool menuOpen_ = false;
    /** Suppress dismiss while the opening right-click is still down. */
    bool ignoreGlobalClickUntilUp_ = false;
    std::unique_ptr<ContextMenu> menu_;
    std::unique_ptr<DismissLayer> dismissLayer_;
    std::unique_ptr<GlobalClickListener> globalListener_;
    std::unique_ptr<FeedbackFloater> floater_;

    friend class GlobalClickListener;

    float designScale() const;
    juce::Point<float> designOrigin() const;
    juce::Rectangle<float> cellSlot(float leftPct, float topPx, float scale, juce::Point<float> origin) const;
    void rebuildHitBounds();

    [[nodiscard]] int layerAt(juce::Point<int> pos, bool includeInactive) const;
    void showContextMenu(int layer, juce::Point<int> clickInOverview);
    void hideContextMenu();
    void handleMenuAction(MenuAction action);
    void notifyLayerDataChanged(int layer);
    void showFloater(const juce::String& text, juce::Point<int> anchorInThis);
    void handleGlobalMouseDown(const juce::MouseEvent& e);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    [[nodiscard]] bool isOverInteractive(juce::Point<int> pos) const;
};
