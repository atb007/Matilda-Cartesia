#include "QuantisePanel.h"
#include "../FiligreeDrawing.h"
#include "../GlassDropdownDrawing.h"
#include "../MatildaFonts.h"
#include "../MatildaImages.h"
#include "../ScaleLayout.h"
#include "../Engine/ScaleConfig.h"
#include "BinaryData.h"

namespace {

using namespace matilda::scale;

juce::Image rasterizeSvg(const char* data, int size, int w, int h) {
    const auto xml = juce::parseXML(juce::String::fromUTF8(data, static_cast<size_t>(size)));
    if (!xml)
        return {};
    const auto drawable = juce::Drawable::createFromSVG(*xml);
    if (!drawable)
        return {};
    juce::Image img(juce::Image::ARGB, w, h, true);
    juce::Graphics g(img);
    g.fillAll(juce::Colours::transparentBlack);
    drawable->drawWithin(g, juce::Rectangle<float>(0.f, 0.f, static_cast<float>(w), static_cast<float>(h)),
                         juce::RectanglePlacement::stretchToFit, 1.f);
    return img;
}

void drawImg(juce::Graphics& g, const juce::Image& img, juce::Rectangle<float> d) {
    if (img.isValid())
        g.drawImage(img, d, juce::RectanglePlacement::stretchToFit);
}

void drawFlipX(juce::Graphics& g, const juce::Image& img, juce::Rectangle<float> d) {
    if (!img.isValid())
        return;
    g.saveState();
    g.addTransform(juce::AffineTransform::scale(-1.f, 1.f, d.getCentreX(), d.getCentreY()));
    g.drawImage(img, d, juce::RectanglePlacement::stretchToFit);
    g.restoreState();
}

void drawNeonTitle(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, float s) {
    const float fs = kTitleFs * s;
    auto font = matilda::fonts::asimovian(fs);
    font.setExtraKerningFactor(kTitleTrack / fs);
    g.setFont(font);
    g.setColour(juce::Colour(0xff77a6ab).withAlpha(0.85f));
    g.drawText(text, area.translated(0.f, kNeonShadowY * s).toNearestInt(), juce::Justification::centred, false);
    g.setColour(juce::Colour(0xff10ffcf).withAlpha(0.35f));
    g.drawText(text, area.toNearestInt(), juce::Justification::centred, false);
    g.setColour(juce::Colours::white);
    g.drawText(text, area.toNearestInt(), juce::Justification::centred, false);
}

void drawDivider(juce::Graphics& g, juce::Rectangle<float> area, float s) {
    const float w = area.getWidth();
    if (w <= 0.5f)
        return;

    const float h = juce::jmax(3.f, kDividerH * s);
    const float x = area.getX();
    const float y = area.getCentreY() - h * 0.5f;
    const float cy = y + h * 0.5f;

    juce::ColourGradient grad(juce::Colours::white.withAlpha(0.f), x, cy, juce::Colours::white.withAlpha(0.7f),
                              x + w * 0.5f, cy, false);
    grad.addColour(1.f, juce::Colours::white.withAlpha(0.f));
    g.setGradientFill(grad);
    g.fillRect(x, y, w, h);
}

void drawChevron(juce::Graphics& g, juce::Rectangle<float> b, bool up) {
    juce::Path tri;
    if (up)
        tri.addTriangle(b.getCentreX(), b.getY(), b.getRight(), b.getBottom(), b.getX(), b.getBottom());
    else
        tri.addTriangle(b.getX(), b.getY(), b.getRight(), b.getY(), b.getCentreX(), b.getBottom());
    g.setColour(juce::Colours::white.withAlpha(0.85f));
    g.fillPath(tri);
}

float itemStride(float s) {
    return kDdItemFs * s * 1.25f + kDdLineGap * s + 1.f + kDdItemGap * s;
}

int menuHeight(int n, float s) {
    const int v = juce::jmin(n, kDdMaxVisibleItems);
    if (v <= 0)
        return juce::roundToInt(kDdPadY * s * 2.f);
    const float block = kDdItemFs * s * 1.25f + kDdLineGap * s + 1.f;
    return juce::roundToInt(kDdPadY * s * 2.f + block * v + kDdItemGap * s * (v - 1));
}

const juce::StringArray& pitches() {
    static const juce::StringArray p{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    return p;
}

void drawHeaderLabel(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> slot, float s) {
    auto font = matilda::fonts::supermercadoOne(kLabelFs * s);
    g.setFont(font);
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.drawText(text, slot.toNearestInt(), juce::Justification::centred, false);
}

} // namespace

enum class PickerVariant { Box, Pill };

class QuantisePanel::PickerRow : public juce::Component {
public:
    PickerRow(QuantisePanel& o, MenuId id, PickerVariant v) : owner_(o), id_(id), variant_(v) {}
    void setLabel(juce::String l) {
        label_ = std::move(l);
        repaint();
    }

