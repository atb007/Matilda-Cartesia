#include "PluginEditor.h"
#include "Engine/ScaleConfig.h"
#include "HeroBackdropDrawing.h"
#include "ReactShellLayout.h"
#include "UiDevConfig.h"
#include "UiScale.h"
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

namespace {

int countActiveLayers(const matilda::PatchState& patch) {
    int count = 0;
    for (const auto& layer : patch.layers)
        if (layer.active)
            ++count;
    return juce::jmax(1, count);
}

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
    setOpaque(true);

    using namespace matilda::react;
    if (matilda::ui::devIsolatedModule()) {
        const auto sz = devWindowSize(matilda::ui::kDevView);
        setSize(sz.x, sz.y);
    } else {
        uiScaleFactor_ = processor_.editorUiScale();
        frame_.setPreviewScale(matilda::ui::effectivePreviewScale(uiScaleFactor_));
        const auto sz = matilda::ui::viewportPixelSize(kExpandedW, uiScaleFactor_);
        setSize(sz.x, sz.y);
        setResizable(true, false);
        updateResizeLimits();
        frame_.setCollapsed(processor_.editorShellCollapsed(), false);
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

    // MIDI-out device selector — visible in plugin + standalone (FL Studio / VST3 routing workaround).
    const bool showMidiOut = !matilda::ui::devIsolatedModule();
    midiOutLabel_.setFont(juce::FontOptions(11.f));
    midiOutLabel_.setColour(juce::Label::textColourId, laf_.textMuted);
    midiOutLabel_.setJustificationType(juce::Justification::centredLeft);
    midiOutLabel_.setVisible(showMidiOut);
    midiOutCombo_.setTooltip("Optional fallback: stream notes to a virtual MIDI port (loopMIDI / IAC). "
                             "Leave as (None) when using Fruity Wrapper internal ports — otherwise audio "
                             "may route to master instead of this channel.");
    midiOutCombo_.setVisible(showMidiOut);
    refreshMidiOutList();
    midiOutCombo_.onChange = [this] { processor_.setMidiOutputByName(midiOutCombo_.getText()); };

    addAndMakeVisible(frame_);
    frame_.onViewportSizeChanged = [this](juce::Point<int> sz) {
        suppressHostResizeSync_ = true;
        if (getWidth() != sz.x || getHeight() != sz.y)
            setSize(sz.x, sz.y);
        updateResizeLimits();
        layoutResizeGrips();
        suppressHostResizeSync_ = false;
    };
    frame_.onCollapsedChanged = [this](bool) { persistEditorLayout(); };
    frame_.shell().applyDevView(matilda::ui::kDevView);
    if (matilda::ui::devIsolatedModule())
        frame_.hero().setVisible(false);

    for (auto& grip : resizeGrips_) {
        const auto id = grip.gripId();
        grip.onDragStart = [this, id] { beginGripResize(id); };
        grip.onDragMove = [this](juce::Point<int> screenPos) { continueGripResize(screenPos); };
        grip.onDragEnd = [this] { gripResizeActive_ = false; };
        addChildComponent(grip);
    }
    for (auto* c : { static_cast<juce::Component*>(&bpmLabel_),
                     static_cast<juce::Component*>(&syncToggle_),
                     static_cast<juce::Component*>(&statusLabel_),
                     static_cast<juce::Component*>(&midiOutLabel_),
                     static_cast<juce::Component*>(&midiOutCombo_) })
        addAndMakeVisible(c);

    processor_.addChangeListener(this);
    bindCallbacks();
    refreshAll();
    startTimerHz(20);
}

MatildaAudioProcessorEditor::~MatildaAudioProcessorEditor() {
    persistEditorLayout();
    processor_.removeChangeListener(this);
    setLookAndFeel(nullptr);
}

void MatildaAudioProcessorEditor::refreshAll() {
    movement_.syncFromPatch();
    transport_.syncFromPatch();
    const int layer = processor_.patch().selectedLayer;
    grid_.setLayer(layer);
    grid_.refresh();
    stepScroll_.setLayer(layer);
    stepScroll_.syncFromPatch();
    polyphonyCrown_.syncFromPatch();
    overview_.refresh();
    quantise_.syncFromPatch();
    presetBar_.syncFromPatch();
    const bool hostSync = processor_.syncHostTransport();
    transport_.setHostSyncLocked(hostSync);
    frame_.dawSyncToggle().setSyncOn(processor_.syncHostTransport());
    frame_.setDawSyncVisible(true);
    syncToggle_.setToggleState(processor_.followExternalTransport(), juce::dontSendNotification);
    updateStatusLine();
    layoutChromeOverlays();
    syncHeroRiveBindings();
}

void MatildaAudioProcessorEditor::syncHeroRiveBindings() {
    frame_.hero().setActiveLayerCount(countActiveLayers(processor_.patch()));
    frame_.hero().setPolyphony(processor_.patch().polyphony);
}

void MatildaAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster*) {
    refreshAll();
}

