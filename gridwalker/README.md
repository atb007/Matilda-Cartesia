# GridWalker — Sandbox Prototype

JUCE cartesian grid sequencer. **Test in GarageBand via Standalone + IAC.**

## Build

```bash
cd gridwalker
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release
```

## How it runs

| Mode | When the grid plays |
|------|---------------------|
| **Plugin in a DAW** | DAW transport playing **and** a MIDI note active (region, keyboard, etc.) |
| **Standalone sandbox** | **Play** button (uses C4 root) **or** MIDI notes received on input |

Incoming trigger notes are **not** passed through — only grid-generated notes are sent out (arp-style).

## Edit cell pitch

| Action | Result |
|--------|--------|
| Click | Toggle gate on/off |
| Scroll wheel on cell | Previous / next scale step |
| Double-click | Next scale step |
| Right-click | Next scale step |

Labels show the resolved note (e.g. `E4`) and scale degree (`#3`). Quantize + scale setting controls which pitches are available.

## GarageBand test — read this carefully

GridWalker and GarageBand are **two separate apps**. A MIDI clip in GarageBand does **not** feed GridWalker — it plays the instrument directly, bypassing GridWalker entirely.

```text
GridWalker (generates sequence)  --MIDI OUT-->  IAC Bus 1  --MIDI IN-->  GarageBand track  -->  synth
```

### Setup (once)

1. **Audio MIDI Setup** → IAC Driver → *Device is online*
2. **GarageBand** → Settings → Audio/MIDI → enable **IAC Driver Bus 1**
3. GridWalker → **Options → MIDI Output → IAC Driver Bus 1**

### Each time you test

1. **Remove or mute** any MIDI regions on the track (they compete with GridWalker)
2. GarageBand: software instrument track, **arm** it, press **Play**
3. GridWalker: click a few grid cells (gates on), then press **Play** in GridWalker's own transport bar (top of the window)
4. Watch the status line — `tick=` should increase. You should hear the armed instrument

**No keyboard needed for sandbox:** root defaults to **C4** until you send MIDI into GridWalker.

Optional: hold a key on your Mac keyboard if GridWalker MIDI input is set to your keyboard — that changes the root/transpose.

### Why it might be silent

| Symptom | Fix |
|---------|-----|
| `tick=0` not moving | Press **Play inside GridWalker** (not just GarageBand) |
| `Root: —` | Rebuild latest — should show `Root: C4 (default)` |
| GB clip plays but no grid | Clip is unrelated — mute it; GridWalker sends its own MIDI |
| Still silent | Confirm IAC enabled in both apps; try a different GB instrument |

## FL Studio (other laptop)

Load **GridWalker.vst3** as MIDI effect before the instrument.
