# Matilda — Cartesia VST v1.0

Grid sequencer inspired by [CV funk Cartesia](https://github.com/codygeary/CVfunk-Modules/blob/main/src/Cartesia.cpp).  
Product UI codename from Figma: **Matilda**.

## Docs

| Doc | Purpose |
|-----|---------|
| [SPEC.md](../docs/cartesia-vst/SPEC.md) | Product + engine spec (source of truth) |
| [DESIGN.md](../docs/cartesia-vst/DESIGN.md) | UI layout, cell interactions, Figma links |
| [MILESTONES.md](../docs/cartesia-vst/MILESTONES.md) | UI milestones + **host/DAW progress log** |
| [ARCHITECTURE.md](../docs/cartesia-vst/ARCHITECTURE.md) | Repo layout, phases, module map |
| [FIGMA-CHECKLIST.md](../docs/cartesia-vst/FIGMA-CHECKLIST.md) | Component handoff checklist |

### Current focus (Jul 2026)

- **Done:** UI shell (M1–M8b), VST3/AU + Standalone builds, beat-quantized start, scale-quantised gem knobs
- **Done:** **FL Studio + virtual MIDI ports** — Standalone two-port wiring (clock + notes) and VST3 direct MIDI-out (v1.0.8+) validated on **FL Studio 20.0.1**
- **GarageBand:** Standalone + IAC + manual BPM (no host tempo sync)
- **Next:** VST3 **Fruity Wrapper internal port** routing on a **newer FL Studio** build (not yet tested; failed on FL 20.0.1)
- Full matrix: [MILESTONES.md](../docs/cartesia-vst/MILESTONES.md)

## Figma

- [Main design — opt3](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886)
- [Layer + grid behaviour](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830)

## Repo layout

```text
matilda/
  assets/figma/     Export manifest + links
  assets/ui/        Raster/SVG from Figma handoff
  presets/          JSON patches
  plugin/           JUCE VST3 + AU (evolves with DAW integration)
  standalone/       JUCE Standalone only (stable audio base + UI QOL)
cartesia/           Python engine prototype (isobar MIDI)
docs/cartesia-vst/  Spec + design docs
gridwalker/         Legacy sandbox (being superseded by matilda/plugin)
cartesia-vst-ui/    React UI prototype (Figma shell reference)
```

## Quick start (Python prototype)

```bash
python3 live_cartesia.py examples/cartesia/default.preset.json --dry-run
```

Engine schema aligns with `cartesia/model.py`. JUCE builds: **[plugin/README.md](plugin/README.md)** (VST3/AU) · **[standalone/README.md](standalone/README.md)** (Standalone app).

## v1 scope

- Layer 1 playback first; 4 layers sequential (Option A)
- Movement dropdown per layer (Forward … Random Skip)
- Cell: gem pitch, octave offset, trigger prob, jitter, hard gate
- Quantise scale panel (tonic-relative min/max)
- Transport + clock in external chrome (see DESIGN.md)
