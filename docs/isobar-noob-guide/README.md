# isobar — Noob guide (patterns, timelines, MIDI)

A plain-language map of **isobar** (the Python library you’re already using with GarageBand).  
Not official docs — a cheat sheet for your VST/canvas idea.

**Official docs:** https://ideoforms.github.io/isobar/

---

## Mental model (read this once)

```text
YOU write rules (Patterns)
        ↓
Timeline schedules them every beat/step
        ↓
Events become MIDI notes (note, velocity, duration, channel…)
        ↓
OutputDevice sends to IAC / file / OSC / etc.
        ↓
GarageBand (or any synth) makes sound
```

**You do not draw notes.** You describe **streams of numbers** that become notes.

**Critical lesson you already learned:** Many patterns are **finite**. When they run out, the **track stops**. Wrap repeating material in `PLoop(...)` or the music dies after a few notes.

---

## The only 12 things you need first

| # | Name | One line |
|---|------|----------|
| 1 | `Pattern` | Anything that produces the next value when asked |
| 2 | `Timeline(120)` | Clock at 120 BPM; schedules and plays events |
| 3 | `timeline.schedule({...})` | “Play this pattern on these keys” |
| 4 | `PSequence([...])` | Cycle through a list |
| 5 | `PSeries(0, 2, 4)` | Count up: 0, 2, 4, 6, … |
| 6 | `PLoop(x)` | Repeat finite pattern forever |
| 7 | `Key` + `PDegree` | Stay in a scale |
| 8 | `PEuclidean(hits, steps)` | Evenly-spaced rhythm (1 = hit, None = rest) |
| 9 | `PPingPong` | Play forward then backward |
| 10 | `PMap` / `PMapEnumerated` | Transform values (custom logic) |
| 11 | `MidiOutputDevice` | Live MIDI to DAW |
| 12 | `MidiFileOutputDevice` | Write a `.mid` file |

Everything else in this doc is **optional depth**. Don’t memorize 80 classes before you ship a canvas UI.

---

## How a live script is structured

```python
from isobar import *
from isobar.io.midi.output import MidiOutputDevice

key = Key("C", "major")
midi = MidiOutputDevice(device_name="IAC Driver Bus 1")

timeline = Timeline(120, output_device=midi)

timeline.schedule({
    "degree": PLoop(PSequence([0, 2, 4, 7], 1)),
    "key": key,
    "octave": 4,
    "duration": 0.25,      # beats until next event
    "amplitude": 90,         # MIDI velocity
    "gate": 0.85,            # how long note holds (fraction of duration)
    "active": PEuclidean(5, 16),  # None = rest
})

timeline.run()   # blocks until Ctrl+C
```

**Schedule dict keys (note events):**

| Key | Meaning |
|-----|---------|
| `note` | MIDI note number 0–127 (60 = middle C) |
| `degree` | Scale degree (use with `key` / `scale`) |
| `amplitude` | Velocity |
| `duration` | Beats until next step |
| `gate` | Sustain length (1.0 = legato to next) |
| `active` | False/None = skip this step |
| `channel` | MIDI channel 0–15 |
| `octave`, `transpose` | Shift pitch |

You can use **`note`** OR **`degree`**, not both for the same musical line.

---

## Music helpers (not Patterns)

### `Key(tonic, scale)`

```python
Key("C", "major")
Key("A minor")          # string form
Key("F", Scale.minor)
```

### `Scale`

Built-in names: `major`, `minor`, `chromatic`, `dorian`, etc.  
Used inside `Key` and `PDegree`.

### `Chord`

Named chord shapes (intervals from a root). More useful for harmonic composition than simple arps.

### `note_name_to_midi_note("C4")` → `60`

From `isobar.util` (imported with `from isobar import *` in many setups).

---

## Pattern cheat sheet by category

Patterns are **iterators**: each step calls `next()` for pitch, rhythm, velocity, etc.

You can combine many with **math**:

```python
PSeries(0, 1) + 60          # add 60 to every value
PSequence([1, 0, 1]) * 62   # multiply (0 = rest in some setups)
```

---

### CORE — building blocks (`pattern/core.py`)

| Class | Simple meaning | Tiny example |
|-------|----------------|--------------|
| `Pattern` | Base class; all others extend this | — |
| `PConstant(42)` | Same value every time | Always velocity 100 |
| `PRef()` | Placeholder; swap `.pattern` later | Live coding / UI targets |
| `PFunc(fn)` | Call a Python function each step | Custom logic |
| `PArrayIndex(arr, index_pat)` | Pick from array by index | |
| `PDict({...})` | Multiple parallel streams as dict | Advanced scheduling |
| `PDictKey(d, "note")` | One key from a PDict | |
| `PConcatenate([a,b,c])` | Play A, then B, then C (**finite** unless looped) | Chain Euclidean sections |
| `PAdd`, `PSub`, `PMul`, `PDiv` | Math on two patterns | `PDegree(...) + 12` octave up |
| `PMod`, `PPow`, comparisons | Logic/math | `PMod(counter, 4)` |
| `PLFO` | Low-frequency oscillator pattern | Slow sweeps |

