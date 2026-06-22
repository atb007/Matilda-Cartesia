#include "PluginEditor.h"
#include "Engine/ScaleConfig.h"
#include "ReactShellLayout.h"
#include "UiDevConfig.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

namespace {

juce::Point<int> devWindowSize(matilda::ui::DevView view) {
    using namespace matilda::react;
    using namespace matilda::ui;
    const float s = kDevPreviewScale;
    const int pad = 48;

    switch (view) {
        case DevView::M1_GemCell:
            return { sx(120.f, s) + pad, sx(130.f, s) + pad };
        case DevView::M2_Grid4x4:
            return { sx(kGridW, s) + pad, sx(kGridH, s) + pad };
        case DevView::M3_LayerOverview:
            return { sx(kLayerOverviewSize.w, s) + pad, sx(kLayerOverviewSize.h, s) + pad };
        case DevView::M4_QuantisePanel:
            return { sx(kScalePanelSize.w, s) + pad, sx(kScalePanelSize.h, s) + pad };
        case DevView::M5_MovementMenu:
            return { sx(kMovementSize.w, s) + pad, sx(kMovementSize.h, s) + pad };
        case DevView::M6_Transport:
            return { sx(kTransportSize.w, s) + pad, sx(kTransportSize.h, s) + pad };
        case DevView::M7_ShellChrome:
            return { sx(kShellW, s) + pad, sx(kFrameOverlayH, s) + pad };
        default:
            return { sx(kExpandedW), sx(kFrameH) };
    }
}

} // namespace

MatildaAudioProcessorEditor::MatildaAudioProcessorEditor(MatildaAudioProcessor& p)
    : AudioProcessorEditor(&p), processor_(p) {
    setLookAndFeel(&laf_);

    using namespace matilda::react;
    if (matilda::ui::devIsolatedModule()) {
        const auto sz = devWindowSize(matilda::ui::kDevView);
        setSize(sz.x, sz.y);
    } else {
        setSize(sx(kExpandedW), sx(kFrameH));
    }

    bpmLabel_.setFont(juce::FontOptions(11.f));
    bpmLabel_.setColour(juce::Label::textColourId, laf_.textMuted);
    bpmLabel_.setEditable(true, true, false);
    bpmLabel_.setTooltip("Double-click to set BPM (GarageBand cannot send tempo to external apps)");
    bpmLabel_.onEditorHide = [this] { applyBpmFromLabel(); };
    statusLabel_.setFont(juce::FontOptions(10.f));
    statusLabel_.setColour(juce::Label::textColourId, laf_.textMuted.withAlpha(0.65f));
    statusLabel_.setInterceptsMouseClicks(false, false);
    syncToggle_.setToggleState(processor_.followExternalTransport(), juce::dontSendNotification);
    syncToggle_.onClick = [this] {
        processor_.setFollowExternalTransport(syncToggle_.getToggleState());
    };

    const bool showSandboxChrome = processor_.isStandaloneWrapper() && !matilda::ui::devIsolatedModule();
    syncToggle_.setVisible(showSandboxChrome);
    statusLabel_.setVisible(showSandboxChrome);
    bpmLabel_.setVisible(showSandboxChrome);

    addAndMakeVisible(frame_);
    frame_.onViewportSizeChanged = [this](juce::Point<int> sz) {
        if (getWidth() != sz.x || getHeight() != sz.y)
            setSize(sz.x, sz.y);
    };
    frame_.shell().applyDevView(matilda::ui::kDevView);
    if (matilda::ui::devIsolatedModule())
        frame_.hero().setVisible(false);
    for (auto* c : { static_cast<juce::Component*>(&bpmLabel_),
                     static_cast<juce::Component*>(&syncToggle_),
                     static_cast<juce::Component*>(&statusLabel_) })
        addAndMakeVisible(c);

    processor_.addChangeListener(this);
    bindCallbacks();
    refreshAll();
    startTimerHz(20);
}

MatildaAudioProcessorEditor::~MatildaAudioProcessorEditor() {
    processor_.removeChangeListener(this);
    setLookAndFeel(nullptr);
}

void MatildaAudioProcessorEditor::refreshAll() {
    movement_.syncFromPatch();
    transport_.syncFromPatch();
    const int layer = processor_.patch().selectedLayer;
    grid_.setLayer(layer);
    grid_.refresh();
    overview_.refresh();
    quantise_.syncFromPatch();
    syncToggle_.setToggleState(processor_.followExternalTransport(), juce::dontSendNotification);
    updateStatusLine();
    layoutChromeOverlays();
}

void MatildaAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster*) {
    refreshAll();
}

void MatildaAudioProcessorEditor::bindCallbacks() {
    transport_.onPlay = [this] {
        processor_.setSequencerRunning(true);
        transport_.setPlaying(true);
        if (auto* h = juce::StandalonePluginHolder::getInstance())
            h->startPlaying();
    };
    transport_.onStop = [this] {
        processor_.setSequencerRunning(false);
        transport_.setPlaying(false);
    };

    quantise_.onChanged = [this] {
        processor_.engine().requantizeAllCells();
        grid_.refresh();
    };

    overview_.onLayerActivated = [this](int) { grid_.refresh(); overview_.refresh(); };
    overview_.onLayerSelected = [this](int layer) {
        processor_.patch().selectedLayer = layer;
        grid_.setLayer(layer);
        movement_.syncFromPatch();
        overview_.refresh();
    };

    movement_.onChanged = [this] { overview_.refresh(); };

    transport_.onSettingsChanged = [this] { updateStatusLine(); };

    grid_.onCellChanged = [this] { overview_.refresh(); };
}

void MatildaAudioProcessorEditor::updateStatusLine() {
    const auto& eng = processor_.engine();
    const auto& patch = processor_.patch();
    const int step = processor_.isSequencerStepping() ? eng.currentStepIndex() : -1;

    statusLabel_.setText(
        "step=" + juce::String(step) + " tick=" + juce::String(eng.masterTick()) + " · L"
            + juce::String(eng.lastPlayingLayer() + 1) + " · " + patch.root + " "
            + matilda::scaleLabelForMode(patch.mode),
        juce::dontSendNotification);
    bpmLabel_.setText(juce::String(juce::roundToInt(processor_.getBpm())) + " BPM",
                      juce::dontSendNotification);
}

void MatildaAudioProcessorEditor::applyBpmFromLabel() {
    auto digits = bpmLabel_.getText().retainCharacters("0123456789");
    int bpm = digits.getIntValue();
    if (bpm < 20)
        bpm = 20;
    if (bpm > 300)
        bpm = 300;
    processor_.setUserBpm(static_cast<double>(bpm));
    updateStatusLine();
}

void MatildaAudioProcessorEditor::timerCallback() {
    const auto& eng = processor_.engine();
    const bool running = processor_.isSequencerRunning();
    const int step = running ? eng.currentStepIndex() : -1;
    overview_.setPlayingLayer(eng.lastPlayingLayer(), step);
    grid_.setPlayhead(step, eng.lastPlayingLayer(), eng.lastStepFired());

    if (running && !lastTransportRunning_) {
        if (auto* h = juce::StandalonePluginHolder::getInstance())
            h->startPlaying();
    }
    lastTransportRunning_ = running;

    transport_.setPlaying(running);
    updateStatusLine();
}

void MatildaAudioProcessorEditor::layoutChromeOverlays() {
    if (!processor_.isStandaloneWrapper() || matilda::ui::devIsolatedModule())
        return;

    using namespace matilda::react;

    const auto shellBounds = frame_.shell().getBounds();
    const int syncH = sx(20.f);
    syncToggle_.setBounds(shellBounds.getX(), shellBounds.getBottom() - syncH - sx(6.f), sx(220.f), syncH);
    bpmLabel_.setBounds(shellBounds.getRight() - sx(78.f), getHeight() - syncH - sx(4.f), sx(72.f), syncH);
    statusLabel_.setBounds(shellBounds.getX(), getHeight() - syncH - sx(4.f),
                           shellBounds.getWidth() - sx(80.f), syncH);
}

void MatildaAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff08060c));
}

void MatildaAudioProcessorEditor::resized() {
    using namespace matilda::ui;
    frame_.setBounds(getLocalBounds());

    if (devIsolatedModule()) {
        frame_.hero().setVisible(false);
        frame_.setPreviewScale(kDevPreviewScale);
        frame_.shell().setPreviewScale(kDevPreviewScale);
        frame_.shell().applyDevView(kDevView);
    } else {
        frame_.hero().setVisible(true);
        frame_.setPreviewScale(matilda::react::kPreviewScale);
        frame_.shell().applyDevView(DevView::FullShell);
    }

    layoutChromeOverlays();
}