    void paint(juce::Graphics& g) override {
        const auto b = getLocalBounds().toFloat();
        const float s = owner_.designScale();
        if (variant_ == PickerVariant::Pill) {
            const float r = b.getHeight() * 0.5f;
            g.setColour(juce::Colour(0x17ddd8d8));
            g.fillRoundedRectangle(b, r);
            g.setColour(juce::Colours::black.withAlpha(0.31f));
            g.drawLine(b.getX(), b.getBottom() - 2.f * s, b.getRight(), b.getBottom(), 3.f * s);
            g.setColour(juce::Colours::black.withAlpha(0.25f));
            g.drawLine(b.getX(), b.getY() + 4.f * s, b.getRight(), b.getY(), 3.f * s);
        } else {
            matilda::ui::glass::drawInlinePickerBox(g, b, s);
        }
        const float chevBlock = kChevronW * s + kChevronGap * s + 8.f * s;
        g.setFont(juce::Font(juce::FontOptions((variant_ == PickerVariant::Pill ? kTonicFs : kNoteFs) * s)
                                 .withStyle("SemiBold")));
        g.setColour(juce::Colours::white);
        g.drawText(label_, b.withTrimmedRight(chevBlock).toNearestInt(), juce::Justification::centred, false);
        const float cx = b.getRight() - chevBlock * 0.5f - kChevronW * s * 0.5f;
        const float my = b.getCentreY();
        drawChevron(g, {cx, my - kChevronH * s - kChevronGap * 0.5f * s, kChevronW * s, kChevronH * s}, true);
        drawChevron(g, {cx, my + kChevronGap * 0.5f * s, kChevronW * s, kChevronH * s}, false);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const float s = owner_.designScale();
        const float chevBlock = kChevronW * s + kChevronGap * s + 8.f * s;
        if (e.x >= getWidth() - chevBlock) {
            const bool up = e.y < getHeight() / 2;
            if (id_ == MenuId::Min)
                up ? owner_.cycleMinOct(1) : owner_.cycleMinOct(-1);
            else if (id_ == MenuId::Tonic)
                up ? owner_.cycleTonic(1) : owner_.cycleTonic(-1);
            else if (id_ == MenuId::Max)
                up ? owner_.cycleMaxOct(1) : owner_.cycleMaxOct(-1);
            return;
        }
        owner_.showMenu(owner_.openMenu_ == id_ ? MenuId::None : id_, this);
    }

private:
    QuantisePanel& owner_;
    MenuId id_;
    PickerVariant variant_;
    juce::String label_;
};

class QuantisePanel::ScaleArrowButton : public juce::Component {
public:
    ScaleArrowButton(QuantisePanel& o, bool left) : owner_(o), left_(left) {}
    void paint(juce::Graphics& g) override {
        const auto b = getLocalBounds().toFloat();
        juce::Path p;
        if (left_)
            p.addTriangle(b.getRight(), b.getY(), b.getRight(), b.getBottom(), b.getX(), b.getCentreY());
        else
            p.addTriangle(b.getX(), b.getY(), b.getX(), b.getBottom(), b.getRight(), b.getCentreY());
        juce::ColourGradient grad(juce::Colour(0xffececec), b.getTopLeft(), juce::Colour(0xff8d8d8d), b.getBottomLeft(),
                                  false);
        g.setGradientFill(grad);
        g.fillPath(p);
    }
    void mouseDown(const juce::MouseEvent&) override { owner_.cycleScale(left_ ? -1 : 1); }

private:
    QuantisePanel& owner_;
    bool left_;
};

class QuantisePanel::ScaleNameButton : public juce::Component {
public:
    explicit ScaleNameButton(QuantisePanel& o) : owner_(o) {}
    void setText(juce::String t) {
        text_ = std::move(t);
        repaint();
    }
    void paint(juce::Graphics& g) override {
        const float s = owner_.designScale();
        g.setFont(matilda::fonts::kodeMonoBold(kScaleBarFs * s));
        g.setColour(juce::Colours::white);
        g.drawText(text_, getLocalBounds(), juce::Justification::centred, false);
    }
    void mouseDown(const juce::MouseEvent&) override {
        owner_.showMenu(owner_.openMenu_ == MenuId::Scale ? MenuId::None : MenuId::Scale, this);
    }

private:
    QuantisePanel& owner_;
    juce::String text_;
};

class QuantisePanel::GlassMenu : public juce::Component {
public:
    explicit GlassMenu(QuantisePanel& o) : owner(o) {}
    QuantisePanel& owner;
    MenuId menuId = MenuId::None;
    int selectedIndex = 0;
    juce::StringArray items;
    int scrollOffset = 0;
    juce::Image backdrop_;

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

