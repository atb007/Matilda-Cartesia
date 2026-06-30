#include "MovementSelector.h"
#include "BinaryData.h"
#include "../FiligreeDrawing.h"
#include "../GlassDropdownDrawing.h"
#include "../MatildaFonts.h"
#include "../MatildaImages.h"
#include "../MovementLayout.h"
#include "../ReactShellLayout.h"

namespace {

using namespace matilda::movement;
using namespace matilda::ui::filigree;

constexpr float kDdW = 316.f;
constexpr float kDdPadY = 22.f;
constexpr float kDdItemFs = matilda::ui::glass::kDdItemFs;
constexpr float kDdItemGap = 11.f;
constexpr float kDdLineGap = 15.f;
constexpr float kDdClose = 18.f;

void drawMovementArrow(juce::Graphics& g, juce::Rectangle<float> bounds, bool left, bool hover) {
    const auto top = hover ? juce::Colour(0xfff8f8f8) : juce::Colour(0xffececec);
    const auto bottom = hover ? juce::Colour(0xffb0b0b0) : juce::Colour(0xff8d8d8d);
    juce::ColourGradient grad(top, bounds.getTopLeft(), bottom, bounds.getBottomLeft(), false);
    g.setGradientFill(grad);

    juce::Path tri;
    if (left)
        tri.addTriangle(bounds.getRight(), bounds.getY(), bounds.getRight(), bounds.getBottom(), bounds.getX(),
                        bounds.getCentreY());
    else
        tri.addTriangle(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom(), bounds.getRight(),
                        bounds.getCentreY());
    g.fillPath(tri);
}

void drawNeonLabel(juce::Graphics& g, const juce::String& text, juce::Rectangle<float> area, float scale) {
    const float fs = kFontSize * scale;
    auto font = matilda::fonts::asimovian(fs);
    font.setExtraKerningFactor(kTracking / fs);
    g.setFont(font);

    g.setColour(juce::Colour(0xff77a6ab).withAlpha(0.85f));
    g.drawText(text, area.translated(0.f, kNeonShadowY * scale).toNearestInt(), juce::Justification::centred, false);
    g.setColour(juce::Colour(0xff10ffcf).withAlpha(0.35f));
    g.drawText(text, area.toNearestInt(), juce::Justification::centred, false);
    g.setColour(juce::Colours::white);
    g.drawText(text, area.toNearestInt(), juce::Justification::centred, false);
}

juce::Rectangle<float> designRectAt(float x, float y, float w, float h, float scale, juce::Point<float> origin) {
    return {origin.x + x * scale, origin.y + y * scale, w * scale, h * scale};
}

} // namespace

class MovementSelector::ArrowButton : public juce::Component {
public:
    ArrowButton(bool left, std::function<void()> onClick) : left_(left), onClick_(std::move(onClick)) {
        setInterceptsMouseClicks(true, false);
    }

    void paint(juce::Graphics& g) override {
        drawMovementArrow(g, getLocalBounds().toFloat(), left_, hover_);
    }

    void mouseEnter(const juce::MouseEvent&) override {
        hover_ = true;
        repaint();
    }
    void mouseExit(const juce::MouseEvent&) override {
        hover_ = false;
        repaint();
    }
    void mouseDown(const juce::MouseEvent&) override {
        if (onClick_)
            onClick_();
    }

private:
    bool left_;
    bool hover_ = false;
    std::function<void()> onClick_;
};

class MovementSelector::ModeButton : public juce::Component {
public:
    std::function<void()> onClick;
    juce::String text;
    float scale_ = 1.f;

    void setScale(float scale) {
        scale_ = scale;
        repaint();
    }

    void paint(juce::Graphics& g) override {
        drawNeonLabel(g, text, getLocalBounds().toFloat(), scale_);
    }

    void mouseDown(const juce::MouseEvent&) override {
        if (onClick)
            onClick();
    }
};

class MovementSelector::GlassDropdown : public juce::Component {
public:
    std::function<void(int index)> onSelect;
    std::function<void()> onClose;
    int selectedIndex = 0;
    juce::Image backdrop_;

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

