# Matilda — UI & interaction design

**Figma file:** [AdMaker-CMS](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS)

| Frame | Link |
|-------|------|
| Main UI (`opt3`) | [4919-97886](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886) |
| Layer + grid behaviour | [4922-103830](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830) |

Product spec: [SPEC.md](./SPEC.md) · Build log: [MILESTONES.md](./MILESTONES.md)

**UI prototype:** `cartesia-vst-ui/` — M1–M8b shipped · entry `MatildaPluginFrame`

---

## Screen regions

```text
┌─ External chrome (Figma: Frame 2147223736) ─────────────────────┐
│  [Play/Pause]   Clock 1/16 ▾   Play mode Note ▾                 │
└─────────────────────────────────────────────────────────────────┘
┌─ Left: Quantise Scale ──┬─ Right: Sequencer ─────────────────────┐
│  Min — Tonic — Max      │  TOP: Layer overview (top cell)         │
│  [crystal art]          │    · 4× mini 4×4 path maps              │
│  Lydian ◀ ▶             │    · crystals = ACTIVATE layer        │
│                         │  MID: Crystal row (All four activated)│
│                         │    · only active layers shown           │
│                         │    · SELECT layer to edit               │
│                         │  Movement: Forward ◀ ▶                │
│                         │  BOT: 4×4 Cell Anatomy grid           │
└─────────────────────────┴───────────────────────────────────────┘
```

**Rule:** Layer **activation** only in **top-right overview**. Never on the quantise panel.

---

## Hero canvas + collapse (M8b)

| Element | Behaviour |
|---------|-----------|
| **Background** | Starfield + aurora/forest (`hero-bg-m8b.png`) — visible through shell glass |
| **Portrait** | Elliptical SVG mask over `matilda-portrait-v2.png`; slides left on collapse |
| **Wordmark** | “Matilda” + “Cartesia - v1.0” — slides with portrait |
| **Chevron** | **70×70** glass button — expanded `>>` at hero top-left; collapsed `<<` inside vines frame |
| **Collapse** | Canvas **2376 → 1515** px width; shell re-centres; **380 ms** ease |
| **Metal strips** | Removed (Figma tweak) |

**Future — Rive hero:** Replace or augment static portrait with a Rive `.riv` (hair/body motion). State machine inputs should reflect UI/engine scenarios — e.g. idle breathing, playing pulse, transport-locked stillness. Keep mask bounds + collapse slide compatible; see `MILESTONES.md` §Future enhancements.

---

## Layer behaviour

### Activate (top-right overview)

- Tap layer crystal → include/exclude from **sequential playback queue**.
- Mini grid: full color + path preview when active; grey when inactive.
- Default: **Layer 1 on**, others off.

### Select (bottom crystal row)

- Crystals appear **only for activated layers** (populated in real time).
- Tap → switch **main 4×4** and **movement dropdown** to that layer.
- Does not stop playback; user can edit layer 3 while layer 1 plays.

### Playback

- **Sequential:** finish layer 1’s 16-step path → layer 2 → … → loop active layers.
- **Playhead:** lit on **playing** layer’s mini grid; main grid playhead when `selected_layer == playing_layer`.

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
| `motion/rive-hero` | (future) Rive timeline / state blend — idle ↔ playing |

---

## Accessibility

- Icon hit targets ≥ 24 px (44 px preferred).
- Focus ring on gems and crystals.
- Tooltip on hover for `%` and movement mode one-liner.

---

*Design doc v1 · pairs with FIGMA-CHECKLIST.md · M8b Jun 2026*
