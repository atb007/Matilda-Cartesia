#include "PresetBar.h"
#include "../GlassDropdownDrawing.h"
#include "../MatildaFonts.h"
#include "../ReactShellLayout.h"
#include "../ScaleLayout.h"
#include "../Engine/PatchStore.h"
#include "BinaryData.h"

namespace {
constexpr float kBaseW = matilda::react::kPresetBarW;
constexpr float kBaseH = matilda::react::kPresetBarH;
/** Match Quantise Scale / Global Settings module titles (ScaleLayout). */
constexpr float kTitleFs = matilda::scale::kTitleFs;
constexpr float kNameFs = 20.f;
constexpr float kTitleGap = 14.f;
constexpr float kDdPadY = 14.f;
constexpr float kDdClose = 18.f;
constexpr float kDropdownH = 38.f;

void drawPresetTitle(juce::Graphics& g, juce::Rectangle<float> area, float s) {
    using namespace matilda::scale;
    const float fs = kTitleFs * s;
    auto font = matilda::fonts::asimovian(fs);
    font.setExtraKerningFactor(kTitleTrack / fs);
    g.setFont(font);
    g.setColour(juce::Colour(0xff77a6ab).withAlpha(0.85f));
    g.drawText("Presets", area.translated(0.f, kNeonShadowY * s).toNearestInt(),
               juce::Justification::centred, false);
    g.setColour(juce::Colour(0xff10ffcf).withAlpha(0.35f));
    g.drawText("Presets", area.toNearestInt(), juce::Justification::centred, false);
    g.setColour(juce::Colours::white);
    g.drawText("Presets", area.toNearestInt(), juce::Justification::centred, false);
}
} // namespace

class PresetBar::DismissLayer : public juce::Component {
public:
    std::function<void()> onDismiss;
    void paint(juce::Graphics&) override {}
    void mouseDown(const juce::MouseEvent&) override {
        if (onDismiss)
            onDismiss();
    }
};

class PresetBar::GlobalClickListener : public juce::MouseListener {
public:
    explicit GlobalClickListener(PresetBar& o) : owner_(o) {}
    void mouseDown(const juce::MouseEvent& e) override { owner_.handleGlobalMouseDown(e); }

private:
    PresetBar& owner_;
};

class PresetBar::GlassMenu : public juce::Component {
public:
    explicit GlassMenu(PresetBar& o) : owner_(o) { setPaintingIsUnclipped(true); }

    juce::StringArray items;
    int selectedIndex = 0;
    int scrollOffset = 0;
    juce::Image backdrop_;
    std::function<void(int)> onSelect;
    std::function<void()> onClose;

    void setBackdrop(juce::Image img) {
        backdrop_ = std::move(img);
        repaint();
    }
    void clearBackdrop() { backdrop_ = {}; }
    void setScale(float s) {
        scale_ = s;
        rebuild();
        repaint();
    }
    void resetScroll() {
        scrollOffset = 0;
        rebuild();
        repaint();
    }

    static int heightFor(int n, float s) {
        using namespace matilda::ui::glass;
        using namespace matilda;
        const int vis = juce::jmin(n, PresetLibrary::kMaxVisibleDropdownItems);
        return ddMenuHeightScreen(vis, s, kDdPadY);
    }

