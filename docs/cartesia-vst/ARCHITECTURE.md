# Matilda — Architecture

**Product:** Matilda (Cartesia VST v1.0)  
**Spec:** [SPEC.md](./SPEC.md) · **UI:** [DESIGN.md](./DESIGN.md)

---

## Repository map

```text
Ideas/
├── matilda/                    # Product root
│   ├── README.md
│   ├── assets/
│   │   ├── figma/README.md     # Figma node IDs + export rules
│   │   └── ui/                 # Exported SVG/PNG from design
│   ├── presets/                # JSON v2 patches
│   └── plugin/                 # JUCE VST3/AU/Standalone (Phase 0)
├── cartesia/                   # Python engine + isobar bridge
│   ├── model.py                # Patch v2 schema
│   ├── engine.py               # Step sequencer logic
│   ├── movement.py             # Path modes (forward … random_skip)
│   └── runner.py
├── live_cartesia.py            # CLI → IAC MIDI
├── examples/cartesia/          # Legacy v1 presets
├── cartesia-vst-ui/            # React + TS UI prototype (M1–M8b ✅)
│   └── src/components/         # MatildaPluginFrame · MatildaShell · modules
├── docs/cartesia-vst/          # Spec, design, Figma checklist
└── gridwalker/                 # Legacy JUCE sandbox (superseded)
```

---

## UI prototype (`cartesia-vst-ui`) — shipped M8b

```text
MatildaPluginFrame (0.52 × uiScaleFactor; default factor 0.9)
├── HeroCanvas          starfield (full-bleed) · masked portrait · wordmark · slide on collapse
├── CollapseToggle      70×70 · collapse-toggle-{expanded|collapsed}@2x.png
├── UiResizeGrips       8 grips — corners + edges (0.7…1.0 user scale)
└── MatildaShell        glass → vines overlay → M1–M7 controls
```

| Layout file | Role |
|-------------|------|
| `heroLayout.ts` | Expanded/collapsed canvas sizes, shell positions, chevron coords |
| `shellLayout.ts` | Control frame + glass rect (1405×1766 vines, 1205×1407 glass) |
| `rasterImageStyle.ts` | Bilinear-friendly PNG overlay CSS |

**Hero portrait:** static masked PNG today. Architecture keeps the portrait region as a **swappable surface** for a future **Rive** runtime (hair/body animation + state inputs for idle/playing/collapsed). Do not flatten hero into a single baked asset in engine code.

---

## Runtime layers

```text
┌─────────────────────────────────────────┐
│  JUCE UI (matilda/plugin)               │
│  Grid · Layer overview · Scale panel    │
│  (+ hero canvas port from cartesia-vst-ui) │
└─────────────────┬───────────────────────┘
                  │ Patch v2
┌─────────────────▼───────────────────────┐
│  SequencerEngine                        │
│  · master clock (DAW BPM × division)    │
│  · sequential layer scheduler           │
│  · MovementPath per layer               │
│  · Cell trigger + jitter + MIDI out     │
└─────────────────┬───────────────────────┘
                  │ MIDI
┌─────────────────▼───────────────────────┐
│  Host / IAC → instrument                │
└─────────────────────────────────────────┘
```

### Host deployment modes (Jun 2026)

