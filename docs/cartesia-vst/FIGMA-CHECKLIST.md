# Figma Component Checklist — Matilda (Cartesia VST v1.0)

**Product name:** Matilda · Cartesia v1.0  
**Figma:** [main UI](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886) · [layer/grid](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830)  
**Spec:** `SPEC.md` · **UI behaviour:** `DESIGN.md` · **Build progress:** `MILESTONES.md` · **Engine:** `cartesia/` + `matilda/`

> **Status (Jun 11, 2026):** M1–M8b **shipped** in `cartesia-vst-ui/`. Entry: `MatildaPluginFrame` (hero + collapse + `MatildaShell`). M4 crystal row **scrapped**. **M9** engine link next. Expanded [`5002:6446`](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5002-6446) · collapsed [`5002:6447`](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5002-6447).

Use this doc as your Figma build order. Check off components as you ship them.

---

## Shipped in code (M8b — Jun 11, 2026)

| Figma node | Code | Notes |
|------------|------|-------|
| `5002:6446` Expanded | `MatildaPluginFrame` | **2376×1805** · shell `x=886` |
| `5002:6447` Collapsed | `MatildaPluginFrame` | **1515×1805** · shell `x=85` (MainFrame-centred) |
| `5001:5121` MainFrame | `HeroCanvas` | Starfield + masked portrait + wordmark |
| `5002:6437` / `5002:6568` icon | `CollapseToggle` | **70×70** px · Figma chevron slots |
| `4976:4727` / `5002:8589` shell | `MatildaShell` | Glass → vines → controls |
| `5007:6100` GlassBg | `ShellGlassBedding` | CSS gradients (embedded) |
| `4976:4729` Frame | `ShellFrameOverlay` | `shell-frame-vines-only.png` + `@2x` |
| `5001:5572` metal strips | — | **Removed** per design tweak |

**Deferred:** nine-slice vines border · **Rive** animated portrait (hair/body + state machine) — see `MILESTONES.md` §Future enhancements.

---

## Figma component map (from file)

| Figma node / component | Code component |
|------------------------|----------------|
| `4919:98982` 1 layer activated | Screen / Main |
| `4919:99118` top cell | `LayerOverview/MiniGrid` |
| ~~`4919:99372` All four activated~~ | ~~`LayerSelector/CrystalRow`~~ **scrapped** — selection via mini-grid hit boxes |
| `4922:103831` 4×4 Grid | `Grid/4×4` |
| `Cell Anatomy States` | `Cell/Gem` |
| `Inactive Cell` | `Cell/Gem/GateOff` |
| `4919:99226` Quantise Scale | `Panel/QuantiseScale` |
| `4919:99373` Presets (movement list) | `Movement/Dropdown` |
| `4919:98600` playPause + Clock + Play mode | `Chrome/Transport` (external frame) |

**Movement dropdown labels:** forward · reverse · ping-pong · pendulum · random · random skip

---

## 0. Figma file setup (do first)

| # | Page | Purpose |
|---|------|---------|
| ☐ | **00 — Cover** | Plugin name, version, aspect ratio targets |
| ☐ | **01 — Tokens** | Color, type, spacing, motion, elevation |
| ☐ | **02 — Atoms** | Smallest reusable pieces |
| ☐ | **03 — Molecules** | Combined atoms (cell, axis strip) |
| ☐ | **04 — Organisms** | Grid, transport, modals |
| ☐ | **05 — Screens** | Main + modal states |
| ☐ | **06 — Illustration** | Hero art, layers, parallax guides 🎨 |
| ☐ | **07 — Motion** | Prototype flows + spec notes |
| ☐ | **08 — Handoff** | Redlines, asset list, JUCE map |

**Frame sizes to design (tech must scale):**

| Frame | Size | Notes |
|-------|------|-------|
| Default | **900 × 620** | Primary plugin window |
| Compact | **720 × 520** | Minimum usable |
| Expanded | **1100 × 720** | Modal open / inspector visible |
| HiDPI | 2× exports | `@2x` PNG/SVG for Retina |

**Layout grid:** 8 px base · 12-column · 16 px outer margin · 8 px gutter.

---

## 1. Creative direction (lock before components)

### Borrow from Dream Eater

