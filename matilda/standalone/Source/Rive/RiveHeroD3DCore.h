#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace matilda::rive::d3d {

/** Rive PLS + D3D11 renderer — offscreen texture + CPU readback (no HWND / swap chain). */
class D3DRiveCore {
public:
    D3DRiveCore();
    ~D3DRiveCore();

    bool loadFromMemory(const void* data, size_t numBytes);
    void setPlaying(bool playing);
    void setActiveLayerCount(int count);
    void setPolyphony(bool enabled);
    void setContentAlignSize(uint32_t width, uint32_t height);

    /** Advance + render into an offscreen RT, then copy RGBA8 pixels (tight row pitch). */
    bool renderToPixels(uint32_t width, uint32_t height, float deltaSeconds, std::vector<uint8_t>& rgbaOut);

    [[nodiscard]] bool isLoaded() const;
    [[nodiscard]] bool hasRenderedFrame() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace matilda::rive::d3d