    void paint(juce::Graphics& g) override {
        using namespace matilda::ui::glass;
        using namespace matilda;
        const auto b = getLocalBounds().toFloat();
        drawPanel(g, b, scale_, backdrop_);
        const float closeSize = kDdClose * scale_;
        drawCloseIcon(g,
                      { b.getRight() - 10.f * scale_ - closeSize, b.getY() + 10.f * scale_, closeSize, closeSize },
                      juce::Colours::white.withAlpha(0.85f));

        const float itemW = b.getWidth() * 0.86f;
        const float itemX = b.getX() + (b.getWidth() - itemW) * 0.5f;
        const float vertPad = ddMenuVertPadScreen(kDdPadY, scale_);
        const float listTop = b.getY() + vertPad;
        const float listBottom = b.getBottom() - vertPad;
        const float scrollStride = ddItemScrollStrideScreen();
        const float textH = ddItemTextHeightScreen();

        g.saveState();
        g.reduceClipRegion(juce::Rectangle<int>(juce::roundToInt(itemX), juce::roundToInt(listTop),
                                                juce::roundToInt(itemW), juce::roundToInt(listBottom - listTop)));
        g.setFont(matilda::fonts::kodeMonoBold(kDdItemScreenFs));
        float y = listTop - static_cast<float>(scrollOffset) * scrollStride;
        for (int i = 0; i < items.size(); ++i) {
            const auto tb = juce::Rectangle<float>(itemX, y, itemW, textH);
            const bool inView = tb.getBottom() >= listTop && tb.getY() <= listBottom;
            if (inView) {
                const bool selected = i == selectedIndex;
                g.setColour(selected ? juce::Colours::white : juce::Colours::white.withAlpha(0.65f));
                g.drawText(items[i], tb.toNearestInt(), juce::Justification::centred, false);
                if (selected) {
                    g.setColour(juce::Colour(0x7310ffcf));
                    g.drawText(items[i], tb.translated(0.f, 1.f).toNearestInt(), juce::Justification::centred,
                               false);
                }
            }
            if (inView && i + 1 < items.size()) {
                const float lineY = y + textH + kDdItemScreenLineGap;
                drawHairline(g, juce::Rectangle<float>(itemX, lineY, itemW, 1.f));
            }
            advanceDropdownItemY(y, i, items.size());
        }
        g.restoreState();
    }

    void mouseDown(const juce::MouseEvent& e) override {
        if (closeBounds_.contains(e.getPosition())) {
            if (onClose)
                onClose();
            return;
        }
        for (int i = 0; i < itemBounds_.size(); ++i) {
            if (itemBounds_[i].contains(e.getPosition())) {
                if (onSelect)
                    onSelect(scrollOffset + i);
                return;
            }
        }
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& w) override {
        using namespace matilda;
        if (items.size() <= PresetLibrary::kMaxVisibleDropdownItems)
            return;
        const int maxS = items.size() - PresetLibrary::kMaxVisibleDropdownItems;
        const int d = w.deltaY > 0 ? -1 : w.deltaY < 0 ? 1 : 0;
        scrollOffset = juce::jlimit(0, maxS, scrollOffset + d);
        rebuild();
        repaint();
    }

private:
    PresetBar& owner_;
    float scale_ = 1.f;
    juce::Rectangle<int> closeBounds_;
    juce::Array<juce::Rectangle<int>> itemBounds_;

    void rebuild() {
        using namespace matilda::ui::glass;
        using namespace matilda;
        const auto b = getLocalBounds().toFloat();
        const float closeSize = kDdClose * scale_;
        closeBounds_ = juce::Rectangle<float>(b.getRight() - 10.f * scale_ - closeSize,
                                              b.getY() + 10.f * scale_, closeSize, closeSize)
                           .toNearestInt();
        itemBounds_.clear();
        const float itemW = b.getWidth() * 0.86f;
        const float itemX = b.getX() + (b.getWidth() - itemW) * 0.5f;
        const float vertPad = ddMenuVertPadScreen(kDdPadY, scale_);
        const float listTop = b.getY() + vertPad;
        const float textH = ddItemTextHeightScreen();
        const int vis = juce::jmin(items.size() - scrollOffset, PresetLibrary::kMaxVisibleDropdownItems);
        float y = listTop;
        for (int i = 0; i < vis; ++i) {
            itemBounds_.add(juce::Rectangle<float>(itemX, y, itemW, textH).toNearestInt());
            advanceDropdownItemY(y, i, vis);
        }
        juce::ignoreUnused(owner_);
    }

    void resized() override { rebuild(); }
};

PresetBar::PresetBar(matilda::PatchState& patch, MatildaLookAndFeel& laf) : patch_(patch), laf_(laf) {
    juce::ignoreUnused(laf_);
    setOpaque(false);
    setPaintingIsUnclipped(true);

    matilda::PresetLibrary::ensureSeeded();

    if (auto xml = juce::XmlDocument::parse(
            juce::String::fromUTF8(BinaryData::presetsave_svg, BinaryData::presetsave_svgSize)))
        saveIcon_ = juce::Drawable::createFromSVG(*xml);
    if (auto xml = juce::XmlDocument::parse(
            juce::String::fromUTF8(BinaryData::presetchevrons_svg, BinaryData::presetchevrons_svgSize)))
        chevronsIcon_ = juce::Drawable::createFromSVG(*xml);

    menu_ = std::make_unique<GlassMenu>(*this);
    menu_->setVisible(false);
    menu_->onSelect = [this](int index) {
        loadPresetAt(index);
        showMenu(false);
    };
    menu_->onClose = [this] { showMenu(false); };

    dismissLayer_ = std::make_unique<DismissLayer>();
    dismissLayer_->setVisible(false);
    dismissLayer_->onDismiss = [this] { showMenu(false); };

    globalClickListener_ = std::make_unique<GlobalClickListener>(*this);

    currentName_ = matilda::PresetLibrary::kInitName;
    markClean();
}

