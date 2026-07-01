# Matilda — JUCE plugin (VST3 / AU)

MIDI arp / grid sequencer for macOS and Windows DAWs. Builds on the Cartesia engine (Patch v2, sequential layers, movement modes, trigger probability, jitter).

> **Standalone app** is built from a separate codebase: [`../standalone/`](../standalone/README.md) (v1.0.9+). This dir builds **VST3 + AU only**.

## Download pre-built plugins

**GitHub Releases:** [github.com/atb007/Matilda-Cartesia/releases](https://github.com/atb007/Matilda-Cartesia/releases)

| Zip | Use |
|-----|-----|
| `Matilda-Windows-vst3.zip` | FL Studio / Windows — copy `Matilda.vst3` to `C:\Program Files\Common Files\VST3\` |
| `Matilda-macOS-vst3.zip` | macOS DAWs — copy to `~/Library/Audio/Plug-Ins/VST3/` |
| `Matilda-*-standalone.zip` | Standalone app — see [`../standalone/README.md`](../standalone/README.md) |

Latest: **[v1.0.9](https://github.com/atb007/Matilda-Cartesia/releases/tag/v1.0.9)**

See `releases/README.md` for CI workflow details.

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

Load **Matilda** as a **MIDI effect** before an instrument (VST3 or AU).

| Play mode | When the grid steps |
|-----------|---------------------|
| **Note** (default preset) | Matilda play gem armed — internal clock at host BPM |
| **Transport** | DAW transport playing **and** Matilda play gem armed |

GarageBand does **not** reliably host MIDI-effect plugins in-track — use **Standalone + IAC** instead.

### FL Studio — routing status (Jul 2026)

| Method | FL version tested | Status | Notes |
|--------|-------------------|--------|-------|
| **Virtual MIDI port** (plugin **MIDI Out** selector) | 20.0.1 build 451 | ✅ **Validated** | Reliable on all FL versions; see below |
| **Standalone + loopMIDI** (two-port clock + notes) | 20.0.1 | ✅ **Validated** | See [`../standalone/README.md`](../standalone/README.md) |
| **Fruity Wrapper internal ports** (Matilda out *N* → synth in *N*) | 20.0.1 | ❌ **Failed** | VST3 MIDI output not forwarded between wrapper plugins |
| **Fruity Wrapper / Patcher** | FL 21+ (newer) | ⬜ **Not yet tested** | May work if Image-Line improved VST3 MIDI-out routing |

#### Recommended: virtual MIDI port (v1.0.8+)

FL Studio 20.x does not reliably forward VST3 **MIDI output** from one wrapper plugin to another. Matilda mirrors the Standalone workaround: stream notes to an OS-level virtual port.

1. **Windows:** [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html) — create e.g. `Matilda-Notes`.
   **macOS:** *Audio MIDI Setup → IAC Driver* — add a port.
2. Load Matilda VST3. In the plugin window, set **MIDI Out** (bottom-left) to that port.
3. On your synth channel: **MIDI Input** → same port (FL: assign port number on Input list; synth channel must match).
4. Press play — Matilda's arp reaches the synth.

The selected port is saved with the project.

#### Alternative: Fruity Wrapper internal ports (BlueARP-style)

Same VST3 binary — load Matilda + synth in one **Fruity Wrapper**, wire output port *N* → synth input port *N* (ports 11+ per BlueARP manual). **Not confirmed on FL 20.0.1.** Retest on a newer FL Studio before relying on this path.

**Patcher:** Matilda → generator, green MIDI cables to synth module — also pending verification on newer FL.

Install: `Matilda.vst3` → `C:\Program Files\Common Files\VST3\` (Windows) or `~/Library/Audio/Plug-Ins/VST3/` (macOS)

### Windows / VST3 UI (Jun 2026)

| Topic | Detail |
|-------|--------|
| **Default size** | Expanded **2376×1805** design px at **0.52 × 0.9** ≈ **1112×845** logical px |
| **User resize** | Drag any **corner or edge** (8 grips) — scale factor **0.7…1.0** |
| **Host oversize** | If FL Studio / Fruity Wrapper leaves empty space right of the UI, starfield wallpaper fills it (`HeroBackdropDrawing`) |
| **Chevron** | `collapse-toggle-expanded@2x.png` / `collapse-toggle-collapsed@2x.png` — re-export to `cartesia-vst-ui/public/assets/` then rebuild |
| **Title filigree** | Module titles (Quantize Scale, Global Settings, etc.) live in fixed design-space shell — not stretched when host resizes |
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

Default patch is embedded from `matilda/presets/default.layer1.json` (Lydian layer 1 grid). Host save/restore uses the same JSON schema as the Python engine (`cartesia/model.py`).

---

## Docs

- [SPEC.md](../../docs/cartesia-vst/SPEC.md) — product + engine behaviour
- [BLUEARP-ENHANCEMENTS.md](../../docs/cartesia-vst/BLUEARP-ENHANCEMENTS.md) — future features backlog (from BlueARP manual)
- [ARCHITECTURE.md](../../docs/cartesia-vst/ARCHITECTURE.md) — repo map
- React UI reference: `cartesia-vst-ui/` (Figma shell — port in progress)
