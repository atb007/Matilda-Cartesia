# Rive Hero Animation Rulebook

Matilda standalone hero portrait — data-binding contract, asset versions, and change log.

**Source of truth (C++):** `matilda/standalone/Source/Rive/RiveHeroConfig.h`  
**Web mirror:** `cartesia-vst-ui/src/riveConfig.ts`  
**Binding logic:** `matilda/standalone/Source/Rive/RiveHeroBindings.h`

---

## Asset

| Field | Value |
|-------|-------|
| File | `cartesia-vst-ui/public/assets/matilda-cartesia-v3.riv` |
| Artboard | `Artboard` |
| View model | `HairStreaksTrimControl` |
| Layout | Same dimensions/artboard as v1/v2 — no panel coordinate changes |

Embedded in the standalone (and plugin) app via `CMakeLists.txt` → JUCE `BinaryData` (`matildacartesiav3_riv`).

---

## How to update the `.riv` asset manually

1. **Drop the file** into `cartesia-vst-ui/public/assets/` as `matilda-cartesia-vN.riv` (keep old versions if you want rollback).
2. **CMake embed** — change the path in both:
   - `matilda/standalone/CMakeLists.txt`
   - `matilda/plugin/CMakeLists.txt`  
   Look for `matilda-cartesia-v*.riv` under `juce_add_binary_data` / asset list.
3. **BinaryData symbol** — JUCE strips non-alphanumerics from the filename. Example: `matilda-cartesia-v3.riv` → `BinaryData::matildacartesiav3_riv` + `…_rivSize`. Update load sites:
   - `matilda/standalone/Source/Components/HeroCanvas.cpp`
   - `matilda/plugin/Source/Components/HeroCanvas.cpp`
4. **Version string / React** — keep in sync:
   - `matilda/standalone/Source/Rive/RiveHeroConfig.h` → `kRiveAssetVersion`
   - `cartesia-vst-ui/src/riveConfig.ts` → `RIVE_SRC`
5. **Optional legacy WebView** — `matilda/standalone/Resources/rive/hero.html` `fetch("…")` if that path is still used.
6. **Docs** — this rulebook (Asset table + changelog).
7. **Rebuild** — reconfigure CMake so BinaryData regenerates, then build:
   ```bash
   cmake -B matilda/standalone/build -DCMAKE_BUILD_TYPE=Release
   cmake --build matilda/standalone/build --config Release -j8
   ```
8. **If bindings change** (new/renamed view-model booleans): update `RiveHeroConfig.h`, `riveConfig.ts`, and `RiveHeroBindings.h` together.

---

## Data bindings (booleans)

All booleans live on view model **`HairStreaksTrimControl`**.

| Property | Type | Rule |
|----------|------|------|
| `streakVisible` | Transport | `true` while sequencer is playing; `false` on pause/stop. |
| `bodyGlow` | Layer count | `true` when **≥ 2** z-axis layers are active. |
| `faceGlowVis` | Layer count | `true` when **≥ 2** z-axis layers are active. |
| `bodyStreak` | Layer count | `true` when **≥ 3** z-axis layers are active. |
| `faceStreakVis` | Polyphony + layers + transport | `true` when **polyphony is on**, **≥ 2** layers active, and transport is playing; `false` when paused/stopped, polyphony off, or only one layer active. |

**Layer source:** top mini grid toggles in `LayerOverview` (`PatchState.layers[i].active`). Layer 0 is always active and cannot be toggled off. Paste onto an inactive layer (mini-grid clipboard) also sets `active = true` and re-evaluates layer-count bindings while playing.

**Polyphony source:** `PatchState.polyphony` (crown toggle). Wired via `HeroCanvas::setPolyphony`. `faceStreakVis` also requires ≥2 active layers (same discoverability as the crown).

### Pause / resume behaviour

1. **On pause/stop:** `streakVisible = false`. All glow/streak booleans (`bodyGlow`, `faceGlowVis`, `bodyStreak`, `faceStreakVis`) are forced **`false`** (including polyphony-driven face streak).
2. **On play / resume:** `streakVisible = true`. Re-count active layers for layer-driven glows; set `faceStreakVis` from **polyphony ∧ activeLayerCount ≥ 2**.
3. **While playing:** toggling a layer re-evaluates layer-count booleans **and** `faceStreakVis` (drops when only one layer remains, even if polyphony flag stays on); toggling the polyphony crown re-evaluates `faceStreakVis`.
4. **While paused:** layer/polyphony toggles update stored inputs only; glow/streak booleans stay **`false`** until play resumes.

**Related UI:** polyphony crown visibility uses active layer count (≥2) independently of Rive — see `PolyphonyCrown` / `SPEC.md`.

---

## Panel placement (unchanged v1 → v2)

Portrait box matches React `MatildaRivePortrait` / static PNG anchor (bottom-left in mask space):

| Constant | Value |
|----------|-------|
| `kPortraitOffsetX` | 20 |
| `kPortraitOffsetY` | 0 |
| `kPortraitHeightScale` | 1.2 |
| `kPortraitContentScale` | 0.88 |
| `kMetalRenderPanX/Y` | 0 / 0 |

---

## Version changelog

### v3 — 2026-07-15 (frozen with standalone Jul 15, 2026)

- **Asset:** `matilda-cartesia-v3.riv` replaces `matilda-cartesia-v2.riv` (same artboard / view model / binding names unless noted in Rive export).
- **Wire-up:** CMake BinaryData + `HeroCanvas` load symbols → `matildacartesiav3_riv`; React `riveConfig.ts`; `kRiveAssetVersion = "v3"`.
- **Wordmark above Metal:** “Matilda” / `Cartesia - v{VERSION}` is a `CALayer` on `MatildaRiveMetalHostView`, stacked **above** `CAMetalLayer` (`RiveHeroMetalView::setWordmarkOverlay`). Peer-sibling JUCE/NSView wordmarks lose z-order when `resizeViewToFit` re-attaches Metal — do not rely on `toFront()` for GPU heroes. Shared paint: `HeroWordmarkDrawing.h`; layout: `heroWordmarkBounds()` in `ReactShellLayout.h`.
- **`faceStreakVis`:** driven by **polyphony + ≥2 active layers + transport** (not “4 layers active”). Property renamed from v2’s `faceStreakVIs`. Off on pause/stop, polyphony off, or single active layer; on resume re-reads polyphony ∧ layer count.

### v2 — 2026-07-05

- **Asset:** `matilda-cartesia-v2.riv` replaces `matilda-cartesia.riv` (same artboard size/position).
- **New bindings:** `bodyStreak`, `bodyGlow`, `faceGlowVis`, `faceStreakVIs` (v2 asset typo — capital **I**; v3 uses `faceStreakVis`).
- **Logic:** Layer-count rules + pause clears all glow/streak booleans; resume restores from active layer count.
- **Implementation:** `RiveHeroBindings.h`, `setActiveLayerCount()` on hero renderer, wired from `PluginEditor` via `LayerOverview` toggles.

### v1 — initial

- **Asset:** `matilda-cartesia.riv`
- **Binding:** `streakVisible` tied to transport play/stop only.
