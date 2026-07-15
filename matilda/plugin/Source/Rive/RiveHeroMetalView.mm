#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>

#include "RiveHeroBackendMetal.h"
#include "RiveHeroConfig.h"
#include "RiveHeroMetalView.h"

@interface MatildaRiveMetalHostView : NSView
- (CAMetalLayer*)riveMetalLayer;
- (void)updateMetalLayerGeometryWithWidth:(CGFloat)width height:(CGFloat)height scale:(CGFloat)scale;
- (void)setWordmarkCGImage:(CGImageRef)image frame:(CGRect)frame;
- (void)clearWordmark;
@end

@implementation MatildaRiveMetalHostView {
    CAMetalLayer* metalLayer_;
    CALayer* wordmarkLayer_;
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self != nil)
        self.clipsToBounds = NO;
    return self;
}

- (void)dealloc {
    [metalLayer_ release];
    metalLayer_ = nil;
    [wordmarkLayer_ release];
    wordmarkLayer_ = nil;
    [super dealloc];
}

- (BOOL)isFlipped {
    return YES;
}

- (void)ensureHostLayer {
    if (!self.wantsLayer)
        self.wantsLayer = YES;
}

- (CAMetalLayer*)riveMetalLayer {
    [self ensureHostLayer];
    if (metalLayer_ == nil) {
        metalLayer_ = [[CAMetalLayer layer] retain];
        metalLayer_.pixelFormat = MTLPixelFormatBGRA8Unorm;
        metalLayer_.framebufferOnly = YES;
        metalLayer_.opaque = NO;
        metalLayer_.backgroundColor = CGColorGetConstantColor(kCGColorClear);
        [self.layer insertSublayer:metalLayer_ atIndex:0];
    }
    return metalLayer_;
}

- (void)ensureWordmarkAboveMetal {
    if (metalLayer_ == nil || wordmarkLayer_ == nil || self.layer == nil)
        return;
    [self.layer insertSublayer:wordmarkLayer_ above:metalLayer_];
}

- (void)updateMetalLayerGeometryWithWidth:(CGFloat)width height:(CGFloat)height scale:(CGFloat)scale {
    if (width <= 0.f || height <= 0.f || scale <= 0.f)
        return;

    CAMetalLayer* layer = [self riveMetalLayer];
    layer.contentsScale = scale;
    layer.frame = NSRectToCGRect(self.bounds);
    layer.drawableSize = CGSizeMake(width * scale, height * scale);
    [self ensureWordmarkAboveMetal];
}

- (void)setWordmarkCGImage:(CGImageRef)image frame:(CGRect)frame {
    [self ensureHostLayer];
    (void) [self riveMetalLayer];

    if (wordmarkLayer_ == nil) {
        wordmarkLayer_ = [[CALayer layer] retain];
        wordmarkLayer_.opaque = NO;
        wordmarkLayer_.contentsGravity = kCAGravityResize;
        wordmarkLayer_.anchorPoint = CGPointMake(0.f, 0.f);
    }

    wordmarkLayer_.frame = frame;
    wordmarkLayer_.contents = (id) image;
    if (image != nullptr && frame.size.width > 0.f)
        wordmarkLayer_.contentsScale = (CGFloat) CGImageGetWidth(image) / frame.size.width;
    else
        wordmarkLayer_.contentsScale = 1.f;
    [self ensureWordmarkAboveMetal];
}

- (void)clearWordmark {
    if (wordmarkLayer_ == nil)
        return;
    wordmarkLayer_.contents = nil;
    wordmarkLayer_.frame = CGRectZero;
}

@end

namespace matilda::rive {

namespace {

CGImageRef createCGImageFromJuceImage(const juce::Image& image) {
    if (!image.isValid())
        return nullptr;

    const juce::Image argb = image.convertedToFormat(juce::Image::ARGB);
    juce::Image::BitmapData bd(argb, juce::Image::BitmapData::readOnly);

    CGColorSpaceRef colourSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    if (colourSpace == nullptr)
        colourSpace = CGColorSpaceCreateDeviceRGB();

    CGContextRef ctx = CGBitmapContextCreate(bd.data,
                                             static_cast<size_t>(argb.getWidth()),
                                             static_cast<size_t>(argb.getHeight()),
                                             8,
                                             static_cast<size_t>(bd.lineStride),
                                             colourSpace,
                                             kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Host);
    CGColorSpaceRelease(colourSpace);
    if (ctx == nullptr)
        return nullptr;

    CGImageRef cgImage = CGBitmapContextCreateImage(ctx);
    CGContextRelease(ctx);
    return cgImage;
}

} // namespace

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

void RiveHeroMetalView::setWordmarkOverlay(const juce::Image& image, juce::Rectangle<int> boundsInOverlay) {
    if (void* nativeView = getView()) {
        auto* view = (__bridge NSView*) nativeView;
        if (![view isKindOfClass:[MatildaRiveMetalHostView class]])
            return;

        CGImageRef cgImage = createCGImageFromJuceImage(image);
        const CGRect frame = CGRectMake(boundsInOverlay.getX(), boundsInOverlay.getY(),
                                        boundsInOverlay.getWidth(), boundsInOverlay.getHeight());
        [(MatildaRiveMetalHostView*) view setWordmarkCGImage:cgImage frame:frame];
        if (cgImage != nullptr)
            CGImageRelease(cgImage);
    }
}

void RiveHeroMetalView::clearWordmarkOverlay() {
    if (void* nativeView = getView()) {
        auto* view = (__bridge NSView*) nativeView;
        if ([view isKindOfClass:[MatildaRiveMetalHostView class]])
            [(MatildaRiveMetalHostView*) view clearWordmark];
    }
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
