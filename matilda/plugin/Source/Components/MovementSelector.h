#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerState.h"
#include "../MatildaLookAndFeel.h"

class MovementSelector : public juce::Component {
public:
    MovementSelector(matilda::PatchState& patch, MatildaLookAndFeel& laf);
    ~MovementSelector() override;

    std::function<void()> onChanged;

    void syncFromPatch();

private:
    class ArrowButton;
    class ModeButton;
    class GlassDropdown;
    class DismissLayer;
    class GlobalClickListener;

    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;

    std::unique_ptr<juce::Drawable> filigreeTop_;
    std::unique_ptr<juce::Drawable> filigreeBottom_;
    juce::Image bgTextureImg_;

    std::unique_ptr<ArrowButton> prev_;
    std::unique_ptr<ArrowButton> next_;
    std::unique_ptr<ModeButton> modeButton_;
    std::unique_ptr<GlassDropdown> dropdown_;
    std::unique_ptr<DismissLayer> dismissLayer_;
    std::unique_ptr<GlobalClickListener> globalClickListener_;

    bool menuOpen_ = false;

    float designScale() const;
    juce::Point<float> designOrigin() const;
    static const juce::StringArray& modeLabels();
    static const juce::StringArray& modeMenuLabels();
    static juce::String barLabelForMode(matilda::MovementMode mode);

    void paint(juce::Graphics& g) override;
    void resized() override;
    void cycleMode(int dir);
    void applyIndex(int index);
    void showMenu(bool show);
    void handleGlobalMouseDown(const juce::MouseEvent& e);
};