PresetBar::~PresetBar() {
    showMenu(false);
    juce::Desktop::getInstance().removeGlobalMouseListener(globalClickListener_.get());
}

float PresetBar::designScale() const {
    return juce::jmin(static_cast<float>(getWidth()) / kBaseW, static_cast<float>(getHeight()) / kBaseH);
}

juce::String PresetBar::displayLabel() const {
    return dirty_ ? currentName_ + "*" : currentName_;
}

void PresetBar::markClean() {
    cleanJson_ = matilda::PatchStore::patchToJson(patch_);
    dirty_ = false;
    repaint();
}

void PresetBar::refreshDirtyFlag() {
    const auto now = matilda::PatchStore::patchToJson(patch_);
    const bool next = now != cleanJson_;
    if (next == dirty_)
        return;
    dirty_ = next;
    repaint();
}

void PresetBar::syncFromPatch() {
    refreshDirtyFlag();
}

void PresetBar::notePatchEdited() {
    refreshDirtyFlag();
}

void PresetBar::resized() {
    const float s = designScale();
    // Pack top-down (Figma): title → gap → dropdown; leftover height is bottom air.
    const float titleBlock = kTitleFs * s + kTitleGap * s;
    const float ddW = matilda::react::kPresetDropdownW * s;
    const float ddH = kDropdownH * s;
    const float save = matilda::react::kPresetSaveSize * s;
    const float gap = matilda::react::kPresetSaveGap * s;
    const float totalW = ddW + gap + save;
    const float x0 = (static_cast<float>(getWidth()) - totalW) * 0.5f;
    dropdownBounds_ = { x0, titleBlock, ddW, ddH };
    saveBounds_ = { x0 + ddW + gap, titleBlock + (ddH - save) * 0.5f, save, save };
}

void PresetBar::paint(juce::Graphics& g) {
    const float s = designScale();
    const float titleH = kTitleFs * s;

    // Centre title on the dropdown width only (not the save icon); Asimovian neon = module titles.
    drawPresetTitle(g,
                    { dropdownBounds_.getX(), 0.f, juce::jmax(1.f, dropdownBounds_.getWidth()), titleH },
                    s);

    matilda::ui::glass::drawInlinePickerBox(g, dropdownBounds_, s);
    g.setFont(matilda::fonts::kodeMonoBold(kNameFs * s));
    g.setColour(juce::Colours::white);
    auto textArea = dropdownBounds_.reduced(15.f * s, 6.f * s);
    textArea.removeFromRight(18.f * s);
    g.drawText(displayLabel(), textArea.toNearestInt(), juce::Justification::centred, true);

    if (chevronsIcon_ != nullptr) {
        const float cw = 9.f * s;
        const float ch = 16.5f * s;
        chevronsIcon_->drawWithin(
            g,
            { dropdownBounds_.getRight() - 15.f * s - cw, dropdownBounds_.getCentreY() - ch * 0.5f, cw, ch },
            juce::RectanglePlacement::centred, 1.f);
    }

    if (saveIcon_ != nullptr)
        saveIcon_->drawWithin(g, saveBounds_, juce::RectanglePlacement::centred, 1.f);
}

void PresetBar::mouseDown(const juce::MouseEvent& e) {
    if (saveBounds_.contains(e.position)) {
        runSaveDialog();
        return;
    }
    if (dropdownBounds_.contains(e.position))
        showMenu(!menuOpen_);
}

