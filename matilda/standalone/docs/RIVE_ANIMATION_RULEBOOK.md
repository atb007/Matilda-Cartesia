# Rive Hero Animation Rulebook

Matilda standalone hero portrait — data-binding contract, asset versions, and change log.

**Source of truth (C++):** `matilda/standalone/Source/Rive/RiveHeroConfig.h`  
**Web mirror:** `cartesia-vst-ui/src/riveConfig.ts`  
**Binding logic:** `matilda/standalone/Source/Rive/RiveHeroBindings.h`

---

## Asset

| Field | Value |
|-------|-------|
| File | `cartesia-vst-ui/public/assets/matilda-cartesia-v2.riv` |
| Artboard | `Artboard` |
| View model | `HairStreaksTrimControl` |
| Layout | Same dimensions/artboard as v1 — no panel coordinate changes |

Embedded in the standalone app via `CMakeLists.txt` → JUCE `BinaryData`.

---

## Data bindings (booleans)

All booleans live on view model **`HairStreaksTrimControl`**.

| Property | Type | Rule |
|----------|------|------|
| `streakVisible` | Transport | `true` while sequencer is playing; `false` on pause/stop. |
| `bodyGlow` | Layer count | `true` when **≥ 2** z-axis layers are active. |
| `faceGlowVis` | Layer count | `true` when **≥ 2** z-axis layers are active. |
| `bodyStreak` | Layer count | `true` when **≥ 3** z-axis layers are active. |
| `faceStreakVis` | Layer count | `true` when **all 4** z-axis layers are active. Rive property name: `faceStreakVIs` (typo in asset — capital **I**). |

**Layer source:** top mini grid toggles in `LayerOverview` (`PatchState.layers[i].active`). Layer 0 is always active and cannot be toggled off.

### Pause / resume behaviour

1. **On pause:** `streakVisible = false`. All layer-driven booleans (`bodyGlow`, `faceGlowVis`, `bodyStreak`, `faceStreakVis`) are forced **`false`** regardless of which layers remain active.
2. **On play / resume:** `streakVisible = true`. Re-count active layers and set layer-driven booleans per the table above.
3. **While playing:** toggling a layer in the mini grid immediately re-evaluates layer-driven booleans.
4. **While paused:** layer toggles update the stored active count only; layer-driven booleans stay **`false`** until play resumes.

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

### v2 — 2026-07-05

- **Asset:** `matilda-cartesia-v2.riv` replaces `matilda-cartesia.riv` (same artboard size/position).
- **New bindings:** `bodyStreak`, `bodyGlow`, `faceGlowVis`, `faceStreakVIs` (bound as face streak vis).
- **Logic:** Layer-count rules + pause clears all glow/streak booleans; resume restores from active layer count.
- **Implementation:** `RiveHeroBindings.h`, `setActiveLayerCount()` on hero renderer, wired from `PluginEditor` via `LayerOverview` toggles.

### v1 — initial

- **Asset:** `matilda-cartesia.riv`
- **Binding:** `streakVisible` tied to transport play/stop only.
