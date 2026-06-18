# Matilda — JUCE plugin (VST3 / AU / Standalone)

MIDI arp / grid sequencer for macOS DAWs. Builds on the Cartesia engine (Patch v2, sequential layers, movement modes, trigger probability, jitter).

## Build

Requires CMake 3.22+ and a C++17 compiler. JUCE is taken from `../../gridwalker/JUCE` when present.

```bash
cd matilda/plugin
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j4
```

### Outputs

| Format | Path |
|--------|------|
| **Standalone** | `build/Matilda_artefacts/Release/Standalone/Matilda.app` |
| **VST3** | `build/Matilda_artefacts/Release/VST3/Matilda.vst3` |
| **AU** | `build/Matilda_artefacts/Release/AU/Matilda.component` |

Install for DAW scanning (optional):

```bash
cp -R build/Matilda_artefacts/Release/VST3/Matilda.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R build/Matilda_artefacts/Release/AU/Matilda.component ~/Library/Audio/Plug-Ins/Components/
```

Rescan plugins in Logic / Ableton / Reaper after copying.

---

## Quick test — Standalone + IAC → GarageBand

Same routing pattern as the GridWalker sandbox:

```text
Matilda.app  --MIDI OUT-->  IAC Bus 1  --MIDI IN-->  GarageBand instrument track
```

### One-time setup

1. **Audio MIDI Setup** → IAC Driver → *Device is online*
2. **GarageBand** → Settings → Audio/MIDI → enable **IAC Driver Bus 1**
3. **Matilda.app** → **Options → MIDI Output → IAC Driver Bus 1**

### GarageBand transport sync (Standalone)

Enable **Sync GarageBand transport** in Matilda’s footer (on by default).

1. **Audio MIDI Setup** → IAC Driver → Device is online  
2. **GarageBand** → Settings → Audio/MIDI → enable **IAC Driver Bus 1** (or Bus 2)  
3. **GarageBand** → Settings → enable **Send MIDI clock** to the same IAC bus  
4. **Matilda** → Options → **MIDI Output** → IAC Bus 1 (notes to GB)  
5. **Matilda** → Options → **MIDI Input** → same IAC bus (Start/Stop/Clock from GB)

When GB transport plays, Matilda receives **MIDI Start + Clock** and begins stepping.  
When GB stops, Matilda receives **MIDI Stop** and sends **all notes off**.

You can still use Matilda’s own Play/Stop when sync is off.

### Each session

1. Open **Matilda.app** from the build path above (or double-click in Finder).
2. GarageBand: create a **software instrument** track, **arm** it, press **Play** on the GB transport (optional — GB just needs to receive MIDI).
3. In Matilda: press the **play gem** (bottom-left). You should hear the default Lydian layer-1 pattern at **120 BPM**, clock **1/16**.
4. Toggle layers 2–4 in the overview (layer 1 stays on). Each layer completes 16 steps before handing off.

### Controls (test UI — Jun 2026 rebuild)

Clean code-drawn layout (no Cartesia SVG overlay). Stepic row-major movement — not XYZ.

| Control | Action |
|---------|--------|
| **Play / Stop** | Start/stop sequencer (standalone) |
| **Clock** combo | 1/64 … 1/4 · triplets (1/12, 1/24, 1/6) · dotted (3/64, 3/32, 3/16, 3/8) |
| **Play mode** | Transport only (v1) |
| **Step path** strip | 16 pills — playhead step 0…15 |
| **Layers** row | `1–4` = active toggle · `E1–E4` = edit layer |
| **Movement** bar | Forward / Reverse / Ping-pong / Pendulum / Random / Random skip |
| **Quantise** panel | Tonic · scale (13 modes) · **Min/Max note** (tonic-relative, e.g. C#1…C#9) |
| **4×4 grid** | Click = gate · scroll = degree · ▲ trigger / ● jitter (drag = %) |
| **Status** line | tick, step, layer, scale |

---

## Plugin in a DAW

Load **Matilda** as a **MIDI effect** before an instrument (VST3 or AU).

- Sequencer runs when **DAW transport is playing** (host BPM + playhead).
- Standalone play button is ignored in plugin mode — use the DAW transport.
- Full patch state (grid, layers, scale, cells) is saved in the project / host preset via Patch v2 JSON inside plugin state.

---

## Presets

Default patch is embedded from `matilda/presets/default.layer1.json` (Lydian layer 1 grid). Host save/restore uses the same JSON schema as the Python engine (`cartesia/model.py`).

---

## Docs

- [SPEC.md](../../docs/cartesia-vst/SPEC.md) — product + engine behaviour
- [BLUEARP-ENHANCEMENTS.md](../../docs/cartesia-vst/BLUEARP-ENHANCEMENTS.md) — future features backlog (from BlueARP manual)
- [ARCHITECTURE.md](../../docs/cartesia-vst/ARCHITECTURE.md) — repo map
- React UI reference: `cartesia-vst-ui/` (Figma shell — port in progress)
