#pragma once

#include <JuceHeader.h>
#include "../ScaleGemPalette.h"

/** Procedural gem orbital sparks + animated scale gem orb — port of React `ScaleGemOrb.tsx`. */
class GemSparksOverlay : public juce::Component, private juce::Timer {
public:
    GemSparksOverlay();

    void setPanelScale(float scale);
    void setScaleModeId(const juce::String& modeId);
    void setGemImage(const juce::Image& gemImage);
    void resetSpawns();

    void paint(juce::Graphics& g) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

private:
    enum class TransitionPhase { idle, out, in };

    struct Vec2 {
        float x = 0.f;
        float y = 0.f;
    };

    struct Streak {
        int id = 0;
        Vec2 p0, p1, p2;
        float duration = 1.f;
        float length = 30.f;
        float thickness = 2.f;
        float tension = 0.f;
        double startMs = 0.0;
    };

    float panelScale_ = 1.f;
    juce::String modeId_{"chromatic"};
    juce::String targetModeId_{"chromatic"};
    matilda::scale::GemPalette colors_;
    juce::Image gemImage_;
    juce::Image pendingGemImage_;
    std::vector<Streak> streaks_;
    int nextId_ = 0;
    double nextSpawnMs_ = 0.0;

    TransitionPhase transitionPhase_ = TransitionPhase::idle;
    double transitionStartMs_ = 0.0;
    float gemVisualScale_ = 1.f;
    float gemVisualAlpha_ = 1.f;
    float floatY_ = 0.f;
    bool hovered_ = false;
    double floatStartMs_ = 0.0;

    void timerCallback() override;
    void beginScaleTransition(const juce::String& modeId, const juce::Image& gemImage);
    void completeTransitionSwap();
    void updateTransitionVisuals(double nowMs);
    void spawnStreak(const char* forceSide = nullptr);
    void scheduleSpawn();
    static Vec2 quadPoint(const Vec2& p0, const Vec2& p1, const Vec2& p2, float t);
    static Vec2 quadTangent(const Vec2& p0, const Vec2& p1, const Vec2& p2, float t);
    void drawStreakLayer(juce::Graphics& g, bool behind) const;
    void drawOneStreak(juce::Graphics& g, const Streak& streak, bool behind, double nowMs) const;
    void paintGem(juce::Graphics& g, juce::Rectangle<float> gemBounds) const;
};
