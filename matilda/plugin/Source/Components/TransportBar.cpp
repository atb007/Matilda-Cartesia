#include "TransportBar.h"
#include "../ClickFeedbackDrawing.h"
#include "../FiligreeDrawing.h"
#include "../GlassDropdownDrawing.h"
#include "../MatildaFonts.h"
#include "../MatildaImages.h"
#include "../TransportConfig.h"
#include "../TransportLayout.h"
#include "BinaryData.h"

namespace {

using namespace matilda::transport;
using namespace matilda::ui::filigree;

inline constexpr float kSectionOrnW = 88.32f;
inline constexpr float kSectionOrnH = 20.f;

juce::Image rasterizePlainSvg(const char* data, int size, int width, int height) {
    const auto xml = juce::parseXML(juce::String::fromUTF8(data, static_cast<size_t>(size)));
    if (!xml)
        return {};
    const auto drawable = juce::Drawable::createFromSVG(*xml);
    if (!drawable)
        return {};

    juce::Image img(juce::Image::ARGB, width, height, true);
    juce::Graphics g(img);
    g.fillAll(juce::Colours::transparentBlack);
    drawable->drawWithin(g, juce::Rectangle<float>(0.f, 0.f, static_cast<float>(width), static_cast<float>(height)),
                         juce::RectanglePlacement::stretchToFit, 1.f);
    return img;
}

void drawNeonTitle(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, float scale) {
    const float fs = kTitleFs * scale;
    auto font = matilda::fonts::asimovian(fs);
    font.setExtraKerningFactor(kTitleTrack / fs);
    g.setFont(font);

    g.setColour(juce::Colour(0xff77a6ab).withAlpha(0.85f));
    g.drawText(text, area.translated(0.f, kNeonShadowY * scale).toNearestInt(), juce::Justification::centred, false);
    g.setColour(juce::Colour(0xff10ffcf).withAlpha(0.35f));
    g.drawText(text, area.toNearestInt(), juce::Justification::centred, false);
    g.setColour(juce::Colours::white);
    g.drawText(text, area.toNearestInt(), juce::Justification::centred, false);
}

void drawSectionHeader(juce::Graphics& g, const juce::Image& ornLeft, const juce::Image& ornRight,
                       const juce::String& text, juce::Rectangle<float> row, float scale) {
    const float ornW = kSectionOrnW * scale;
    const float ornH = kSectionOrnH * scale;
    const float ornY = row.getY() + (row.getHeight() - ornH) * 0.5f + 2.f * scale;

    drawImage(g, ornLeft, {row.getX(), ornY, ornW, ornH});
    drawImageFlipped180ScaleX(g, ornRight, {row.getRight() - ornW, ornY, ornW, ornH});

    g.setFont(matilda::fonts::supermercadoOne(kLabelFs * scale));
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.drawText(text, row.toNearestInt(), juce::Justification::centred, false);
}

void drawChevron(juce::Graphics& g, juce::Rectangle<float> bounds, bool up) {
    juce::Path tri;
    if (up)
        tri.addTriangle(bounds.getCentreX(), bounds.getY(), bounds.getRight(), bounds.getBottom(), bounds.getX(),
                        bounds.getBottom());
    else
        tri.addTriangle(bounds.getX(), bounds.getY(), bounds.getRight(), bounds.getY(), bounds.getCentreX(),
                        bounds.getBottom());
    g.setColour(juce::Colour(0xff9c9c9c));
    g.fillPath(tri);
}

juce::Rectangle<float> glassInsetRect(juce::Rectangle<float> outer) {
    return {outer.getX() + outer.getWidth() * kGlassInsetLeft,
            outer.getY() + outer.getHeight() * kGlassInsetTop,
            outer.getWidth() * (1.f - kGlassInsetLeft - kGlassInsetRight),
            outer.getHeight() * (1.f - kGlassInsetTop - kGlassInsetBottom)};
}

float itemBlockHeight(float scale) {
    return kDdItemFs * scale * 1.25f + kDdLineGap * scale + 1.f;
}

float itemStride(float scale) {
    return itemBlockHeight(scale) + kDdItemGap * scale;
}

int menuHeightForItems(int itemCount, float scale) {
    const int visible = juce::jmin(itemCount, kDdMaxVisibleClockItems);
    if (visible <= 0)
        return juce::roundToInt(kDdPadY * scale * 2.f);
    const float block = itemBlockHeight(scale);
    return juce::roundToInt(kDdPadY * scale * 2.f + block * static_cast<float>(visible)
                            + kDdItemGap * scale * static_cast<float>(visible - 1));
}

} // namespace

