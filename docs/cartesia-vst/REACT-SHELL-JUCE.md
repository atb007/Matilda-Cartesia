# JUCE React shell layout

Pixel constants for the Matilda control shell, ported from `cartesia-vst-ui`:

| Source | Constants |
|--------|-----------|
| `shellLayout.ts` | `SHELL_W/H`, glass bedding, frame overlay |
| `heroLayout.ts` | Plugin frame, shell anchor |
| `MatildaShell.tsx` | Module positions (scale, layers, movement, grid, transport) |
| Component `BASE_*` | ScalePanel, LayerMiniGrid, MovementMenu, Grid4x4, TransportChrome |

**C++ header:** `matilda/plugin/Source/ReactShellLayout.h`  
**UI scale:** `matilda/plugin/Source/UiScale.h` · React mirror `cartesia-vst-ui/src/uiScale.ts`  
**Host backdrop:** `matilda/plugin/Source/HeroBackdropDrawing.h` (VST3 void fill)  
**Shell container:** `Components/MatildaShellPanel` + `ShellChrome`

Default preview scale **0.52 × 0.9** (user factor; was 1.0). React `MatildaPluginFrame` matches JUCE `NativePluginFrame`.

### User UI scale (Jun 2026)

| Constant | Value |
|----------|-------|
| Design 100% | `kPreviewScale` = 0.52 |
| Default open | factor **0.9** |
| Drag range | **0.7 … 1.0** (same absolute minimum as before) |
| Grips | 4 corners + 4 edges → `UiResizeGrip` / `UiResizeGrips` |

### VST3 / Windows host parity

| Concern | JUCE handling |
|---------|----------------|
| Host window wider than UI | `PluginEditor` + `NativePluginFrame` paint hero starfield cover |
| Hero left gutter | Full-bleed bg from `x=0` (`HeroCanvas`) |
| Host resize drift | `syncEditorToViewport()` + `setResizeLimits()` |
| Chevron | `@2x` PNG per state (transparent); circular clip |
| Standalone debug footer | BPM / sync / status — not shown in VST3 |

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
