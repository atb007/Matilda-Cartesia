#pragma once

#include <JuceHeader.h>
#include "../Engine/SequencerState.h"
#include "../MatildaLookAndFeel.h"

class TransportBar : public juce::Component {
public:
    TransportBar(matilda::PatchState& patch, MatildaLookAndFeel& laf);
    ~TransportBar() override;

    std::function<void()> onPlay;
    std::function<void()> onStop;
    std::function<void()> onSettingsChanged;

    void setPlaying(bool playing);
    void syncFromPatch();

private:
    enum class MenuId { None, PlayMode, Clock };

    class PlayButton;
    class SettingRow;
    class GlassMenu;
    class DismissLayer;
    class GlobalClickListener;

    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;
    bool playing_ = false;
    MenuId openMenu_ = MenuId::None;

    juce::Image filigreeTopImg_;
    juce::Image filigreeBottomImg_;
    juce::Image bgTextureImg_;
    juce::Image sectionOrnLeftImg_;
    juce::Image sectionOrnRightImg_;

    std::unique_ptr<PlayButton> playButton_;
    std::unique_ptr<SettingRow> playModeRow_;
    std::unique_ptr<SettingRow> clockRow_;
    std::unique_ptr<GlassMenu> glassMenu_;
    std::unique_ptr<DismissLayer> dismissLayer_;
    std::unique_ptr<GlobalClickListener> globalClickListener_;

    float designScale() const;
    juce::Point<float> designOrigin() const;
    juce::Rectangle<float> designRect(float x, float y, float w, float h) const;

    void showMenu(MenuId menu);
    void closeMenu();
    void handleGlobalMouseDown(const juce::MouseEvent& e);
    void cyclePlayMode(int dir);
    void cycleClock(int dir);
    void applyPlayMode(int index);
    void applyClock(int index);

    void paint(juce::Graphics& g) override;
    void resized() override;
};