class TransportBar::PlayButton : public juce::Component {
public:
    explicit PlayButton(TransportBar& owner) : owner_(owner) {
        setPaintingIsUnclipped(true);
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();
        const float bleed = kPlayFrameBleed * owner_.designScale();
        const auto core = bounds.reduced(bleed);
        matilda::ui::paintWithPressScale(g, core, pressed_ && !owner_.hostSyncLocked_, 0.94f);
        const auto frame = matilda::images::transportPlayFrame();
        const auto glass = matilda::images::transportGlassBg();
        const auto playIcon = matilda::images::transportPlayIcon();
        const auto stopIcon = matilda::images::transportStopIcon();
        const auto linkIcon = matilda::images::transportPlayLinkIcon();
        const auto inset = glassInsetRect(core);

        if (glass.isValid())
            g.drawImage(glass, inset, juce::RectanglePlacement::stretchToFit);

        if (frame.isValid())
            g.drawImage(frame, core, juce::RectanglePlacement::centred);

        if (owner_.hostSyncLocked_ && linkIcon.isValid()) {
            constexpr float kLinkAspect = 75.f / 45.f;
            const float maxW = inset.getWidth() * 0.58f;
            const float maxH = inset.getHeight() * 0.36f;
            float iconH = maxH;
            float iconW = iconH * kLinkAspect;
            if (iconW > maxW) {
                iconW = maxW;
                iconH = iconW / kLinkAspect;
            }
            const auto iconDest = juce::Rectangle<float>(iconW, iconH).withCentre(inset.getCentre());
            g.drawImage(linkIcon, iconDest, juce::RectanglePlacement::centred);
        } else {
            const auto& icon = owner_.playing_ ? stopIcon : playIcon;
            if (icon.isValid()) {
                const float iconW = owner_.playing_ ? inset.getWidth() * 0.473f : inset.getWidth() * 0.429f;
                const float iconH = owner_.playing_ ? inset.getHeight() * 0.484f : inset.getHeight() * 0.545f;
                const auto iconDest = juce::Rectangle<float>(iconW, iconH).withCentre(inset.getCentre());
                g.drawImage(icon, iconDest, juce::RectanglePlacement::centred);
            }
        }
    }

    void mouseDown(const juce::MouseEvent&) override {
        if (owner_.hostSyncLocked_)
            return;
        pressed_ = true;
        repaint();
    }

    void mouseUp(const juce::MouseEvent&) override {
        const bool wasPressed = pressed_;
        pressed_ = false;
        repaint();
        if (!wasPressed || owner_.hostSyncLocked_)
            return;
        if (owner_.playing_) {
            if (owner_.onStop)
                owner_.onStop();
        } else if (owner_.onPlay)
            owner_.onPlay();
    }

    void mouseEnter(const juce::MouseEvent&) override {
        if (!owner_.hostSyncLocked_)
            setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }

    void mouseExit(const juce::MouseEvent&) override {
        if (pressed_) {
            pressed_ = false;
            repaint();
        }
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }

private:
    TransportBar& owner_;
    bool pressed_ = false;
};

class TransportBar::SettingRow : public juce::Component {
public:
    SettingRow(TransportBar& owner, MenuId menuId) : owner_(owner), menuId_(menuId) {}

    void setLabel(juce::String label) {
        label_ = std::move(label);
        repaint();
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();
        const float s = owner_.designScale();

        matilda::ui::glass::drawInlinePickerBox(g, bounds, s);

        g.setFont(matilda::fonts::kodeMonoBold(kValueFs * s));
        g.setColour(juce::Colours::white);
        const float chevronBlock = kChevronW * s + kChevronGap * s + 4.f * s;
        const auto textArea = bounds.withTrimmedRight(chevronBlock);
        g.drawText(label_, textArea.toNearestInt(), juce::Justification::centred, false);

        const float chevronW = kChevronW * s;
        const float chevronH = 4.5f * s;
        const float chevronX = bounds.getRight() - chevronBlock * 0.5f - chevronW * 0.5f;
        const float chevronMidY = bounds.getCentreY();
        drawChevron(g, {chevronX, chevronMidY - chevronH - kChevronGap * 0.5f * s, chevronW, chevronH}, true);
        drawChevron(g, {chevronX, chevronMidY + kChevronGap * 0.5f * s, chevronW, chevronH}, false);
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const auto pos = e.getPosition().toFloat();
        const float s = owner_.designScale();
        const float chevronBlock = kChevronW * s + kChevronGap * s + 4.f * s;
        const auto bounds = getLocalBounds().toFloat();

        if (pos.x >= bounds.getRight() - chevronBlock) {
            const float chevronH = 4.5f * s;
            const float chevronMidY = bounds.getCentreY();
            if (pos.y < chevronMidY) {
                if (menuId_ == MenuId::PlayMode)
                    owner_.cyclePlayMode(-1);
                else
                    owner_.cycleClock(-1);
            } else {
                if (menuId_ == MenuId::PlayMode)
                    owner_.cyclePlayMode(1);
                else
                    owner_.cycleClock(1);
            }
            return;
        }

        owner_.showMenu(owner_.openMenu_ == menuId_ ? MenuId::None : menuId_);
    }

private:
    TransportBar& owner_;
    MenuId menuId_;
    juce::String label_;
};

