# JUCE React shell layout

Pixel constants for the Matilda control shell, ported from `cartesia-vst-ui`:

| Source | Constants |
|--------|-----------|
| `shellLayout.ts` | `SHELL_W/H`, glass bedding, frame overlay |
| `heroLayout.ts` | Plugin frame, shell anchor |
| `MatildaShell.tsx` | Module positions (scale, layers, movement, grid, transport) |
| Component `BASE_*` | ScalePanel, LayerMiniGrid, MovementMenu, Grid4x4, TransportChrome |

**C++ header:** `matilda/plugin/Source/ReactShellLayout.h`  
**Shell container:** `Components/MatildaShellPanel` + `ShellChrome`

Default preview scale **0.52** matches React `MatildaPluginFrame`.

## Module positions (design px, shell-relative)

| Module | x | y | w × h |
|--------|---|---|-------|
| Scale panel | 118 | 200 | 418 × 598 |
| Layer overview | 628.879 | 218.21 | 609.565 × 301.43 |
| Movement | 675.18 | 667.796 | 514 × 89 |
| 4×4 grid | 629 | 818.945 | ~605 × ~615 (cell metrics in header) |
| Transport | 103.218 | 887.449 | 439 × 485 |

## Next shell milestones

1. Bundle `shell-frame-vines-only.png` + glass raster (replace gradient placeholder)
2. Port ScalePanel / TransportChrome visual styling (glass dropdowns, play gem PNG)
3. `MatildaPluginFrame` — hero canvas + collapse toggle at `heroLayout.ts` coords
4. Layer mini-grid rope-net frame assets + column-major playhead mapping

See also [BLUEARP-ENHANCEMENTS.md](./BLUEARP-ENHANCEMENTS.md).
