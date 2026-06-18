# PRD — GridWalker Sandbox (superseded)

> **Superseded by [Matilda v2 spec](./SPEC.md)** and [`matilda/`](../../matilda/).  
> GridWalker remains a JUCE timing/MIDI sandbox under `gridwalker/`. New UI follows Figma Matilda.

**Codename:** GridWalker Sandbox  
**Goal:** A working JUCE plugin frame that proves the cartesian sequencer concept feels right in a DAW — UI borrowed from [CV funk Cartesia](https://github.com/codygeary/CVfunk-Modules), visual language inspired by [Vital](https://github.com/mtytel/vital) (look only).  
**Explicitly out of scope:** Figma polish, Teleiso/script layer, chord intelligence, preset store, mod matrix.

---

## 1. Executive summary

Build a **MIDI-effect sandbox** (BlueARP-style routing), not an audio effect and not a full synth. The user holds a single note; the plugin walks a **4×4 grid** on independent X/Y clocks and emits MIDI note-ons to whatever synth sits downstream.

Default sandbox configuration:

| Setting | Default |
|---------|---------|
| Grid | 4×4 (XY plane) |
| Z layers | 1 visible layer; Z movement **off** |
| Master step | 1/16 note |
| X clock | ÷1 (every master tick) |
| Y clock | ÷4 |
| Z clock | disabled |
| Movement | Forward on X and Y; reverse toggles present but can ship stubbed |

Success = load plugin → draw a few cells → press play in DAW → hear sequenced melody through an external synth within ~30 seconds.

---

## 2. Recommended build approach (phased)

**Do not implement all eight feature areas at once.** The sandbox should prove *movement + gates + pitch range + clocks* first; probability and randomness second.

### Phase 0 — “It moves and makes sound” (sandbox MVP)

| # | Feature | In? | Notes |
|---|---------|-----|-------|
| 1 | Forward movement all axes | **Partial** | X + Y only; Z UI visible but movement off |
| 2 | Toggle individual cells | **Yes** | Gate on/off per cell |
| 3 | Min / max note range | **Yes** | Low note + range (or min/max MIDI) |
| 4 | Quantize / scale | **Yes** | Scale dropdown + quantize on/off |
| 5 | Per-axis clocks | **Partial** | X and Y divisors; Z divisor greyed out |
| 6 | Jitter on trigger cycles | **Defer** | Phase 1 |
| 7 | XY 16-step default, Z off | **Yes** | Hard-coded default preset |
| 8 | Random skips / movements | **Defer** | Phase 1 |

**Phase 0 deliverable:** JUCE **Standalone** (primary) + VST3/AU MIDI effect, one screen, audible output via host routing.

### GarageBand testing (primary host)

GarageBand **does not load third-party MIDI FX plugins** (no BlueARP-style insert slot). Your existing workflow still works:

```text
GridWalker Standalone  →  IAC Driver Bus 1  →  GarageBand software instrument track
```

Same setup as `live_arp_garageband.py`:

1. **Audio MIDI Setup** → IAC Driver → *Device is online*
2. GarageBand → Settings → Audio/MIDI → enable **IAC Driver Bus 1**
3. Software instrument track → arm (Record) → Play
4. Run GridWalker Standalone; it sends sequenced MIDI to IAC

| Target | GarageBand | FL Studio (other laptop) |
|--------|------------|--------------------------|
| **Standalone → IAC** | ✅ Primary test path | ✅ Also works |
| **AU plugin** | ⚠️ Only if built as instrument + internal preview synth; MIDI FX AU not usable | — |
| **VST3 MIDI FX** | ❌ GarageBand has no VST / no MIDI FX rack | ✅ Full BlueARP-style routing |

**Sandbox decision:** Ship **Standalone first** so you can hear results in GarageBand on day one. VST3 MIDI FX is for FL Studio / Ableton / Logic validation later.

### Phase 1 — “Feels like Cartesia”

- Reverse toggles per axis (functional)
- Z layer tabs (Z1–Z4); Z clock enabled
- Global skip-step and random-jump probability (single knob each)
- Playhead animation + cell inspector (pitch degree, velocity)

### Phase 2 — “Performance hooks”

- Jitter (timing + pitch micro-offset per trigger)
- Note-trigger modes (retrigger policy, note-count → random seed)
- Scan movement mode
- Preset save/load (JSON)

---

## 3. Architecture decision: how sound is triggered

### Recommended: MIDI effect (BlueARP model)

```
[ MIDI keyboard ] → [ GridWalker MIDI FX ] → [ Any synth ] → [ Audio out ]
```

| Approach | Verdict | Why |
|----------|---------|-----|
| **MIDI effect / arp insert** | ✅ **Sandbox choice** | Matches Cartesia’s CV/gate output model. Zero audio code. Works in Ableton MIDI effect rack, Logic MIDI FX, Reaper JS, etc. |
| **Instrument (internal synth)** | ⚠️ Optional add-on | Faster “no routing” demo, but doubles scope (voice management, envelopes). Add a sine test oscillator only if Phase 0 user testing shows routing friction. |
| **Audio effect in chain** | ❌ Wrong model | Cartesian grid emits **notes**, not audio. An audio FX cannot arpeggiate without pitch detection (low quality, high latency). |

### Single-note policy (your question)

**Yes — you are correct.** Cartesia does not identify chords. It treats input as **transpose/root context**, not harmonic analysis.

Sandbox rules:

| Input | Behavior |
|-------|----------|
| 0 notes held | Sequence runs silently **or** uses last root (configurable; default: silent) |
| 1 note held | Root = that note; grid degrees offset from root |
| 2+ notes held | **Sandbox:** use **lowest note** as root; ignore others. Show “mono input” badge in UI. No chord voicing. |

This matches modular Cartesia (1V/oct CV in) and avoids BlueARP-style chord modes entirely.

### Re-trigger behavior (sandbox options)

Expose one dropdown in Phase 1; implement **Latch** + **Retrig** in Phase 0 backend:

| Mode | Behavior |
|------|----------|
| **Free run** (default) | DAW transport drives clocks; held note only sets root. New notes change root without resetting playhead. |
| **Retrig on note-on** | Each new note-on resets playhead to (0,0) and restarts axis counters. |
| **Retrig on first note** | Only the transition 0→1 held notes retriggers. |
| **Note-count seed** (Phase 2) | Number of currently held notes modulates RNG seed for skip/jump — not chord detection. |

---

## 4. Feature specification (sandbox frame)

### 4.1 Main view layout

Bare-bone single panel (~900×620 logical px, scalable):

```text
┌─────────────────────────────────────────────────────────────┐
│  GRIDWALKER          [▶ transport follows DAW]             │
├─────────────────────────────────────────────────────────────┤
│  Scale ▾   Quantize ☐   Root: C4   Min ▾   Range ▾         │
├─────────────────────────────────────────────────────────────┤
│  [Z1] [Z2] [Z3] [Z4]   ← Z2–Z4 disabled/grey in Phase 0     │
│                                                             │
│              ┌───┬───┬───┬───┐                              │
│              │   │   │ ● │   │  ← 4×4 cell grid             │
│              ├───┼───┼───┼───┤     ● = playhead             │
│              │   │ ■ │   │   │     ■ = gate on (toggle)     │
│              ├───┼───┼───┼───┤                              │
│              │   │   │   │   │                              │
│              └───┴───┴───┴───┘                              │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  Master: 1/16 ▾                                             │
│  X  ÷[1] ▾  [Fwd ●] [Rev ○]     Y  ÷[4] ▾  [Fwd ●] [Rev ○] │
│  Z  ÷[—] ▾  [Off]               (Phase 0: Z movement off)   │
└─────────────────────────────────────────────────────────────┘
```

### 4.2 Cell model (Phase 0)

Each active cell stores:

- `gate` (bool) — toggled by click
- `degree` (int 0–127 within range window) — edited via click+drag or inspector
- `velocity` (int, default 100) — inspector only in sandbox

### 4.3 Axis clocks (Phase 0)

- One **master tick** derived from DAW BPM + division (default 1/16).
- Independent counters for X and Y: advance when `masterTick % division == 0`.
- Direction: +1 forward; reverse toggle flips sign (Phase 1 logic; UI present in Phase 0).
- Wrap at grid edges (0↔3) with optional ping-pong deferred.

### 4.4 Pitch pipeline

```text
cell.degree → scale quantize → clamp(min, max) → + rootMidi → note number
```

Scales for sandbox: Chromatic, Major, Natural Minor, Dorian, Pentatonic Major.

### 4.5 MIDI output

On each master tick where a cell fires (gate on):

1. Resolve pitch from current (x, y, z).
2. Emit `Note On` with velocity.
3. Schedule `Note Off` after gate length (default: next master tick or 50% of step — pick one, document in UI).

Polyphony: Phase 0 = **monophonic output** (one voice). Z-layer poly deferred to Phase 1.

---

## 5. UI / visual direction

### Borrow from Cartesia (CV funk)

Reference: [Cartesia.cpp](https://github.com/codygeary/CVfunk-Modules/blob/main/src/Cartesia.cpp) module layout — dense 4×4 stage buttons, axis clock rows, min/range/quantize cluster. Copy **information architecture**, not artwork.

### Look & feel from Vital (visual only)

Reference: [Vital source](https://github.com/mtytel/vital) — dark navy background, soft accent rings, rounded rotary controls, high-contrast labels.

**License note:** Vital is **GPLv3**. For a proprietary sandbox, do **not** copy Vital C++ components verbatim. Instead:

- Implement a custom `juce::LookAndFeel_V4` subclass with Vital-*inspired* colors, radii, and knob arcs.
- Reuse open JUCE widgets (`Slider`, `TextButton`, `ComboBox`) styled to match.

### Can the UI be handled without Figma?

**Yes.** Phase 0 UI is entirely code-drawn JUCE:

| Component | Implementation |
|-----------|----------------|
| 4×4 grid | `GridCell` component (16× toggle buttons + playhead overlay) |
| Axis strips | Horizontal `Component` rows with labels |
| Knobs | `juce::Slider` rotary, custom LAF |
| Z tabs | `TextButton` bar |
| Modals | Deferred; use inline controls for sandbox |

No Figma required. Optional: export a single background PNG later.

### Phase 0 UI inventory (everything on one screen)

| Zone | Control | Type | Phase 0 behaviour |
|------|---------|------|-------------------|
| **Header** | Plugin title | Label | "GridWalker" |
| | Transport hint | Label | "Follows DAW / internal clock" |
| | Root display | Label | Shows lowest held MIDI note (e.g. `C4`) or `—` |
| | Mono badge | Label | "MONO IN" when 2+ keys held |
| **Pitch row** | Scale | Dropdown | Major, Minor, Dorian, Pentatonic, Chromatic |
| | Quantize | Toggle | On = scale degrees; off = semitone walk |
| | Min degree | Rotary/stepper | 0–11 within octave window |
| | Range | Rotary/stepper | Semitone span (e.g. 12) |
| **Layers** | Z1 Z2 Z3 Z4 | Tab buttons | Z1 active; Z2–4 greyed/disabled |
| **Grid** | 16 cells | Toggle buttons | Click = gate on/off |
| | Cell label | Text in cell | Note name when gate on (e.g. `D4`) |
| | Playhead ring | Overlay | Glow on current (x, y) cell |
| | Selected cell | Highlight border | Click+hold opens inline degree nudge (↑↓) |
| **Clocks** | Master division | Dropdown | 1/4, 1/8, 1/16, 1/32 |
| | X division | Dropdown | ÷1, ÷2, ÷4, ÷8, ÷16 |
| | X Fwd / Rev | Toggle pair | Fwd active; Rev visible, stubbed Phase 0 |
| | Y division | Dropdown | same options, default ÷4 |
| | Y Fwd / Rev | Toggle pair | same |
| | Z division | Dropdown | Greyed — shows "Off" |
| | Z movement | Toggle | Off (locked) |
| **Footer** | MIDI out port | Dropdown | Standalone only: IAC Driver Bus 1, etc. |
| | Status | Label | `x=2 y=1 tick=48` debug line |

**Not in Phase 0 UI:** probability modal, randomize, scan mode, retrigger dropdown, preset browser, jitter knobs.

**Visual style (Vital-inspired, code-only):**

- Background `#0B1020`, panels `#1A2238`
- Accent `#7B9FD4` (playhead), gate-on cells `#8FB8A8` @ 40% fill
- Rotary knobs: 32 px, arc stroke, no bitmap skins
- Font: system default (SF Pro on macOS) until licensed font added

---

## 6. Technical stack

| Layer | Choice |
|-------|--------|
| Framework | JUCE 7/8 (Projucer or CMake) |
| Plugin formats | **VST3 + AU** (MIDI effect bus) |
| Bus layout | MIDI in → MIDI out (no audio I/O in Phase 0) |
| State | `AudioProcessorValueTreeState` for parameters |
| Grid data | `std::array<Cell, 16>` per layer; 4 layers allocated, 1 active |
| Timing | `AudioPlayHead` for BPM/ppq; sample-accurate MIDI in `processBlock` |
| Build targets | macOS first (GarageBand + Standalone/IAC); FL Studio VST3 on second laptop |
| Primary test target | **Standalone** → IAC → GarageBand |

### JUCE MIDI effect notes

- Set `JucePlugin_WantsMidiInput` = 1, `ProducesMidiOutput` = 1, `IsMidiEffect` = 1.
- In Ableton: place on track **before** instrument.
- Plugin category: `kPlugCategEffect` with MIDI-only I/O (host-dependent labeling).

---

## 7. Phase 0 acceptance criteria

1. Plugin loads in DAW as MIDI effect without crash.
2. Clicking cells toggles gate; playhead visibly moves on X/Y during playback.
3. Changing Y divisor from ÷4 → ÷8 audibly slows vertical movement.
4. Quantize + scale changes pitch class set correctly.
5. Holding middle C and enabling gates produces pitched sequence through downstream synth.
6. Default preset loads with XY movement, Z off, 1/16 master step.
7. UI is readable at 100% and 125% scale; no Figma assets required.

---

## 8. Open questions (resolved for sandbox)

| Question | Decision |
|----------|----------|
| Trigger sounds how? | MIDI out to host → user’s synth |
| BlueARP routing? | **Yes** — same slot, same workflow |
| Identify incoming MIDI note? | **Lowest held note = root**; no chord ID |
| Re-trigger on new note? | Phase 0: free run; Phase 1: retrigger dropdown |
| Effect chain vs MIDI FX? | **MIDI FX**, not audio insert |
| Chords? | **Not in scope** — mono input policy |
| Internal synth? | Optional sine preview toggle (nice-to-have) |
| Name / branding | Not “Vital”, not “Cartesia” in shipping binary (trademark) |

---

## 9. Risks & mitigations

| Risk | Mitigation |
|------|------------|
| Host doesn’t expose MIDI FX slot | Document routing; ship standalone JUCE app with virtual MIDI port for testing |
| GPLv3 contamination from Vital | Visual inspiration only; own LAF code |
| Scope creep on probability | Hard Phase 0 gate; jitter/random in Phase 1+ |
| Grid UI feel wrong without polish | Prioritize playhead motion + gate toggle responsiveness over aesthetics |

---

## 10. Suggested timeline (solo dev)

| Week | Focus |
|------|-------|
| 1 | JUCE MIDI FX shell, master clock, X/Y step, MIDI out |
| 2 | Grid UI, cell gates, min/range/quantize, default preset |
| 3 | Reverse, Z tabs, skip/jump probability, retrigger modes |
| 4 | Jitter, presets JSON, bug pass |

---

## 11. What to call this in conversation

- **Product:** GridWalker (placeholder)
- **Engine:** Cartesian step sequencer
- **This doc:** Sandbox PRD — not the final product spec

---

*Sandbox PRD v0.1 — frame-only prototype; features phased intentionally.*
