# Matilda — UI & interaction design

**Figma file:** [AdMaker-CMS](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS)

| Frame | Link |
|-------|------|
| Main UI (`opt3`) | [4919-97886](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886) |
| Layer + grid behaviour | [4922-103830](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830) |
| Polyphony crown (`glowPolyphony`) | [5171-102837](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5171-102837) |

Product spec: [SPEC.md](./SPEC.md) · Build log: [MILESTONES.md](./MILESTONES.md)

**UI prototype:** `cartesia-vst-ui/` — M1–M8b shipped · entry `MatildaPluginFrame`

---

## Screen regions

```text
┌─ External chrome (Figma: Frame 2147223736) ─────────────────────┐
│  [Play/Pause]   Clock 1/16 ▾   Play mode Note ▾                 │
└─────────────────────────────────────────────────────────────────┘
┌─ Left: Quantise Scale ──┬─ Right: Sequencer ─────────────────────┐
│  Min — Tonic — Max      │  TOP: Layer overview (mini-grid)        │
│  [crystal art]          │    · 4× mini 4×4 path maps              │
│  Lydian ◀ ▶             │    · top gems = ACTIVATE layer          │
│                         │    · crown = polyphony (2+ layers)      │
│                         │    · right-click layer = copy/paste     │
│                         │  MID: Movement: Forward ◀ ▶             │
│                         │  BOT: 4×4 Cell Anatomy grid             │
│                         │       + vine step-count scroll          │
└─────────────────────────┴───────────────────────────────────────┘
```

**Rule:** Layer **activation** only in **top-right overview**. Never on the quantise panel.

---

## Hero canvas + collapse (M8b)

| Element | Behaviour |
|---------|-----------|
| **Background** | Starfield + aurora/forest (`hero-bg-m8b.png`) — visible through shell glass |
| **Portrait** | macOS Metal overlay / Windows D3D offscreen→`juce::Image` Rive (`matilda-cartesia-v3.riv`); static PNG until first frame. Masked; slides left on collapse |
| **Wordmark** | “Matilda” + `Cartesia - v{VERSION}` (Jacquard 24) — slides with portrait; on Metal, drawn **above** the Rive layer (see below) |
| **Chevron** | **70×70** glass button — expanded `>>` at hero top-left; collapsed `<<` inside vines frame |
| **Collapse** | Canvas **2376 → 1515** px width; shell re-centres; **380 ms** ease |
| **Metal strips** | Removed (Figma tweak) |
| **Shell glass (standalone)** | Frosted bedding: blurred wallpaper under translucent glass (`ShellChrome`); shell `opaque = false` |

**Rive hero (shipped — macOS Metal / Windows D3D):** Live portrait via native Rive runtime. macOS presents into a Metal layer; Windows renders D3D11 PLS offscreen and blits into `juce::Image` (no child HWND swap chain — that path stayed on the static PNG in hosts). Bindings (play + layer-count glows) in `matilda/standalone/docs/RIVE_ANIMATION_RULEBOOK.md`. **Wordmark compositing (Metal):** rasterize labels into a `CALayer` on the same NSView as `CAMetalLayer`, stacked above Rive — peer `toFront()` cannot win over GPU `resizeViewToFit` re-attach.

---

## Layer behaviour

### Activate (top-right overview)

- Tap layer crystal → include/exclude from playback (sequential queue, or simultaneous set when polyphony is on).
- Mini grid: full color + path preview when active; grey when inactive.
- Default: **Layer 1 on**, others off. Layer 1 cannot be deactivated.

### Select (mini-grid body)

- Tap an **active** layer’s cell array → switch **main 4×4**, **movement**, and **step scroll** to that layer.
- Does not stop playback; user can edit layer 3 while layer 1 plays.

### Polyphony crown (standalone)

- Sits in the housing diamond above the mini-grid ([Figma glowPolyphony](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5171-102837)).
- **Idle (poly off, ≥2 layers):** white frontGlow + soft bgGlow pulse only (no opaque union gem).
- **On:** morphing colour bloom (bgGlow + frontGlow).
- **≤1 layer:** crown fades out. Click toggles `patch.polyphony`.
- Files: `PolyphonyCrown.cpp/h`, layout in `ReactShellLayout.h`.

### Copy / paste / reset (standalone — mini-grid right-click) — frozen Jul 15, 2026

