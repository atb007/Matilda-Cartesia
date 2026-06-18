#include "GemSparksOverlay.h"
#include "../ScaleGemPalette.h"

namespace {

constexpr int kMaxAlive = 7;
constexpr int kMinGapMs = 700;
constexpr int kMaxGapMs = 1800;

float rand01() { return juce::Random::getSystemRandom().nextFloat(); }

} // namespace

GemSparksOverlay::GemSparksOverlay() {
    setOpaque(false);
    setInterceptsMouseClicks(false, false);
    setPaintingIsUnclipped(true);
    colors_ = matilda::scale::gemPaletteForModeId(modeId_);
    resetSpawns();
    startTimerHz(60);
}

void GemSparksOverlay::setPanelScale(float scale) {
    panelScale_ = scale;
}

void GemSparksOverlay::setScaleModeId(const juce::String& modeId) {
    if (modeId_ == modeId)
        return;
    modeId_ = modeId;
    colors_ = matilda::scale::gemPaletteForModeId(modeId_);
    resetSpawns();
}

void GemSparksOverlay::setGemImage(const juce::Image& gemImage) {
    gemImage_ = gemImage;
    repaint();
}

void GemSparksOverlay::resetSpawns() {
    streaks_.clear();
    nextId_ = 0;
    nextSpawnMs_ = juce::Time::getMillisecondCounterHiRes();
    spawnStreak("left");
    spawnStreak("right");
    scheduleSpawn();
}

GemSparksOverlay::Vec2 GemSparksOverlay::quadPoint(const Vec2& p0, const Vec2& p1, const Vec2& p2, float t) {
    const float u = 1.f - t;
    return {u * u * p0.x + 2.f * u * t * p1.x + t * t * p2.x,
            u * u * p0.y + 2.f * u * t * p1.y + t * t * p2.y};
}

GemSparksOverlay::Vec2 GemSparksOverlay::quadTangent(const Vec2& p0, const Vec2& p1, const Vec2& p2, float t) {
    const float u = 1.f - t;
    return {2.f * u * (p1.x - p0.x) + 2.f * t * (p2.x - p1.x), 2.f * u * (p1.y - p0.y) + 2.f * t * (p2.y - p1.y)};
}

void GemSparksOverlay::spawnStreak(const char* forceSide) {
    const auto bounds = getLocalBounds().toFloat();
    const float w = bounds.getWidth();
    const float h = bounds.getHeight();
    if (w <= 1.f || h <= 1.f)
        return;

    const bool left = forceSide != nullptr ? forceSide[0] == 'l' : rand01() < 0.5f;
    const float cx = w * 0.5f;
    const float cy = h * 0.5f;
    const float spreadY = h * 0.38f;
    const float y0 = cy + (rand01() - 0.5f) * spreadY;
    const float y2 = cy + (rand01() - 0.5f) * spreadY * 0.9f;
    const float arch = (rand01() - 0.5f) * h * 0.35f;
    const float pull = left ? 0.28f + rand01() * 0.22f : 0.72f - rand01() * 0.22f;

    Streak s;
    s.id = ++nextId_;
    s.p0 = left ? Vec2{-w * 0.08f, y0} : Vec2{w * 1.08f, y0};
    s.p2 = left ? Vec2{w * (0.92f + rand01() * 0.2f), y2} : Vec2{w * (0.08f - rand01() * 0.2f), y2};
    s.p1 = {cx + (pull - 0.5f) * w * 0.35f, cy + arch};
    s.duration = 0.85f + rand01() * 0.7f;
    s.length = (28.f + rand01() * 36.f) * panelScale_;
    s.thickness = (1.4f + rand01() * 1.6f) * panelScale_;
    s.tension = rand01();
    s.startMs = juce::Time::getMillisecondCounterHiRes();

    if (static_cast<int>(streaks_.size()) >= kMaxAlive)
        streaks_.erase(streaks_.begin());
    streaks_.push_back(s);
}

void GemSparksOverlay::scheduleSpawn() {
    nextSpawnMs_ = juce::Time::getMillisecondCounterHiRes() + kMinGapMs + rand01() * (kMaxGapMs - kMinGapMs);
}

