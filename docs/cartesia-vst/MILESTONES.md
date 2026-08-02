# Matilda (Cartesia VST) — UI Milestones

**Approach:** Build the UI **module by module**, UI only — no engine/JUCE wiring until all modules are visually approved. Each module is built and frozen in isolation inside `cartesia-vst-ui/` (React + TS + Vite + Tailwind), then assembled into the full shell at the end.

**Figma source:** [main UI `4919:97886`](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886) · [cell master component `4789:104854`](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4789-104854)

---

## Milestone log

### ✅ M1 — Cell / knob anatomy (completed Jun 9, 2026)

The `Cell Anatomy States` component, fully interactive and frozen.

| Piece | Status |
|-------|--------|
| Main sequencer knob — 4-layer Figma assets (ring / sphere / gloss / soft-light) | ✅ |
| Note range C1–C12, indicator tick at 7 o'clock → drag/scroll to change | ✅ |
| Click knob centre → toggle active/off (cross-fade between Figma state assets) | ✅ |
| 4 color variants — orange, red, green, blue (from master component `4789:104854`) | ✅ |
| Off state — per-variant off gloss/soft-light + greyed outer rim (CSS filter) | ✅ |
| Mini-knobs (note-trigger ▲ amber, jitter ● teal) — hover reveal, click to arm at 50%, drag arc, click again to disarm | ✅ |
| Mini-knob tooltips — label when off; `%` when armed (hover + drag) | ✅ |
| LED indicator — static gray pill, bottom-right | ✅ |
| LED **lit** variant — glows in layer color while the playhead/note passes over the cell; gate-off cells stay dark (added Jun 10 with M2) | ✅ |
| Uniform `scale` prop for reuse in the 4×4 grid | ✅ |

**Files:** `src/components/CellAnatomyStates.tsx`, `SequencerKnob.tsx`, `MiniKnob.tsx`, `Led.tsx`, `knobColors.ts`

---

### ✅ M2 — 4×4 gem grid (completed Jun 10, 2026)

One layer = a 4×4 grid of 16 cells in a single color variant. The 4 variants (orange / red / green / blue) map to the 4 layers shown in the layer-overview block.

| Piece | Status |
|-------|--------|
| `Grid4x4` component — 16× `CellAnatomyStates`, row-major step mapping (0…15, top-left → right → next row) | ✅ |
| One grid per color variant / layer, switchable | ✅ |
| `playhead` prop on cell → LED lights in the layer's color as the step passes | ✅ |
| Demo Forward playhead (interval-driven; engine will drive the real step index in M9) | ✅ |
| Per-variant `ledColor` added to `knobColors.ts` | ✅ |

**Files:** `src/components/Grid4x4.tsx`, `Led.tsx` (lit state), `knobColors.ts` (`ledColor`), `App.tsx` (layer-switch showcase)