| Dream Eater pattern | Your serene translation |
|---------------------|-------------------------|
| Controls = body parts | Controls = **world elements** (moon, tide, lantern, petal, ripple) |
| One hero illustration | **One hero scene** with depth (foreground grid, mid mist, far sky) |
| Weird but memorable | **Calm but memorable** — users remember the “floating grid garden” |
| Color as emotion | **Twilight palette** — not neon chaos; bioluminescent accents |
| Non-grid layout | Organic layout with **invisible 8 px grid** underneath for dev |

### Mood board keywords (for you + illustrator)

`mist` · `tide` · `lantern` · `glass` · `soft glow` · `deep blue` · `dawn gold` · `breathing` · `orbit` · `layered depth`

### What NOT to copy

Grotesque character, harsh contrast, joke UI copy, unreadable control placement, controls that only work at one window size.

---

## 2. Design tokens (Page 01 — build as Figma Variables)

### Color variables

| Token | Role | Suggested direction |
|-------|------|---------------------|
| `bg/deep` | Window background | `#0B1020` – `#121828` |
| `bg/mist` | Overlay panels | `#1A2238` @ 72% |
| `surface/glass` | Cards, modals | white @ 6–10% + blur |
| `accent/z1` | Layer 1 | soft coral `#E8A598` |
| `accent/z2` | Layer 2 | sage `#8FB8A8` |
| `accent/z3` | Layer 3 | dusk blue `#7B9FD4` |
| `accent/z4` | Layer 4 | pale gold `#D4C4A0` |
| `playhead/glow` | Current cell | `#B8E0FF` + outer blur |
| `gate/on` | Active step | accent @ 100% |
| `gate/off` | Rest | accent @ 18% |
| `prob/ring` | Probability arc | white @ 40% |
| `text/primary` | Labels | `#E8EDF5` |
| `text/muted` | Hints | `#8B95A8` |
| `danger` | Destructive random | muted rose (not alarm red) |
| `focus` | Keyboard focus | `#A8D4FF` 2 px ring |

### Typography

| Token | Use | Spec |
|-------|-----|------|
| `type/display` | Plugin title | 1 weight only — e.g. serif or soft geometric |
| `type/label` | Knobs, axes | 11–12 px, medium, +0.02 em tracking |
| `type/cell` | Note names in grid | 13–15 px, tabular nums |
| `type/micro` | Prob %, divisions | 10 px |
| `type/modal-title` | Sheet headers | 16–18 px |

☐ Font files licensed for plugin embedding (OTF/TTF)  
☐ Line heights defined per token  
☐ Missing-glyph fallback noted for handoff

### Spacing & radius

| Token | Value |
|-------|-------|
| `space/xs–xl` | 4, 8, 12, 16, 24, 32, 48 |
| `radius/sm–xl` | 6, 10, 16, 24, pill |
| `elevation/1–3` | soft shadow + inner highlight (glass) |

### Motion tokens (Page 07 — dev implements)

| Token | Duration | Easing | Use |
|-------|----------|--------|-----|
| `motion/breathe` | 3.2 s loop | sine | Idle hero — **future Rive** hair/body state |
| `motion/collapse` | 380 ms | cubic-bezier(0.4,0,0.2,1) | Hero panel expand/collapse ✅ |
| `motion/playhead` | 120 ms | ease-out | Cell highlight move |
| `motion/modal-in` | 220 ms | ease-out | Sheet open |
| `motion/modal-out` | 160 ms | ease-in | Sheet close |
| `motion/glow-pulse` | 800 ms | ease-in-out | Gate fire feedback |
| `motion/layer-switch` | 180 ms | ease | Z tab crossfade |

---

## 3. Atoms (Page 02)

Each atom: **Default · Hover · Active · Disabled · Focus** variants unless noted.

### 3.1 Core controls

| ☐ | Component | Variants | Tech notes |
|---|-----------|----------|------------|
| ☐ | `Button/Icon` | play, stop, reset, dice, close | 44×44 min hit target; SVG icon |
| ☐ | `Button/Text` | primary, ghost | Preset actions |
| ☐ | `Toggle/Pill` | on/off | Quantize, reverse forced |
| ☐ | `Toggle/LED` | 4 layer colors | Maps to Z1–Z4 accents |
| ☐ | `Knob/Micro` | 0–100% | Probability modal |
| ☐ | `Knob/Standard` | value + label | Min, range, oct, vel |
| ☐ | `Slider/Horizontal` | 0–100% | Randomize amount |
| ☐ | `Dropdown/Compact` | scale, movement, division | Max 8 visible items |
| ☐ | `Stepper/Numeric` | ÷1–÷32 | Clock divisions |
| ☐ | `Checkbox/Soft` | on/off | Randomize scopes |