    void paint(juce::Graphics& g) override {
        const auto bounds = getLocalBounds().toFloat();
        matilda::ui::glass::drawPanel(g, bounds, scale_, backdrop_);

        const float closeSize = kDdClose * scale_;
        const auto closeBounds = juce::Rectangle<float>(bounds.getRight() - 17.f * scale_ - closeSize,
                                                       bounds.getY() + 20.f * scale_, closeSize, closeSize);
        matilda::ui::glass::drawCloseIcon(
            g, closeBounds, juce::Colours::white.withAlpha(closeHover_ ? 1.f : 0.85f));

        const float itemW = bounds.getWidth() * 0.83f;
        const float itemX = bounds.getX() + (bounds.getWidth() - itemW) * 0.5f;
        float y = bounds.getY() + kDdPadY * scale_;

        const auto& items = MovementSelector::modeMenuLabels();
        g.setFont(matilda::fonts::kodeMonoBold(kDdItemFs * scale_));

        for (int i = 0; i < items.size(); ++i) {
            const float textH = kDdItemFs * scale_ * 1.25f;
            const auto textBounds = juce::Rectangle<float>(itemX, y, itemW, textH);
            const bool selected = i == selectedIndex;

            g.setColour(selected ? juce::Colours::white : juce::Colours::white.withAlpha(0.65f));
            g.drawText(items[i], textBounds.toNearestInt(), juce::Justification::centred, false);
            if (selected) {
                g.setColour(juce::Colour(0x7310ffcf));
                g.drawText(items[i], textBounds.translated(0.f, 1.f * scale_).toNearestInt(),
                           juce::Justification::centred, false);
            }

            y += textH + kDdLineGap * scale_;
            matilda::ui::glass::drawHairline(g, juce::Rectangle<float>(itemX, y, itemW, 1.f));
            y += 1.f + kDdItemGap * scale_;
        }
    }