void PresetBar::showMenu(bool show) {
    menuOpen_ = show;
    if (!show) {
        juce::Desktop::getInstance().removeGlobalMouseListener(globalClickListener_.get());
        if (menu_->getParentComponent())
            menu_->getParentComponent()->removeChildComponent(menu_.get());
        if (dismissLayer_->getParentComponent())
            dismissLayer_->getParentComponent()->removeChildComponent(dismissLayer_.get());
        menu_->setVisible(false);
        dismissLayer_->setVisible(false);
        menu_->clearBackdrop();
        return;
    }

    if (auto* top = getTopLevelComponent()) {
        const float s = designScale();
        menu_->items = matilda::PresetLibrary::listPresetNames();
        menu_->selectedIndex = juce::jmax(0, menu_->items.indexOf(currentName_));
        menu_->setScale(s);
        menu_->resetScroll();
        const int maxScroll =
            juce::jmax(0, menu_->items.size() - matilda::PresetLibrary::kMaxVisibleDropdownItems);
        menu_->scrollOffset =
            juce::jlimit(0, maxScroll, menu_->selectedIndex - matilda::PresetLibrary::kMaxVisibleDropdownItems / 2);

        const auto ddScreen = localAreaToGlobal(dropdownBounds_.toNearestInt());
        const int minW = juce::roundToInt(matilda::react::kPresetDropdownW * s);
        const int ddW = matilda::ui::glass::ddMenuWidthForItems(menu_->items, minW);
        const int ddH = GlassMenu::heightFor(menu_->items.size(), s);
        const int ddX = ddScreen.getCentreX() - ddW / 2;
        const int ddY = ddScreen.getBottom() + juce::roundToInt(4.f * s);
        const auto ddTopLeft = top->getLocalPoint(nullptr, juce::Point<int>(ddX, ddY));
        const auto snapArea = juce::Rectangle<int>(ddTopLeft.x, ddTopLeft.y, ddW, ddH);
        menu_->setBackdrop(matilda::ui::glass::captureBackdrop(*top, snapArea));

        top->addAndMakeVisible(*dismissLayer_);
        dismissLayer_->setBounds(top->getLocalBounds());
        dismissLayer_->setAlwaysOnTop(true);
        top->addAndMakeVisible(*menu_);
        menu_->setBounds(snapArea);
        menu_->setAlwaysOnTop(true);
        menu_->toFront(true);
        dismissLayer_->toBehind(menu_.get());
        menu_->setVisible(true);
        dismissLayer_->setVisible(true);
        juce::Desktop::getInstance().addGlobalMouseListener(globalClickListener_.get());
    }
}

void PresetBar::handleGlobalMouseDown(const juce::MouseEvent& e) {
    if (!menuOpen_ || !menu_->isVisible())
        return;
    const auto screen = e.getScreenPosition();
    if (getScreenBounds().contains(screen) || menu_->getScreenBounds().contains(screen))
        return;
    showMenu(false);
}

void PresetBar::loadPresetAt(int index) {
    if (!juce::isPositiveAndBelow(index, menu_->items.size()))
        return;
    const auto name = menu_->items[index];
    if (onLoadPreset)
        onLoadPreset(name);
    currentName_ = name;
    markClean();
}

void PresetBar::runSaveDialog() {
    matilda::PresetLibrary::ensureSeeded();
    // Start in the library folder. macOS native panels often wander; we always
    // register the chosen basename into presetsDirectory so the dropdown stays in sync.
    auto chooser = std::make_shared<juce::FileChooser>(
        "Save Matilda preset", matilda::PresetLibrary::fileForName(currentName_), "*.json");
    constexpr auto flags =
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
        | juce::FileBrowserComponent::warnAboutOverwriting;
    chooser->launchAsync(flags, [this, chooser](const juce::FileChooser& fc) {
        juce::ignoreUnused(chooser);
        auto chosen = fc.getResult();
        if (chosen == juce::File())
            return;
        if (chosen.getFileExtension().isEmpty())
            chosen = chosen.withFileExtension(".json");

        const auto name = matilda::PresetLibrary::displayNameFromFile(chosen);
        const auto libraryFile = matilda::PresetLibrary::fileForName(name);
        if (!matilda::PresetLibrary::saveToFile(libraryFile, patch_))
            return;
        // Optional export copy when the user picked a folder outside the library.
        if (chosen.getFullPathName() != libraryFile.getFullPathName())
            (void) matilda::PresetLibrary::saveToFile(chosen, patch_);

        currentName_ = name;
        markClean();
        if (onSaved)
            onSaved();
    });
}