**Shorthand:** `patternA + patternB` uses `PAdd` automatically.

---

### SEQUENCE — rhythm, shape, repetition (`pattern/sequence.py`)

| Class | Simple meaning | Tiny example |
|-------|----------------|--------------|
| `PSequence(vals, repeats=∞)` | Loop a list | `PSequence([0,2,4], 1)` once then STOP |
| `PSeries(start, step)` | Arithmetic: 0,1,2,3… or 0,2,4,6… | Arp skeleton |
| `PRange(start, stop, step)` | Like Python `range` | |
| `PGeom(start, ratio)` | Multiply each step | Exponential spacing |
| `PImpulse(period)` | 1 every N steps, else 0 | Triggers, counters |
| `PLoop(pat, count=∞)` | **Repeat finite pattern** | **Use this or music stops** |
| `PPingPong(pat, times=1)` | Forward then backward | Classic arp motion |
| `PCreep(pat, length, creep, repeats)` | Loop a slice, nudge start | Slowly shifting loop |
| `PStutter(pat, count)` | Repeat each value N times | |
| `PSubsequence(pat, start, end)` | Slice | |
| `PReverse(pat)` | Reverse one pass | |
| `PReset(pat, trigger)` | Reset when trigger fires | |
| `PCounter(trigger)` | Count trigger pulses | Step index |
| `PCollapse(pat)` | Skip rests in stream | Tighter rhythm |
| `PNoRepeats(pat)` | Skip repeated values | |
| `PPad(pat, length)` | Pad with rests to length | |
| `PPadToMultiple(pat, n)` | Pad to multiple of n | |
| `PArpeggiator(...)` | Arp over held notes | Chord → broken notes |
| `PEuclidean(hits, steps, phase=0)` | Evenly distributed hits | `PEuclidean(5, 16)` |
| `PExplorer(...)` | Explore Euclidean variants | Experimental |
| `PPermut(...)` | All permutations | Huge output |
| `PPatternGeneratorAction(fn)` | When pattern ends, ask fn for new one | **Evolution** |
| `PSequenceAction(...)` | Run function per list item | |
| `PMetropolis(...)` | Metropolis algorithm walk | Generative melody |
| `PInterpolate(...)` | Glide between values | Smooth CC / pitch |

**Your Euclidean + evolving rhythm** uses `PEuclidean`, `PConcatenate`, and `PLoop`.

---

### CHANCE — randomness (`pattern/chance.py`)

| Class | Simple meaning | Tiny example |
|-------|----------------|--------------|
| `PWhite(min, max)` | Uniform random | Random velocity |
| `PBrown(min, max, delta_min, delta_max)` | Random **walk** | Drifting dynamics |
| `PCoin(probability)` | 0 or 1 | Gates, decisions |
| `PRandomWalk(list)` | Wander in a list | |
| `PChoice(values, weights=None)` | Pick one | Random scale degree |
| `PSample(values, n, weights=None)` | Pick several | |
| `PShuffle(list)` | Shuffled order | |
| `PShuffleInput(pat, n)` | Every N steps, reshuffle N values | |
| `PSkip(pat, play=0.8)` | `play` = probability note **sounds** | Random gaps |
| `PFlipFlop(p_on, p_off)` | Binary toggle | |
| `PSwitchOne(pat, length)` | Swap adjacent pairs sometimes | Glitch |
| `PRandomExponential(...)` | Skewed random | |
| `PRandomImpulseSequence(...)` | Random sparse hits | |

**Your mutation-at-end-of-phrase** used Python `random` inside `PMapEnumerated` — totally valid; you could also use `PChoice` + rising `PCoin` probability.

---

### TONAL — scales and harmony (`pattern/tonal.py`)

| Class | Simple meaning | Tiny example |
|-------|----------------|--------------|
| `PDegree(degree_pat, key)` | Scale degree → MIDI note | `PDegree(PSeries(0,2,4), key)` |
| `PFilterByKey(notes, key)` | Drop notes not in key | |
| `PNearestNoteInKey(note, key)` | Snap wrong note to scale | |
| `PMidiNoteToFrequency(n)` | Pitch → Hz | Sonification / OSC |
| `PMidiSemitonesToFrequencyRatio` | Interval → ratio | |
| `PKeyTonic(key)` | Output tonic MIDI | |
| `PKeyScale(key)` | Output scale object | |

---

### SCALAR — transform streams (`pattern/scalar.py`)

