#include "LayerOverview.h"
#include "../GlassDropdownDrawing.h"
#include "../KnobDrawing.h"
#include "../MatildaFonts.h"
#include "../MatildaImages.h"
#include "../MiniGridLayout.h"

namespace {

using namespace matilda::minigrid;

void drawInactiveSocket(juce::Graphics& g, juce::Rectangle<float> slot, float scale, float opacity) {
    const auto img = matilda::images::miniGridInactive();
    if (!img.isValid() || opacity <= 0.f)
        return;

    auto dest = slot.withCentre(slot.getCentre() + juce::Point<float>(0.f, kInactiveYNudge * scale));
    g.setOpacity(opacity);
    g.drawImage(img, dest, juce::RectanglePlacement::centred);
    g.setOpacity(1.f);
}

void drawColoredGem(juce::Graphics& g,
                    juce::Rectangle<float> slot,
                    int layer,
                    bool on,
                    bool glow,
                    float scale,
                    float opacity) {
    const auto img = on ? matilda::images::miniGridOn(layer) : matilda::images::miniGridOff(layer);
    if (!img.isValid() || opacity <= 0.f)
        return;

    if (glow && on) {
        const auto led = matilda::knob::ledColour(matilda::knob::variantForLayer(layer));
        const auto c = slot.getCentre();
        const auto radius = slot.getWidth() * 0.5f + 7.f * scale;
        juce::ColourGradient softGlow(led.withAlpha(0.34f), c.x, c.y, led.withAlpha(0.f), c.x, c.y + radius, true);
        softGlow.addColour(0.45, led.withAlpha(0.15f));
        softGlow.addColour(0.80, led.withAlpha(0.04f));
        g.setGradientFill(softGlow);
        g.fillEllipse(slot.expanded(8.f * scale));
    }

    g.setOpacity(opacity);
    g.drawImage(img, slot, juce::RectanglePlacement::stretchToFit);
    g.setOpacity(1.f);
}

bool cellGateOn(const matilda::LayerState& layer, int miniIndex) {
    const int step = rowMajorFromMiniGridIndex(miniIndex);
    const int y = step / matilda::kGridSize;
    const int x = step % matilda::kGridSize;
    return layer.cells[static_cast<size_t>(y)][static_cast<size_t>(x)].gate;
}

bool cellInActiveRange(const matilda::LayerState& layer, int miniIndex) {
    return rowMajorFromMiniGridIndex(miniIndex) < matilda::clampActiveStepCount(layer.activeStepCount);
}

juce::String actionLabel(int action) {
    switch (action) {
        case 0: return "COPY NOTES";
        case 1: return "COPY NOTES & KNOBS";
        case 2: return "PASTE NOTES";
        case 3: return "PASTE NOTES & KNOBS";
        case 4: return "RESET VALUES";
        case 5: return "UNDO";
        default: return {};
    }
}

} // namespace

// ---------------------------------------------------------------------------
class LayerOverview::ContextMenu : public juce::Component {
public:
    ContextMenu() { setPaintingIsUnclipped(true); }

    std::function<void(int index)> onSelect;
    std::function<void()> onClose;
    juce::Image backdrop_;
    std::array<bool, static_cast<int>(MenuAction::Count)> enabled{};

    /** Compact metrics — smaller than Movement/Quantise glass menus. */
    static constexpr float kFontPx = 12.f;
    static constexpr float kTextH = 14.f;
    static constexpr float kLineGap = 5.f;
    static constexpr float kItemGap = 3.f;
    static constexpr float kVertPad = 10.f;
    static constexpr float kClose = 11.f;
    static constexpr int kMenuW = 168;

    void setBackdrop(juce::Image img) {
        backdrop_ = std::move(img);
        repaint();
    }

    void clearBackdrop() { backdrop_ = {}; }

    void setScale(float scale) {
        scale_ = scale;
        rebuildHitAreas();
        repaint();
    }

