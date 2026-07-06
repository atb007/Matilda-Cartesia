#pragma once

#include <memory>

namespace juce {
template <typename ValueType>
class Rectangle;
class Component;
class Image;
} // namespace juce

namespace matilda::rive {

class RiveHeroBackend {
public:
    virtual ~RiveHeroBackend() = default;

    virtual bool loadFromMemory(const void* data, size_t numBytes) = 0;
    virtual void setPlaying(bool playing) = 0;
    /** Active z-axis layers from LayerOverview toggles (layer 0 always on). */
    virtual void setActiveLayerCount(int count) = 0;
    virtual void setDisplayRect(juce::Rectangle<int> rect) = 0;
    virtual void setContentAlignRect(juce::Rectangle<int> rect) = 0;
    virtual bool tick(float deltaSeconds) = 0;

    [[nodiscard]] virtual bool isLoaded() const = 0;
    [[nodiscard]] virtual bool hasVisibleOutput() const = 0;
    [[nodiscard]] virtual juce::Component* overlayComponent() { return nullptr; }
    [[nodiscard]] virtual const juce::Image& frameImage() const = 0;
};

std::unique_ptr<RiveHeroBackend> createRiveHeroBackend();

} // namespace matilda::rive