class TransportBar::GlassMenu : public juce::Component {
public:
    explicit GlassMenu(TransportBar& o) : owner(o) {}

    TransportBar& owner;
    MenuId menuId = MenuId::None;
    int selectedIndex = 0;
    juce::StringArray items;
    int scrollOffset = 0;
    juce::Image backdrop_;

    void setBackdrop(juce::Image img) {
        backdrop_ = std::move(img);
        repaint();
    }

    void setScale(float scale) {
        scale_ = scale;
        rebuildHitAreas();
        repaint();
    }

    void clearBackdrop() { backdrop_ = {}; }

    void resetScroll() {
        scrollOffset = 0;
        rebuildHitAreas();
        repaint();
    }

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();
        matilda::ui::glass::drawPanel(g, bounds, scale_, backdrop_);

        const float closeSize = kDdClose * scale_;
        const auto closeBounds = juce::Rectangle<float>(bounds.getRight() - 10.f * scale_ - closeSize,
                                                       bounds.getY() + 10.f * scale_, closeSize, closeSize);
        matilda::ui::glass::drawCloseIcon(
            g, closeBounds, juce::Colours::white.withAlpha(closeHover_ ? 1.f : 0.85f));

        const float itemW = bounds.getWidth() * 0.86f;
        const float itemX = bounds.getX() + (bounds.getWidth() - itemW) * 0.5f;
        const float listTop = bounds.getY() + kDdPadY * scale_;
        const float listBottom = bounds.getBottom() - kDdPadY * scale_;
        const float stride = itemStride(scale_);

        g.saveState();
        g.reduceClipRegion(juce::Rectangle<int>(juce::roundToInt(itemX), juce::roundToInt(listTop),
                                                juce::roundToInt(itemW), juce::roundToInt(listBottom - listTop)));
        g.setFont(matilda::fonts::kodeMonoBold(kDdItemFs * scale_));

        float y = listTop - static_cast<float>(scrollOffset) * stride;
        for (int i = 0; i < items.size(); ++i) {
            const float textH = kDdItemFs * scale_ * 1.25f;
            const auto textBounds = juce::Rectangle<float>(itemX, y, itemW, textH);
            if (textBounds.getBottom() >= listTop && textBounds.getY() <= listBottom) {
                const bool selected = i == selectedIndex;
                g.setColour(selected ? juce::Colours::white : juce::Colours::white.withAlpha(0.65f));
                g.drawText(items[i], textBounds.toNearestInt(), juce::Justification::centred, false);
                if (selected) {
                    g.setColour(juce::Colour(0x7310ffcf));
                    g.drawText(items[i], textBounds.translated(0.f, 1.f * scale_).toNearestInt(),
                               juce::Justification::centred, false);
                }
            }
            y += textH + kDdLineGap * scale_;
            if (textBounds.getBottom() >= listTop && textBounds.getY() <= listBottom)
                matilda::ui::glass::drawHairline(g, juce::Rectangle<float>(itemX, y, itemW, 1.f));
            y += 1.f + kDdItemGap * scale_;
        }
        g.restoreState();