void MatildaAudioProcessorEditor::bindCallbacks() {
    transport_.onPlay = [this] {
        processor_.setSequencerRunning(true);
        transport_.setPlaying(true);
        syncHeroRiveBindings();
        frame_.hero().setPlaying(true);
        if (auto* h = juce::StandalonePluginHolder::getInstance())
            h->startPlaying();
    };
    transport_.onStop = [this] {
        processor_.setSequencerRunning(false);
        transport_.setPlaying(false);
        frame_.hero().setPlaying(false);
    };

    quantise_.onChanged = [this] {
        processor_.engine().requantizeAllCells();
        grid_.refresh();
        presetBar_.notePatchEdited();
    };

    overview_.onLayerActivated = [this](int) {
        grid_.refresh();
        overview_.refresh();
        polyphonyCrown_.syncFromPatch();
        syncHeroRiveBindings();
        presetBar_.notePatchEdited();
    };
    overview_.onLayerSelected = [this](int layer) {
        processor_.patch().selectedLayer = layer;
        grid_.setLayer(layer);
        stepScroll_.setLayer(layer);
        movement_.syncFromPatch();
        overview_.refresh();
        presetBar_.notePatchEdited();
    };
    overview_.onLayerDataChanged = [this](int layer) {
        processor_.engine().onActiveStepCountChanged(layer);
        overview_.refresh();
        polyphonyCrown_.syncFromPatch();
        syncHeroRiveBindings();
        if (processor_.patch().selectedLayer == layer) {
            grid_.refresh();
            stepScroll_.syncFromPatch();
        }
        updateStatusLine();
        presetBar_.notePatchEdited();
    };

    movement_.onChanged = [this] {
        overview_.refresh();
        presetBar_.notePatchEdited();
    };

    transport_.onSettingsChanged = [this] {
        updateStatusLine();
        presetBar_.notePatchEdited();
    };

    frame_.dawSyncToggle().onToggle = [this](bool enabled) {
        processor_.setSyncHostTransport(enabled);
        refreshAll();
    };

    grid_.onCellChanged = [this] {
        overview_.refresh();
        presetBar_.notePatchEdited();
    };

    stepScroll_.onChanged = [this] {
        const int layer = processor_.patch().selectedLayer;
        processor_.engine().onActiveStepCountChanged(layer);
        grid_.refresh();
        overview_.refresh();
        updateStatusLine();
        presetBar_.notePatchEdited();
    };

    polyphonyCrown_.onChanged = [this] {
        overview_.refresh();
        polyphonyCrown_.syncFromPatch();
        syncHeroRiveBindings();
        updateStatusLine();
        presetBar_.notePatchEdited();
    };

    presetBar_.onLoadPreset = [this](const juce::String& name) {
        matilda::PatchState next;
        if (!matilda::PresetLibrary::loadNamed(name, next))
            return;
        processor_.patch() = next;
        processor_.engine().reset();
        processor_.engine().requantizeAllCells();
        refreshAll();
        syncHeroRiveBindings();
        frame_.hero().setPlaying(processor_.isSequencerRunning());
    };
}

void MatildaAudioProcessorEditor::updateStatusLine() {
    const auto& eng = processor_.engine();
    const auto& patch = processor_.patch();
    const int step = processor_.isSequencerStepping() ? eng.lastStepIndex() : -1;

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
    const auto& patch = processor_.patch();

    if (running && patch.polyphony) {
        std::array<int, matilda::kLayerCount> steps{};
        steps.fill(-1);
        for (const auto& r : eng.lastTickResults())
            steps[static_cast<size_t>(r.layer)] = r.stepIndex;
        overview_.setPolyPlayheads(steps, true);
    } else {
        const int step = running ? eng.lastStepIndex() : -1;
        overview_.setPlayingLayer(eng.lastPlayingLayer(), step);
    }

    const int step = running ? eng.lastStepIndex() : -1;
    grid_.setPlayhead(step, eng.lastPlayingLayer(), eng.lastStepFired());

    if (running && !lastTransportRunning_) {
        if (auto* h = juce::StandalonePluginHolder::getInstance())
            h->startPlaying();
    }
    const bool transportChanged = running != lastTransportRunning_;
    lastTransportRunning_ = running;

    transport_.setPlaying(running);
    if (transportChanged)
        syncHeroRiveBindings();
    frame_.hero().setPlaying(running);
    updateStatusLine();

    // Pick up virtual MIDI ports created after the editor opened (loopMIDI / IAC).
    if (midiOutCombo_.isShowing() && !midiOutCombo_.isPopupActive()
        && processor_.midiOutputDeviceNames().size() != midiOutCombo_.getNumItems())
        refreshMidiOutList();
}

