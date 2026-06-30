#pragma once

#include <JuceHeader.h>
#include <array>
#include "../Engine/SequencerState.h"
#include "../MatildaLookAndFeel.h"
#include "GemSparksOverlay.h"

class QuantisePanel : public juce::Component {
public:
    QuantisePanel(matilda::PatchState& patch, MatildaLookAndFeel& laf);
    ~QuantisePanel() override;

    std::function<void()> onChanged;
    void syncFromPatch();

private:
    enum class MenuId { None, Min, Tonic, Max, Scale };

    class PickerRow;
    class ScaleArrowButton;
    class ScaleNameButton;
    class GlassMenu;
    class DismissLayer;
    class GlobalClickListener;
    class MinMaxChromeOverlay;

    matilda::PatchState& patch_;
    MatildaLookAndFeel& laf_;
    MenuId openMenu_ = MenuId::None;

    juce::Image filigreeTopImg_;
    juce::Image filigreeBottomImg_;
    juce::Image bgTextureImg_;
    juce::Image minMaxOrnLeftImg_;
    juce::Image minMaxOrnRightImg_;
    juce::Image connectorLeftImg_;
    juce::Image connectorRightImg_;

    GemSparksOverlay gemSparks_;

    std::unique_ptr<PickerRow> minRow_;
    std::unique_ptr<PickerRow> tonicRow_;
    std::unique_ptr<PickerRow> maxRow_;
    std::unique_ptr<ScaleArrowButton> scalePrev_;
    std::unique_ptr<ScaleArrowButton> scaleNext_;
    std::unique_ptr<ScaleNameButton> scaleName_;
    std::unique_ptr<GlassMenu> glassMenu_;
    std::unique_ptr<DismissLayer> dismissLayer_;
    std::unique_ptr<GlobalClickListener> globalClickListener_;
    std::unique_ptr<MinMaxChromeOverlay> minMaxChrome_;

    float designScale() const;
    juce::Point<float> designOrigin() const;
    juce::Rectangle<float> designRect(float x, float y, float w, float h) const;

    void showMenu(MenuId menu, juce::Component* anchor);
    void closeMenu();
    void handleGlobalMouseDown(const juce::MouseEvent& e);
    void cycleScale(int dir);
    void applyScaleIndex(int index);
    void cycleTonic(int dir);
    void applyTonicIndex(int index);
    void cycleMinOct(int dir);
    void cycleMaxOct(int dir);
    void applyMinOct(int oct);
    void applyMaxOct(int oct);
    void notifyChanged();

    struct MinMaxChromeLayout {
        float pickY = 0.f;
        float pickRowH = 0.f;
        float pickCentreY = 0.f;
        float tonicY = 0.f;
        float minX = 0.f;
        float tonicX = 0.f;
        float maxX = 0.f;
        float conn1X = 0.f;
        float conn2X = 0.f;
        float minBoxW = 0.f;
        float maxBoxW = 0.f;
        std::array<float, 7> labelX{};
        std::array<float, 7> labelW{};
    };

    MinMaxChromeLayout minMaxChromeLayout() const;
    void paintMinMaxHeader(juce::Graphics& g) const;
    void paintMinMaxConnectors(juce::Graphics& g) const;

    void paint(juce::Graphics& g) override;
    void resized() override;
};