void GemSparksOverlay::timerCallback() {
    const double now = juce::Time::getMillisecondCounterHiRes();
    streaks_.erase(std::remove_if(streaks_.begin(), streaks_.end(),
                                  [&](const Streak& s) {
                                      return (now - s.startMs) > (s.duration * 1000.0 + 120.0);
                                  }),
                   streaks_.end());

    if (now >= nextSpawnMs_) {
        const float roll = rand01();
        if (roll < 0.5f) {
            spawnStreak("left");
            spawnStreak("right");
        } else if (roll < 0.72f) {
            spawnStreak();
            spawnStreak();
        } else {
            spawnStreak();
        }
        scheduleSpawn();
    }

    repaint();
}

void GemSparksOverlay::drawOneStreak(juce::Graphics& g, const Streak& streak, bool behind,
                                     double nowMs) const {
    const float t = juce::jlimit(0.f, 1.f, static_cast<float>((nowMs - streak.startMs) / (streak.duration * 1000.0)));
    const float eased = t < 0.5f ? 2.f * t * t : 1.f - juce::square(-2.f * t + 2.f) * 0.5f;
    const auto pos = quadPoint(streak.p0, streak.p1, streak.p2, eased);
    const auto tan = quadTangent(streak.p0, streak.p1, streak.p2, eased);
    const float angle = std::atan2(tan.y, tan.x);

    const bool isBehind = eased < 0.42f || pos.y < streak.p1.y - streak.thickness * 2.f;
    const bool onLayer = behind ? isBehind : !isBehind;
    const float fadeIn = juce::jmin(1.f, eased * 5.f);
    const float fadeOut = eased > 0.78f ? juce::jmax(0.f, 1.f - (eased - 0.78f) / 0.22f) : 1.f;
    const float life = fadeIn * fadeOut * std::sin(juce::jmin(eased * 1.4f, 1.f) * juce::MathConstants<float>::pi * 0.95f);

    if (!onLayer || life <= 0.f)
        return;

    const float opacity = behind ? life * 0.5f : life * 0.88f;
    const float stretch = 0.7f + life * 0.5f + streak.tension * 0.15f;

    g.saveState();
    g.addTransform(juce::AffineTransform::rotation(angle, pos.x, pos.y)
                      .scaled(stretch, 1.f, pos.x, pos.y)
                      .translated(pos.x, pos.y));

    const float hw = streak.length * 0.5f;
    const float hh = streak.thickness * 0.5f;
    const auto body = juce::Rectangle<float>(-hw, -hh, streak.length, streak.thickness);

    juce::ColourGradient outer(colors_.glow.withAlpha(0.33f * opacity), body.getX(), body.getCentreY(),
                               colors_.core.withAlpha(0.6f * opacity), body.getCentreX(), body.getCentreY(), false);
    outer.addColour(0.5, colors_.core.withAlpha(0.6f * opacity));
    outer.addColour(0.82, colors_.glow.withAlpha(0.33f * opacity));
    g.setGradientFill(outer);
    g.fillRoundedRectangle(body.expanded(streak.thickness * 0.8f, 4.f), streak.thickness * 2.f);

    juce::ColourGradient core(juce::Colours::transparentBlack, body.getX(), body.getCentreY(),
                              colors_.hot.withAlpha(0.93f * opacity), body.getCentreX(), body.getCentreY(), false);
    core.addColour(0.5, juce::Colours::white.withAlpha(0.95f * opacity));
    core.addColour(1.f, juce::Colours::transparentBlack);
    g.setGradientFill(core);
    g.fillRect(body.reduced(streak.thickness * 0.3f, streak.thickness * 0.28f));

    g.restoreState();
}

void GemSparksOverlay::drawStreakLayer(juce::Graphics& g, bool behind) const {
    const double now = juce::Time::getMillisecondCounterHiRes();
    for (const auto& streak : streaks_)
        drawOneStreak(g, streak, behind, now);
}

void GemSparksOverlay::paint(juce::Graphics& g) {
    paintGem(g, getLocalBounds().toFloat());
}

void GemSparksOverlay::paintGem(juce::Graphics& g, juce::Rectangle<float> gemBounds) const {
    drawStreakLayer(g, true);

    if (gemImage_.isValid())
        g.drawImage(gemImage_, gemBounds, juce::RectanglePlacement::centred);

    drawStreakLayer(g, false);
}