    void mouseDown(const juce::MouseEvent& e) override {
        const auto pos = e.getPosition();
        if (closeBounds_.contains(pos)) {
            if (onClose)
                onClose();
            return;
        }
        for (int i = 0; i < itemBounds_.size(); ++i) {
            if (itemBounds_[i].contains(pos)) {
                if (onSelect)
                    onSelect(i);
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
    }

    static int dropdownHeight(float scale) {
        const float itemBlock = kDdItemFs * scale * 1.25f + kDdLineGap * scale + 1.f;
        return juce::roundToInt(kDdPadY * scale * 2.f + itemBlock * 6.f + kDdItemGap * scale * 5.f);
    }

private:
    float scale_ = 1.f;
    juce::Rectangle<int> closeBounds_;
    juce::Array<juce::Rectangle<int>> itemBounds_;
    bool closeHover_ = false;

    void rebuildHitAreas() {
        const auto bounds = getLocalBounds();
        const float closeSize = kDdClose * scale_;
        closeBounds_ = juce::Rectangle<int>(
            juce::roundToInt(bounds.getRight() - 17.f * scale_ - closeSize),
            juce::roundToInt(bounds.getY() + 20.f * scale_),
            juce::roundToInt(closeSize), juce::roundToInt(closeSize));

        itemBounds_.clear();
        const float itemW = bounds.getWidth() * 0.83f;
        const float itemX = bounds.getX() + (bounds.getWidth() - itemW) * 0.5f;
        float y = bounds.getY() + kDdPadY * scale_;
        for (int i = 0; i < 6; ++i) {
            const float textH = kDdItemFs * scale_ * 1.25f;
            itemBounds_.add(juce::Rectangle<int>(juce::roundToInt(itemX), juce::roundToInt(y),
                                                juce::roundToInt(itemW), juce::roundToInt(textH)));
            y += textH + kDdLineGap * scale_ + 1.f + kDdItemGap * scale_;
        }
    }

    void resized() override { rebuildHitAreas(); }
};

class MovementSelector::GlobalClickListener : public juce::MouseListener {
public:
    explicit GlobalClickListener(MovementSelector& owner) : owner_(owner) {}

    void mouseDown(const juce::MouseEvent& e) override { owner_.handleGlobalMouseDown(e); }

private:
    MovementSelector& owner_;
};

class MovementSelector::DismissLayer : public juce::Component {
public:
    std::function<void()> onDismiss;

    void mouseDown(const juce::MouseEvent&) override {
        if (onDismiss)
            onDismiss();
    }
};

const juce::StringArray& MovementSelector::modeLabels() {
    static const juce::StringArray k{"Forward", "Reverse", "Ping-pong", "Pendulum", "Random", "Random skip"};
    return k;
}

const juce::StringArray& MovementSelector::modeMenuLabels() {
    static const juce::StringArray k{"FORWARD", "REVERSE", "PING-PONG", "PENDULUM", "RANDOM", "RANDOM SKIP"};
    return k;
}

juce::String MovementSelector::barLabelForMode(matilda::MovementMode mode) {
    return modeLabels()[static_cast<int>(mode)];
}

MovementSelector::MovementSelector(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    setOpaque(false);
    setPaintingIsUnclipped(true);

    bgTextureImg_ = matilda::images::movementBgTexture();

    const int filigreeW2x = juce::roundToInt(kFiligreeW * 2.f);
    const int filigreeH2x = juce::roundToInt(kFiligreeH * 2.f);

    Layout filigreeLayout;
    filigreeLayout.filigreeW = kFiligreeW;
    filigreeLayout.filigreeLeft = (kBaseW - kFiligreeW) * 0.5f;
    filigreeLayout.textureW = kTextureW;
    filigreeLayout.textureLeft = kTextureLeft;
    filigreeLayout.alphaScale = kFiligreeAlphaScale;
    filigreeLayout.grey = kFiligreeGrey;

    filigreeTopImg_ = rasterizeSvg(BinaryData::movementfiligreetop_svg, BinaryData::movementfiligreetop_svgSize,
                                   filigreeW2x, filigreeH2x, bgTextureImg_, filigreeLayout);
    filigreeBottomImg_ = flipImageVertically(filigreeTopImg_);

    prev_ = std::make_unique<ArrowButton>(true, [this] { cycleMode(-1); });
    next_ = std::make_unique<ArrowButton>(false, [this] { cycleMode(1); });
    modeButton_ = std::make_unique<ModeButton>();
    modeButton_->onClick = [this] { showMenu(!menuOpen_); };

    dropdown_ = std::make_unique<GlassDropdown>();
    dropdown_->setVisible(false);
    dropdown_->onSelect = [this](int index) {
        applyIndex(index);
        showMenu(false);
    };
    dropdown_->onClose = [this] { showMenu(false); };

    dismissLayer_ = std::make_unique<DismissLayer>();
    dismissLayer_->setVisible(false);
    dismissLayer_->setInterceptsMouseClicks(true, true);
    dismissLayer_->onDismiss = [this] { showMenu(false); };

    globalClickListener_ = std::make_unique<GlobalClickListener>(*this);

    addAndMakeVisible(*prev_);
    addAndMakeVisible(*next_);
    addAndMakeVisible(*modeButton_);

    syncFromPatch();
}

MovementSelector::~MovementSelector() {
    juce::Desktop::getInstance().removeGlobalMouseListener(globalClickListener_.get());
}

float MovementSelector::designScale() const {
    return juce::jmin(static_cast<float>(getWidth()) / kBaseW, static_cast<float>(getHeight()) / kBaseH);
}

juce::Point<float> MovementSelector::designOrigin() const {
    const float s = designScale();
    return {(static_cast<float>(getWidth()) - kBaseW * s) * 0.5f,
            (static_cast<float>(getHeight()) - kBaseH * s) * 0.5f};
}

void MovementSelector::syncFromPatch() {
    const auto& layer = patch_.layers[static_cast<size_t>(patch_.selectedLayer)];
    modeButton_->text = barLabelForMode(layer.movement);
    modeButton_->setScale(designScale());
    modeButton_->repaint();
    if (dropdown_)
        dropdown_->selectedIndex = static_cast<int>(layer.movement);
    resized();
}

void MovementSelector::cycleMode(int dir) {
    auto& layer = patch_.layers[static_cast<size_t>(patch_.selectedLayer)];
    const int count = modeLabels().size();
    layer.movement =
        static_cast<matilda::MovementMode>((static_cast<int>(layer.movement) + dir + count) % count);
    syncFromPatch();
    if (onChanged)
        onChanged();
}

void MovementSelector::applyIndex(int index) {
    auto& layer = patch_.layers[static_cast<size_t>(patch_.selectedLayer)];
    layer.movement = static_cast<matilda::MovementMode>(index);
    syncFromPatch();
    if (onChanged)
        onChanged();
}

void MovementSelector::showMenu(bool show) {
    menuOpen_ = show;

    if (!show) {
        juce::Desktop::getInstance().removeGlobalMouseListener(globalClickListener_.get());
        if (dropdown_->getParentComponent())
            dropdown_->getParentComponent()->removeChildComponent(dropdown_.get());
        if (dismissLayer_->getParentComponent())
            dismissLayer_->getParentComponent()->removeChildComponent(dismissLayer_.get());
        dropdown_->setVisible(false);
        dismissLayer_->setVisible(false);
        dropdown_->clearBackdrop();
        return;
    }

    if (auto* top = getTopLevelComponent()) {
        const float s = designScale();
        dropdown_->setScale(s);
        dropdown_->selectedIndex = static_cast<int>(
            patch_.layers[static_cast<size_t>(patch_.selectedLayer)].movement);

        const auto modeScreen = localAreaToGlobal(getLocalBounds());
        const int ddW = juce::roundToInt(kDdW * s);
        const int ddH = GlassDropdown::dropdownHeight(s);
        const int ddX = modeScreen.getCentreX() - ddW / 2;
        const int ddY = modeScreen.getBottom() + juce::roundToInt(4.f * s);
        const auto ddTopLeft = top->getLocalPoint(nullptr, juce::Point<int>(ddX, ddY));
        const auto snapArea = juce::Rectangle<int>(ddTopLeft.x, ddTopLeft.y, ddW, ddH);
        dropdown_->setBackdrop(matilda::ui::glass::captureBackdrop(*top, snapArea));

        top->addAndMakeVisible(*dismissLayer_);
        dismissLayer_->onDismiss = [this] { showMenu(false); };
        dismissLayer_->setBounds(top->getLocalBounds());
        dismissLayer_->setAlwaysOnTop(true);

        top->addAndMakeVisible(*dropdown_);
        dropdown_->setBounds(snapArea);
        dropdown_->setAlwaysOnTop(true);
        dropdown_->toFront(true);
        dismissLayer_->toBehind(dropdown_.get());
        dropdown_->setVisible(true);
        dismissLayer_->setVisible(true);

        juce::Desktop::getInstance().addGlobalMouseListener(globalClickListener_.get());
    }
}

void MovementSelector::handleGlobalMouseDown(const juce::MouseEvent& e) {
    if (!menuOpen_ || !dropdown_->isVisible())
        return;

    const auto screen = e.getScreenPosition();
    if (getScreenBounds().contains(screen) || dropdown_->getScreenBounds().contains(screen))
        return;

    showMenu(false);
}

void MovementSelector::paint(juce::Graphics& g) {
    const float s = designScale();
    const auto origin = designOrigin();

    const auto filigreeDest =
        designRectAt((kBaseW - kFiligreeW) * 0.5f, 0.f, kFiligreeW, kFiligreeH, s, origin);
    drawImage(g, filigreeTopImg_, filigreeDest);

    const auto textureDest = designRectAt(kTextureLeft, kTextureY, kTextureW, kTextureH, s, origin);
    drawImage(g, bgTextureImg_, textureDest);

    const auto filigreeBottomDest =
        designRectAt((kBaseW - kFiligreeW) * 0.5f, kBaseH - kFiligreeH, kFiligreeW, kFiligreeH, s, origin);
    drawImage(g, filigreeBottomImg_, filigreeBottomDest);
}

void MovementSelector::resized() {
    const float s = designScale();
    const auto origin = designOrigin();

    prev_->setBounds(designRectAt(kLeftArrowX, kArrowRowY, kArrowW, kArrowH, s, origin).toNearestInt());
    next_->setBounds(designRectAt(kRightArrowX, kArrowRowY, kArrowW, kArrowH, s, origin).toNearestInt());

    const auto font = matilda::fonts::asimovian(kFontSize * s);
    const float textW = font.getStringWidthFloat(modeButton_->text);
    const int btnW = juce::roundToInt(textW + kLabelPadX * 2.f * s);
    const int btnH = juce::roundToInt(kFontSize * s + kLabelPadY * 2.f * s);
    const int btnX = juce::roundToInt(origin.x + (kBaseW * s - btnW) * 0.5f);
    const int btnY = juce::roundToInt(origin.y + kLabelCenterY * s - btnH * 0.5f);
    modeButton_->setBounds(btnX, btnY, btnW, btnH);
    modeButton_->setScale(s);
}
