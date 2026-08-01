#pragma once

// Legacy HWND swap-chain host — no longer used.
// Windows Rive now renders offscreen (D3D11 PLS → CPU → juce::Image) in
// RiveHeroBackendD3D / RiveHeroD3DCore, which is reliable in standalone and VST hosts.

namespace matilda::rive {}