    static int menuHeight(float scale) {
        const int n = static_cast<int>(MenuAction::Count);
        const float pad = juce::jmax(8.f, kVertPad * scale);
        const float block = kTextH * static_cast<float>(n)
                            + (kLineGap + 1.f + kItemGap) * static_cast<float>(n - 1);
        return juce::roundToInt(pad * 2.f + block);
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();
        matilda::ui::glass::drawPanel(g, bounds, scale_ * 0.85f, backdrop_);

        const float closeSize = kClose * scale_;
        const auto closeBounds = juce::Rectangle<float>(bounds.getRight() - 10.f * scale_ - closeSize,
                                                       bounds.getY() + 10.f * scale_, closeSize, closeSize);
        matilda::ui::glass::drawCloseIcon(
            g, closeBounds, juce::Colours::white.withAlpha(closeHover_ ? 1.f : 0.85f));

        const float itemW = bounds.getWidth() * 0.9f;
        const float itemX = bounds.getX() + (bounds.getWidth() - itemW) * 0.5f;
        const float vertPad = juce::jmax(8.f, kVertPad * scale_);
        float y = bounds.getY() + vertPad;
        g.setFont(matilda::fonts::kodeMonoBold(kFontPx));

        const int n = static_cast<int>(MenuAction::Count);
        for (int i = 0; i < n; ++i) {
            const bool on = enabled[static_cast<size_t>(i)];
            const float a = on ? (i == hoverIndex_ ? 1.f : 0.85f) : 0.28f;
            g.setColour(juce::Colours::white.withAlpha(a));
            g.drawText(actionLabel(i),
                       juce::Rectangle<float>(itemX, y, itemW, kTextH).toNearestInt(),
                       juce::Justification::centred, false);
            if (i + 1 < n) {
                const float lineY = y + kTextH + kLineGap;
                matilda::ui::glass::drawHairline(g, juce::Rectangle<float>(itemX, lineY, itemW, 1.f));
            }
            y += kTextH;
            if (i + 1 < n)
                y += kLineGap + 1.f + kItemGap;
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        if (closeBounds_.contains(e.getPosition())) {
            if (onClose)
                onClose();
            return;
        }
        for (int i = 0; i < itemBounds_.size(); ++i) {
            if (itemBounds_[i].contains(e.getPosition()) && enabled[static_cast<size_t>(i)]) {
                if (onSelect)
                    onSelect(i);
                return;
            }
        }
    }

    void mouseMove(const juce::MouseEvent& e) override {
        const bool hoverClose = closeBounds_.contains(e.getPosition());
        int hover = -1;
        for (int i = 0; i < itemBounds_.size(); ++i)
            if (itemBounds_[i].contains(e.getPosition()) && enabled[static_cast<size_t>(i)])
                hover = i;
        if (hoverClose != closeHover_ || hover != hoverIndex_) {
            closeHover_ = hoverClose;
            hoverIndex_ = hover;
            repaint();
        }
        setMouseCursor(hoverClose || hover >= 0 ? juce::MouseCursor::PointingHandCursor
                                                : juce::MouseCursor::NormalCursor);
    }

    void mouseExit(const juce::MouseEvent&) override {
        closeHover_ = false;
        hoverIndex_ = -1;
        setMouseCursor(juce::MouseCursor::NormalCursor);
        repaint();
    }

private:
    float scale_ = 1.f;
    juce::Rectangle<int> closeBounds_;
    juce::Array<juce::Rectangle<int>> itemBounds_;
    bool closeHover_ = false;
    int hoverIndex_ = -1;

    void rebuildHitAreas() {
        const auto bounds = getLocalBounds();
        const float closeSize = kClose * scale_;
        closeBounds_ = juce::Rectangle<int>(
            juce::roundToInt(bounds.getRight() - 10.f * scale_ - closeSize),
            juce::roundToInt(bounds.getY() + 10.f * scale_),
            juce::roundToInt(closeSize), juce::roundToInt(closeSize));

        itemBounds_.clear();
        const float itemW = bounds.getWidth() * 0.9f;
        const float itemX = bounds.getX() + (bounds.getWidth() - itemW) * 0.5f;
        const float vertPad = juce::jmax(8.f, kVertPad * scale_);
        float y = bounds.getY() + vertPad;
        const int n = static_cast<int>(MenuAction::Count);
        for (int i = 0; i < n; ++i) {
            itemBounds_.add(juce::Rectangle<int>(juce::roundToInt(itemX), juce::roundToInt(y),
                                                juce::roundToInt(itemW), juce::roundToInt(kTextH)));
            y += kTextH;
            if (i + 1 < n)
                y += kLineGap + 1.f + kItemGap;
        }
    }

    void resized() override { rebuildHitAreas(); }
};

class LayerOverview::DismissLayer : public juce::Component {
public:
    std::function<void()> onDismiss;
    void mouseDown(const juce::MouseEvent&) override {
        if (onDismiss)
            onDismiss();
    }
};

class LayerOverview::GlobalClickListener : public juce::MouseListener {
public:
    explicit GlobalClickListener(LayerOverview& owner) : owner_(owner) {}
    void mouseDown(const juce::MouseEvent& e) override { owner_.handleGlobalMouseDown(e); }
    void mouseUp(const juce::MouseEvent&) override { owner_.ignoreGlobalClickUntilUp_ = false; }

private:
    LayerOverview& owner_;
};

class LayerOverview::FeedbackFloater : public juce::Component, private juce::Timer {
public:
    FeedbackFloater() {
        setOpaque(false);
        setInterceptsMouseClicks(false, false);
        setPaintingIsUnclipped(true);
    }

