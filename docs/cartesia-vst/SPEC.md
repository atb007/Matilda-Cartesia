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
- **Mini/main playhead sync:** The playhead uses the **same step index** (0…15, row-major: left → right, then next row) on the main 4×4 grid and the **playing** layer’s mini-grid cell (column-major layout in the draped array — index remapped). The mini-grid gem switches to the Figma **on** asset at the playhead step; main grid LED pill lights in parallel. **Gate-off cells** stay dim on both — no mini-grid on-state or LED at that step.
- **Per-layer cell state:** Gate, note, trigger-prob, and jitter settings belong to **one cell index on one layer only**; switching the edited layer shows that layer’s own 16-cell data (no copy across layers).

### Playback — sequential (v1)

```text
For each active layer in order (1 → 2 → 3 → 4):
  Run that layer’s movement through 16 steps (respecting gates)
  Then advance to next active layer
Loop
```

- **Mono output** — one note at a time; **no polyphony** in v1 (schema reserves `poly_voices` for future).
- **v1 implementation order:** Layer 1 engine + UI first, then layers 2–4.
- **Multi-layer playhead (engine — M9, not UI demo):** When layers 2–4 are active, the playhead finishes layer 1’s 16 steps (per that layer’s movement mode), then continues on the next active layer — not a single shared step counter across layers. Forward / reverse / ping-pong / etc. all affect *when* the hand-off happens. The current UI demo only animates the **selected** layer’s grid; full sequencing is owned by the arp engine.

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
  step_index: int             # 0..15
  step_dir: int               # +1 / -1 for ping-pong/pendulum
  cells[4][4]: Cell

Patch:
  title, version
  root, mode, quantize
  min_octave, max_octave      # tonic-relative display
  master_division
  play_mode, play_on_transport
  selected_layer: int         # 0..3 editor focus
  layers[4]: Layer
  seed: optional int
  poly_voices: int            # reserved, default 1
```

Preset JSON: `matilda/presets/*.json`

---

## Phase boundaries

### v1 — Layer 1 + Matilda UI shell

| In | Out |
|----|-----|
| Layer 1 sequential playback | Polyphony |
| 6 movement modes | X/Y/Z independent clock UI |
| Cell gate, trigger prob, jitter, octave offset | Global wobble macro |
| Quantise panel + tonic-relative min/max | Mod matrix |
| Layer overview activate + mini-grid edit selection | Full 4-layer engine (UI ready) |
| Playhead lighting (Cartesia) | AUv3 iOS |
| Preset JSON v2 | Teleiso script |

### v1.1 — All layers sequential

- Layers 2–4 engine + mini grid playheads
- Per-layer movement in dropdown when switching edited layer (mini-grid selection)

### Phase B

- Independent XYZ clock divisions
- Play on transport · MIDI play modes
- Randomize modal · preset browser

### Future UI (post-M8b — not in v1 scope)

- **Rive hero animation** — animated Matilda hair/body via Rive state machine; inputs driven by transport/playback/collapse scenarios. `HeroCanvas` portrait slot stays swappable (static PNG today). See `MILESTONES.md` §Future enhancements.

---

## Code map

| Path | Role |
|------|------|
| `matilda/` | Product root, assets, presets, JUCE plugin target |
| `matilda/plugin/` | JUCE sources (Phase 0) |
| `cartesia/model.py` | v2 Patch schema |
| `cartesia/engine.py` | Stepping engine (migration in progress) |
| `live_cartesia.py` | Python MIDI prototype CLI |
| `cartesia-vst-ui/` | React + TS UI prototype — **M1–M8b complete** |
| `cartesia-vst-ui/src/components/MatildaPluginFrame.tsx` | Full window: hero + collapse + shell |
| `cartesia-vst-ui/src/components/MatildaShell.tsx` | Control cluster assembly (M8) |
| `cartesia-vst-ui/src/components/HeroCanvas.tsx` | Starfield, portrait, wordmark (Rive-ready slot) |
| `cartesia-vst-ui/src/heroLayout.ts` | Expand/collapse layout constants |
| `gridwalker/` | Legacy sandbox — reference only |

---

## Success criteria (v1)

1. Layer 1 Forward walks 16 cells top-left → bottom-right; playhead lights match Cartesia.
2. Hard gate off → dim gem; mini grid dot does not light on that step.
3. Orange ▲ at 50% thins triggers without breaking step clock.
4. Green ● wobble audible; octave offset shifts base pitch (C3 + 2 oct → C5).
5. Tonic change updates Min/Max labels; degrees unchanged.
6. Preset save/load restores grid + movement + layer active flags.

---

*Spec v2 · aligned with Figma Matilda · UI shell M8b complete Jun 2026*
