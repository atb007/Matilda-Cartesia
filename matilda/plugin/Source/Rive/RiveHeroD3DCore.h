#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>

struct ID3D11Device;
struct ID3D11Texture2D;
struct IDXGIFactory2;

namespace matilda::rive::d3d {

/** Rive PLS + D3D11 renderer — no JUCE headers. */
class D3DRiveCore {
public:
    D3DRiveCore();
    ~D3DRiveCore();

    bool loadFromMemory(const void* data, size_t numBytes);
    void setPlaying(bool playing);
    void setActiveLayerCount(int count);
    void setPolyphony(bool enabled);
    void setContentAlignSize(uint32_t width, uint32_t height);
    bool resizeRenderTarget(uint32_t width, uint32_t height);

    [[nodiscard]] bool isLoaded() const;
    [[nodiscard]] bool hasRenderedFrame() const;
    [[nodiscard]] ID3D11Device* device() const;
    [[nodiscard]] IDXGIFactory2* dxgiFactory() const;

    bool render(ID3D11Texture2D* backbuffer, uint32_t width, uint32_t height, float deltaSeconds);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace matilda::rive::d3d