    void show(const juce::String& text) {
        text_ = text;
        opacity_ = 1.f;
        setVisible(true);
        startTimerHz(30);
        repaint();
    }

    void paint(juce::Graphics& g) override {
        if (text_.isEmpty() || opacity_ <= 0.01f)
            return;
        auto bounds = getLocalBounds().toFloat().reduced(2.f);
        g.setColour(juce::Colour(0xcc1a1e24).withMultipliedAlpha(opacity_));
        g.fillRoundedRectangle(bounds, 10.f);
        g.setColour(juce::Colours::white.withAlpha(0.35f * opacity_));
        g.drawRoundedRectangle(bounds, 10.f, 1.f);
        g.setFont(matilda::fonts::kodeMonoBold(13.f));
        g.setColour(juce::Colours::white.withAlpha(opacity_));
        g.drawText(text_, getLocalBounds(), juce::Justification::centred, false);
    }

private:
    juce::String text_;
    float opacity_ = 0.f;

    void timerCallback() override {
        opacity_ -= 0.035f;
        if (opacity_ <= 0.f) {
            opacity_ = 0.f;
            stopTimer();
            setVisible(false);
        }
        repaint();
    }
};

// ---------------------------------------------------------------------------
LayerOverview::LayerOverview(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    juce::ignoreUnused(laf_);
    setOpaque(false);

    menu_ = std::make_unique<ContextMenu>();
    menu_->setVisible(false);
    menu_->onSelect = [this](int index) {
        handleMenuAction(static_cast<MenuAction>(index));
        hideContextMenu();
    };
    menu_->onClose = [this] { hideContextMenu(); };

    dismissLayer_ = std::make_unique<DismissLayer>();
    dismissLayer_->setVisible(false);
    dismissLayer_->setInterceptsMouseClicks(true, true);
    dismissLayer_->onDismiss = [this] {
        if (!ignoreGlobalClickUntilUp_)
            hideContextMenu();
    };

    floater_ = std::make_unique<FeedbackFloater>();
    addChildComponent(*floater_);

    globalListener_ = std::make_unique<GlobalClickListener>(*this);
    refresh();
}

LayerOverview::~LayerOverview() {
    hideContextMenu();
}

void LayerOverview::setPlayingLayer(int layer, int playheadStep) {
    polyMode_ = false;
    transportRunning_ = playheadStep >= 0;
    playingLayer_ = layer;
    playheadStep_ = playheadStep;
    repaint();
}

void LayerOverview::setPolyPlayheads(const std::array<int, matilda::kLayerCount>& stepsPerLayer, bool running) {
    polyMode_ = true;
    transportRunning_ = running;
    polySteps_ = stepsPerLayer;
    playheadStep_ = running ? 0 : -1;
    repaint();
}

void LayerOverview::refresh() {
    patch_.layers[0].active = true;
    rebuildHitBounds();
    repaint();
}

float LayerOverview::designScale() const {
    return juce::jmin(static_cast<float>(getWidth()) / kBaseW, static_cast<float>(getHeight()) / kBaseH);
}

juce::Point<float> LayerOverview::designOrigin() const {
    const float s = designScale();
    return {(static_cast<float>(getWidth()) - kBaseW * s) * 0.5f,
            (static_cast<float>(getHeight()) - kBaseH * s) * 0.5f};
}

juce::Rectangle<float> LayerOverview::cellSlot(float leftPct, float topPx, float scale, juce::Point<float> origin) const {
    const float slot = kCell * scale;
    return {origin.x + (leftPct / 100.f) * kBaseW * scale, origin.y + topPx * scale, slot, slot};
}

void LayerOverview::rebuildHitBounds() {
    const float s = designScale();
    const auto origin = designOrigin();
    const float pad = 4.f * s;
    const float slot = kCell * s;

    for (int layer = 0; layer < kLayerCount; ++layer) {
        const auto* cells = layerCells(layer);
        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        for (int i = 0; i < kCellsPerLayer; ++i) {
            const auto slotRect = cellSlot(cells[i].leftPct, cells[i].topPx, s, origin);
            minX = juce::jmin(minX, slotRect.getX());
            minY = juce::jmin(minY, slotRect.getY());
            maxX = juce::jmax(maxX, slotRect.getRight());
            maxY = juce::jmax(maxY, slotRect.getBottom());
        }
        layerHitBounds_[static_cast<size_t>(layer)] =
            juce::Rectangle<float>(minX - pad, minY - pad, maxX - minX + slot + pad * 2.f, maxY - minY + slot + pad * 2.f)
                .toNearestInt();

        const auto& toggle = kToggles[layer];
        toggleHitBounds_[static_cast<size_t>(layer)] =
            cellSlot(toggle.leftPct, toggle.topPx, s, origin).toNearestInt();
    }
}

int LayerOverview::layerAt(juce::Point<int> pos, bool includeInactive) const {
    for (int layer = 0; layer < kLayerCount; ++layer) {
        if (!includeInactive && !patch_.layers[static_cast<size_t>(layer)].active)
            continue;
        if (layerHitBounds_[static_cast<size_t>(layer)].contains(pos))
            return layer;
    }
    return -1;
}

void LayerOverview::notifyLayerDataChanged(int layer) {
    if (onLayerDataChanged)
        onLayerDataChanged(layer);
    refresh();
}

void LayerOverview::showFloater(const juce::String& text, juce::Point<int> anchorInThis) {
    if (!floater_)
        return;
    constexpr int w = 120;
    constexpr int h = 28;
    int x = anchorInThis.x - w / 2;
    int y = anchorInThis.y - h - 8;
    x = juce::jlimit(4, juce::jmax(4, getWidth() - w - 4), x);
    y = juce::jlimit(4, juce::jmax(4, getHeight() - h - 4), y);
    floater_->setBounds(x, y, w, h);
    floater_->show(text);
    floater_->toFront(false);
}

void LayerOverview::showContextMenu(int layer, juce::Point<int> clickInOverview) {
    menuLayer_ = layer;
    const auto& layerState = patch_.layers[static_cast<size_t>(layer)];
    const bool active = layerState.active;

    menu_->enabled[static_cast<size_t>(MenuAction::CopyNotes)] = active;
    menu_->enabled[static_cast<size_t>(MenuAction::CopyNotesAndKnobs)] = active;
    menu_->enabled[static_cast<size_t>(MenuAction::PasteNotes)] = clipboard_.hasClipboard();
    menu_->enabled[static_cast<size_t>(MenuAction::PasteNotesAndKnobs)] = clipboard_.hasNotesAndKnobs();
    menu_->enabled[static_cast<size_t>(MenuAction::ResetValues)] = active;
    menu_->enabled[static_cast<size_t>(MenuAction::Undo)] = clipboard_.canUndo();

    auto* top = getTopLevelComponent();
    if (top == nullptr)
        return;

    // Close any prior menu before reopening (avoids duplicate listeners).
    if (menuOpen_)
        hideContextMenu();

    menuLayer_ = layer;
    menuOpen_ = true;
    ignoreGlobalClickUntilUp_ = true;

    top->addAndMakeVisible(*dismissLayer_);
    top->addAndMakeVisible(*menu_);

    dismissLayer_->setBounds(top->getLocalBounds());
    dismissLayer_->toFront(false);

    const float s = juce::jmax(0.75f, designScale() * 0.9f);
    const int menuW = ContextMenu::kMenuW;
    const int menuH = ContextMenu::menuHeight(s);

    // Anchor at the right-click point (flip if it would leave the window).
    const auto clickInTop = top->getLocalPoint(this, clickInOverview);
    int mx = clickInTop.x;
    int my = clickInTop.y;
    if (mx + menuW > top->getWidth() - 8)
        mx = clickInTop.x - menuW;
    if (my + menuH > top->getHeight() - 8)
        my = clickInTop.y - menuH;
    mx = juce::jlimit(8, juce::jmax(8, top->getWidth() - menuW - 8), mx);
    my = juce::jlimit(8, juce::jmax(8, top->getHeight() - menuH - 8), my);
    menu_->setBounds(mx, my, menuW, menuH);
    menu_->setScale(s);

    auto backdrop = matilda::ui::glass::captureBackdrop(*top, menu_->getBounds());
    menu_->setBackdrop(std::move(backdrop));
    menu_->setVisible(true);
    menu_->toFront(true);

    // Defer global dismiss listener — registering during this mouseDown would
    // see the same right-click and immediately close the menu.
    juce::Component::SafePointer<LayerOverview> safe(this);
    juce::MessageManager::callAsync([safe] {
        if (safe == nullptr || !safe->menuOpen_ || safe->globalListener_ == nullptr)
            return;
        if (auto* t = safe->getTopLevelComponent())
            t->addMouseListener(safe->globalListener_.get(), true);
    });
}

void LayerOverview::hideContextMenu() {
    if (!menuOpen_ && menuLayer_ < 0)
        return;
    menuOpen_ = false;
    menuLayer_ = -1;
    ignoreGlobalClickUntilUp_ = false;
    if (auto* top = getTopLevelComponent())
        top->removeMouseListener(globalListener_.get());
    if (menu_) {
        menu_->setVisible(false);
        menu_->clearBackdrop();
        if (menu_->getParentComponent() != nullptr)
            menu_->getParentComponent()->removeChildComponent(menu_.get());
    }
    if (dismissLayer_) {
        dismissLayer_->setVisible(false);
        if (dismissLayer_->getParentComponent() != nullptr)
            dismissLayer_->getParentComponent()->removeChildComponent(dismissLayer_.get());
    }
}

void LayerOverview::handleGlobalMouseDown(const juce::MouseEvent& e) {
    if (!menuOpen_ || menu_ == nullptr)
        return;
    if (ignoreGlobalClickUntilUp_)
        return;
    if (e.eventComponent == menu_.get() || menu_->isParentOf(e.eventComponent))
        return;
    if (e.eventComponent == dismissLayer_.get()) {
        hideContextMenu();
        return;
    }
    hideContextMenu();
}

void LayerOverview::handleMenuAction(MenuAction action) {
    if (menuLayer_ < 0 || menuLayer_ >= matilda::kLayerCount)
        return;

    const int layer = menuLayer_;
    auto& layerState = patch_.layers[static_cast<size_t>(layer)];
    const auto floaterAnchor = layerHitBounds_[static_cast<size_t>(layer)].getCentre();

    switch (action) {
        case MenuAction::CopyNotes:
            if (!layerState.active)
                return;
            clipboard_.copyFrom(layerState, matilda::LayerClipboard::Mode::NotesOnly);
            showFloater("Copied", floaterAnchor);
            break;

        case MenuAction::CopyNotesAndKnobs:
            if (!layerState.active)
                return;
            clipboard_.copyFrom(layerState, matilda::LayerClipboard::Mode::NotesAndKnobs);
            showFloater("Copied", floaterAnchor);
            break;

        case MenuAction::PasteNotes:
        case MenuAction::PasteNotesAndKnobs: {
            const bool knobs = action == MenuAction::PasteNotesAndKnobs;
            if (knobs ? !clipboard_.hasNotesAndKnobs() : !clipboard_.hasClipboard())
                return;
            clipboard_.pushUndo(matilda::LayerClipboard::snapshot(layer, layerState));
            const bool wasInactive = !layerState.active;
            if (!clipboard_.pasteOnto(layerState, knobs))
                return;
            if (wasInactive && onLayerActivated)
                onLayerActivated(layer);
            showFloater("Pasted", floaterAnchor);
            notifyLayerDataChanged(layer);
            break;
        }

        case MenuAction::ResetValues:
            if (!layerState.active)
                return;
            clipboard_.pushUndo(matilda::LayerClipboard::snapshot(layer, layerState));
            clipboard_.resetLayer(layerState);
            showFloater("Reset", floaterAnchor);
            notifyLayerDataChanged(layer);
            break;

        case MenuAction::Undo: {
            const int undone = clipboard_.undoOnto(patch_);
            if (undone < 0)
                return;
            const auto anchor = layerHitBounds_[static_cast<size_t>(undone)].getCentre();
            showFloater("Undo", anchor);
            if (onLayerActivated)
                onLayerActivated(undone);
            notifyLayerDataChanged(undone);
            break;
        }

        case MenuAction::Count:
            break;
    }
}

void LayerOverview::paint(juce::Graphics& g) {
    const float s = designScale();
    const auto origin = designOrigin();
    const int monoMiniPlayhead =
        playheadStep_ >= 0 ? miniGridIndexFromRowMajorStep(playheadStep_) : -1;

    const auto frame = matilda::images::miniGridFrame();
    if (frame.isValid()) {
        const auto frameDest = juce::Rectangle<float>(origin.x, origin.y + kFrameTop * s, kBaseW * s, kFrameH * s);
        g.drawImage(frame, frameDest, juce::RectanglePlacement::stretchToFit);
    }

    for (int layer = 0; layer < kLayerCount; ++layer) {
        const auto& layerState = patch_.layers[static_cast<size_t>(layer)];
        const bool active = layerState.active;
        int miniPlayheadIndex = -1;
        if (transportRunning_ && active) {
            if (polyMode_) {
                const int step = polySteps_[static_cast<size_t>(layer)];
                if (step >= 0)
                    miniPlayheadIndex = miniGridIndexFromRowMajorStep(step);
            } else if (layer == playingLayer_ && monoMiniPlayhead >= 0) {
                miniPlayheadIndex = monoMiniPlayhead;
            }
        }
        const auto* cells = layerCells(layer);

        for (int i = 0; i < kCellsPerLayer; ++i) {
            const auto slot = cellSlot(cells[i].leftPct, cells[i].topPx, s, origin);
            const bool isPlayhead = miniPlayheadIndex >= 0 && i == miniPlayheadIndex;
            const bool inRange = cellInActiveRange(layerState, i);
            if (!inRange)
                continue;

            const bool gateOn = cellGateOn(layerState, i);
            const bool showPlayheadLit = isPlayhead && gateOn;
            const bool showInactiveGem = !active || !gateOn;

            drawInactiveSocket(g, slot, s, showInactiveGem ? 1.f : 0.f);
            drawColoredGem(g, slot, layer, showPlayheadLit, showPlayheadLit, s, showInactiveGem ? 0.f : 1.f);
        }
    }

    for (int layer = 0; layer < kLayerCount; ++layer) {
        const auto& layerState = patch_.layers[static_cast<size_t>(layer)];
        const bool active = layerState.active;
        const bool selected = patch_.selectedLayer == layer;
        const auto& toggle = kToggles[layer];
        const auto slot = cellSlot(toggle.leftPct, toggle.topPx, s, origin);

        drawInactiveSocket(g, slot, s, active ? 0.f : 1.f);
        bool toggleOn = false;
        if (active) {
            if (!transportRunning_)
                toggleOn = selected;
            else if (polyMode_)
                toggleOn = true;
            else
                toggleOn = layer == playingLayer_;
        }
        drawColoredGem(g, slot, layer, toggleOn, toggleOn, s, active ? 1.f : 0.f);
    }
}

void LayerOverview::resized() {
    rebuildHitBounds();
}

void LayerOverview::mouseDown(const juce::MouseEvent& e) {
    const auto pos = e.getPosition();

    // macOS: secondary click / ctrl-click (isPopupMenu) or explicit right button.
    if (e.mods.isPopupMenu() || e.mods.isRightButtonDown()) {
        const int layer = layerAt(pos, true);
        if (layer >= 0)
            showContextMenu(layer, pos);
        return;
    }

    for (int layer = 0; layer < kLayerCount; ++layer) {
        if (toggleHitBounds_[static_cast<size_t>(layer)].contains(pos)) {
            if (layer == 0)
                return;
            auto& layerState = patch_.layers[static_cast<size_t>(layer)];
            layerState.active = !layerState.active;
            if (!layerState.active && patch_.selectedLayer == layer) {
                patch_.selectedLayer = 0;
                if (onLayerSelected)
                    onLayerSelected(0);
            }
            if (onLayerActivated)
                onLayerActivated(layer);
            refresh();
            return;
        }
    }

    for (int layer = 0; layer < kLayerCount; ++layer) {
        if (!patch_.layers[static_cast<size_t>(layer)].active)
            continue;
        if (!layerHitBounds_[static_cast<size_t>(layer)].contains(pos))
            continue;
        patch_.selectedLayer = layer;
        if (onLayerSelected)
            onLayerSelected(layer);
        refresh();
        return;
    }
}

void LayerOverview::mouseUp(const juce::MouseEvent&) {
    ignoreGlobalClickUntilUp_ = false;
}

bool LayerOverview::isOverInteractive(juce::Point<int> pos) const {
    for (int layer = 1; layer < kLayerCount; ++layer) {
        if (toggleHitBounds_[static_cast<size_t>(layer)].contains(pos))
            return true;
    }
    for (int layer = 0; layer < kLayerCount; ++layer) {
        // Right-click works on inactive; show hand over any layer body.
        if (layerHitBounds_[static_cast<size_t>(layer)].contains(pos))
            return true;
    }
    return false;
}

void LayerOverview::mouseMove(const juce::MouseEvent& e) {
    setMouseCursor(isOverInteractive(e.getPosition()) ? juce::MouseCursor::PointingHandCursor
                                                      : juce::MouseCursor::NormalCursor);
}

void LayerOverview::mouseExit(const juce::MouseEvent&) {
    setMouseCursor(juce::MouseCursor::NormalCursor);
}
