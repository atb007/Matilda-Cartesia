#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include "RiveHeroBackendMetal.h"
#include "RiveHeroConfig.h"
#include "RiveHeroMetalView.h"

@interface MatildaRiveMetalHostView : NSView
- (CAMetalLayer*)riveMetalLayer;
- (void)updateMetalLayerGeometryWithWidth:(CGFloat)width height:(CGFloat)height scale:(CGFloat)scale;
@end

@implementation MatildaRiveMetalHostView {
    CAMetalLayer* metalLayer_;
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self != nil)
        self.clipsToBounds = NO;
    return self;
}

- (BOOL)isFlipped {
    return YES;
}

- (CAMetalLayer*)riveMetalLayer {
    if (metalLayer_ == nil) {
        self.wantsLayer = YES;
        metalLayer_ = [CAMetalLayer layer];
        metalLayer_.pixelFormat = MTLPixelFormatBGRA8Unorm;
        metalLayer_.framebufferOnly = YES;
        metalLayer_.opaque = NO;
        metalLayer_.backgroundColor = CGColorGetConstantColor(kCGColorClear);
        [self.layer addSublayer:metalLayer_];
    }
    return metalLayer_;
}

- (void)updateMetalLayerGeometryWithWidth:(CGFloat)width height:(CGFloat)height scale:(CGFloat)scale {
    if (width <= 0.f || height <= 0.f || scale <= 0.f)
        return;

    CAMetalLayer* layer = [self riveMetalLayer];
    layer.contentsScale = scale;
    layer.frame = NSRectToCGRect(self.bounds);
    layer.drawableSize = CGSizeMake(width * scale, height * scale);
}

@end

namespace matilda::rive {

RiveHeroMetalView::RiveHeroMetalView(RiveHeroBackendMetal& backend) : backend_(backend) {
    setInterceptsMouseClicks(false, false);
    setOpaque(false);

    auto* host = [[MatildaRiveMetalHostView alloc] initWithFrame:NSMakeRect(0, 0, 1, 1)];
    setView(host);
    [host release];
}

RiveHeroMetalView::~RiveHeroMetalView() { stopTimer(); }

void RiveHeroMetalView::setPlaying(bool playing) {
    if (playing_ == playing)
        return;
    playing_ = playing;
    syncTimer();
}

void RiveHeroMetalView::attachRiveBytes(const void* data, size_t numBytes) {
    if (data == nullptr || numBytes == 0)
        return;
    rivBytes_.setSize(numBytes, true);
    rivBytes_.copyFrom(data, 0, numBytes);
    riveReady_ = true;
    refreshDisplay();
}

void RiveHeroMetalView::refreshDisplay() {
    updateMetalLayerGeometry();
    syncTimer();
}

void RiveHeroMetalView::updateMetalLayerGeometry() {
    if (getWidth() <= 0 || getHeight() <= 0)
        return;

    resizeViewToFit();

    if (void* nativeView = getView()) {
        auto* view = (__bridge NSView*) nativeView;
        if (![view isKindOfClass:[MatildaRiveMetalHostView class]])
            return;

        const float scale = static_cast<float>(getDesktopScaleFactor());
        [(MatildaRiveMetalHostView*) view updateMetalLayerGeometryWithWidth:static_cast<CGFloat>(getWidth())
                                                                     height:static_cast<CGFloat>(getHeight())
                                                                      scale:static_cast<CGFloat>(scale)];
    }
}

void* RiveHeroMetalView::metalLayerHandle() const {
    if (void* nativeView = getView()) {
        auto* view = (__bridge NSView*) nativeView;
        if ([view isKindOfClass:[MatildaRiveMetalHostView class]])
            return (__bridge void*) [(MatildaRiveMetalHostView*) view riveMetalLayer];
    }
    return nullptr;
}

void RiveHeroMetalView::resized() {
    updateMetalLayerGeometry();
    syncTimer();
}

void RiveHeroMetalView::parentHierarchyChanged() {
    NSViewComponent::parentHierarchyChanged();
    refreshDisplay();
}

void RiveHeroMetalView::visibilityChanged() {
    NSViewComponent::visibilityChanged();
    refreshDisplay();
}

void RiveHeroMetalView::syncTimer() {
    stopTimer();
    const bool shouldRun =
        riveReady_ && getParentComponent() != nullptr && (playing_ || kAnimateWhenIdle);
    if (shouldRun) {
        const int fps = playing_ ? kPlayingFps : kIdleFps;
        startTimerHz(fps);
    }
}

void RiveHeroMetalView::timerCallback() {
    if (void* layer = metalLayerHandle()) {
        const int fps = playing_ ? kPlayingFps : kIdleFps;
        if (backend_.renderMetalLayer(layer, 1.f / static_cast<float>(fps))) {
            const bool firstFrame = !hasRenderedFrame_;
            hasRenderedFrame_ = true;
            if (firstFrame)
                if (auto* parent = getParentComponent())
                    parent->repaint();
            return;
        }
    }
}

} // namespace matilda::rive