| Class | Simple meaning | Tiny example |
|-------|----------------|--------------|
| `PChanged(pat)` | 1 when value changes | Detect new note |
| `PDiff(pat)` | Current − previous | |
| `PSkipIf(pat, skip_pat)` | If skip true → rest (None) | **Mute mask** |
| `PNormalise(pat)` | Auto 0–1 from history | |
| `PMap(pat, fn, *args)` | Apply function | Duck velocity |
| `PMapEnumerated(pat, fn)` | `fn(step, value)` | **Your phrase mutation** |
| `PScaleLinLin(pat, a,b,c,d)` | Map number ranges | Probability ramp |
| `PScaleLinExp` | Linear → exponential range | |
| `PRound`, `PFloor`, `PCeil` | Rounding | |
| `PScalar(pat, method="mean")` | Collapse tuples | Chord → one value |
| `PWrap(pat, min, max)` | Wrap MIDI range | |
| `PIndexOf(list, pat)` | Index lookup | |

---

### MARKOV & L-SYSTEMS — “generative” melody

| Class | Simple meaning | Tiny example |
|-------|----------------|--------------|
| `PMarkov(transitions)` | Next note depends on previous | Learn from MIDI file (see examples) |
| `PLSystem(axiom, rules, n)` | Lindenmayer fractal sequences | Rhythms / melodies |

Cool for installations; **overkill for v1 arp canvas** unless you market “generative theory.”

---

### FADE — gradual appearance (`pattern/fade.py`)

| Class | Simple meaning |
|-------|----------------|
| `PFadeNotewise` | Introduce notes of a pattern gradually |
| `PFadeNotewiseRandom` | Random subset fades in |

Good for **ambient** builds; less for tight arps.

---

### WARP — time manipulation (`pattern/warp.py`)

| Class | Simple meaning |
|-------|----------------|
| `PWInterpolate` | Smoothly change warp amount |
| `PWSine` | Sine tempo/feeling warp |
| `PWRallantando` | Slow down over N beats |

Needs timeline context; **advanced**. Your “half/double speed” is usually just `duration` or `timeline.tempo`.

---

### OSCILLATOR — numeric waves (`pattern/oscillator.py`)

| Class | Simple meaning |
|-------|----------------|
| `PTri`, `PSaw` | Triangle / saw LFO-style numbers |

For control signals, not audio.

---

### MIDI / HARDWARE niche

| Class | Simple meaning |
|-------|----------------|
| `PMIDIControl` | Pattern of MIDI CC values |
| `PMonomeArcControl` | Monome Arc hardware |

Skip unless you target that hardware.

---

### STATIC — globals & time (`pattern/static.py`)

| Class | Simple meaning |
|-------|----------------|
| `PGlobals("name")` | Read a named global value |
| `PStaticPattern(pat, element_duration)` | Hold value for N beats |
| `PCurrentTime` | Current position in timeline (beats) |

Use `PCurrentTime` for “mutation increases over the **song**” (not just 32-step loop).

---

## Timeline & playback

| Thing | Simple meaning |
|-------|----------------|
| `Timeline(tempo, output_device=..., clock_source=...)` | Master clock + scheduler |
| `timeline.schedule(event_dict, delay=0, quantize=0, count=0)` | Add a part; `count` = max repeats |
| `timeline.run()` | Start (blocking) |
| `timeline.background()` | Run in another thread |
| `timeline.stop()` | Stop |
| `timeline.tempo = 140` | Change BPM live |
| `Track` | Usually created for you by `schedule()` |

**Sync options:**

- `MidiInputDevice()` as `clock_source` — follow DAW MIDI clock  
- `clock_source="link"` — Ableton Link  
- `MidiOutputDevice(send_clock=True)` — you are the master clock  

---

## Output devices (where MIDI goes)

| Device | Use |
|--------|-----|
| `MidiOutputDevice("IAC Driver Bus 1")` | Live → GarageBand |
| `MidiOutputDevice(virtual=True)` | Creates “Isobar Live Arp” port |
| `MidiFileOutputDevice("out.mid")` | Write file; call `.write()` after `run()` |
| `MidiInputDevice()` | Receive MIDI / clock |
| `DummyOutputDevice` | Print events (debug) |
| `OSCOutputDevice` | Send OSC |
| `FluidSynthOutputDevice` | Built-in GM synth (hear without DAW) |
| `SuperColliderOutputDevice` | SC integration |

List ports:

```python
from isobar.io.midi import get_midi_output_names, get_midi_input_names
print(get_midi_output_names())
```

---

## `Instrument` (optional sugar)

Wraps `timeline.schedule` with named parameters and keywords — useful for **blocks** in your canvas API, not required for scripts.

---

## Recipes (copy-paste ideas)

### Half / double speed (arp rate)