    void paint(juce::Graphics& g) override {
        const auto b = getLocalBounds().toFloat();
        matilda::ui::glass::drawPanel(g, b, scale_, backdrop_);
        const float closeSize = kDdClose * scale_;
        matilda::ui::glass::drawCloseIcon(
            g, {b.getRight() - 10.f * scale_ - closeSize, b.getY() + 10.f * scale_, closeSize, closeSize},
            juce::Colours::white.withAlpha(0.85f));
        const float itemW = b.getWidth() * 0.86f;
        const float itemX = b.getX() + (b.getWidth() - itemW) * 0.5f;
        const float listTop = b.getY() + kDdPadY * scale_;
        const float listBottom = b.getBottom() - kDdPadY * scale_;
        const float stride = itemStride(scale_);
        g.saveState();
        g.reduceClipRegion(juce::Rectangle<int>(juce::roundToInt(itemX), juce::roundToInt(listTop),
                                                juce::roundToInt(itemW), juce::roundToInt(listBottom - listTop)));
        g.setFont(matilda::fonts::kodeMonoBold(kDdItemFs * scale_));
        float y = listTop - scrollOffset * stride;
        for (int i = 0; i < items.size(); ++i) {
            const float th = kDdItemFs * scale_ * 1.25f;
            const auto tb = juce::Rectangle<float>(itemX, y, itemW, th);
            if (tb.getBottom() >= listTop && tb.getY() <= listBottom) {
                g.setColour(i == selectedIndex ? juce::Colours::white : juce::Colours::white.withAlpha(0.65f));
                g.drawText(items[i], tb.toNearestInt(), juce::Justification::centred, false);
            }
            y += th + kDdLineGap * scale_ + 1.f + kDdItemGap * scale_;
        }
        g.restoreState();
    }