        if (items.size() > kDdMaxVisibleClockItems) {
            const float trackX = bounds.getRight() - 8.f * scale_;
            const float trackW = 5.f * scale_;
            const float trackH = listBottom - listTop;
            const float trackY = listTop;
            g.setColour(juce::Colours::transparentBlack);
            g.fillRect(trackX, trackY, trackW, trackH);

            const float thumbRatio = static_cast<float>(kDdMaxVisibleClockItems) / static_cast<float>(items.size());
            const float thumbH = juce::jmax(12.f * scale_, trackH * thumbRatio);
            const int maxScroll = juce::jmax(0, items.size() - kDdMaxVisibleClockItems);
            const float thumbTravel = trackH - thumbH;
            const float thumbY = maxScroll > 0 ? trackY + thumbTravel * (static_cast<float>(scrollOffset) / maxScroll)
                                               : trackY;
            g.setColour(juce::Colours::white.withAlpha(scrollHover_ ? 0.32f : 0.18f));
            g.fillRoundedRectangle(trackX, thumbY, trackW, thumbH, trackW * 0.5f);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const auto pos = e.getPosition();
        if (closeBounds_.contains(pos)) {
            owner.closeMenu();
            return;
        }
        for (int i = 0; i < itemBounds_.size(); ++i) {
            if (itemBounds_[i].contains(pos)) {
                const int itemIndex = scrollOffset + i;
                if (menuId == MenuId::PlayMode)
                    owner.applyPlayMode(itemIndex);
                else
                    owner.applyClock(itemIndex);
                owner.closeMenu();
                return;
            }
        }
    }

    void mouseMove(const juce::MouseEvent& e) override {
        const bool hover = closeBounds_.contains(e.getPosition());
        if (hover != closeHover_) {
            closeHover_ = hover;
            repaint();
        }

        if (items.size() > kDdMaxVisibleClockItems) {
            const auto bounds = getLocalBounds().toFloat();
            const float listTop = bounds.getY() + kDdPadY * scale_;
            const float listBottom = bounds.getBottom() - kDdPadY * scale_;
            const float trackX = bounds.getRight() - 8.f * scale_;
            const bool thumbHover = e.position.x >= trackX && e.y >= listTop && e.y <= listBottom;
            if (thumbHover != scrollHover_) {
                scrollHover_ = thumbHover;
                repaint();
            }
        }
    }

    void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails& wheel) override {
        if (items.size() <= kDdMaxVisibleClockItems)
            return;

        const int maxScroll = items.size() - kDdMaxVisibleClockItems;
        const int delta = wheel.deltaY > 0.f ? -1 : wheel.deltaY < 0.f ? 1 : 0;
        const int next = juce::jlimit(0, maxScroll, scrollOffset + delta);
        if (next != scrollOffset) {
            scrollOffset = next;
            rebuildHitAreas();
            repaint();
        }
    }

    static int menuHeight(int itemCount, float scale) { return menuHeightForItems(itemCount, scale); }

private:
    float scale_ = 1.f;
    juce::Rectangle<int> closeBounds_;
    juce::Array<juce::Rectangle<int>> itemBounds_;
    bool closeHover_ = false;
    bool scrollHover_ = false;

    void rebuildHitAreas() {
        const auto bounds = getLocalBounds();
        const float closeSize = kDdClose * scale_;
        closeBounds_ = juce::Rectangle<int>(
            juce::roundToInt(bounds.getRight() - 10.f * scale_ - closeSize),
            juce::roundToInt(bounds.getY() + 10.f * scale_),
            juce::roundToInt(closeSize), juce::roundToInt(closeSize));

        itemBounds_.clear();
        const float itemW = bounds.getWidth() * 0.86f;
        const float itemX = bounds.getX() + (bounds.getWidth() - itemW) * 0.5f;
        const float listTop = bounds.getY() + kDdPadY * scale_;
        const float stride = itemStride(scale_);
        float y = listTop;
        const int visible = juce::jmin(items.size(), kDdMaxVisibleClockItems);
        for (int i = 0; i < visible; ++i) {
            const float textH = kDdItemFs * scale_ * 1.25f;
            itemBounds_.add(juce::Rectangle<int>(juce::roundToInt(itemX), juce::roundToInt(y),
                                                juce::roundToInt(itemW), juce::roundToInt(textH)));
            y += stride;
        }
    }

    void resized() override { rebuildHitAreas(); }
};

class TransportBar::GlobalClickListener : public juce::MouseListener {
public:
    explicit GlobalClickListener(TransportBar& owner) : owner_(owner) {}
    void mouseDown(const juce::MouseEvent& e) override { owner_.handleGlobalMouseDown(e); }

private:
    TransportBar& owner_;
};

class TransportBar::DismissLayer : public juce::Component {
public:
    std::function<void()> onDismiss;
    void mouseDown(const juce::MouseEvent&) override {
        if (onDismiss)
            onDismiss();
    }
};

