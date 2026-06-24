# Matilda — JUCE plugin (VST3 / AU / Standalone)

MIDI arp / grid sequencer for macOS and Windows DAWs. Builds on the Cartesia engine (Patch v2, sequential layers, movement modes, trigger probability, jitter).

## Download pre-built plugins

**GitHub Releases:** [github.com/atb007/Matilda-Cartesia/releases](https://github.com/atb007/Matilda-Cartesia/releases)

| Zip | Use |
|-----|-----|
| `Matilda-Windows-vst3.zip` | FL Studio / Windows — copy `Matilda.vst3` to `C:\Program Files\Common Files\VST3\` |
| `Matilda-macOS-vst3.zip` | macOS DAWs — copy to `~/Library/Audio/Plug-Ins/VST3/` |
| `*-standalone.zip` | Standalone app (no DAW) |

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

## Quick test — Standalone + IAC → GarageBand (recommended)

**Matilda and GarageBand are two separate apps** — same pattern as GridWalker. A MIDI clip on a GB track does **not** feed Matilda; it plays the instrument directly.

```text
Matilda.app  --MIDI OUT-->  IAC Bus 1  --MIDI IN-->  GarageBand instrument track
```

### One-time setup

1. **Audio MIDI Setup** → IAC Driver → *Device is online*
2. **GarageBand** → Settings → Audio/MIDI → enable **IAC Driver Bus 1**
3. **Matilda.app** → **Options → MIDI Output → IAC Driver Bus 1**

### Each session

1. **Match tempo:** GarageBand does **not** send MIDI clock or tempo to external apps — there is no setting for it in GB Preferences. **Double-click the BPM** in Matilda’s footer and set it to your project tempo (e.g. `60`).
2. Leave **Sync GB transport** **off** (GarageBand cannot drive external transport either).
3. GarageBand: software instrument track, **arm** it, press **Play** on GB transport (optional — for monitoring).
4. Matilda: press the **play gem** (Global Settings, bottom-left).
5. Footer status should show `tick=` increasing (e.g. `step=3 tick=12 · L1 · C lydian`).

| Symptom | Fix |
|---------|-----|
| BPM stuck at 120 | **Double-click footer BPM** → enter your GB project tempo |
| `tick=0` not moving | Press **play inside Matilda**; turn **Sync GB transport** off |
| GB clip plays but no grid | Mute the clip — Matilda sends its own MIDI |
| Still silent | Confirm IAC in both apps; try a different GB instrument |

### External MIDI transport sync (Logic / Ableton / Reaper only)

GarageBand **cannot** be a MIDI clock master. For DAWs that support it:

1. Enable **Sync external transport** in Matilda’s footer.
2. DAW MIDI preferences → enable **MIDI clock** on the IAC bus.
3. Matilda → Options → **MIDI Input** → same IAC bus.

BPM then follows the host playhead (in-plugin) or incoming MIDI clock (standalone + IAC).

---

## Plugin in a DAW (FL Studio, Logic, Reaper)

Load **Matilda** as a **MIDI effect** before an instrument (VST3 or AU).

| Play mode | When the grid steps |
|-----------|---------------------|
| **Note** (default preset) | Matilda play gem armed — internal clock at host BPM |
| **Transport** | DAW transport playing **and** Matilda play gem armed |

GarageBand does **not** reliably host MIDI-effect plugins in-track — use **Standalone + IAC** above instead.

### FL Studio (BlueARP-style routing)

Same **VST3 MIDI-FX** binary — no separate codebase:

1. **Patcher** — Matilda → generator, wire green MIDI cables; or
2. **Fruity Wrapper** — Matilda Settings → output port *N*; synth input port *N* (use ports 11+; see BlueARP manual).

Install: `build/Matilda_artefacts/Release/VST3/Matilda.vst3` → `~/Library/Audio/Plug-Ins/VST3/`

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
