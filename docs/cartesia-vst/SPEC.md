# Matilda (Cartesia VST) — Product Spec v2

**Product name:** Matilda · Cartesia v1.0  
**Engine reference:** [CV funk Cartesia](https://github.com/codygeary/CVfunk-Modules/blob/main/src/Cartesia.cpp)  
**Figma:** [main UI](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886) · [layer/grid behaviour](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830)  
**UI build progress:** `MILESTONES.md` — module-by-module · **M1–M8b complete** · **M9 in progress** (engine link) · Figma `5002:6446` / `5002:6447`

---

## Product promise

Open the plugin → see a **4×4 gem grid** and layer path preview → hear MIDI in seconds.

Not a piano roll. A **layered step field** with movement modes, per-cell probability, jitter, and scale-aware pitch.

---

## Layout (from Figma)

**Control shell** — `4976:4727` · native **`1405 × 1765` px** (scale in UI via `MatildaShell` `scale` prop; 50% ≈ `703 × 883`).

**Full plugin window** (M8b) — `MatildaPluginFrame` composes hero canvas + collapsible control shell.

| State | Canvas | Shell position | Hero |
|-------|--------|----------------|------|
| **Expanded** (default) | **2376 × 1805** | `x=886`, `y=50` | Portrait + wordmark visible (left) |
| **Collapsed** | **1515 × 1805** | `x=85`, `y=50` (centred in MainFrame) | Portrait slides off (`−660 px`) |

Preview scale **0.52 × uiScaleFactor** (default factor **0.9** → ~1112×845 px expanded). User scale **0.7…1.0** via corner/edge drag (`UiScale.h` / `uiScale.ts`). Chevron toggle **70 × 70** px — `@2x` PNG pair (`collapse-toggle-expanded@2x.png` / `collapsed`); expanded: top-left hero `(83, 17)` · collapsed: inside vines frame `(95, 67)`.

**VST3 / DAW hosts (FL Studio, etc.):** Editor declares `setResizeLimits` (collapsed min × 0.7 … expanded max × user scale). If the host window is wider than content, `HeroBackdropDrawing` aspect-covers the starfield so no black/teal void appears. Module title filigree stays in design-space shell layout (not stretched by host). Debug footer (BPM / sync / status) is **Standalone-only**.

```text
┌─ Expanded (2376 × 1805) ────────────────────────────────────────────────┐
│ [≫]  [Hero — starfield · Matilda portrait · wordmark]                   │
│       ┌──────────────┬────────────────────────────────────────────┐   │
│       │ Quantise     │  Layer overview · Movement ▾ · 4×4 grid     │   │
│       │ Scale        │                                              │   │
│       │ Global       │                                              │   │
│       │ Settings     │                                              │   │
│       └──────────────┴────────────────────────────────────────────┘   │
│       [CSS glass bedding → vines frame overlay → controls — M8]         │
└─────────────────────────────────────────────────────────────────────────┘
```

No film-strip metal bars (removed per Figma tweak). Target shipping window sizes in `FIGMA-CHECKLIST.md` (900×620 default; scale down from design master).

| Zone | Role |
|------|------|
| **Left** | Quantise scale — Min / Tonic / Max (tonic-relative), scale list |
| **Top-right** | Layer overview — mini grids + toggles; **activate/deactivate** layers (top row) AND **select** layer to edit (click an activated array's hit box) |
| **Movement dropdown** | Path mode for **selected** layer |
| **Main grid** | 16 cells for **selected** layer |
| **External chrome** | Transport, master clock division, play mode (Note / future MIDI) |

---

## Layers

### Activation (top-right overview only)

- Click layer toggle cell (top row of **layer overview**) → toggle **active** in playback sequence.
- **Layer 1 is always active** — its toggle cannot deactivate it; layers 2–4 may be toggled on/off.
- **Default:** Layer 1 active; layers 2–4 inactive.
- Inactive layers: mini grid greyed; no audio.

### Editing (mini-grid selection — crystal row scrapped Jun 10, 2026)

- Each layer's cell array in the **layer overview** has a bounding hit box.
- Click an **activated** layer's array → **selected layer**; main 4×4 shows that layer's cells. Hit boxes are invisible (no outline stroke) but keep the same click target.
- Inactive arrays are not selectable.
- **Edit while playing:** selected grid can differ from currently **playing** layer; playhead shows on **playing** layer’s mini grid (+ main grid when selected = playing).
- **Mini/main playhead sync:** Playhead UI (mini-grid + main 4×4 LED) shows the **last fired** step — the same index used for MIDI emission (`SequencerEngine::lastStepIndex` / poly `lastTickResults`). Do **not** light from live `layer.stepIndex` after `advancePath` (that is the *next* cell and causes one-ahead lights + handoff glitches). Index space is 0…15 row-major on the main grid; mini-grid remaps to column-major for the draped array. **Gate-off cells** stay dim on both — no mini-grid on-state or LED at that step. Sequential pass: cells **1→N** then next active layer from **1**. Play/resume after stop: `engine_.reset()` then first tick fires cell 1.
- **Per-layer cell state:** Gate, note, trigger-prob, and jitter settings belong to **one cell index on one layer only**; switching the edited layer shows that layer’s own cell data.
- **Copy / paste / reset (standalone — mini-grid only):** Right-click a layer body in the **layer overview** for a glass context menu. Copy/paste transfers **visible steps** only (`active_step_count`, 1…16), including gate-off cells. Modes: notes only · notes + mini-knobs. Paste overrides those steps on the target, sets the target’s step count to the clipboard length, and **activates** an inactive target. Reset (active layers only) clears all 16 cells (**gates on**, degree 0 / note defaults, mini-knobs disarmed). In-memory undo stack for paste/reset. Floater feedback: Copied / Pasted / Reset / Undo. Does **not** copy movement mode.

### Playback — sequential (default) vs polyphony

```text
Polyphony OFF (default):
  For each active layer in order (1 → 2 → 3 → 4):
    Run that layer’s movement through its active_step_count steps (respecting gates)
    Then advance to next active layer
  Loop

Polyphony ON (standalone — crown toggle):
  All active layers advance and may fire on the same clock tick
  Mini-grid shows per-layer playheads; main 4×4 playhead = selected layer only
```

- **Default:** polyphony **off**; persisted as `polyphony` in patch JSON.
- **Crown UI:** discoverable when **≥ 2** layers active; idle = soft white frontGlow pulse; on = morphing hue bloom (union metallic gem omitted — glow-only). Dropping to 1 active layer fades the crown out. Figma: `glowPolyphony` ([5171:102837](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5171-102837)).
- **v1 implementation order:** Layer 1 engine + UI first, then layers 2–4 (done in JUCE).
- **Multi-layer playhead:** Sequential mode hands off after each layer’s path; poly mode ticks all active layers together. Main grid always shows the **selected** layer.

### Per-layer step count (standalone)

- Each layer has `active_step_count` (1…16, default 16). Engine loop / path length uses this count (supports polyrhythms across layers).
- **UI:** vine / diamond **step scroll** under the main 4×4 (`StepScroll`). Snap points at 4 / 8 / 12 / 16; smooth between. Out-of-range cells hidden on main grid + mini-grid.

---

## Movement modes (per layer)

Step index `0…15` maps to `(x, y)` row-major: top-left → right → next row.

| Mode | ID | Behaviour |
|------|-----|-----------|
| **Forward** | `forward` | 0 → 15, loop |
| **Reverse** | `reverse` | 15 → 0, loop |
| **Ping-pong** | `ping_pong` | 0 → 15; **end cell twice**; 15 → 0; start cell twice; repeat |
| **Pendulum** | `pendulum` | 0 → 15; **end cell once**; reverse; repeat |
| **Random** | `random` | Permutation of 0…15; **every cell visited once** per cycle; reshuffle each cycle |
| **Random skip** | `random_skip` | Forward order; each step index **probabilistically skipped** (`random_skip_prob`) |

Phase B (deferred UI): independent X/Y/Z clock divisions (Cartesia-style cartesian crawl).

---

## Cell model

Each cell stores **scale degree** (not absolute MIDI); display resolves via tonic + scale + octave offset.

| Field | Type | UI | Behaviour |
|-------|------|-----|-----------|
| `degree` | int | Gem dial + note label | Index into quantised scale |
| `gate` | bool | **Click gem centre** | Hard gate; off = dim gem, skipped in mini grid lights |
| `velocity` | int | Inspector / future | Note velocity |
| `octave_offset` | int | `+ N Oct` below gem | Transpose triggered note by N octaves before jitter |
| `trigger_armed` | bool | Orange ▲ (hover → latch) | Off = 100% trigger when gate on |
| `trigger_prob` | float 0–1 | Ring around ▲ | Play chance when armed; default **0.5** on arm |
| `jitter_armed` | bool | Green ● (hover → latch) | Off = no pitch wobble |
| `jitter_amount` | float 0–1 | Ring around ● | Wobble strength when armed; default **0.5** on arm |

### Cell interactions

1. **Hover cell** → show orange ▲ and green ● (hidden when idle).
2. **Click icon** → arm modifier, show ring at 50%, icon stays visible.
3. **Drag icon ↕** → adjust probability / jitter amount; tooltip shows exact %.
4. **Click gem centre** → toggle hard gate.
5. **Drag gem ↕ or scroll wheel** → step through **all quantised pitches** in the Min…Max window sequentially (0% = Min tonic, 100% = highest in-scale note; **no wrap** at ends).
6. **Octave label** → drag or step ± octaves (hide when 0).

### Note pipeline

```text
quantised_set = all in-scale MIDI notes from Min tonic … Max (ascending)
knob index    = position in quantised_set (0 … N-1)
resolve       = quantised_set[knob index]
if gate && roll(trigger_prob): emit(resolve + jitter in scale-degree space)
```

**Octave mapping (JUCE engine):** UI label `C#4` → MIDI `(minOctave + 1) * 12 + pitchClass`. Scale degrees use **octave carry** when `root + scaleInterval` crosses 12 (e.g. B# → C at octave boundary).

Jitter applies within the quantised set when quantize is on.

---

## Playhead lighting (Cartesia parity)

From [Cartesia.cpp](https://github.com/codygeary/CVfunk-Modules/blob/main/src/Cartesia.cpp) widget `step()`:

| Element | Brightness / state |
|---------|---------------------|
| **Playhead cell** (mini + main when selected=playing) | 100%; **same step index** on both grids |
| **Other cells on layer** | ~12% dim field |
| **Gate off cell** | Dim gem asset; **no light** on mini grid when visited |
| **Stage / playhead ring** | 50% secondary hint on main gem |
| **Cell LED pill** | Glows in layer color while the playhead/note passes over that cell; gray otherwise; gate-off cells never light |

---

## Quantise scale (left panel)

| Control | Behaviour |
|---------|-----------|
| **Min** | Lowest octave — **glass dropdown**; label is tonic-relative (e.g. C1 → G1 if tonic G) |
| **Tonic** | Root note — **glass dropdown** (12 pitch classes) |
| **Max** | Highest octave — **glass dropdown**; label tonic-relative (e.g. C9) |
| **Scale** | Chromatic, Major, Minor, Pentatonic, Lydian, Phrygian, … — glass dropdown (`4918:101473`); gem orb image per scale id |

**Tonic change:** Min/Max **labels recalc**; cells **re-snap** to nearest quantised pitch when scale window changes.

**Scale / Min / Max change (engine — M9):** Changing scale, tonic, or octave window **re-snaps every cell** to the nearest pitch in the new quantised set. Knob 0% always matches the **Min** picker label (e.g. C#4). Only in-scale note names appear above gems.

Scales stored as mode id; engine maps degrees to pitch classes.

---

## External chrome (outside main panel)

| Control | Field | Notes |
|---------|-------|-------|
| Play / pause | transport | Sync to DAW |
| Clock | `master_division` | e.g. 1/16 |
| Play mode | `play_mode` | Glass dropdown — **Transport** (default) · **Note** (v1 held-note root) |
| Play on transport | `play_on_transport` | **Deferred** — auto-start with DAW |

---

## Data model (v2)

```python
Cell:
  degree, gate, velocity
  octave_offset: int          # -3..+3 typical
  trigger_armed: bool
  trigger_prob: float         # 0..1, default 0.5 when armed
  jitter_armed: bool
  jitter_amount: float        # 0..1

Layer:
  active: bool
  movement: MovementMode
  random_skip_prob: float     # for random_skip mode
  active_step_count: int      # 1..16 pattern length (standalone UI: step scroll)
  step_index: int             # 0..active_step_count-1 (runtime)
  step_dir: int               # +1 / -1 for ping-pong/pendulum
  cells[4][4]: Cell

Patch:
  title, version
  root, mode, quantize
  min_octave, max_octave      # tonic-relative display
  master_division
  play_mode, play_on_transport
  selected_layer: int         # 0..3 editor focus
  polyphony: bool             # standalone: simultaneous active-layer ticks (default false)
  layers[4]: Layer
  seed: optional int
  poly_voices: int            # reserved legacy field; prefer `polyphony` bool for layer simultaneity
```

Preset JSON: `matilda/presets/*.json`

---

## Phase boundaries

### v1 — Layer 1 + Matilda UI shell

| In | Out |
|----|-----|
| Layer 1 sequential playback | X/Y/Z independent clock UI |
| 6 movement modes | Global wobble macro |
| Cell gate, trigger prob, jitter, octave offset | Mod matrix |
| Quantise panel + tonic-relative min/max | Teleiso script |
| Layer overview activate + mini-grid edit selection | AUv3 iOS |
| Playhead lighting (Cartesia) | |
| Preset JSON v2 | |

### v1.1 — All layers + standalone enhancements (Jul 2026)

| In | Notes |
|----|-------|
| Layers 2–4 sequential engine + mini playheads | Shipped in JUCE |
| Per-layer movement when switching edited layer | Mini-grid selection |
| **Polyphony** (simultaneous active layers) | Standalone crown toggle; patch `polyphony` |
| **Per-layer step count** | Vine step scroll; `active_step_count` |
| **Layer copy / paste / reset / undo** | Mini-grid right-click only |
| Frosted shell glass + glow-only polyphony crown | Standalone UI |

### Phase B

- Independent XYZ clock divisions
- Play on transport · MIDI play modes
- Randomize modal
- Port standalone-only UI (poly crown, step scroll, layer clipboard, presets) into `matilda/plugin/` when ready

### Standalone presets (Jul 2026)

- **UI:** Figma `PresetModule` ([5193:102814](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5193-102814)) — name dropdown + save; dirty `*` suffix.
- **Library:** `~/Library/Application Support/IdeasLab/Matilda/presets/` (macOS) / `%AppData%\IdeasLab\Matilda\presets\` (Windows); seed `Init.json` on first launch.
  - **Note:** this JUCE tree’s `userApplicationDataDirectory` is `~/Library` on macOS (not Application Support). `PresetLibrary` therefore uses an explicit Application Support path, and one-time-migrates any older files from `~/Library/IdeasLab/Matilda/presets/`.
- **Save:** native Save dialog every time (unlimited files). Chosen basename is always written into the library folder (dropdown stays in sync even if the panel saves a copy elsewhere). Max **10** visible rows (scroll for more).
- **Load:** full patch replace; **BPM + transport play/stop preserved**.

### Mini-grid RESET VALUES

- Clears the layer’s cells to **gate on**, **degree 0** (lowest), default velocity/knob fields — not gated-off empties.

---

## Windows VST3 port status (post–v1.0.11)

Shipped / polished in **`matilda/standalone/`** and ported into **`matilda/plugin/`** on Jul 16, 2026. **Windows Rive hero validated Aug 1–2, 2026** on standalone (**v1.0.15–v1.0.17**); same D3D path ships in Windows VST3.

| # | Enhancement | Standalone status | Windows VST3 notes |
|---|-------------|-------------------|--------------------|
| 1 | **Rive hero v3** (`matilda-cartesia-v3.riv`) | ✅ **validated v1.0.17** | D3D11 PLS offscreen→`juce::Image` (no HWND swap chain); mask-right framing |
| 2 | **Wordmark above GPU** (`CALayer` / peer-safe) | ✅ validated | Metal: `CALayer` on host; Windows: JUCE sibling above painted frame |
| 3 | **`faceStreakVis` ← polyphony ∧ ≥2 layers + transport** | ✅ source port | Uses `setPolyphony` + layer count; off with a single active layer |
| 4 | **Polyphony crown + engine** | ✅ source port | `PolyphonyCrown`, `PatchState::polyphony`, poly tick path |
| 5 | **Per-layer step scroll** | ✅ source port | `StepScroll` + `active_step_count` |
| 6 | **Layer clipboard menu** (copy/paste/reset/undo) | ✅ source port | Mini-grid right-click; reset = gate-on / degree 0 |
| 7 | **Frosted shell glass** | ✅ source port | `ShellChrome` frost; shell is non-opaque |
| 8 | **Presets bar + user library** | ✅ source port | `PresetBar` / `PresetLibrary` (AppData presets folder + Init seed) |
| 9 | **Playhead UI = last fired step** | ✅ source port | Uses `lastStepIndex` / tick results — not post-advance `currentStepIndex` |

**Already in v1.0.11 Windows VST3:** glass dropdown polish, FL Fruity Wrapper instrument mode, DAW sync / MIDI out (as of that release). Do not re-port those unless regressions appear.

**Standalone freeze:** Jul 15, 2026 — see [MILESTONES — Standalone freeze](./MILESTONES.md#standalone-freeze--jul-15-2026).

### Shipped UI (hero)

- **Rive hero** — macOS Metal overlay / Windows D3D offscreen→image (validated **v1.0.17**); play + layer-count view-model bindings. Static PNG until first visible frame. Windows drawable extends to hero-mask right (Metal framing parity). See `matilda/standalone/docs/RIVE_ANIMATION_RULEBOOK.md` and [MILESTONES — Windows Rive validated](./MILESTONES.md#-windows-rive-hero-validated--aug-12-2026).
- **Wordmark above Rive (Metal)** — `CALayer` on the Metal host above `CAMetalLayer` (`RiveHeroMetalView::setWordmarkOverlay`). Version label: `Cartesia - v` + `JucePlugin_VersionString`. Windows wordmark is a JUCE sibling.

---

## Code map

| Path | Role |
|------|------|
| `matilda/` | Product root, assets, presets, JUCE plugin target |
| `matilda/plugin/` | JUCE VST3 + AU |
| `matilda/standalone/` | JUCE Standalone (ahead on Jul 2026 UI) |
| `cartesia/model.py` | v2 Patch schema |
| `cartesia/engine.py` | Stepping engine (migration in progress) |
| `live_cartesia.py` | Python MIDI prototype CLI |
| `cartesia-vst-ui/` | React + TS UI prototype — **M1–M8b complete** |
| `cartesia-vst-ui/src/components/MatildaPluginFrame.tsx` | Full window: hero + collapse + shell |
| `cartesia-vst-ui/src/components/MatildaShell.tsx` | Control cluster assembly (M8) |
| `cartesia-vst-ui/src/components/HeroCanvas.tsx` | Starfield, portrait, wordmark (web / design ref) |
| `matilda/*/Source/Components/HeroCanvas.cpp` | Native hero + Rive load |
| `matilda/*/Source/Rive/RiveHeroMetalView.mm` | Metal host + wordmark `CALayer` |
| `matilda/*/Source/Components/HeroWordmarkDrawing.h` | Shared Jacquard wordmark paint |
| `cartesia-vst-ui/src/heroLayout.ts` | Expand/collapse layout constants |
| `gridwalker/` | Legacy sandbox — reference only |

---

## Success criteria (v1)

1. Layer 1 Forward walks 16 cells top-left → bottom-right; playhead lights match the sounding cell (last fired step).
2. Hard gate off → dim gem; mini grid does not light on that step.
3. Orange ▲ at 50% thins triggers without breaking step clock.
4. Green ● wobble audible; octave offset shifts base pitch (C3 + 2 oct → C5).
5. Tonic change updates Min/Max labels; degrees unchanged.
6. Preset save/load restores grid + movement + layer active flags (standalone: BPM/transport preserved on load).
7. Sequential handoff: finish layer N at last visible step, next layer lights/sounds from step 1.
8. Stop then play: clean restart from cell 1 (audio + UI).

---

*Spec v2 · aligned with Figma Matilda · UI shell M8b Jun 2026 · standalone freeze Jul 15, 2026*