> Tweak (Jun 10): LED glow tightened — white-hot neon core + crisp small halo instead of the broad blurry glow; inset shading eased back while lit.
> Tweak (Jun 10): LED state change de-jittered — lit visual is an always-mounted overlay cross-fading via opacity only (gradients/box-shadows can't be interpolated, which caused snapping).

---

### ✅ M3 — Movement menu (completed Jun 10, 2026)

Bar from Figma `4957:103346`, dropdown from Figma `4960:3435`. All geometry derives from base px × `scale` prop, and ornament assets are true SVGs — the module vector-scales.

| Piece | Status |
|-------|--------|
| BgTexture grunge strip + filigree vine ornaments (top/bottom, SVG assets) | ✅ |
| ◄ ► silvery SVG arrows — cycle through modes, hover brighten | ✅ |
| Mode label — **Asimovian** 18.875px, 0.755px tracking, teal neon text-shadow (`#77a6ab` / `#10ffcf`) per Figma | ✅ |
| Dropdown — 24px-radius **frosted glass** sheet (backdrop blur 14px + translucent sheen), **Kode Mono Bold** 16px uppercase items, gradient hairline rules, ✕ close | ✅ |
| 6 modes: forward · reverse · ping-pong · pendulum · random · random skip | ✅ |
| Controlled/uncontrolled `value`/`onChange` API for engine hookup later | ✅ |
| Fonts loaded via Google Fonts (Asimovian, Kode Mono, Jost) in `index.html` | ✅ |

**Files:** `src/components/MovementMenu.tsx`, `index.html` (fonts), assets `public/assets/movement-{filigree-top,filigree-bottom,bgtexture}.svg`

> Tweak (Jun 10): dropdown glass tuned to Figma glass material params — Light -45° @ 80%, Refraction 80, Depth 85, Dispersion 50, Frost 20 → light 8px blur, -45° sheen + bright top/left edges, dark bottom/right bevel, inner glow, faint cool/warm chromatic fringes.

---

### ❌ M4 — Layer crystal row (SCRAPPED Jun 10, 2026)

Functionally replaced — no separate crystal selector row. **Layer selection for editing now happens directly in the layer-overview mini-grid (M5):** each layer's cell array has a bounding hit box; clicking an activated array selects that layer in the main 4×4 grid. Simpler, one less UI block, selection lives where the layers are visualised.

---

### ✅ M5 — Layer overview mini-grid (completed Jun 10, 2026)

Figma `4957:102758` (component states `4957:102757` "1 layer" / `4957:104400` "4 layers").

| Piece | Status |
|-------|--------|
| Rope-net frame + 4 layers × 16 draped mini-cells, exact Figma positions | ✅ |
| Activation toggles (top row) — layers 2–4 activate/deactivate; **layer 1 always on** | ✅ |
| Gray ↔ colored-rosette cross-fade (opacity only, 220ms) per layer state | ✅ |
| **Layer-select hit boxes** (replaces M4) — click an active array to edit it; invisible hit target (no outline stroke) | ✅ |
| Inactive mini-cells — Figma 4910:97550 composite PNG (`minigrid-inactive.png`), full slot width | ✅ |
| Playhead sync — row-major step remapped to draped column-major index; on-state gem + halo | ✅ |
| Selected toggle glow — orange uses the true Figma "on" asset; red/green/blue approximate with a CSS drop-shadow until their "on" assets are exported | ✅ |
| Deactivating the selected layer falls back to the first active layer | ✅ |
| Vector-scales via base px × `scale` prop | ✅ |

**Files:** `src/components/LayerMiniGrid.tsx`, assets `public/assets/minigrid-{frame,cell-orange,cell-orange-on,cell-red,cell-green,cell-blue,cell-inactive}.png`

---

## Upcoming modules (build order)

### ✅ M6 — Quantise Scale panel (completed Jun 10, 2026)

Figma `4976:3937` (left panel) · scale dropdown `4918:101473`.

| Piece | Status |
|-------|--------|
| Title + filigree ornaments + rule | ✅ |
| Min / Tonic / Max glass dropdowns (tonic-relative octave labels on Min/Max) | ✅ |
| Scale gem orb — all 13 scales in `scaleConfig.ts` + `ScaleGemOrb.tsx` animations | ✅ |
| `◄ Scale ►` bar + glass dropdown (same material as MovementMenu) | ✅ |
| Standalone showcase in `App.tsx` (separate from right-panel modules) | ✅ |
| Full-box dropdown hit targets + click-outside dismiss | ✅ |
| Tonic pill sizing + chevrons retained on Min/Tonic/Max | ✅ |

**Files:** `src/components/ScalePanel.tsx`, `src/scaleConfig.ts`, `src/components/glassDropdownStyle.ts`, `src/hooks/useClickOutside.ts`, assets `public/assets/scale-*`

> **Engine note (M9):** Scale / tonic / min / max changes re-resolve 4×4 grid note labels from each cell’s `degree` within the quantised scale and octave window — degrees unchanged, displayed pitches update.

### ✅ M7 — Global Settings / transport chrome (completed Jun 10, 2026)

Figma `4991:4644` (replaces earlier `4919:98600` frame).

| Piece | Status |
|-------|--------|
| Title + filigree + rule (“Global Settings”) | ✅ |
| Play/pause gem button (Figma composite PNG) | ✅ |
| Play Mode dropdown — Transport / Note; chevrons + glass list | ✅ |
| Clock dropdown — 1/4 · 1/8 · 1/16 · 1/32; chevrons + glass list | ✅ |
| Full-box hit targets + click-outside dismiss | ✅ |
| Standalone showcase in `App.tsx` | ✅ |

**Files:** `src/components/TransportChrome.tsx`, `src/transportConfig.ts`, assets `public/assets/transport-*`

### ✅ M8 — Control-panel shell assembly (completed Jun 11, 2026)

Figma [`4976:4727`](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4976-4727) — **Full Frame** (control cluster only; hero canvas deferred to M8b).

| Piece | Status |
|-------|--------|
| `MatildaShell` — all M1–M7 modules composed at Figma px positions | ✅ |
| Glass panel raster (`shell-glass-bg.png`, `1205×1407`) under controls | ✅ |
| Ornate frame raster (`shell-frame.png`, `1405×1766`) — **under** UI | ✅ (M8b: vines-only keyed overlay + CSS glass) |
| 4×4 grid slot sizing — Figma cell `122.355×140.977` + gaps | ✅ |
| Demo playhead + layer select wired across mini-grid ↔ main grid | ✅ |
| Default viewport scale `0.78` (~1096×1377 effective) | ✅ |

**Native design resolution:** `1405 × 1765` px (Figma frame). At **50%** → `703 × 883` px.

**Files:** `src/components/MatildaShell.tsx`, `src/App.tsx`, `public/assets/shell-{glass-bg,frame}.png`

> **Layer-order note:** Figma frame PNG is fully opaque (no alpha cut-outs). Stacking it above UI hid every module while `pointer-events: none` still allowed hover — frame must sit **between** glass and modules (Figma sibling order).
>
> **Polish (Jun 11):** Mini-knob hover tooltips — label when off (`Note Trigger` / `Jitter`), `%` when armed (hover + drag). Tonic pill fixed width `75px` (Figma `4976:5321`).

### ✅ M8b — Hero canvas + collapsible shell (completed Jun 11, 2026)

Figma expanded [`5002:6446`](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5002-6446) · collapsed [`5002:6447`](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5002-6447).

| Piece | Status |
|-------|--------|
| `MatildaPluginFrame` — outer window, default preview scale **0.52** | ✅ |
| Starfield + forest background (`hero-bg-m8b.png`) | ✅ |
| Matilda portrait + elliptical SVG mask (`matilda-portrait-v2.png` + `matilda-mask-alpha.svg`) | ✅ |
| Wordmark (“Matilda” / `Cartesia - v{VERSION}`) — Jacquard 24 | ✅ |
| Film-strip metal bars | ❌ removed (Figma tweak) |
| Glass chevron toggle (`CollapseToggle` · **70×70** px · Figma slot layout) | ✅ |
| Expanded chevron — top-left hero MainFrame `(83, 17)` · shows `>>` | ✅ |
| Collapsed chevron — inside shell frame `(95, 67)` · shows `<<` | ✅ |
| Chevron direction — per-glyph `scaleX` flip (no parent `rotate` / `none` transform bug) | ✅ |
| **Expanded** viewport **2376×1805** — shell at `x=886`, `y=50` | ✅ |
| **Collapsed** viewport **1515×1805** — shell centred in MainFrame at `x=85` | ✅ |
| Default state: **expanded** | ✅ |
| Animated width + hero slide (`−660 px`) + shell reposition (**380 ms**) | ✅ |
| Glass bedding — CSS gradients (`ShellGlassBedding` · embedded mode) | ✅ |
| Vines frame overlay (`shell-frame-vines-only.png` · flat fill keyed + feathered α) | ✅ |
| Vines raster — 1:1 **1405×1766** display + `@2x` `srcSet` + `rasterImageStyle.ts` | ✅ |
| Border-only vines frame overlay (nine-slice) | ⬜ deferred |

**Layer stack (shell):** glass bedding (z1) → vines frame (z2) → M1–M7 modules (z3).

**Files:** `MatildaPluginFrame.tsx`, `HeroCanvas.tsx`, `CollapseToggle.tsx`, `MatildaShell.tsx`, `ShellGlassBedding.tsx`, `ShellFrameOverlay.tsx`, `heroLayout.ts`, `shellLayout.ts`, `rasterImageStyle.ts`, `shellGlassStyle.ts`, `App.tsx`

**Assets:** `hero-bg-m8b.png`, `matilda-portrait-v2.png`, `matilda-mask-alpha.svg`, `shell-frame-vines-only.png`, `shell-frame-vines-only@2x.png`, `collapse-icon-*.svg`

Control modules from M8 (`MatildaShell`) are reused unchanged; only the outer canvas width and hero visibility animate.

> **Polish (Jun 11):** Chevron shrunk 30 % (100 → 70 px). Collapsed shell alignment fixed to Figma MainFrame centre (`x=85`). Vine aliasing reduced — integer 1:1 frame dimensions, feathered alpha on keyed PNG, bilinear-friendly CSS under `transform: scale(0.52)`.

> **Polish (Jun 25):** User UI scale — default **0.9**, drag **0.7…1.0** via 8 grips (corners + edges). Hero wallpaper full-bleed (no left/right void). Chevron uses corrected `@2x` PNGs with circular clip. **VST3/Windows:** `setResizeLimits`, hero backdrop cover for host oversize windows, `syncEditorToViewport()` — fixes FL Studio Fruity Wrapper void + title/filigree drift when host stretches plugin frame.

### 🔄 M8c — VST3 / Windows host UI parity (Jun 25, 2026)

| Piece | Status |
|-------|--------|
| `UiScale.h` / `uiScale.ts` — default 0.9, min 0.7, max 1.0 | ✅ |
| 8 resize grips (corners + edges) — Standalone + VST3 | ✅ |
| Hero full-bleed wallpaper (React + JUCE) | ✅ |
| Chevron `@2x` PNG per state + circular clip | ✅ |
| `HeroBackdropDrawing` — aspect-cover when host window > content | ✅ |
| `setResizeLimits` + `syncEditorToViewport()` in `PluginEditor` | ✅ |
| FL Studio Fruity Wrapper — void + title decoration alignment | 🔄 verify on Win build |

**Files:** `PluginEditor.cpp/h`, `NativePluginFrame.cpp/h`, `HeroCanvas.cpp`, `HeroBackdropDrawing.h`, `UiScale.h`, `CollapseToggle.cpp`, `UiResizeGrip.cpp/h`, `MatildaPluginFrame.tsx`, `HeroCanvas.tsx`, `CollapseToggle.tsx`, `uiScale.ts`

### 🔄 M9 — JUCE / Cartesia engine linking (in progress Jun 11, 2026)

Wire transport, playhead, layer scheduler, quantise scale resolution, and preset load/save from `cartesia/` into the UI prototype and JUCE shell.

| Piece | Status |
|-------|--------|
| TypeScript engine port (`cartesia-vst-ui/src/engine/`) — model, movement, pitch, `CartesiaEngine` | ✅ |
| `useMatildaEngine` hook — transport clock, playhead, layer scheduler | ✅ |
| `MatildaShell` wired — scale panel, movement, grid cells, mini-grid playhead | ✅ |
| Default preset load (`default.layer1.json`) | ✅ |
| Scale/tonic/min/max → knob note label re-resolution | ✅ |
| JUCE — scale-aware MIDI + playhead step tracking | ✅ |
| JUCE — transport / tempo / host sync | 🔄 GB standalone ✅; FL loopMIDI ✅; in-DAW matrix pending |
| JUCE — full UI port from `cartesia-vst-ui` (hero, glass shell) | ✅ M8b frozen |
| Preset save/load in JUCE + web | ⬜ |
| Web Audio / Web MIDI preview output | ⬜ |

---

## Future enhancements (not scheduled — keep architecture flexible)

Documented so M9+ work does not paint us into a static-hero corner.

### Rive hero animation — shipped (Jul 2026 · Windows validated Aug 2026)

| Piece | Notes |
|-------|-------|
| **Runtime** | Native Rive C++ — macOS **Metal** overlay / Windows **D3D11 PLS offscreen → `juce::Image`**; asset `matilda-cartesia-v3.riv` |
| **Bindings** | `streakVisible` (transport) + layer-count glows — see `RIVE_ANIMATION_RULEBOOK.md` |
| **Wordmark** | Metal: `CALayer` above `CAMetalLayer` on the same host (`setWordmarkOverlay`). Windows: JUCE sibling above painted frame. Do not use peer NSView/`toFront` for Metal GPU heroes |
| **Fallback** | Static masked PNG until first visible frame |
| **Integration** | `HeroCanvas` + `RiveHeroMetalView` (macOS) / `RiveHeroBackendD3D` image blit (Windows); collapse slide + mask bounds unchanged |
| **Windows framing** | Drawable extends to hero-mask right (Metal parity); Cover+CenterLeft aligns to portrait content box |

Further artboard/binding changes stay in the rulebook changelog.

### ✅ Windows Rive hero validated — Aug 1–2, 2026

**Milestone:** live animated hero on **Windows standalone** (user-validated) with framing parity to macOS Metal. Same D3D offscreen path is in the Windows VST3 binary.

| Release | What landed |
|---------|-------------|
| **v1.0.15** | Drop HWND swap-chain presentation → D3D11 PLS offscreen + CPU readback → `juce::Image` paint |
| **v1.0.16** | Fix DXGI factory init (`CreateDXGIFactory1`; prior `CreateDXGIFactory`+`IDXGIFactory1` → `E_NOINTERFACE` / static PNG) |
| **v1.0.17** | Mask-right drawable extend + content-align (fixes hard vertical clip on hair/dress) |

**Diagnostics:** `%TEMP%\MatildaRiveD3D.log` — expect `DXGI factory via CreateDXGIFactory1` → `loadBytes: ok` → `first D3D offscreen frame blitted to juce::Image`.

**Do not regress to HWND child swap chains for Windows Rive** without a new milestone — that path stayed on the static PNG in real standalone/VST hosts.

---

## Scraped assets (ready for use)

| File | Source node | Used by |
|------|-------------|---------|
| `public/assets/shell-glass-bg.png` | `4976:4728` | M8 ✅ |
| `public/assets/shell-frame.png` | `4976:4729` | M8 ✅ |
| `public/assets/shell-frame-vines-only.png` | keyed from `4976:4729` | M8b ✅ |
| `public/assets/shell-frame-vines-only@2x.png` | LANCZOS mip | M8b ✅ |
| `public/assets/hero-bg-m8b.png` | `5001:5122` | M8b ✅ |
| `public/assets/matilda-portrait-v2.png` | `5001:5125` | M8b ✅ (Rive swap target) |
| `public/assets/matilda-mask-alpha.svg` | mask group | M8b ✅ |
| `public/assets/collapse-icon-*.svg` | `5002:6437` / `5002:6568` | M8b ✅ |
| `figma-assets/scale-gem.png` | `4919:98985` | M6 |
| `figma-assets/crystal-row.png` | `4919:99372` (orange crystal) | M4 |
| `figma-assets/chrome-transport.png` | `4919:98600` | M7 (reference) |
| `public/assets/layer-overview.png` | `4919:99118` | M5 (reference) |
| `figma-assets/main-panel.png` | `4919:98982` | layout reference |
| Knob layer assets (ring/sphere/gloss/soft-light ×4 colors ×2 states) | `4789:104854` | M1 ✅ (Figma CDN URLs in `knobColors.ts`) |

---

*Milestones v1 · pairs with `SPEC.md` and `FIGMA-CHECKLIST.md` · M8b final Jun 11, 2026 · next: M9*

---

## JUCE port progress (`matilda/plugin/`)

Native port tracks React modules via `UiDevConfig.h` isolated dev views, then `FullShell` integration.

| Module | React | JUCE | Status |
|--------|-------|------|--------|
| M1 Gem cell | `CellAnatomyStates.tsx` | `GemCell.cpp` | ✅ integrated |
| M2 4×4 grid | `Grid4x4.tsx` | `GemGrid.cpp` | ✅ integrated |
| M3 Movement menu | `MovementMenu.tsx` | `MovementSelector.cpp` | ✅ **frozen** Jun 17, 2026 |
| M5 Layer overview | `LayerMiniGrid.tsx` | `LayerOverview.cpp` | ✅ **frozen** Jun 17, 2026 · clipboard ported to plugin Jul 16, 2026 |
| M6 Quantise panel | `ScalePanel.tsx` | `QuantisePanel.cpp` | ✅ **frozen** Jun 18, 2026 |
| M7 Global Settings | `TransportChrome.tsx` | `TransportBar.cpp` | ✅ **frozen** Jun 17, 2026 |
| M8b Shell / hero | `MatildaPluginFrame.tsx` | `NativePluginFrame.cpp` | ✅ **frozen** Jun 18, 2026 · Rive v3 + wordmark Jul 15, 2026 |
| Step scroll | — | `StepScroll.cpp` | ✅ ported to plugin Jul 16, 2026 |
| Polyphony crown | Figma `glowPolyphony` | `PolyphonyCrown.cpp` | ✅ ported to plugin Jul 16, 2026 |
| Presets bar | Figma `PresetModule` | `PresetBar` / `PresetLibrary` | ✅ ported to plugin Jul 16, 2026 |
| Post–v1.0.11 → Windows VST3 | — | — | ✅ source port Jul 16, 2026 · Rive hero Windows-validated **v1.0.17** (Aug 2026) |

### M3 Movement — frozen (Jun 17, 2026)

- Figma geometry via `MovementLayout.h` · filigree + bg texture + arrows + Asimovian label + glass dropdown
- Filigree: SVG raster + horizontal gradient tint (muted `#BABABA`, ~12% peak alpha) — does not compete with label
- Fonts: Asimovian + Kode Mono Bold bundled
- Assets: `movement-filigree-*.svg`, `movement-bgtexture@2xpng.png`

### M5 Layer overview — frozen (Jun 17, 2026)

- Figma geometry via `MiniGridLayout.h` · 4×16 draped cells + rope frame + top toggles
- Playhead: row-major step → column-major mini index; on-gem + soft radial glow at gate-on steps
- Top toggle: lit for **playing layer** while transport runs; **selected layer** when stopped; all active lit in poly mode
- Gate-off cells: `minigrid-inactive.png` socket (syncs from main 4×4 grid)
- Layer activate / select hit boxes wired to `PatchState`
- **Jul 15, 2026 (standalone — frozen):** right-click context menu — copy notes / notes+knobs, paste (activates inactive), reset (gates **on**, degree 0), undo; click-anchored compact glass menu + floater; `LayerClipboard.h`

### Standalone freeze — Jul 15, 2026

**Scope:** `matilda/standalone/` only. Do not change behaviour in this set without a new milestone note. Port target: [Windows VST3 backlog](./SPEC.md#windows-vst3-port-backlog-postv1011). Spec: [SPEC.md](./SPEC.md) · Design: [DESIGN.md](./DESIGN.md) §Layer behaviour.

| Piece | Status | Notes |
|-------|--------|-------|
| Per-layer `active_step_count` (1…16) | ✅ frozen | Engine path length; persist in patch JSON |
| Vine step scroll under 4×4 | ✅ frozen | `StepScroll` · snap 4/8/12/16 · `assets/ui/stepscroll/` |
| Hide out-of-range cells (grid + mini) | ✅ frozen | `GemGrid` / `LayerOverview` |
| Frosted shell glass | ✅ frozen | Blurred wallpaper under translucent bedding |
| Polyphony engine + MIDI | ✅ frozen | `PatchState::polyphony`; simultaneous ticks; per-layer notes |
| Polyphony crown UI | ✅ frozen | Glow-only; idle pulse; morph on; fade if &lt;2 layers |
| Mini-grid multi-playheads (poly) | ✅ frozen | `LayerOverview::setPolyPlayheads` from `lastTickResults` |
| Layer copy / paste / reset / undo | ✅ frozen | Reset = gate on + degree 0; visible-step clipboard |
| Presets bar + user library | ✅ frozen | `PresetBar` / `PresetLibrary`; dirty `*`; load keeps BPM/transport |
| Rive hero v3 + Metal wordmark | ✅ frozen | `matilda-cartesia-v3.riv`; wordmark `CALayer` above GPU |
| Windows Rive hero (D3D offscreen) | ✅ validated Aug 2026 | **v1.0.15–v1.0.17** — see [Windows Rive hero validated](#-windows-rive-hero-validated--aug-12-2026) |
| `faceStreakVis` ← polyphony ∧ ≥2 layers + transport | ✅ frozen | Rulebook; off when only one layer active |
| Playhead UI ↔ note sync | ✅ frozen | Lights use **last fired** step (`lastStepIndex` / tick results), not post-advance `stepIndex` |
| Crown / scroll / preset layout | ✅ frozen | `ReactShellLayout.h` |

**Out of freeze / next:** random gen · movement-mode follow-ups · remaining Windows VST3 host soak (non-Rive).

### Plugin / Windows VST3 source port — Jul 16, 2026

The standalone freeze table above is now merged into `matilda/plugin/` for the Windows VST3 path while preserving the plugin-only silent-instrument registration, DAW sync toggle, host transport behavior, and direct MIDI-out selector.

| Area | Status | Notes |
|------|--------|-------|
| Polyphony engine + MIDI | ✅ source port | Per-layer active notes + `lastTickResults`; mono note path preserved |
| Polyphony crown + Rive `faceStreakVis` | ✅ source port | `faceStreakVis = polyphony ∧ ≥2 active layers ∧ transport` |
| Per-layer step count + vine | ✅ source port | `active_step_count`, `StepScroll`, hidden out-of-range cells |
| Layer clipboard | ✅ source port | Right-click copy/paste/reset/undo; reset gates on at degree 0 |
| Presets | ✅ source port | User library + native Save dialog; load preserves BPM/transport state |
| Frosted shell glass | ✅ source port | Shell non-opaque + frosted backdrop sample |
| Build verification | ✅ macOS + Windows CI | Release zips via tag workflow; Windows Rive hero validated on standalone **v1.0.17** |

### M7 Global Settings — frozen (Jun 17, 2026)

**Files:** `TransportBar.cpp/h`, `TransportLayout.h`, `TransportConfig.h`, `FiligreeDrawing.h`, `GlassDropdownDrawing.h`

| Piece | Status |
|-------|--------|
| Title filigree — movement SVG + bg-texture tint (no chevrons) | ✅ |
| Play gem — frame / glass / play·stop icons (correct layer order) | ✅ |
| Play Mode + Clock headers — clock ornaments + Supermercado One | ✅ |
| Clock dropdown — 13 divisions · max 6 visible · scroll | ✅ |
| Frosted glass menus — backdrop blur + tuned frost | ✅ |
| Legacy `clockBox_` removed · wired to patch | ✅ |

### M6 Quantise Scale — frozen (Jun 18, 2026) · gems + motion (Jun 23, 2026)

React reference: `ScalePanel.tsx` · Figma `4976:3937` · isolated dev view `DevView::M4_QuantisePanel`.

**Files:** `QuantisePanel.cpp/h`, `ScaleLayout.h`, `GemSparksOverlay.cpp/h`, `ScaleGemPalette.h`, `GlassDropdownDrawing.h`, `ScaleGemOrb.tsx`, `scaleConfig.ts`, `MatildaImages.cpp`

| Piece | Status |
|-------|--------|
| Title filigree — movement-style SVG + bg texture tint | ✅ |
| Min / Tonic / Max pickers — glass boxes + connectors + labels | ✅ |
| Scale gem orb art — all 13 scales (`scale-gem-{id}.png`) | ✅ Jun 23 |
| Per-scale gem lookup — React `SCALE_GEM_IMAGES` + JUCE `scaleGemForMode()` | ✅ Jun 23 |
| Scale gem + spark overlay | ✅ |
| Scale change transition — shrink-out 160ms → swap → grow-in 220ms | ✅ Jun 23 |
| Idle float (3.2s sine, ~3px) + hover boost — React + JUCE parity | ✅ Jun 23 |
| Scale bar ◄ ► + mode dropdown | ✅ |
| Supermercado One labels · 20px dropdown items | ✅ |

> Polish deferred: divider ornaments (minor).

**Asset drop:** `cartesia-vst-ui/public/assets/scale-gem-{chromatic,major,minor,dorian,phrygian,lydian,mixolydian,locrian,harmonic_minor,melodic_minor,pentatonic,pentatonic_minor,blues}.png` — embedded in JUCE via `CMakeLists.txt` · container uses contain / centred fit for varying export sizes.

### M8b Shell / hero — frozen (Jun 18, 2026)

**Files:** `NativePluginFrame.cpp/h`, `ShellChrome.cpp/h`, `ShellGlassDrawing.cpp/h`, `HeroCanvas.cpp`, `CollapseToggle.cpp/h`, `ReactShellLayout.h`, `MatildaFonts.cpp` (Jacquard 24)

| Piece | Status |
|-------|--------|
| Shell chrome scale fix — glass + vines match module `previewScale_` | ✅ |
| Full-shell module layout — React `MatildaShell.tsx` positions | ✅ |
| Collapse / expand — viewport clip + 380ms ease + chevron toggle | ✅ |
| Collapse icon — Figma `5002:6419` PNGs · fixed Y (83px inset) | ✅ |
| Hero wordmark — Jacquard 24 (`Matilda` / `Cartesia - v{VERSION}`); Metal = `CALayer` above Rive | ✅ Jul 15, 2026 freeze |
| Glass bedding — CSS/Figma gradients (radial @ 20% + linear fade) | ✅ |

### M8 Shell chrome — superseded by M8b freeze above

**Problem (fixed):** `ShellChrome` painted at design px inside scaled shell — frame overlapped knobs.

**Dev view:** `DevView::M7_ShellChrome` — isolate chrome alignment before `FullShell`.

---

## Host / transport integration (`matilda/plugin/` — Jun 17–18, 2026)

First end-to-end MIDI playback path validated. GarageBand exposed hard platform limits; multi-DAW testing is the next gate before locking host architecture.

### What works today

| Piece | Status | Notes |
|-------|--------|-------|
| Standalone → IAC → GB instrument track | ✅ | Matilda MIDI **out** to GB; grid steps, notes audible |
| Play gem arms sequencer | ✅ | Note mode (default preset) — internal sample clock |
| Transport gating (GridWalker parity) | ✅ | Standalone sync off = Matilda play; sync on = external MIDI clock steps |
| Host playhead BPM (in-plugin) | ✅ | Logic / Ableton / Reaper / FL / Bitwig when loaded as MIDI effect |
| Manual BPM (standalone footer) | ✅ | Double-click footer BPM; persisted in plugin state |
| MIDI clock BPM estimate | ✅ | When a DAW sends clock on IAC/loopMIDI input (v1.0.9: sample-accurate) |
| `play_mode` in preset JSON | ✅ | `note` \| `transport` parsed/saved |
| Debug footer (standalone) | ✅ | `step=` / `tick=` / layer / scale + BPM + sync toggle |
| Git repo on GitHub | ✅ | `origin/main` — HTTPS remote |

**Key files:** `PluginProcessor.cpp/h`, `PluginEditor.cpp`, `PatchStore.cpp`, `matilda/plugin/README.md`

### GarageBand findings (blockers documented)

GarageBand is a valid **MIDI sink** (receives Matilda notes via IAC) but **cannot** be a MIDI master for external apps:

| Capability | GarageBand | Implication |
|------------|------------|-------------|
| MIDI out (notes) to IAC | ✅ (Matilda → GB) | Recommended workflow |
| MIDI clock / transport out | ❌ | No "Send MIDI clock" preference exists |
| Host BPM to external app | ❌ | Matilda cannot auto-read GB project tempo |
| In-track MIDI effect plugin | ⚠️ unreliable | Use Standalone + IAC instead |

**Workaround (current):** Match tempo manually — double-click Matilda footer BPM to GB transport-bar tempo (e.g. 60). Keep **Sync external transport** off. Press play in Matilda.

### Transport / tempo model (current)

```text
                    ┌─────────────────────────────────────┐
                    │           MatildaAudioProcessor      │
                    └─────────────────────────────────────┘
                                      │
          ┌───────────────────────────┼───────────────────────────┐
          │                           │                           │
   Host playhead BPM            MIDI clock (24 ppq)          userBpm_ (manual)
   (plugin in DAW)              (standalone + IAC)           (standalone + GB)
          │                           │                           │
          └───────────────────────────┴───────────────────────────┘
                                      │
                              effective bpm_ → sample clock
```

| Mode | Stepping | Tempo source |
|------|----------|--------------|
| Standalone, sync **off**, play gem | Internal sample clock | `userBpm_` (or clock/playhead if present) |
| Standalone, sync **on**, play gem | Incoming MIDI Start/Clock on IAC | Clock interval + playhead |
| Plugin, `play_mode: note` | Play gem armed | Host playhead BPM |
| Plugin, `play_mode: transport` | DAW playing **and** play gem armed | Host playhead BPM |

Default: `followExternalTransport_ = false` (GB-safe). `userBpm_` defaults to 120.

**Beat-quantized start (Jun 18, 2026):** Play (DAW transport or Matilda play gem) arms the sequencer but the **first arp step waits for the next downbeat** — host PPQ when in-plugin, internal beat phase when standalone, 24 MIDI clocks when external sync is on. Stop is immediate.

### Engine QOL — pitch / knob (Jun 18, 2026)

| Piece | Status | Notes |
|-------|--------|-------|
| Scale-quantised note list | ✅ | All in-scale MIDI notes between Min…Max, ascending |
| Min octave = knob 0% | ✅ | Display octave matches UI (`minOctave + 1` MIDI base) |
| Octave-boundary carry | ✅ | B#→C crossings (e.g. A#5 → C6 → C#6) without jumps |
| Knob arc 0…100% | ✅ | Full window mapped to indicator arc; no wrap at ends |
| Drag + scroll | ✅ | ~154px full turn; trackpad accumulator; clamp at min/max |
| Re-quantise on scale change | ✅ | Tonic / scale / min / max snaps all cells |

**Key files:** `SequencerEngine.cpp/h`, `GemCell.cpp/h`, `KnobDrawing.h`

**FL Studio / BlueARP routing:** Virtual MIDI ports validated on FL 20.0.1 (Jul 2026). Fruity Wrapper internal ports failed on same version; retest on FL 21+ pending. See [Integration milestones (Jul 2026)](#integration-milestones-jul-2026) and `matilda/plugin/README.md`.

---

## Integration milestones (Jul 2026)

Major end-to-end validation on **FL Studio 20.0.1 (build 451, 64-bit, Windows 11 + loopMIDI)**.

### ✅ Virtual-port routing — production path

| Setup | What was tested | Result |
|-------|-----------------|--------|
| **Standalone + loopMIDI (two ports)** | Port A: FL master sync → Matilda input. Port B: Matilda notes → FL synth channel | ✅ Notes audible, transport sync, BPM tracks DAW (v1.0.9 clock fix) |
| **VST3 + loopMIDI (MIDI Out selector)** | Matilda plugin → virtual port → synth channel input port | ✅ Notes audible; bypasses FL's broken VST3 MIDI-out plumbing |

**Why virtual ports work:** FL 20.x does not forward VST3 **MIDI output** between Fruity Wrapper plugins. OS-level virtual ports (loopMIDI / IAC) sit outside FL's internal MIDI router — same pattern that made Standalone + GarageBand work.

**Shipped in:** v1.0.8 (plugin direct MIDI out) · v1.0.9 (standalone clock sync + UI QOL)

**Docs:** `matilda/plugin/README.md` · `matilda/standalone/README.md` · `matilda/plugin/releases/v1.0.8-notes.md` · `v1.0.9-notes.md`

### ✅ Fruity Wrapper internal ports — validated on FL 25 (v1.0.11)

BlueARP-style wiring (Matilda output port *N* → synth input port *N* in the same Fruity Wrapper) **works on FL Studio 25.1.4** when Matilda registers as a **silent instrument** (not MIDI-FX). v1.0.5–v1.0.10 MIDI-FX registration caused audio/MIDI to land on **master** only — channel meters, step lanes, and armed recording were empty.

**Required setup (v1.0.11+):**

1. Load Matilda VST3 + synth in one Fruity Wrapper.
2. Matilda Settings → output port *N*; synth → input port *N* (use 11+).
3. Matilda plugin **MIDI Out → (None)**.
4. Route wrapper channel to a mixer insert (not master).
5. FL transport play + Matilda play gem → confirm channel meter + step rack activity.
4. Compare against virtual-port path (should be equivalent if wrapper routing works).
5. Record FL version + build number in this table.

### Standalone codebase split (Jun–Jul 2026)

| Piece | Status |
|-------|--------|
| `matilda/standalone/` forked from v1.0.2 audio base | ✅ |
| Plugin builds VST3/AU only; standalone builds Standalone only | ✅ |
| Forward-port filigree + resize persistence (no DAW-sync UI) | ✅ v1.0.9 |
| Sample-accurate MIDI clock tempo sync | ✅ v1.0.9 |

---

## DAW compatibility — test matrix

Use this checklist before freezing host architecture. Build outputs: VST3 + AU from `matilda/plugin/`; Standalone from `matilda/standalone/`.

| DAW | Load as | Transport sync | Auto BPM | MIDI out to instrument | Priority | Status |
|-----|---------|----------------|----------|------------------------|----------|--------|
| **GarageBand** | Standalone + IAC | ❌ external master | Manual BPM | ✅ virtual port | P0 | ✅ smoke-tested |
| **FL Studio 20.0.1** | Standalone + loopMIDI (2 ports) | ✅ MIDI clock | ✅ v1.0.9+ | ✅ virtual port | P0 | ✅ validated Jul 2026 |
| **FL Studio 20.0.1** | VST3 + loopMIDI (MIDI Out) | Host playhead | Host playhead | ✅ virtual port | P0 | ✅ validated Jul 2026 |
| **FL Studio 25.1.4** | VST3 Fruity Wrapper ports | Host playhead | Host playhead | ✅ wrapper ports (v1.0.11 instrument mode) | P0 | ✅ validated Jul 2026 |
| **FL Studio 20.0.1** | VST3 Fruity Wrapper ports | Host playhead | Host playhead | ❌ wrapper ports | P1 | ❌ failed — use virtual port |
| **Logic Pro** | AU MIDI effect | ? | Host playhead | In-chain | P1 | ⬜ |
| **Ableton Live** | VST3 MIDI effect | ? | Host playhead | In-chain | P1 | ⬜ |
| **Bitwig** | VST3 / CLAP? | ? | Host playhead | In-chain | P1 | ⬜ |
| **Reaper** | VST3 / AU | ? | Host playhead | In-chain | P2 | ⬜ |

**Per-DAW test script (copy for each row):**

1. Install `Matilda.vst3` / `Matilda.component`; rescan plugins.
2. New project @ **60 BPM**; add instrument track; insert Matilda **before** instrument.
3. Arm Matilda play gem → confirm footer/host shows **60 BPM** (not 120).
4. DAW transport play → grid steps (`play_mode: transport`) or gem-only (`play_mode: note`).
5. Confirm MIDI reaches instrument; no stuck notes on stop.
6. Save/reload session → patch + BPM + sync state restored.
7. *(Optional)* Standalone + IAC: enable DAW MIDI clock on bus; toggle sync on; confirm steps follow DAW transport.

Record pass/fail + quirks in a row below or in `matilda/plugin/README.md`.

---

## Open architecture questions (Jun 18, 2026)

Not decided — keep flexible until P1 DAW matrix is complete.

| Question | Options | Leaning |
|----------|---------|---------|
| **Primary deployment** | A) In-DAW plugin only · B) Standalone + IAC only · C) Both first-class | **C** — GB forces Standalone; pro DAWs prefer in-plugin |
| **Tempo when host can't sync** | Manual BPM · Tap tempo · Audio click follower | **Manual BPM** shipped; tap/audio later |
| **Default play mode** | `note` (gem gates clock) vs `transport` (DAW + gem) | **`note`** for GB standalone; document per-DAW |
| **Sync toggle default** | On (Logic/Ableton) vs Off (GB) | **Off** — safe default; enable when clock confirmed |
| **Plugin format priority** | AU (Logic) · VST3 (Win/cross) · CLAP (Bitwig) | AU + VST3 now; CLAP if Bitwig needs it |
| **MIDI effect vs instrument wrapper** | Pure MIDI FX vs silent instrument shell | **MIDI FX** — matches spec; verify each host accepts it |
| **Web UI vs native JUCE** | Finish JUCE port vs embed WebView | **JUCE** — M8b shell frozen natively; no WebView for v1 |
| **State persistence** | Host preset only vs `.matilda` file export | Host preset + JSON in state blob (partial ✅) |

### Possible architecture shifts (if DAW tests fail)

- **Instrument shell** — some hosts reject MIDI-only plugins; wrap as instrument with dummy audio pass-through.
- **Dedicated clock bus** — separate IAC port for clock vs note data (avoid feedback loops).
- **Per-host play mode presets** — e.g. `garageband-standalone.json` vs `logic-transport.json`.
- **Remove sync toggle from shipping UI** — dev-only when only Logic users need it; or rename + contextual help per host.
- **Bidirectional sync** — Matilda as clock master (Ableton slave) — only if product needs it.

---

*Milestones v1 · pairs with `SPEC.md` and `FIGMA-CHECKLIST.md` · M8b final Jun 11, 2026 · host + engine QOL Jun 18, 2026 · FL virtual-port Jul 2026 · **standalone Jul 15, 2026 freeze** (polyphony · step scroll · clipboard · presets · Rive v3 · playhead sync)*