```python
"duration": 0.125,   # 2× faster steps at same BPM
"duration": 0.5,     # 2× slower
# or
timeline.tempo = 60  # half BPM for everything
```

### Mute specific steps

```python
"active": PEuclidean(5, 16),   # rhythm gate
# or fixed mask:
"active": PLoop(PSequence([1, 1, None, 1, 1, None, 1, 1])),
```

### Duck (quieter, not silent)

```python
"amplitude": PMap(
    PSequence([90, 90, 40, 90], 1),
    PLoop(PSequence([1, 1, 0.3, 1])),
    lambda vel, duck: int(vel * duck) if vel else None,
),
```

### Combine rhythms (both must allow hit)

Use `PMap` or multiply patterns; simplest: one `active` pattern you design by hand.

---

## What to wrap in your canvas “blocks” (product focus)

Suggested **first 10 blocks** — maps to your 3 layers:

| Block name | Built from |
|------------|------------|
| `scale_arp` | `Key`, `PDegree`, `PSeries`/`PSequence`, `PLoop` |
| `euclidean_gate` | `PEuclidean`, `PLoop` |
| `pingpong` | `PPingPong` |
| `velocity_walk` | `PSequence` + `PBrown` |
| `mutate_phrase` | `PMapEnumerated` + your prob curve |
| `evolve_rhythm` | `PConcatenate` of `PEuclidean`s + `PLoop` |
| `duck_mask` | `PMap` on `amplitude` |
| `mute_mask` | `PSkipIf` or `active` |
| `schedule_part` | `timeline.schedule(...)` |
| `output_daw` | `MidiOutputDevice` / IAC |

**Do not** expose all 80 patterns in v1 UI. You’ll ship never.

---

## Honest challenges (not a yes-man)

### 1. You don’t need to master isobar — you need a thin API

The library is huge because it’s a **research/composition** toolkit, not a product UI. Your canvas should expose **10–15 blocks**, not every `P*` class. Creators who want more get a code panel (`from isobar import *`).

### 2. “Python for everything” in a plugin is a trap

Documenting patterns in Python is right for **now** (scripts + IAC). Shipping **Python inside a VST** is heavy (size, GIL, timing). Plan: **Python on the canvas, compiled/native at runtime** later — don’t fall in love with embedded CPython in JUCE because the docs are nice.

### 3. Figma first is correct — but test ugly scripts early

Pretty “live animation” on a block means nothing if `PConcatenate` without `PLoop` still kills the track in 1 second. Every UI state needs a **real pattern rule** behind it (playing / muted / finite-ended).

### 4. GarageBand is a bad north star

You proved the engine. **Don’t** design the whole product around GB’s MIDI quirks. Design for **Logic/Ableton/Reaper** MIDI FX; treat GB as export-MIDI-only.

### 5. `from isobar import *` is great for you, bad for learners

Your noob doc should teach **explicit imports** eventually (`from isobar import Timeline, Key, PEuclidean, PLoop`). Star imports hide what’s a pattern vs a device.

### 6. Random in `PMapEnumerated` is not repeatable

You used `random.random()` for mutation — fun live, **bad for reproducible presets**. For a product: seeded RNG (`random.seed` per preset) or isobar’s `PCoin`/`PChoice` patterns.

### 7. Competing on “Euclidean arp” alone is weak

Stepic, ACDGEN, RIFFER exist. Your edge is **composable rules + code canvas + creator signifiers**, not another 5-over-16 slider.

### 8. Don’t build the node graph before the function library

Nodes should **compile to** the same 10 functions. If you can’t write `live_euclidean_c_major.py` in 30 lines without nodes, nodes won’t save you.

---

## Your scripts in this repo

| File | What it teaches |
|------|-----------------|
| `make_demo_mid.py` | MIDI file export |
| `live_arp_garageband.py` | Basic live arp + IAC |
| `live_euclidean_c_major.py` | Euclidean + mutation + `PLoop` |
| `check_midi_ports.py` | List MIDI ports |

---

## Learning order (1–2 evenings)

1. Change numbers in `live_arp_garageband.py` (duration, `PSeries`).  
2. Add `PEuclidean` and `active` in schedule dict.  
3. Read **SEQUENCE** + **CHANCE** + **TONAL** sections above only.  
4. Skim official example `03.ex-euclidean.py` in the isobar GitHub repo.  
5. Ignore **L-system**, **Markov**, **Monome**, **CV** until a preset needs them.

---

## Quick reference: operators on patterns

```python
a + b      # PAdd
a - b      # PSub
a * b      # PMul  (often used as gate: note * 0 = rest in some examples)
a / b      # PDiv
a % b      # PMod
a ** b     # PPow
```

---

*Generated for the VST/Ideas project. isobar © Daniel Jones / ideoforms — MIT license.*
