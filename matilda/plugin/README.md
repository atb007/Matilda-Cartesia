# Matilda — JUCE plugin (VST3 / AU)

MIDI arp / grid sequencer for macOS and Windows DAWs. Builds on the Cartesia engine (Patch v2, sequential layers, optional polyphony, movement modes, trigger probability, jitter, per-layer step count).

> **Standalone app** is built from a separate codebase: [`../standalone/`](../standalone/README.md) (v1.0.9+). This dir builds **VST3 + AU only**.

## Download pre-built plugins

**GitHub Releases:** [github.com/atb007/Matilda-Cartesia/releases](https://github.com/atb007/Matilda-Cartesia/releases)

| Zip | Use |
|-----|-----|
| `Matilda-Windows-vst3.zip` | FL Studio / Windows — copy `Matilda.vst3` to `C:\Program Files\Common Files\VST3\` |
| `Matilda-macOS-vst3.zip` | macOS DAWs — copy to `~/Library/Audio/Plug-Ins/VST3/` |
| `Matilda-*-standalone.zip` | Standalone app — see [`../standalone/README.md`](../standalone/README.md) |

Latest: **[v1.0.14](https://github.com/atb007/Matilda-Cartesia/releases/tag/v1.0.14)**

See `releases/README.md` for CI workflow details.

**Source status:** Jul 16, 2026 Windows VST3 port is merged in `matilda/plugin/` (polyphony crown/engine, step scroll, layer clipboard, presets, frosted shell, Rive `faceStreakVis`, playhead sync). Release artifacts are built by the tag-triggered Matilda Release workflow.

## Build locally

Requires CMake 3.22+ and a C++17 compiler. JUCE is taken from `../../gridwalker/JUCE` when present.

```bash
cd matilda/plugin
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j4
```

### Outputs

| Format | Path |
|--------|------|
| **VST3** | `build/Matilda_artefacts/Release/VST3/Matilda.vst3` |
| **AU** | `build/Matilda_artefacts/Release/AU/Matilda.component` |

Standalone: build from [`../standalone/`](../standalone/README.md).

Install for DAW scanning (optional):

```bash
cp -R build/Matilda_artefacts/Release/VST3/Matilda.vst3 ~/Library/Audio/Plug-Ins/VST3/
cp -R build/Matilda_artefacts/Release/AU/Matilda.component ~/Library/Audio/Plug-Ins/Components/
```

Rescan plugins in Logic / Ableton / Reaper after copying.

---

## Quick test — Standalone + IAC → GarageBand

The Standalone app lives in [`../standalone/`](../standalone/README.md). Same IAC pattern as below — GB cannot send tempo; match BPM manually.

---

## Plugin in a DAW (FL Studio, Logic, Reaper)

Load **Matilda** as a **silent instrument** in FL Fruity Wrapper (slot 1) or as a MIDI processor before a synth in other DAWs (VST3 or AU).

| Play mode | When the grid steps |
|-----------|---------------------|
| **Note** (default preset) | Matilda play gem armed — internal clock at host BPM |
| **Transport** | DAW transport playing **and** Matilda play gem armed |

GarageBand does **not** reliably host MIDI-effect plugins in-track — use **Standalone + IAC** instead.

### FL Studio — routing status (Jul 2026)

| Method | FL version tested | Status | Notes |
|--------|-------------------|--------|-------|
| **Fruity Wrapper internal ports** (Matilda out *N* → synth in *N*) | **25.1.4** build 4951 | ✅ **Validated** (v1.0.11+) | Requires silent-instrument registration; set **MIDI Out → (None)** |
| **Virtual MIDI port** (plugin **MIDI Out** selector) | 20.0.1 build 451 | ✅ **Validated** | FL 20.x fallback when wrapper ports fail |
| **Standalone + loopMIDI** (two-port clock + notes) | 20.0.1 | ✅ **Validated** | See [`../standalone/README.md`](../standalone/README.md) |
| **Fruity Wrapper internal ports** | 20.0.1 | ❌ **Failed** | VST3 MIDI output not forwarded between wrapper plugins |

#### Recommended: Fruity Wrapper (FL 21+, v1.0.11+)

BlueARP-style wiring — sound, meters, step lanes, and armed MIDI recording stay on the **channel's mixer insert** (not master).

1. Channel rack → **Fruity Wrapper** (generator).
2. Slot 1: **Matilda** · Slot 2: synth (3xOsc, Sytrus, etc.).
3. Wrapper settings: Matilda **MIDI output port N** → synth **MIDI input port N** (matching number ≥ 11).
4. Matilda plugin window: **MIDI Out → (None)** — critical; a virtual port steals routing to master.
5. Route wrapper channel to a **mixer insert** (not master).
6. Press Play — channel meter + step rack should show activity; arm record to capture generated MIDI.

#### Fallback: virtual MIDI port (FL 20.x, v1.0.8+)

FL Studio 20.x does not reliably forward VST3 **MIDI output** from one wrapper plugin to another. Matilda mirrors the Standalone workaround: stream notes to an OS-level virtual port.

1. **Windows:** [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html) — create e.g. `Matilda-Notes`.
   **macOS:** *Audio MIDI Setup → IAC Driver* — add a port.
2. Load Matilda VST3. In the plugin window, set **MIDI Out** (bottom-left) to that port.
3. On your synth channel: **MIDI Input** → same port (FL: assign port number on Input list; synth channel must match).
4. Press play — Matilda's arp reaches the synth.

The selected port is saved with the project. Use only when wrapper internal ports fail.

#### Patcher

Matilda → generator inside Patcher, green MIDI cables to synth — same **MIDI Out → (None)** rule applies when routing inside the patch.

Install: `Matilda.vst3` → `C:\Program Files\Common Files\VST3\` (Windows) or `~/Library/Audio/Plug-Ins/VST3/` (macOS)

### Windows / VST3 UI (Jul 2026 source port)

| Topic | Detail |
|-------|--------|
| **Default size** | Expanded **2376×1805** design px at **0.52 × 0.9** ≈ **1112×845** logical px |
| **User resize** | Drag any **corner or edge** (8 grips) — scale factor **0.7…1.0** |
| **Host oversize** | If FL Studio / Fruity Wrapper leaves empty space right of the UI, starfield wallpaper fills it (`HeroBackdropDrawing`) |
| **Chevron** | `collapse-toggle-expanded@2x.png` / `collapse-toggle-collapsed@2x.png` — re-export to `cartesia-vst-ui/public/assets/` then rebuild |
| **Title filigree** | Module titles (Quantize Scale, Global Settings, etc.) live in fixed design-space shell — not stretched when host resizes |
| **Jul 16 port** | Polyphony crown, step scroll, layer clipboard, presets, frosted shell, and last-fired playhead sync are in plugin source |
| **Not in VST3** | Footer BPM label, sync toggle, debug status (Standalone sandbox only) |

After replacing chevron PNGs: `cmake --build build --config Release` (re-embeds `MatildaAssets`).

---

## Gem knob / scale quantisation

| Behaviour | Detail |
|-----------|--------|
| Note list | All in-scale pitches from **Min** tonic through **Max**, ascending |
| Knob 0% | Matches Min picker (e.g. C#4) |
| Knob 100% | Highest in-scale note in window |
| Scroll / drag | Sequential steps; **stops** at ends (no wrap) |
| Scale change | All cells re-snap to quantised set |

Arp start is **beat-quantized** — first step waits for next downbeat (DAW playhead or internal clock).

---

## Presets

Default patch is embedded from `matilda/presets/default.layer1.json` (Lydian layer 1 grid). Host save/restore uses Patch JSON v2. The Jul 16 source port adds the standalone-style preset bar: user presets seed to `IdeasLab/Matilda/presets`, dirty `*` marks edited patches, and load preserves current BPM/transport state.

---

## Docs

- [SPEC.md](../../docs/cartesia-vst/SPEC.md) — product + engine behaviour
- [BLUEARP-ENHANCEMENTS.md](../../docs/cartesia-vst/BLUEARP-ENHANCEMENTS.md) — future features backlog (from BlueARP manual)
- [ARCHITECTURE.md](../../docs/cartesia-vst/ARCHITECTURE.md) — repo map
- React UI reference: `cartesia-vst-ui/` (Figma shell — port in progress)