void MatildaAudioProcessorEditor::applyUiScale() {
    using namespace matilda::react;
    if (matilda::ui::devIsolatedModule())
        return;

    frame_.setPreviewScale(matilda::ui::effectivePreviewScale(uiScaleFactor_));
    updateResizeLimits();
    layoutChromeOverlays();
    layoutResizeGrips();
}

void MatildaAudioProcessorEditor::beginGripResize(matilda::ui::UiResizeGripId grip) {
    using namespace matilda::react;
    gripResizeActive_ = true;
    activeResizeGrip_ = grip;
    gripResizeStartMouse_ = juce::Desktop::getInstance().getMousePosition();
    gripResizeStartWidth_ = getWidth();
    gripResizeStartHeight_ = getHeight();
    const float viewportW = frame_.isCollapsed() ? kCollapsedW : kExpandedW;
    gripResizeRefWidth100_ = matilda::ui::referenceViewportWidth100(viewportW);
    gripResizeRefHeight100_ = matilda::ui::referenceViewportHeight100();
}

void MatildaAudioProcessorEditor::continueGripResize(juce::Point<int> screenMouse) {
    if (!gripResizeActive_ || gripResizeRefWidth100_ <= 0 || gripResizeRefHeight100_ <= 0)
        return;

    const int deltaX = screenMouse.x - gripResizeStartMouse_.x;
    const int deltaY = screenMouse.y - gripResizeStartMouse_.y;
    const float factor = matilda::ui::uiScaleFactorFromGripDrag(
        activeResizeGrip_, deltaX, deltaY, gripResizeStartWidth_, gripResizeStartHeight_,
        gripResizeRefWidth100_, gripResizeRefHeight100_);

    if (std::abs(factor - uiScaleFactor_) < 0.001f)
        return;

    uiScaleFactor_ = factor;
    applyUiScale();
    persistEditorLayout();
}

void MatildaAudioProcessorEditor::persistEditorLayout() {
    if (matilda::ui::devIsolatedModule())
        return;
    processor_.setEditorUiScale(uiScaleFactor_);
    processor_.setEditorShellCollapsed(frame_.isCollapsed());
}

juce::Point<int> MatildaAudioProcessorEditor::intendedEditorSize() const {
    if (matilda::ui::devIsolatedModule())
        return devWindowSize(matilda::ui::kDevView);
    return frame_.currentViewportPixelSize();
}

void MatildaAudioProcessorEditor::updateResizeLimits() {
    if (matilda::ui::devIsolatedModule())
        return;

    const auto minSz = matilda::ui::editorResizeLimitsMin();
    const auto maxSz = matilda::ui::editorResizeLimitsMax(uiScaleFactor_);
    setResizeLimits(minSz.x, minSz.y, maxSz.x, maxSz.y);
}

void MatildaAudioProcessorEditor::syncEditorToViewport() {
    if (matilda::ui::devIsolatedModule() || suppressHostResizeSync_ || gripResizeActive_)
        return;

    const auto intended = intendedEditorSize();
    if (getWidth() != intended.x || getHeight() != intended.y) {
        suppressHostResizeSync_ = true;
        setSize(intended.x, intended.y);
        suppressHostResizeSync_ = false;
    }
}

