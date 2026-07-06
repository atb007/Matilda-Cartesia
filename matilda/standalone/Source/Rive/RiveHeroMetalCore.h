#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

namespace matilda::rive::metal {

/** Rive PLS + Metal renderer — no JUCE headers (avoids Point/Component SDK clashes). */
class MetalRiveCore {
public:
    MetalRiveCore();
    ~MetalRiveCore();

    bool loadFromMemory(const void* data, size_t numBytes);
    void setPlaying(bool playing);
    void setActiveLayerCount(int count);
    void setContentAlignSize(uint32_t width, uint32_t height);

    [[nodiscard]] bool isLoaded() const;
    [[nodiscard]] bool hasRenderedFrame() const;

    /** `cametalLayer` is a bridged CAMetalLayer*. */
    bool render(void* cametalLayer, float deltaSeconds);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace matilda::rive::metal