### 3.2 Grid atoms

| ☐ | Component | Variants | Tech notes |
|---|-----------|----------|------------|
| ☐ | `Cell/TriggerIcon` | idle/hover/armed + ring | orange ▲; default 50% on arm |
| ☐ | `Cell/JitterIcon` | idle/hover/armed + ring | green ● |
| ☐ | `Cell/OctaveLabel` | hidden at 0 | `+ N Oct` below gem |
| ☐ | `Cell/Gem` | on/off/playhead/selected | centre click = hard gate |
| ☐ | `LayerOverview/MiniGrid` | active/inactive | playhead 100% / field 12% |
| ☐ | `LayerOverview/Crystal` | on/off | activate layer (top-right) |
| ✗ | ~~`LayerSelector/Crystal`~~ | — | scrapped — edit layer selected by clicking its mini-grid array |
| ☐ | `Movement/Dropdown` | 6 modes | per selected layer |
| ☐ | `Panel/QuantiseScale` | — | Min / Tonic / Max + scale — **all four are glass dropdowns** |
| ☐ | `Chrome/Transport` | playing/stopped | external frame |

### 3.3 Feedback atoms

| ☐ | Component | Use |
|---|-----------|-----|
| ☐ | `Tooltip/Soft` | Hover hints on illustrated controls |
| ☐ | `Badge/Micro` | “÷4”, “↻”, “73%” on control |
| ☐ | `Toast/Quiet` | “Preset saved”, “Randomized layer 2” |
| ☐ | `Progress/Scan` | 0–15 scan position indicator |

### 3.4 Illustration atoms 🎨

| ☐ | Component | Slot purpose | Tech notes |
|---|-----------|--------------|------------|
| ☐ | `Illus/Hero/Base` | Full-window background | PNG/WebP @1x @2x; parallax layer 0 |
| ☐ | `Illus/Hero/Mid` | Mist, hills, water | Optional parallax layer 1 |
| ☐ | `Illus/Hero/Sky` | Stars, moon, aurora | Parallax layer 2; slow drift |
| ☐ | `Illus/Grid/Frame` | Decorative border around 4×4 | 9-slice or fixed inset |
| ☐ | `Illus/Control/Moon` | Master clock metaphor 🎨 | Hotspot map overlay |
| ☐ | `Illus/Control/Tide` | Reverse X/Y 🎨 | Flip animation L↔R |
| ☐ | `Illus/Control/Depth` | Z axis / layers 🎨 | 4 depth planes |
| ☐ | `Illus/Mascot/Idle` | Optional serene creature or orb 🎨 | Breathing loop; **must not block grid** |
| ☐ | `Illus/Empty` | First-run state | “Touch a cell to begin” |

**Illustration handoff rules:**

- Safe zone: **center 420×420** always visible (grid + playhead)
- Hot zones: vector rectangles named `hit/…` in Figma for dev mapping
- Never place essential text inside raster art without duplicate HTML/JUCE label
- Export: SVG for vectors, PNG/WebP for painted layers, Lottie optional for breathe

---

## 4. Molecules (Page 03)

