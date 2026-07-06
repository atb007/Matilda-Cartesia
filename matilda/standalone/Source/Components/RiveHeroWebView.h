#pragma once

#include <JuceHeader.h>

/** macOS standalone test — WKWebView Rive hero (Artboard + streakVisible bind). */
class RiveHeroWebView : public juce::Component {
public:
    RiveHeroWebView();
    void setPlaying(bool playing);
    void resized() override;

private:
    std::unique_ptr<juce::WebBrowserComponent> browser_;
    bool playing_ = false;
    bool pageLoaded_ = false;

    static juce::File heroBundleDirectory();
    void syncPlaying();
    void onPageFinished();
};
