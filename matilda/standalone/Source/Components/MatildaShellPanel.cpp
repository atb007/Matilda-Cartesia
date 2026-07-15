#include "MatildaShellPanel.h"
#include "../UiDevConfig.h"

MatildaShellPanel::MatildaShellPanel(matilda::PatchState& patch,
                                   matilda::SequencerEngine& engine,
                                   MatildaLookAndFeel& laf,
                                   TransportBar& transport,
                                   QuantisePanel& quantise,
                                   LayerOverview& overview,
                                   MovementSelector& movement,
                                   GemGrid& grid,
                                   StepScroll& stepScroll,
                                   PolyphonyCrown& polyphonyCrown,
                                   PresetBar& presetBar)
    : patch_(patch),
      engine_(engine),
      laf_(laf),
      transport_(transport),
      quantise_(quantise),
      overview_(overview),
      movement_(movement),
      grid_(grid),
      stepScroll_(stepScroll),
      polyphonyCrown_(polyphonyCrown),
      presetBar_(presetBar) {
    juce::ignoreUnused(patch_, engine_, laf_);
    addAndMakeVisible(chrome_);
    addAndMakeVisible(presetBar_);
    addAndMakeVisible(quantise_);
    addAndMakeVisible(overview_);
    addAndMakeVisible(movement_);
    addAndMakeVisible(grid_);
    addAndMakeVisible(stepScroll_);
    addAndMakeVisible(polyphonyCrown_);
    addAndMakeVisible(transport_);

    setPaintingIsUnclipped(true);
}

void MatildaShellPanel::setPreviewScale(float scale) {
    previewScale_ = scale;
    chrome_.setPreviewScale(scale);
    resized();
}

void MatildaShellPanel::applyDevView(matilda::ui::DevView view) {
    using namespace matilda::ui;
    const bool full = view == DevView::FullShell;

    chrome_.setVisible(full || view == DevView::M7_ShellChrome);
    quantise_.setVisible(full || view == DevView::M4_QuantisePanel);
    overview_.setVisible(full || view == DevView::M3_LayerOverview);
    movement_.setVisible(full || view == DevView::M5_MovementMenu);
    transport_.setVisible(full || view == DevView::M6_Transport);
    grid_.setVisible(full || view == DevView::M2_Grid4x4 || view == DevView::M1_GemCell);
    stepScroll_.setVisible(full || view == DevView::M2_Grid4x4);
    polyphonyCrown_.setVisible(full || view == DevView::M3_LayerOverview);
    presetBar_.setVisible(full);

    grid_.setSingleCellDevPreview(view == DevView::M1_GemCell);
    resized();
}

void MatildaShellPanel::resized() {
    using namespace matilda::react;

    chrome_.setBounds(getLocalBounds());
    chrome_.setPreviewScale(previewScale_);

    const bool isolated = matilda::ui::devIsolatedModule();
    if (isolated) {
        const auto area = getLocalBounds().reduced(juce::roundToInt(12.f * previewScale_));
        switch (matilda::ui::kDevView) {
            case matilda::ui::DevView::M1_GemCell:
                grid_.setBounds(area);
                stepScroll_.setVisible(false);
                break;
            case matilda::ui::DevView::M2_Grid4x4: {
                const int gridH = matilda::react::sx(kGridH, previewScale_);
                const int scrollH = matilda::react::sx(kStepScrollSize.h, previewScale_);
                const int gap = matilda::react::sx(kStepScrollPos.y - (kGridPos.y + kGridH), previewScale_);
                const int totalH = gridH + juce::jmax(0, gap) + scrollH;
                auto block = juce::Rectangle<int>(matilda::react::sx(kGridW, previewScale_), totalH)
                                 .withCentre(area.getCentre());
                grid_.setBounds(block.removeFromTop(gridH));
                block.removeFromTop(juce::jmax(0, gap));
                stepScroll_.setBounds(block.removeFromTop(scrollH));
                grid_.setGridMetrics(kGridCellW, kGridCellH, kGridColGap, kGridRowGap, previewScale_);
                break;
            }
            case matilda::ui::DevView::M3_LayerOverview: {
                const int w = matilda::react::sx(matilda::react::kLayerOverviewSize.w, previewScale_);
                const int h = matilda::react::sx(matilda::react::kLayerOverviewSize.h, previewScale_);
                overview_.setBounds(juce::Rectangle<int>(w, h).withCentre(area.getCentre()));
                break;
            }
            case matilda::ui::DevView::M4_QuantisePanel: {
                const int w = matilda::react::sx(matilda::react::kScalePanelSize.w, previewScale_);
                const int h = matilda::react::sx(matilda::react::kScalePanelSize.h, previewScale_);
                quantise_.setBounds(juce::Rectangle<int>(w, h).withCentre(area.getCentre()));
                break;
            }
            case matilda::ui::DevView::M5_MovementMenu: {
                const int w = matilda::react::sx(matilda::react::kMovementSize.w, previewScale_);
                const int h = matilda::react::sx(matilda::react::kMovementSize.h, previewScale_);
                movement_.setBounds(juce::Rectangle<int>(w, h).withCentre(area.getCentre()));
                break;
            }
            case matilda::ui::DevView::M6_Transport: {
                const int w = matilda::react::sx(matilda::react::kTransportSize.w, previewScale_);
                const int h = matilda::react::sx(matilda::react::kTransportSize.h, previewScale_);
                transport_.setBounds(juce::Rectangle<int>(w, h).withCentre(area.getCentre()));
                break;
            }
            case matilda::ui::DevView::M7_ShellChrome:
                chrome_.setBounds(area);
                chrome_.setPreviewScale(previewScale_);
                break;
            default:
                break;
        }
        return;
    }

    presetBar_.setBounds(designRect(kPresetBarPos.x, kPresetBarPos.y,
                                    kPresetBarW, kPresetBarH, previewScale_));
    quantise_.setBounds(designRect(kScalePanelPos.x, kScalePanelPos.y,
                                   kScalePanelSize.w, kScalePanelSize.h, previewScale_));
    overview_.setBounds(designRect(kLayerOverviewPos.x, kLayerOverviewPos.y,
                                   kLayerOverviewSize.w, kLayerOverviewSize.h, previewScale_));
    polyphonyCrown_.setBounds(designRect(kPolyphonyCrownPos.x, kPolyphonyCrownPos.y,
                                         kPolyphonyCrownW, kPolyphonyCrownH, previewScale_));
    polyphonyCrown_.toFront(false);
    presetBar_.toFront(false);
    movement_.setBounds(designRect(kMovementPos.x, kMovementPos.y,
                                   kMovementSize.w, kMovementSize.h, previewScale_));
    grid_.setBounds(designRect(kGridPos.x, kGridPos.y, kGridW, kGridH, previewScale_));
    grid_.setGridMetrics(kGridCellW, kGridCellH, kGridColGap, kGridRowGap, previewScale_);
    stepScroll_.setBounds(designRect(kStepScrollPos.x, kStepScrollPos.y,
                                     kStepScrollSize.w, kStepScrollSize.h, previewScale_));
    transport_.setBounds(designRect(kTransportPos.x, kTransportPos.y,
                                    kTransportSize.w, kTransportSize.h, previewScale_));
}