| ☐ | Component | Composed of | States |
|---|-----------|-------------|--------|
| ☐ | `Transport/Bar` | play, reset, dice + sync badge | playing, stopped, synced |
| ☐ | `Pitch/Bar` | quantize, scale▾, min, range, oct | quantize on/off |
| ☐ | `Layer/Tabs` | 4× Toggle/LED + layer name | Z1–Z4 active |
| ☐ | `Grid/4×4` | 16× Cell/* + playhead overlay | per-layer data swap |
| ☐ | `Grid/CellInspector` | note, gate, gate_p, pitch_p, vel_p | slides from right |
| ☐ | `Axis/ClockStrip` | label X/Y/Z + Stepper + speed + ⟲ toggle | reverse forced |
| ☐ | `Axis/ReverseRow` | 3× Toggle + prob micro knob | |
| ☐ | `Movement/Picker` | dropdown + scan progress | free/scan/ping-pong/… |
| ☐ | `Poly/Stack` | 1–4 voice selector | visual stack preview |
| ☐ | `Preset/Bar` | prev, name, next, save | dirty indicator |
| ☐ | `Modal/SheetHeader` | title, close, optional tabs | |
| ☐ | `Prob/Row` | label + Knob/Micro + value % | |
| ☐ | `Random/Scope` | 5× checkbox + amount slider | |

---

## 5. Organisms (Page 04)

| ☐ | Organism | Contains | Illustration tie-in 🎨 |
|---|----------|----------|------------------------|
| ☐ | **Hero/Stage** | Illus/Hero/* + Grid/4×4 + playhead | Grid sits in “clearing” or “pool” |
| ☐ | **Top/CommandRail** | Transport + Pitch/Bar + Preset/Bar | Floating glass bar over hero |
| ☐ | **Bottom/ClockDock** | 3× Axis/ClockStrip + master division | Ripples or orbit rings under grid |
| ☐ | **Side/QuickActions** | icons → Probability, Clocks, Movement, Random | Constellation icon set |
| ☐ | **Modal/Probability** | global prob rows + footer apply | Frosted sheet over hero |
| ☐ | **Modal/Clocks** | divisions, speeds, recipes dropdown | “Tide tables” copy optional |
| ☐ | **Modal/Movement** | mode picker + reverse row + ping-pong | Path preview overlay on grid |
| ☐ | **Modal/Randomize** | scope + amount + dice CTA | Brief particle burst on apply |
| ☐ | **Modal/PresetBrowser** | list + search (Phase B) | defer if not v1 |

---

## 6. Screens & flows (Page 05)

| ☐ | Screen | Key states to frame |
|---|--------|---------------------|
| ☐ | **Main / Playing** | playhead moving, transport active |
| ☐ | **Main / Stopped** | playhead visible, dim idle glow |
| ☐ | **Main / Cell selected** | inspector open, cell Selected |
| ☐ | **Main / Each Z layer** | 4 frames — accent color shift |
| ☐ | **Main / Scan mode** | scan progress visible on grid |
| ☐ | **Modal open** ×4 | Probability, Clocks, Movement, Random |
| ☐ | **Empty / First open** | illustration + one CTA |
| ☐ | **Compact 720×520** | reflow: inspector collapses to bottom sheet |
| ☐ | **Expanded 1100×720** | inspector side-by-side |

**Prototype flows in Figma:**

1. Tap cell → inspector → edit prob → hear change (annotate)
2. Z2 tab → grid content swap + color accent
3. Movement: Free → Scan → ping-pong (show scan overlay)
4. Randomize scopes → apply → toast
5. Reverse X forced → arrow flip on grid edge

---

## 7. Component ↔ engine mapping (tech stack must handle)

Design every component with this binding in mind (JUCE v1 or web UI shell).

| UI component | Engine field | Type | Real-time |
|--------------|--------------|------|-----------|
| Grid cell degree | `layers[z].cells[y][x].degree` | int | on edit |
| Hard gate | `.gate` | bool | gem centre click |
| Octave offset | `.octave_offset` | int | on edit |
| Trigger armed / prob | `.trigger_armed`, `.trigger_prob` | bool, float | on edit |
| Jitter armed / amount | `.jitter_armed`, `.jitter_amount` | bool, float | on edit |
| Overview toggle (top row) | `layers[i].active` | bool | instant |
| Mini-grid array hit box (crystal row scrapped) | `selected_layer` | 0–3 | instant |
| Movement ▾ | `layers[selected].movement` | enum | instant |
| Random skip % | `layers[selected].random_skip_prob` | float | instant |
| Playhead (mini/main) | `layers[playing].path.step_index` | read-only | every tick |
| Tonic / scale | `root`, `mode` | string | glass dropdown; instant; min/max labels recalc |
| Min / Max octave | `min_octave`, `max_octave` | int | glass dropdown; tonic-relative labels |
| Master clock | `master_division` | float | external chrome |
| Play mode | `play_mode` | string | external chrome |
| Preset | `matilda/presets/*.json` v2 | load/save | on action |

**Tech stack checklist (GUI layer):**

| ☐ | Requirement |
|---|-------------|
| ☐ | Fixed aspect ratio window with responsive **compact** breakpoint |
| ☐ | **60 fps** UI; audio/MIDI on separate thread |
| ☐ | **HiDPI** / multi-scale factor (1×, 1.5×, 2×) |
| ✅ | Raster hero (M8b) — masked PNG portrait + starfield; **Rive swap planned** (hair/body + state inputs) |
| ☐ | Vector icons + optional parallax (2–3 layers) |
| ☐ | **Hit testing** on non-rect controls (illustrated moon/knob hotspots) |
| ☐ | Keyboard: arrow keys move selected cell; Space play; R reset |
| ☐ | **Accessibility**: focus ring on every control; min 44 px targets |
| ☐ | Tooltip system (not buried in art alone) |
| ☐ | State sync: UI ↔ `Patch` model ↔ preset JSON |
| ☐ | **Animation** without blocking audio — CSS collapse ✅ · **Rive** hero states (future) · JUCE ComponentAnimator |
| ☐ | Modal = overlay; main grid still visible dimmed beneath |
| ☐ | Preset dirty flag when any param changes |

---

## 8. Illustration production checklist 🎨 (your lane)

| ☐ | Deliverable | Spec |
|---|-------------|------|
| ☐ | Hero illustration — **serene** key visual | 900×620 + 1800×1240 @2x |
| ☐ | Layer accent variants (×4) | Recolor guide, not necessarily 4 full paints |
| ☐ | Playhead glow sprite | 128×128 PNG with alpha |
| ☐ | Gate on/off cell overlays | SVG or 9-slice |
| ☐ | Prob ring graphic | SVG stroke template |
| ☐ | Icon set (12–16 icons) | SVG 24×24 viewBox |
| ☐ | Empty state art | 600×400 |
| ☐ | Optional mascot idle loop | Lottie or sprite sheet; ≤ 2 MB |
| ☐ | **Hotspot map** overlay (Figma) | Named frames: `hit/play`, `hit/grid`, etc. |
| ☐ | Parallax depth guide | Layer 0/1/2 move rates: 0 / 0.15 / 0.3 |

**Serene Dream Eater rule:** illustration carries emotion; **grid + playhead + transport** carry function. If art and UX conflict, UX wins — art adapts.

---

## 9. Asset export & naming (Page 08 — Handoff)

| Asset | Format | Naming |
|-------|--------|--------|
| Icons | SVG | `icon/{name}.svg` |
| Hero layers | PNG/WebP | `illus/hero_{layer}@{1x|2x}.png` |
| Cell states | SVG | `cell/{state}.svg` |
| Fonts | OTF/TTF | `font/{Family}-{Weight}.otf` |
| Lottie | JSON | `motion/{name}.json` |
| Figma components | Dev Mode | Map to `component_id` in code |

☐ Export preset: **SVG stroke expands**, PNG sRGB, compress WebP for hero  
☐ Document safe area insets in px from window edge  
☐ Color tokens exported as JSON for JUCE `LookAndFeel` or CSS vars

---

## 10. Build order (recommended)

**Week 1 — Foundation**  
Tokens → Atoms (buttons, toggles, cell states) → Grid/4×4 molecule

**Week 2 — Main screen**  
Hero stage layout (placeholder rectangles for art 🎨) → Transport + Layer tabs → Clock dock

**Week 3 — Modals**  
Probability → Clocks → Movement → Randomize

**Week 4 — Illustration integration 🎨**  
Drop hero art → hotspot map → motion tokens → compact breakpoint

**Week 5 — Handoff**  
Dev Mode specs · asset zip · component map (Section 7)

---

## 11. v1 scope guard (do not Figma yet)

Defer until Phase B+:

- Independent X/Y/Z clock strips
- Global wobble macro · probability modal
- Polyphony UI (`poly_voices` reserved in schema)
- Mod matrix · Teleiso script · preset browser
- Play on transport (field exists; UI deferred)

---

## 12. Sign-off checklist (before dev)

| ☐ | Criteria |
|---|----------|
| ☐ | Every interactive control has Default/Hover/Active/Disabled/Focus |
| ☐ | All 4 modals framed at default + compact width |
| ☐ | Z1–Z4 accent system applied consistently |
| ☐ | Playhead + scan path readable on top of hero art |
| ☐ | Probability visible at cell + global level |
| ☐ | Clock X/Y/Z visually distinct (not three identical knobs) |
| ☐ | Illustration safe zones documented 🎨 |
| ☐ | Section 7 mapping reviewed with engineer |
| ☐ | Asset naming matches Section 9 |
| ☐ | Plugin window mock feels **serene**, not cluttered |

---

*Matilda · Figma checklist v2 · pairs with `SPEC.md`, `DESIGN.md`, `matilda/`*