    void mouseDown(const juce::MouseEvent& e) override {
        if (closeBounds_.contains(e.getPosition())) {
            owner.closeMenu();
            return;
        }
        for (int i = 0; i < itemBounds_.size(); ++i) {
            if (itemBounds_[i].contains(e.getPosition())) {
                const int idx = scrollOffset + i;
                switch (menuId) {
                    case MenuId::Min:
                        owner.applyMinOct(idx);
                        break;
                    case MenuId::Tonic:
                        owner.applyTonicIndex(idx);
                        break;
                    case MenuId::Max:
                        owner.applyMaxOct(idx);
                        break;
                    case MenuId::Scale:
                        owner.applyScaleIndex(idx);
                        break;
                    default:
                        break;
                }
                owner.closeMenu();
                return;
            }
        }
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& w) override {
        if (items.size() <= kDdMaxVisibleItems)
            return;
        const int maxS = items.size() - kDdMaxVisibleItems;
        const int d = w.deltaY > 0 ? -1 : w.deltaY < 0 ? 1 : 0;
        scrollOffset = juce::jlimit(0, maxS, scrollOffset + d);
        rebuild();
        repaint();
    }

    static int heightFor(int n, float s) { return menuHeight(n, s); }

private:
    float scale_ = 1.f;
    juce::Rectangle<int> closeBounds_;
    juce::Array<juce::Rectangle<int>> itemBounds_;

    void rebuild() {
        const auto b = getLocalBounds();
        const float cs = kDdClose * scale_;
        closeBounds_ = {juce::roundToInt(b.getRight() - 10.f * scale_ - cs), juce::roundToInt(b.getY() + 10.f * scale_),
                        juce::roundToInt(cs), juce::roundToInt(cs)};
        itemBounds_.clear();
        const float itemW = b.getWidth() * 0.86f;
        const float itemX = b.getX() + (b.getWidth() - itemW) * 0.5f;
        float y = b.getY() + kDdPadY * scale_;
        const int vis = juce::jmin(items.size(), kDdMaxVisibleItems);
        for (int i = 0; i < vis; ++i) {
            const float th = kDdItemFs * scale_ * 1.25f;
            itemBounds_.add({juce::roundToInt(itemX), juce::roundToInt(y), juce::roundToInt(itemW), juce::roundToInt(th)});
            y += itemStride(scale_);
        }
    }

    void resized() override { rebuild(); }
};

class QuantisePanel::GlobalClickListener : public juce::MouseListener {
public:
    explicit GlobalClickListener(QuantisePanel& o) : owner_(o) {}
    void mouseDown(const juce::MouseEvent& e) override { owner_.handleGlobalMouseDown(e); }

private:
    QuantisePanel& owner_;
};

class QuantisePanel::DismissLayer : public juce::Component {
public:
    std::function<void()> onDismiss;
    void mouseDown(const juce::MouseEvent&) override {
        if (onDismiss)
            onDismiss();
    }
};

class QuantisePanel::MinMaxChromeOverlay : public juce::Component {
public:
    explicit MinMaxChromeOverlay(QuantisePanel& panel) : panel_(panel) {
        setInterceptsMouseClicks(false, false);
        setPaintingIsUnclipped(true);
    }

