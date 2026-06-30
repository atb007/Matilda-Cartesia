#include "MatildaShellPanel.h"
#include "../UiDevConfig.h"

MatildaShellPanel::MatildaShellPanel(matilda::PatchState& patch,
                                   matilda::SequencerEngine& engine,
                                   MatildaLookAndFeel& laf,
                                   TransportBar& transport,
                                   QuantisePanel& quantise,
                                   LayerOverview& overview,
                                   MovementSelector& movement,
                                   GemGrid& grid)
    : patch_(patch),
      engine_(engine),
      laf_(laf),
      transport_(transport),
      quantise_(quantise),
      overview_(overview),
      movement_(movement),
      grid_(grid) {
    addAndMakeVisible(chrome_);
    addAndMakeVisible(quantise_);
    addAndMakeVisible(overview_);
    addAndMakeVisible(movement_);
    addAndMakeVisible(grid_);
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
                break;
            case matilda::ui::DevView::M2_Grid4x4:
                grid_.setBounds(area);
                grid_.setGridMetrics(kGridCellW, kGridCellH, kGridColGap, kGridRowGap, previewScale_);
                break;
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

    quantise_.setBounds(designRect(kScalePanelPos.x, kScalePanelPos.y,
                                   kScalePanelSize.w, kScalePanelSize.h, previewScale_));
    overview_.setBounds(designRect(kLayerOverviewPos.x, kLayerOverviewPos.y,
                                   kLayerOverviewSize.w, kLayerOverviewSize.h, previewScale_));
    movement_.setBounds(designRect(kMovementPos.x, kMovementPos.y,
                                   kMovementSize.w, kMovementSize.h, previewScale_));
    grid_.setBounds(designRect(kGridPos.x, kGridPos.y, kGridW, kGridH, previewScale_));
    grid_.setGridMetrics(kGridCellW, kGridCellH, kGridColGap, kGridRowGap, previewScale_);
    transport_.setBounds(designRect(kTransportPos.x, kTransportPos.y,
                                    kTransportSize.w, kTransportSize.h, previewScale_));
}
