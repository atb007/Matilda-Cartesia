# Matilda — Standalone app (macOS / Windows)

> **Standalone-only codebase** (`matilda/standalone/`). Forked from the plugin tree at **v1.0.2** for a stable audio/MIDI engine, then forward-ported with UI QOL through **v1.0.11**, plus the **Jul 15, 2026 freeze** below. The frozen feature set was source-ported to `matilda/plugin/` on Jul 16, 2026; Windows package validation is still pending.
>
> - **This dir** builds **Standalone only** (`Matilda.app` / `Matilda.exe`).
> - **`matilda/plugin/`** builds **VST3 + AU only**.
> - Latest tagged release base: **v1.0.11** · standalone feature freeze: **Jul 15, 2026**

MIDI arp / grid sequencer. Cartesia engine (Patch v2, sequential layers, optional polyphony, movement modes, trigger probability, jitter, per-layer step count).

## Download

**GitHub Releases:** [github.com/atb007/Matilda-Cartesia/releases](https://github.com/atb007/Matilda-Cartesia/releases)

| Zip | Use |
|-----|-----|
| `Matilda-Windows-standalone.zip` | Windows — run `Matilda.exe` |
| `Matilda-macOS-standalone.zip` | macOS — open `Matilda.app` |

See [`../plugin/releases/README.md`](../plugin/releases/README.md) for CI workflow details.

## Build locally

Requires CMake 3.22+ and a C++17 compiler. JUCE is taken from `../../gridwalker/JUCE` when present.

```bash
cd matilda/standalone
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j4
```

**Output:** `build/Matilda_artefacts/Release/Standalone/Matilda.app` (macOS) or `Matilda.exe` (Windows)

---

## Jul 15, 2026 freeze

**Frozen** — behaviour below is locked for the next standalone cut / Windows VST3 port. Details: [MILESTONES freeze](../../docs/cartesia-vst/MILESTONES.md#standalone-freeze--jul-15-2026).

| Feature | Behaviour | Key files |
|---------|-----------|-----------|
| **Polyphony** | Crown above mini-grid; off = sequential hand-off; on = all active layers tick together | `PolyphonyCrown`, `SequencerEngine`, `PatchState::polyphony` |
| **Step count** | Per-layer 1…16 via vine scroll under 4×4; shortens that layer’s loop | `StepScroll`, `active_step_count` |
| **Frosted glass** | Blurred wallpaper under shell bedding | `ShellChrome` |
| **Layer clipboard** | Mini-grid right-click: copy/paste (± knobs), reset (gates **on**, degree 0), undo + floater | `LayerClipboard.h`, `LayerOverview` |
| **Playhead sync** | Mini + main lights = **last fired** step (matches MIDI); 1→N then next layer from 1; resume from cell 1 | `PluginEditor` timer · `lastStepIndex` |
| **Rive hero + wordmark** | v3 `.riv` on Metal; Matilda/Cartesia labels as `CALayer` above GPU; `faceStreakVis` ← poly ∧ ≥2 layers | `RiveHeroMetalView`, rulebook |
| **Presets** | Dropdown + save; dirty `*`; user App Support folder; seed Init; load keeps BPM/transport | `PresetBar`, `PresetLibrary` |

**Windows VST3 port status (post–v1.0.11):** see [SPEC.md — Windows VST3 port status](../../docs/cartesia-vst/SPEC.md#windows-vst3-port-status-postv1011).

Product detail: [SPEC.md](../../docs/cartesia-vst/SPEC.md) · [DESIGN.md](../../docs/cartesia-vst/DESIGN.md) · [MILESTONES.md](../../docs/cartesia-vst/MILESTONES.md) · Rive: [docs/RIVE_ANIMATION_RULEBOOK.md](docs/RIVE_ANIMATION_RULEBOOK.md)

---

## Quick test — macOS Standalone + IAC → GarageBand

```text
Matilda.app  --MIDI OUT-->  IAC Bus 1  --MIDI IN-->  GarageBand instrument track
```

GarageBand cannot send MIDI clock or tempo to external apps — **double-click footer BPM** to match your project tempo. Leave **Sync external transport** off.

See [plugin/README.md](../plugin/README.md) for the full GB setup table.

---

## FL Studio + loopMIDI — two-port wiring (validated Jul 2026)

Use **two separate virtual MIDI ports** so clock and notes flow in opposite directions without feedback.

```text
Port A (clock):   FL Studio  --FL-Sync-->       Matilda  (MIDI input)
Port B (notes):   Matilda    --Matilda-Notes-->  FL Studio --> synth channel
```

### One-time setup (Windows)

1. Install [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html). Create two ports, e.g. **`FL-Sync`** and **`Matilda-Notes`**.
2. **FL Studio → Options → MIDI settings**
   - **Output** list → select **`FL-Sync`** → enable → tick **Send master sync**
   - **Options menu** → enable **Enable MIDI master sync** (global switch)
   - **Input** list → select **`Matilda-Notes`** → enable → assign a **port number** (e.g. `5`)
3. **Matilda standalone → Options**
   - **MIDI Input** → enable **`FL-Sync`**
   - **MIDI Output** → select **`Matilda-Notes`**
4. Footer → turn **Sync external transport** **ON**.
5. On your **synth channel** in FL: set its **MIDI input port** to the **same number** as step 2 (e.g. `5`).

### Each session

1. Press **Play in FL** (clock only streams while transport is rolling).
2. Press **play gem** in Matilda (or let FL Start/Stop drive it when sync is on).
3. Footer BPM should track FL tempo within ~1 s. Change FL tempo while playing to confirm.

| Symptom | Fix |
|---------|-----|
| Notes but wrong tempo | Port A: confirm FL-Sync is Matilda **input** + master sync enabled + FL is **playing** |
| Tempo stuck at 120 | Upgrade to **v1.0.9+** (sample-accurate clock fix); confirm clock arrives (MIDI-OX on FL-Sync) |
| Silent synth | Port B: synth channel input port must match Matilda-Notes port number in FL Input list |
| Playhead stops when sync off | Expected — internal clock needs play gem; or turn sync back on for DAW-driven steps |

### Manual BPM fallback

Turn **Sync external transport OFF**, click the footer **BPM** label, type your tempo. Notes still route via Port B.

---

## External MIDI transport sync (Logic / Ableton / Reaper / FL)

For DAWs that send MIDI clock:

1. Enable **Sync external transport** in Matilda's footer.
2. DAW → send master sync on a virtual port (IAC / loopMIDI).
3. Matilda → Options → **MIDI Input** → same port.

BPM follows incoming MIDI clock (v1.0.9+: sample-accurate). Steps can follow MIDI Start/Stop/Clock when sync is on.

---

## What's in this codebase vs `matilda/plugin/`

| Feature | Standalone (`matilda/standalone/`) | Plugin (`matilda/plugin/`) |
|---------|-------------------------------------|----------------------------|
| Stable v1.0.2 audio engine base | ✅ | Evolves with DAW integration |
| Footer BPM + sync toggle | ✅ | Hidden in VST3 (host provides tempo) |
| DAW clock sync over IAC/loopMIDI | ✅ v1.0.9 | Host playhead BPM in-plugin |
| Direct MIDI-out device selector | ❌ (use Options → MIDI Output) | ✅ v1.0.8+ (loopMIDI workaround) |
| Filigree / UI scale persistence | ✅ v1.0.9 | ✅ v1.0.7+ |
| Polyphony crown + simultaneous layers | ✅ frozen Jul 15, 2026 | ✅ source port Jul 16, 2026 |
| Per-layer step-count vine | ✅ frozen Jul 15, 2026 | ✅ source port Jul 16, 2026 |
| Mini-grid copy/paste/reset/undo | ✅ frozen Jul 15, 2026 | ✅ source port Jul 16, 2026 |
| Frosted shell glass | ✅ frozen Jul 15, 2026 | ✅ source port Jul 16, 2026 |
| Presets bar + user library | ✅ frozen Jul 15, 2026 | ✅ source port Jul 16, 2026 |
| Playhead UI = last fired step | ✅ frozen Jul 15, 2026 | ✅ source port Jul 16, 2026 |
| Rive v3 + Metal wordmark / `faceStreakVis` | ✅ frozen Jul 15, 2026 | ✅ source port Jul 16, 2026 |

Do **not** port the plugin's opaque-host transport fallback or VST MIDI-out UI back into this tree without review — keep standalone audio behaviour stable.

---

## Docs

- [MILESTONES.md](../../docs/cartesia-vst/MILESTONES.md) — DAW test matrix + integration log
- [SPEC.md](../../docs/cartesia-vst/SPEC.md) — product + engine behaviour
- [plugin/README.md](../plugin/README.md) — VST3/AU + FL virtual-port plugin routing