void MatildaAudioProcessorEditor::layoutResizeGrips() {
    if (matilda::ui::devIsolatedModule()) {
        for (auto& grip : resizeGrips_)
            grip.setVisible(false);
        return;
    }

    constexpr int kCorner = 22;
    constexpr int kEdge = 8;
    const int w = getWidth();
    const int h = getHeight();

    for (auto& grip : resizeGrips_) {
        grip.setVisible(true);
        switch (grip.gripId()) {
            case matilda::ui::UiResizeGripId::topLeft:
                grip.setBounds(0, 0, kCorner, kCorner);
                break;
            case matilda::ui::UiResizeGripId::top:
                grip.setBounds(kCorner, 0, w - kCorner * 2, kEdge);
                break;
            case matilda::ui::UiResizeGripId::topRight:
                grip.setBounds(w - kCorner, 0, kCorner, kCorner);
                break;
            case matilda::ui::UiResizeGripId::left:
                grip.setBounds(0, kCorner, kEdge, h - kCorner * 2);
                break;
            case matilda::ui::UiResizeGripId::right:
                grip.setBounds(w - kEdge, kCorner, kEdge, h - kCorner * 2);
                break;
            case matilda::ui::UiResizeGripId::bottomLeft:
                grip.setBounds(0, h - kCorner, kCorner, kCorner);
                break;
            case matilda::ui::UiResizeGripId::bottom:
                grip.setBounds(kCorner, h - kEdge, w - kCorner * 2, kEdge);
                break;
            case matilda::ui::UiResizeGripId::bottomRight:
                grip.setBounds(w - kCorner, h - kCorner, kCorner, kCorner);
                break;
        }
    }

    for (auto& grip : resizeGrips_) {
        if (matilda::ui::isCornerGrip(grip.gripId()))
            grip.toFront(false);
    }
}

void MatildaAudioProcessorEditor::layoutChromeOverlays() {
    if (!processor_.isStandaloneWrapper() || matilda::ui::devIsolatedModule())
        return;

    using namespace matilda::react;

    const float previewScale = matilda::ui::effectivePreviewScale(uiScaleFactor_);
    const auto shellBounds = frame_.shell().getBounds();
    const int syncH = sx(20.f, previewScale);
    syncToggle_.setBounds(shellBounds.getX(), shellBounds.getBottom() - syncH - sx(6.f, previewScale),
                          sx(220.f, previewScale), syncH);
    bpmLabel_.setBounds(shellBounds.getRight() - sx(78.f, previewScale), getHeight() - syncH - sx(4.f, previewScale),
                        sx(72.f, previewScale), syncH);
    statusLabel_.setBounds(shellBounds.getX(), getHeight() - syncH - sx(4.f, previewScale),
                           shellBounds.getWidth() - sx(80.f, previewScale), syncH);
}

void MatildaAudioProcessorEditor::paint(juce::Graphics& g) {
    matilda::ui::paintHeroBackdropCover(g, getLocalBounds());
}

void MatildaAudioProcessorEditor::resized() {
    using namespace matilda::ui;
    syncEditorToViewport();
    frame_.setBounds(getLocalBounds());

    if (devIsolatedModule()) {
        frame_.hero().setVisible(false);
        frame_.setPreviewScale(kDevPreviewScale);
        frame_.shell().setPreviewScale(kDevPreviewScale);
        frame_.shell().applyDevView(kDevView);
    } else {
        frame_.hero().setVisible(true);
        frame_.setPreviewScale(matilda::ui::effectivePreviewScale(uiScaleFactor_));
        frame_.shell().applyDevView(DevView::FullShell);
    }

    layoutChromeOverlays();
    layoutMidiOutSelector();
    layoutResizeGrips();
}

void MatildaAudioProcessorEditor::refreshMidiOutList() {
    const auto names = processor_.midiOutputDeviceNames();
    const auto current = processor_.currentMidiOutputName();
    midiOutCombo_.clear(juce::dontSendNotification);
    int idToSelect = 1;
    for (int i = 0; i < names.size(); ++i) {
        midiOutCombo_.addItem(names[i], i + 1);
        if (current.isNotEmpty() && names[i] == current)
            idToSelect = i + 1;
    }
    midiOutCombo_.setSelectedId(idToSelect, juce::dontSendNotification);
}

void MatildaAudioProcessorEditor::layoutMidiOutSelector() {
    const bool show = !matilda::ui::devIsolatedModule();
    midiOutLabel_.setVisible(show);
    midiOutCombo_.setVisible(show);
    if (!show)
        return;

    const float previewScale = matilda::ui::effectivePreviewScale(uiScaleFactor_);
    const int h = matilda::react::sx(20.f, previewScale);
    const int labelW = matilda::react::sx(54.f, previewScale);
    const int comboW = matilda::react::sx(190.f, previewScale);
    const int pad = matilda::react::sx(8.f, previewScale);
    const int y = getHeight() - h - pad;
    midiOutLabel_.setBounds(pad, y, labelW, h);
    midiOutCombo_.setBounds(pad + labelW, y, comboW, h);
    midiOutLabel_.toFront(false);
    midiOutCombo_.toFront(false);
}