TransportBar::TransportBar(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    setOpaque(false);
    setPaintingIsUnclipped(true);

    bgTextureImg_ = matilda::images::movementBgTexture();

    Layout filigreeLayout;
    filigreeLayout.filigreeW = kFiligreeW;
    filigreeLayout.filigreeLeft = kFiligreeTopLeft;
    filigreeLayout.textureW = kTitleTextureW;
    filigreeLayout.textureLeft = kTitleTextureLeft;
    filigreeLayout.alphaScale = kTitleFiligreeAlphaScale;
    filigreeLayout.grey = kTitleFiligreeGrey;

    const int filigreeW2x = juce::roundToInt(kFiligreeW * 2.f);
    const int filigreeH2x = juce::roundToInt(kFiligreeH * 2.f);
    filigreeTopImg_ = rasterizeSvg(BinaryData::movementfiligreetop_svg, BinaryData::movementfiligreetop_svgSize,
                                  filigreeW2x, filigreeH2x, bgTextureImg_, filigreeLayout);
    filigreeBottomImg_ = flipImageVertically(filigreeTopImg_);

    sectionOrnLeftImg_ = rasterizePlainSvg(BinaryData::transportornamentclockleft_svg,
                                           BinaryData::transportornamentclockleft_svgSize,
                                           juce::roundToInt(kSectionOrnW * 2.f), 40);
    sectionOrnRightImg_ = rasterizePlainSvg(BinaryData::transportornamentclockright_svg,
                                            BinaryData::transportornamentclockright_svgSize,
                                            juce::roundToInt(kSectionOrnW * 2.f), 40);

    playButton_ = std::make_unique<PlayButton>(*this);
    playModeRow_ = std::make_unique<SettingRow>(*this, MenuId::PlayMode);
    clockRow_ = std::make_unique<SettingRow>(*this, MenuId::Clock);
    glassMenu_ = std::make_unique<GlassMenu>(*this);
    dismissLayer_ = std::make_unique<DismissLayer>();
    globalClickListener_ = std::make_unique<GlobalClickListener>(*this);

    addAndMakeVisible(*playButton_);
    addAndMakeVisible(*playModeRow_);
    addAndMakeVisible(*clockRow_);

    syncFromPatch();
}

TransportBar::~TransportBar() {
    juce::Desktop::getInstance().removeGlobalMouseListener(globalClickListener_.get());
}

void TransportBar::setPlaying(bool playing) {
    playing_ = playing;
    playButton_->repaint();
}

void TransportBar::setHostSyncLocked(bool locked) {
    if (hostSyncLocked_ != locked) {
        hostSyncLocked_ = locked;
        playButton_->repaint();
    }
}

void TransportBar::syncFromPatch() {
    playModeRow_->setLabel(playModeLabel(patch_.playMode));
    clockRow_->setLabel(clockLabelAtIndex(clockIndexForDivision(patch_.masterDivision)));
    repaint();
}

float TransportBar::designScale() const {
    return juce::jmin(static_cast<float>(getWidth()) / kBaseW, static_cast<float>(getHeight()) / kBaseH);
}

juce::Point<float> TransportBar::designOrigin() const {
    const float s = designScale();
    return {(static_cast<float>(getWidth()) - kBaseW * s) * 0.5f,
            (static_cast<float>(getHeight()) - kBaseH * s) * 0.5f};
}

juce::Rectangle<float> TransportBar::designRect(float x, float y, float w, float h) const {
    const float s = designScale();
    const auto origin = designOrigin();
    return {origin.x + x * s, origin.y + y * s, w * s, h * s};
}

void TransportBar::cyclePlayMode(int dir) {
    const int current = patch_.playMode == matilda::PlayMode::Transport ? 0 : 1;
    applyPlayMode((current + dir + 2) % 2);
}

void TransportBar::cycleClock(int dir) {
    const int current = clockIndexForDivision(patch_.masterDivision);
    applyClock((current + dir + matilda::kClockDivisionCount) % matilda::kClockDivisionCount);
}

void TransportBar::applyPlayMode(int index) {
    patch_.playMode = index == 0 ? matilda::PlayMode::Transport : matilda::PlayMode::Note;
    syncFromPatch();
    if (onSettingsChanged)
        onSettingsChanged();
}

void TransportBar::applyClock(int index) {
    patch_.masterDivision = clockDivisionAtIndex(index);
    syncFromPatch();
    if (onSettingsChanged)
        onSettingsChanged();
}