    void paint(juce::Graphics& g) override { panel_.paintMinMaxConnectors(g); }

private:
    QuantisePanel& panel_;
};

QuantisePanel::QuantisePanel(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    setOpaque(false);
    setPaintingIsUnclipped(true);

    bgTextureImg_ = matilda::images::movementBgTexture();
    filigreeTop_ = matilda::ui::filigree::loadSvgDrawable(BinaryData::scaletitlefiligreetop_svg,
                                                          BinaryData::scaletitlefiligreetop_svgSize);
    filigreeBottom_ = matilda::ui::filigree::loadSvgDrawable(BinaryData::scaletitlefiligreebottom_svg,
                                                             BinaryData::scaletitlefiligreebottom_svgSize);
    minMaxOrnLeftImg_ = rasterizeSvg(BinaryData::scaleminmaxornamentleft_svg,
                                     BinaryData::scaleminmaxornamentleft_svgSize, 120, 40);
    minMaxOrnRightImg_ = rasterizeSvg(BinaryData::scaleminmaxornamentright_svg,
                                      BinaryData::scaleminmaxornamentright_svgSize, 120, 40);
    connectorLeftImg_ =
        rasterizeSvg(BinaryData::scaleconnectorleft_svg, BinaryData::scaleconnectorleft_svgSize, 236, 16);
    connectorRightImg_ =
        rasterizeSvg(BinaryData::scaleconnectorright_svg, BinaryData::scaleconnectorright_svgSize, 236, 16);

    minRow_ = std::make_unique<PickerRow>(*this, MenuId::Min, PickerVariant::Box);
    tonicRow_ = std::make_unique<PickerRow>(*this, MenuId::Tonic, PickerVariant::Pill);
    maxRow_ = std::make_unique<PickerRow>(*this, MenuId::Max, PickerVariant::Box);
    scalePrev_ = std::make_unique<ScaleArrowButton>(*this, true);
    scaleNext_ = std::make_unique<ScaleArrowButton>(*this, false);
    scaleName_ = std::make_unique<ScaleNameButton>(*this);
    glassMenu_ = std::make_unique<GlassMenu>(*this);
    dismissLayer_ = std::make_unique<DismissLayer>();
    globalClickListener_ = std::make_unique<GlobalClickListener>(*this);
    minMaxChrome_ = std::make_unique<MinMaxChromeOverlay>(*this);

    addAndMakeVisible(gemSparks_);
    addAndMakeVisible(*minRow_);
    addAndMakeVisible(*tonicRow_);
    addAndMakeVisible(*maxRow_);
    addAndMakeVisible(*scalePrev_);
    addAndMakeVisible(*scaleName_);
    addAndMakeVisible(*scaleNext_);
    addAndMakeVisible(*minMaxChrome_);

    syncFromPatch();
}

QuantisePanel::~QuantisePanel() {
    juce::Desktop::getInstance().removeGlobalMouseListener(globalClickListener_.get());
}

float QuantisePanel::designScale() const {
    return juce::jmin(static_cast<float>(getWidth()) / kBaseW, static_cast<float>(getHeight()) / kBaseH);
}

juce::Point<float> QuantisePanel::designOrigin() const {
    const float s = designScale();
    return {(static_cast<float>(getWidth()) - kBaseW * s) * 0.5f,
            (static_cast<float>(getHeight()) - kBaseH * s) * 0.5f};
}

juce::Rectangle<float> QuantisePanel::designRect(float x, float y, float w, float h) const {
    const float s = designScale();
    const auto o = designOrigin();
    return {o.x + x * s, o.y + y * s, w * s, h * s};
}

void QuantisePanel::syncFromPatch() {
    minRow_->setLabel(matilda::formatTonicRelativeNote(patch_.root, patch_.minOctave));
    tonicRow_->setLabel(patch_.root);
    maxRow_->setLabel(matilda::formatTonicRelativeNote(patch_.root, patch_.maxOctave));
    scaleName_->setText(matilda::scaleLabelForMode(patch_.mode));
    gemSparks_.setPanelScale(designScale());
    gemSparks_.setScaleModeId(patch_.mode);
    gemSparks_.setGemImage(matilda::images::scaleGemForMode(patch_.mode));
    minMaxChrome_->repaint();
    repaint();
}

void QuantisePanel::notifyChanged() {
    if (onChanged)
        onChanged();
}

void QuantisePanel::cycleScale(int dir) {
    const int idx = matilda::scaleModeIndex(patch_.mode);
    applyScaleIndex((idx + dir + matilda::kScaleModeCount) % matilda::kScaleModeCount);
}

void QuantisePanel::applyScaleIndex(int index) {
    patch_.mode = matilda::kScaleModes[static_cast<size_t>(index)].id;
    patch_.quantize = patch_.mode != "chromatic";
    syncFromPatch();
    notifyChanged();
}

void QuantisePanel::cycleTonic(int dir) {
    const int idx = juce::jmax(0, pitches().indexOf(patch_.root));
    applyTonicIndex((idx + dir + 12) % 12);
}

void QuantisePanel::applyTonicIndex(int index) {
    patch_.root = pitches()[index];
    syncFromPatch();
    notifyChanged();
}

void QuantisePanel::cycleMinOct(int dir) {
    applyMinOct(juce::jlimit(0, patch_.maxOctave - 1, patch_.minOctave + dir));
}

void QuantisePanel::cycleMaxOct(int dir) {
    applyMaxOct(juce::jlimit(patch_.minOctave + 1, 9, patch_.maxOctave + dir));
}

void QuantisePanel::applyMinOct(int oct) {
    patch_.minOctave = oct;
    if (patch_.maxOctave <= patch_.minOctave)
        patch_.maxOctave = juce::jmin(9, patch_.minOctave + 1);
    syncFromPatch();
    notifyChanged();
}

void QuantisePanel::applyMaxOct(int oct) {
    patch_.maxOctave = oct;
    if (patch_.maxOctave <= patch_.minOctave)
        patch_.minOctave = juce::jmax(0, patch_.maxOctave - 1);
    syncFromPatch();
    notifyChanged();
}

void QuantisePanel::showMenu(MenuId menu, juce::Component* anchor) {
    closeMenu();
    if (menu == MenuId::None || anchor == nullptr)
        return;
    openMenu_ = menu;
    if (auto* top = getTopLevelComponent()) {
        const float s = designScale();
        glassMenu_->menuId = menu;
        glassMenu_->setScale(s);
        glassMenu_->resetScroll();
        glassMenu_->items.clear();

        if (menu == MenuId::Min) {
            for (int o = 0; o < patch_.maxOctave; ++o)
                glassMenu_->items.add(matilda::formatTonicRelativeNote(patch_.root, o));
            glassMenu_->selectedIndex = patch_.minOctave;
        } else if (menu == MenuId::Tonic) {
            glassMenu_->items = pitches();
            glassMenu_->selectedIndex = juce::jmax(0, pitches().indexOf(patch_.root));
        } else if (menu == MenuId::Max) {
            for (int o = patch_.minOctave + 1; o <= 9; ++o)
                glassMenu_->items.add(matilda::formatTonicRelativeNote(patch_.root, o));
            glassMenu_->selectedIndex = patch_.maxOctave - patch_.minOctave - 1;
        } else {
            for (int i = 0; i < matilda::kScaleModeCount; ++i)
                glassMenu_->items.add(matilda::kScaleModes[static_cast<size_t>(i)].label);
            glassMenu_->selectedIndex = matilda::scaleModeIndex(patch_.mode);
            const int maxScroll = juce::jmax(0, glassMenu_->items.size() - kDdMaxVisibleItems);
            glassMenu_->scrollOffset = juce::jlimit(0, maxScroll, glassMenu_->selectedIndex - 2);
        }

        const auto rowScreen = anchor->localAreaToGlobal(anchor->getLocalBounds());
        const int ddW = juce::roundToInt(kDdW * s);
        const int ddH = GlassMenu::heightFor(glassMenu_->items.size(), s);
        const int ddX = rowScreen.getCentreX() - ddW / 2;
        const int ddY = rowScreen.getBottom() + juce::roundToInt((menu == MenuId::Scale ? 4 : 35) * s);
        const auto ddTopLeft = top->getLocalPoint(nullptr, juce::Point<int>(ddX, ddY));
        const auto snapArea = juce::Rectangle<int>(ddTopLeft.x, ddTopLeft.y, ddW, ddH);
        glassMenu_->setBackdrop(matilda::ui::glass::captureBackdrop(*top, snapArea));

        top->addAndMakeVisible(*dismissLayer_);
        dismissLayer_->onDismiss = [this] { closeMenu(); };
        dismissLayer_->setBounds(top->getLocalBounds());
        dismissLayer_->setAlwaysOnTop(true);
        top->addAndMakeVisible(*glassMenu_);
        glassMenu_->setBounds(snapArea);
        glassMenu_->setAlwaysOnTop(true);
        glassMenu_->toFront(true);
        dismissLayer_->toBehind(glassMenu_.get());
        glassMenu_->setVisible(true);
        dismissLayer_->setVisible(true);
        juce::Desktop::getInstance().addGlobalMouseListener(globalClickListener_.get());
    }
}

void QuantisePanel::closeMenu() {
    openMenu_ = MenuId::None;
    juce::Desktop::getInstance().removeGlobalMouseListener(globalClickListener_.get());
    if (glassMenu_->getParentComponent())
        glassMenu_->getParentComponent()->removeChildComponent(glassMenu_.get());
    if (dismissLayer_->getParentComponent())
        dismissLayer_->getParentComponent()->removeChildComponent(dismissLayer_.get());
    glassMenu_->setVisible(false);
    dismissLayer_->setVisible(false);
    glassMenu_->clearBackdrop();
}

void QuantisePanel::handleGlobalMouseDown(const juce::MouseEvent& e) {
    if (openMenu_ == MenuId::None || !glassMenu_->isVisible())
        return;
    const auto screen = e.getScreenPosition();
    if (getScreenBounds().contains(screen) || glassMenu_->getScreenBounds().contains(screen))
        return;
    closeMenu();
}

QuantisePanel::MinMaxChromeLayout QuantisePanel::minMaxChromeLayout() const {
    const float s = designScale();
    const auto minLabel = matilda::formatTonicRelativeNote(patch_.root, patch_.minOctave);
    const auto maxLabel = matilda::formatTonicRelativeNote(patch_.root, patch_.maxOctave);
    const float chevBlock = kChevronW + kChevronGap + 8.f;

    auto measureBox = [&](const juce::String& text) {
        juce::Font font(juce::FontOptions(kNoteFs * s).withStyle("SemiBold"));
        return font.getStringWidthFloat(text) / s + 2.f * kBoxPadX + chevBlock;
    };

    MinMaxChromeLayout layout;
    layout.minBoxW = measureBox(minLabel);
    layout.maxBoxW = measureBox(maxLabel);

    const float pickerSpan = layout.minBoxW + kPickerGap + kConnectorW + kPickerGap + kTonicPillW + kPickerGap
                           + kConnectorW + kPickerGap + layout.maxBoxW;
    const float pickerLeft = kMinMaxRowLeft + (kMinMaxRowW - pickerSpan) * 0.5f;

    float x = pickerLeft;
    layout.minX = x;
    x += layout.minBoxW + kPickerGap;
    layout.conn1X = x;
    x += kConnectorW + kPickerGap;
    layout.tonicX = x;
    x += kTonicPillW + kPickerGap;
    layout.conn2X = x;
    x += kConnectorW + kPickerGap;
    layout.maxX = x;

    const float labelRowH = juce::jmax(kLabelFs, kOrnamentH);
    layout.pickY = kMinMaxTop + labelRowH + kLabelRowGap;
    layout.pickRowH = juce::jmax(kMinMaxBoxH, kTonicPillMinH);
    layout.pickCentreY = layout.pickY + layout.pickRowH * 0.5f;
    layout.tonicY = layout.pickY + (layout.pickRowH - kTonicPillMinH) * 0.5f;

    layout.labelW = {kOrnamentW, kMinLabelSlotW, kDividerW, kTonicLabelSlotW, kDividerW, kMaxLabelSlotW, kOrnamentW};

    float sumW = 0.f;
    for (float w : layout.labelW)
        sumW += w;
    const float gap = juce::jmax(0.f, (kMinMaxRowW - sumW) / 6.f);
    float lx = kMinMaxRowLeft;
    for (size_t i = 0; i < layout.labelX.size(); ++i) {
        layout.labelX[i] = lx;
        lx += layout.labelW[i] + gap;
    }

    return layout;
}

void QuantisePanel::paintMinMaxHeader(juce::Graphics& g) const {
    const float s = designScale();
    const auto layout = minMaxChromeLayout();
    const float rowY = kMinMaxTop;
    const float rowH = juce::jmax(kLabelFs, kOrnamentH);
    const float ornY = rowY + (rowH - kOrnamentH) * 0.5f;
    const float divY = rowY + rowH * 0.5f - kDividerH * 0.5f;

    drawImg(g, minMaxOrnLeftImg_, designRect(layout.labelX[0], ornY, kOrnamentW, kOrnamentH));
    drawFlipX(g, minMaxOrnRightImg_, designRect(layout.labelX[6], ornY, kOrnamentW, kOrnamentH));

    drawDivider(g, designRect(layout.labelX[2], divY, kDividerW, kDividerH), s);
    drawDivider(g, designRect(layout.labelX[4], divY, kDividerW, kDividerH), s);

    drawHeaderLabel(g, "Min", designRect(layout.labelX[1], rowY, layout.labelW[1], rowH), s);
    drawHeaderLabel(g, "Tonic", designRect(layout.labelX[3], rowY, layout.labelW[3], rowH), s);
    drawHeaderLabel(g, "Max", designRect(layout.labelX[5], rowY, layout.labelW[5], rowH), s);
}

void QuantisePanel::paintMinMaxConnectors(juce::Graphics& g) const {
    const float s = designScale();
    const auto layout = minMaxChromeLayout();
    const float connDrawH = juce::jmax(kConnectorH, 3.f / s);
    drawImg(g, connectorLeftImg_,
            designRect(layout.conn1X, layout.pickCentreY - connDrawH * 0.5f, kConnectorW, connDrawH));
    drawImg(g, connectorRightImg_,
            designRect(layout.conn2X, layout.pickCentreY - connDrawH * 0.5f, kConnectorW, connDrawH));
}

void QuantisePanel::paint(juce::Graphics& g) {
    const float s = designScale();
    if (filigreeTop_)
        matilda::ui::filigree::drawDrawableInRect(g, *filigreeTop_,
                                                  designRect(kFiligreeCentreLeft, kFiligreeTop, kFiligreeW, kFiligreeH));
    matilda::ui::filigree::drawImage(g, bgTextureImg_,
                                     designRect(kTitleTextureLeft, kTitleTextureY, kTitleTextureW, kTitleTextureH));
    if (filigreeTop_)
        matilda::ui::filigree::drawDrawableFlippedVertical(
            g, *filigreeTop_, designRect(kFiligreeCentreLeft, kFiligreeBotTop, kFiligreeW, kFiligreeH));
    drawNeonTitle(g, "Quantise Scale", designRect(0.f, kTitleCenterY - kTitleFs * 0.5f, kBaseW, kTitleFs), s);
    paintMinMaxHeader(g);
}

void QuantisePanel::resized() {
    const float s = designScale();
    const auto layout = minMaxChromeLayout();
    minMaxChrome_->setBounds(getLocalBounds());

    minRow_->setBounds(
        designRect(layout.minX, layout.pickY + (layout.pickRowH - kMinMaxBoxH) * 0.5f, layout.minBoxW, kMinMaxBoxH)
            .toNearestInt());
    tonicRow_->setBounds(designRect(layout.tonicX, layout.tonicY, kTonicPillW, kTonicPillMinH).toNearestInt());
    maxRow_->setBounds(
        designRect(layout.maxX, layout.pickY + (layout.pickRowH - kMinMaxBoxH) * 0.5f, layout.maxBoxW, kMinMaxBoxH)
            .toNearestInt());

    gemSparks_.setBounds(designRect(kGemLeft, kGemTop, kGemW, kGemH).toNearestInt());
    gemSparks_.setPanelScale(s);

    const float barY = kScaleBarTop;
    const float barX = kScaleBarLeft;
    scalePrev_->setBounds(designRect(barX, barY + (kScaleBarH - kArrowH) * 0.5f, kArrowW, kArrowH).toNearestInt());
    scaleNext_->setBounds(
        designRect(barX + kScaleBarW - kArrowW, barY + (kScaleBarH - kArrowH) * 0.5f, kArrowW, kArrowH).toNearestInt());
    scaleName_->setBounds(
        designRect(barX + kArrowW, barY, kScaleBarW - 2.f * kArrowW, kScaleBarH).toNearestInt());
}
