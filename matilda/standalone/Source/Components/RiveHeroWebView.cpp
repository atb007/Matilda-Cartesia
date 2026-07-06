#include "RiveHeroWebView.h"

#if defined(MATILDA_RIVE_HERO) && JUCE_MAC

namespace {

juce::File bundleResourceFile(const juce::String& relativePath) {
    const auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    return appFile.getChildFile("Contents/Resources/rive").getChildFile(relativePath);
}

class HeroWebBrowser : public juce::WebBrowserComponent {
public:
    explicit HeroWebBrowser(std::function<void()> onLoaded)
        : juce::WebBrowserComponent(
              juce::WebBrowserComponent::Options {}.withKeepPageLoadedWhenBrowserIsHidden()),
          onLoaded_(std::move(onLoaded)) {
        setInterceptsMouseClicks(false, false);
    }

    void pageFinishedLoading(const juce::String&) override {
        if (onLoaded_)
            onLoaded_();
    }

private:
    std::function<void()> onLoaded_;
};

} // namespace

RiveHeroWebView::RiveHeroWebView() {
    setInterceptsMouseClicks(false, false);

    browser_ = std::make_unique<HeroWebBrowser>([this] { onPageFinished(); });
    addAndMakeVisible(*browser_);

    const auto heroHtml = bundleResourceFile("hero.html");
    if (heroHtml.existsAsFile())
        browser_->goToURL(juce::URL(heroHtml).toString(true));
}

void RiveHeroWebView::setPlaying(bool playing) {
    if (playing_ == playing)
        return;
    playing_ = playing;
    syncPlaying();
}

void RiveHeroWebView::resized() {
    if (browser_ != nullptr)
        browser_->setBounds(getLocalBounds());
}

void RiveHeroWebView::onPageFinished() {
    pageLoaded_ = true;
    syncPlaying();
}

void RiveHeroWebView::syncPlaying() {
    if (!pageLoaded_ || browser_ == nullptr)
        return;

    const auto js = juce::String("window.setPlaying && window.setPlaying(")
                        + (playing_ ? "true" : "false") + ");";
    browser_->evaluateJavascript(js);
}

juce::File RiveHeroWebView::heroBundleDirectory() {
    return bundleResourceFile({});
}

#else

RiveHeroWebView::RiveHeroWebView() { setInterceptsMouseClicks(false, false); }
void RiveHeroWebView::setPlaying(bool) {}
void RiveHeroWebView::resized() {}

#endif