Matilda ships three binaries from one codebase: **Standalone**, **VST3**, **AU**. Host behaviour is not uniform — see [MILESTONES.md — Host / transport integration](./MILESTONES.md#host--transport-integration-matildaplugin--jun-1718-2026).

| Deployment | Best for | Tempo | Transport |
|------------|----------|-------|-----------|
| **Standalone + IAC** | GarageBand (no in-track MIDI FX) | Manual BPM or MIDI clock from DAW | Matilda play gem; optional external sync |
| **In-DAW plugin** | Logic, Ableton, Bitwig, FL Studio, Reaper | Host playhead BPM | `play_mode`: note or transport |

**GarageBand constraint:** GB receives MIDI from Matilda via IAC but cannot send MIDI clock or project tempo to external apps. Do not assume auto-BPM sync with GB.

**Next gate:** Run the [DAW compatibility test matrix](./MILESTONES.md#daw-compatibility--test-matrix-not-yet-run) before locking transport architecture.

### Pitch quantisation (Jun 2026)

`SequencerEngine` builds a sorted list of in-scale MIDI notes between the Min and Max pickers. Gem knobs index into this list (not raw semitones). Octave labels use `minOctave + 1` as the MIDI octave base so UI `C#4` matches knob 0%. Knob drag/scroll clamps at min/max — no wrap.

Python prototype (`cartesia/`) mirrors engine logic for fast iteration without rebuilding JUCE.

---

## Engine modules (target)

| Module | Responsibility |
|--------|----------------|
| `model.Patch` | Serialize/deserialize v2 JSON |
| `movement.PathState` | step_index, direction, mode-specific advance |
| `movement.advance(mode, state, skip_prob)` | Next index 0…15 |
| `engine.SequencerEngine` | Tick → layer queue → cell → MIDI event |
| `engine.resolve_pitch(cell, patch)` | degree + octave + jitter + scale → MIDI note from quantised window |
| `engine.roll_trigger(cell)` | gate + trigger_prob |

---

## Layer scheduler (v1)

```python
active = [i for i, L in enumerate(layers) if L.active]
playing_layer_idx = 0  # index into active list
# On layer complete (16 steps or mode cycle): playing_layer_idx += 1; wrap
```

Each layer maintains its own `PathState` (step index, ping-pong direction, random bag, etc.).

---

## UI ↔ engine binding

| UI | Patch field |
|----|-------------|
| Overview toggle click (top row) | `layers[i].active` |
| Mini-grid array hit-box click | `selected_layer` |
| Movement ▾ | `layers[selected_layer].movement` |
| Mini grid dots | read `layers[i].step_index` → (x,y) |
| Main grid | `layers[selected_layer].cells` |
| Quantise panel (Min/Tonic/Max/Scale glass dropdowns) | `root`, `mode`, `min_octave`, `max_octave` — changing any of these re-resolves grid knob note labels from cell `degree` within the quantised min…max window (M9) |
| Global Settings — play/pause | `transport` (engine) |
| Global Settings — clock dropdown | `master_division` |
| Global Settings — play mode dropdown | `play_mode` (`transport` \| `note`) |
| Collapse chevron | UI-only layout state (not persisted in Patch) |
| Hero Rive states (future) | `idle` · `playing` · `transport_sync` · … — drive Rive inputs from engine transport |

---

## Implementation phases

| Phase | Deliverable |
|-------|-------------|
| **0** | Docs + folder layout + model v2 + preset JSON |
| **1** | Python engine: movement modes + layer 1 MIDI |
| **2** | JUCE shell: `cartesia-vst-ui` prototype (M1–M8b ✅) ported to JUCE + layer 1 playback |
| **3** | Layers 2–4 sequential + edit-while-playing |
| Phase | Deliverable |
|-------|-------------|
| **4** | External chrome wiring · play on transport | 🔄 GB standalone ✅; beat-quantized start ✅; knob quantise ✅; multi-DAW matrix pending |
| **B** | XYZ clock divisions · polyphony · randomize modal |
| **UI+** | Rive hero animation · state machine wired to transport/playback (deferred) |

---

## Gridwalker migration

`gridwalker/` proved MIDI-via-IAC and a basic 4×4. **Do not extend** GridWalker UI for Matilda.

- Reuse: `GridEngine` timing patterns, `GridComponent` hit testing ideas, CMake/JUCE setup.
- Replace: axis clock UI → movement dropdown; Z tabs → layer overview model.

New binary name: **Matilda** (avoid Cartesia trademark in shipping build).

---

## Preset format

- **Version:** `2` in JSON root.
- **Default:** `matilda/presets/default.layer1.json`
- Legacy `examples/cartesia/*.json` remain v1 until migrated.

---

*Architecture v1 · UI prototype M8b complete Jun 2026 · host integration notes Jun 18, 2026*
