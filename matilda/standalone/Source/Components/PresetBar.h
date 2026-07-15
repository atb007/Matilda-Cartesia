#pragma once

#include <JuceHeader.h>
#include "../Engine/PresetLibrary.h"
#include "../Engine/SequencerState.h"

class MatildaLookAndFeel;

/** Figma PresetModule (5193:102814) — title + glass name dropdown + save. */
class PresetBar : public juce::Component {
public:
    PresetBar(matilda::PatchState& patch, MatildaLookAndFeel& laf);
    ~PresetBar() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void syncFromPatch();
    void notePatchEdited();
    [[nodiscard]] juce::String currentName() const { return currentName_; }
    [[nodiscard]] bool isDirty() const { return dirty_; }

    std::function<void(const juce::String& name)> onLoadPreset;
    std::function<void()> onSaved;

private:
    class GlassMenu;
    class DismissLayer;
    class GlobalClickListener;

    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;

    juce::String currentName_{ matilda::PresetLibrary::kInitName };
    bool dirty_ = false;
    juce::String cleanJson_;
    bool menuOpen_ = false;

    std::unique_ptr<juce::Drawable> saveIcon_;
    std::unique_ptr<juce::Drawable> chevronsIcon_;
    std::unique_ptr<GlassMenu> menu_;
    std::unique_ptr<DismissLayer> dismissLayer_;
    std::unique_ptr<GlobalClickListener> globalClickListener_;

    juce::Rectangle<float> dropdownBounds_;
    juce::Rectangle<float> saveBounds_;

    [[nodiscard]] float designScale() const;
    [[nodiscard]] juce::String displayLabel() const;
    void markClean();
    void refreshDirtyFlag();
    void showMenu(bool show);
    void handleGlobalMouseDown(const juce::MouseEvent& e);
    void loadPresetAt(int index);
    void runSaveDialog();

    void mouseDown(const juce::MouseEvent& e) override;
};