void TransportBar::showMenu(MenuId menu) {
    closeMenu();
    if (menu == MenuId::None)
        return;

    openMenu_ = menu;
    if (auto* top = getTopLevelComponent()) {
        const float s = designScale();
        SettingRow* row = menu == MenuId::PlayMode ? playModeRow_.get() : clockRow_.get();
        glassMenu_->menuId = menu;
        glassMenu_->setScale(s);
        glassMenu_->resetScroll();
        glassMenu_->items.clear();
        if (menu == MenuId::PlayMode) {
            glassMenu_->items = {"Transport", "Note"};
            glassMenu_->selectedIndex = patch_.playMode == matilda::PlayMode::Transport ? 0 : 1;
        } else {
            for (int i = 0; i < matilda::kClockDivisionCount; ++i)
                glassMenu_->items.add(matilda::kClockDivisions[static_cast<size_t>(i)].label);
            glassMenu_->selectedIndex = clockIndexForDivision(patch_.masterDivision);
            const int maxScroll = juce::jmax(0, glassMenu_->items.size() - kDdMaxVisibleClockItems);
            glassMenu_->scrollOffset = juce::jlimit(0, maxScroll, glassMenu_->selectedIndex - 2);
        }

        const auto rowScreen = row->localAreaToGlobal(row->getLocalBounds());
        const int ddW = juce::roundToInt(kDdW * s);
        const int ddH = GlassMenu::menuHeight(glassMenu_->items.size(), s);
        const int ddX = rowScreen.getCentreX() - ddW / 2;
        const int ddY = rowScreen.getBottom() + juce::roundToInt(6.f * s);
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

void TransportBar::closeMenu() {
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

void TransportBar::handleGlobalMouseDown(const juce::MouseEvent& e) {
    if (openMenu_ == MenuId::None || !glassMenu_->isVisible())
        return;
    const auto screen = e.getScreenPosition();
    if (getScreenBounds().contains(screen) || glassMenu_->getScreenBounds().contains(screen))
        return;
    closeMenu();
}

void TransportBar::paint(juce::Graphics& g) {
    const float s = designScale();

    drawImage(g, filigreeTopImg_, designRect(kFiligreeTopLeft, 0.f, kFiligreeW, kFiligreeH));
    drawImage(g, bgTextureImg_, designRect(kTitleTextureLeft, kTitleTextureY, kTitleTextureW, kTitleTextureH));
    drawImage(g, filigreeBottomImg_, designRect(kFiligreeTopLeft, kFiligreeBotTop, kFiligreeW, kFiligreeH));
    drawNeonTitle(g, "Global Settings", designRect(0.f, kTitleCenterY - kTitleFs * 0.5f, kBaseW, kTitleFs), s);

    const float playModeHeaderY = kColTop + kPlaySize + kColGap;
    const float playModeHeaderH = kLabelFs;
    drawSectionHeader(g, sectionOrnLeftImg_, sectionOrnRightImg_, "Play Mode",
                      designRect(kColLeft, playModeHeaderY, kPlayModeW, playModeHeaderH), s);

    const float clockHeaderY = playModeHeaderY + playModeHeaderH + kRowGap + 44.f;
    drawSectionHeader(g, sectionOrnLeftImg_, sectionOrnRightImg_, "Clock",
                      designRect(kColLeft + (kPlayModeW - kClockW) * 0.5f, clockHeaderY, kClockW, kLabelFs), s);
}

void TransportBar::resized() {
    const float s = designScale();
    const float bleed = kPlayFrameBleed * s;
    playButton_->setBounds(designRect(kColLeft + (kPlayModeW - kPlaySize) * 0.5f, kColTop, kPlaySize, kPlaySize)
                               .expanded(bleed)
                               .toNearestInt());

    const float playModeRowY = kColTop + kPlaySize + kColGap + kLabelFs + kRowGap;
    playModeRow_->setBounds(designRect(kColLeft, playModeRowY, kPlayModeW, kValueFs + kDropdownPadY * 2.f)
                                .toNearestInt());

    const float clockRowY = playModeRowY + kValueFs + kDropdownPadY * 2.f + kRowGap + kLabelFs + kRowGap;
    clockRow_->setBounds(
        designRect(kColLeft + (kPlayModeW - kClockW) * 0.5f, clockRowY, kClockW, kValueFs + kDropdownPadY * 2.f)
            .toNearestInt());
}