- Compact glass menu at the click point: Copy notes · Copy notes & knobs · Paste notes · Paste notes & knobs · Reset · Undo.
- Copy from **active** layers only; paste onto active **or** inactive (paste activates). Reset on **active** only.
- Clipboard = visible `active_step_count` cells; paste overrides those steps + syncs step count.
- **Reset:** all 16 cells **gate on**, degree 0 (lowest), knobs disarmed — not gated-off empties.
- Floater: Copied / Pasted / Reset / Undo. Files: `LayerClipboard.h`, `LayerOverview.cpp`.

### Presets bar (standalone) — frozen Jul 15, 2026

- Above shell glass, left column: Asimovian “Presets” title + glass name dropdown + save ([Figma PresetModule](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5193-102814)).
- Dirty `*` on name after edits; Save = native dialog every time; dropdown max 10 visible rows (scroll).
- Files: `PresetBar`, `PresetLibrary`, layout in `ReactShellLayout.h`.

### Step count vine (standalone) — frozen Jul 15, 2026

- Under the main 4×4: drag/click/wheel diamond on vine track (`StepScroll`). Per-layer `active_step_count` 1…16; snap at 4/8/12/16.
- Shortens engine loop for that layer; out-of-range cells hidden on grid + mini-grid.

### Playback — frozen Jul 15, 2026

- **Sequential (poly off):** finish layer N’s path (`active_step_count` steps) → next active layer → loop.
- **Polyphony on:** all active layers tick together; mini-grid shows all playheads; main 4×4 = selected layer only.
- **Playhead:** lights the **cell that just sounded** (last fired step), not the next index after advance.

---

## Cell anatomy (`Cell Anatomy States`)

```text
      [C3]              ← resolved note (scale degree → name)
   ▲                   ← trigger probability (orange)
   ●                   ← jitter amount (green)
    ( gem )             ← dial / pitch; centre click = hard gate
   +2 Oct               ← octave offset (NOT jitter)
```

### Icon states (Figma variants)

| State | Orange ▲ / Green ● |
|-------|---------------------|
| **Idle** | Hidden |
| **Hover** | Fade in |
| **Armed** | Visible + ring (latched); default 50% on first click |
| **Adjust** | Drag ↕; tooltip `%` above/below |

### Hard gate off

- Figma asset: **dimmed gem** (`Inactive Cell`).
- Mini overview: dot **does not light** for that step.
- Engine may still advance index (Cartesia visits gated-off steps); no MIDI, no visual flash.

---

## Movement dropdown

Figma preset list (`4919:99373`):

| Label | Engine id |
|-------|-----------|
| forward | `forward` |
| reverse | `reverse` |
| ping-pong | `ping_pong` |
| pendulum | `pendulum` |
| random | `random` |
| random skip | `random_skip` |

**Ping-pong vs pendulum:** end cell **twice** vs **once** before direction flip (see SPEC).

**Random skip:** forward index order; skip each index with probability `random_skip_prob`.

Mini grid should **preview path shape** per mode (animation in Figma prototype).

---

## Quantise scale panel

- **Min / Tonic / Max** — glass dropdowns (same material as Scale list); not steppers.
- **Min / Max** octave labels follow **tonic** (recalc on tonic change).
- **Scale** — `◄ name ►` bar + glass dropdown: Major, Minor, Pentatonic, Lydian, Phrygian, …
- Cell gems show **note name** from degree within active scale.

---

## External chrome

Documented in Figma node `4991:4644` (“Global Settings”):

| Control | Purpose |
|---------|---------|
| **playPause** | Start/stop sequencer |
| **Clock** | Master step rate (`master_division`) — glass dropdown: 1/4 … 1/32 |
| **Play mode** | Glass dropdown — **Transport** (default in Figma) · **Note** (v1 held-note root) |

Attach full-frame mockups with chrome in handoff zip.

---

## Motion tokens

| Token | Use |
|-------|-----|
| `motion/playhead` | 120 ms ease-out — cell highlight move |
| `motion/layer-switch` | 180 ms — grid crossfade on crystal select |
| `motion/glow-pulse` | 800 ms — gate fire on playhead visit |
| `motion/icon-arm` | 100 ms — ring appear on ▲/● latch |
| `motion/collapse` | 380 ms — canvas width, hero slide, shell reposition |
| `motion/rive-hero` | Native Rive v3 — idle ↔ playing + layer glows (Metal/D3D) |

---

## Accessibility

- Icon hit targets ≥ 24 px (44 px preferred).
- Focus ring on gems and crystals.
- Tooltip on hover for `%` and movement mode one-liner.

---

*Design doc v1 · pairs with FIGMA-CHECKLIST.md · M8b Jun 2026 · standalone freeze Jul 15, 2026*
