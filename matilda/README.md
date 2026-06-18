# Matilda — Cartesia VST v1.0

Grid sequencer inspired by [CV funk Cartesia](https://github.com/codygeary/CVfunk-Modules/blob/main/src/Cartesia.cpp).  
Product UI codename from Figma: **Matilda**.

## Docs

| Doc | Purpose |
|-----|---------|
| [SPEC.md](../docs/cartesia-vst/SPEC.md) | Product + engine spec (source of truth) |
| [DESIGN.md](../docs/cartesia-vst/DESIGN.md) | UI layout, cell interactions, Figma links |
| [ARCHITECTURE.md](../docs/cartesia-vst/ARCHITECTURE.md) | Repo layout, phases, module map |
| [FIGMA-CHECKLIST.md](../docs/cartesia-vst/FIGMA-CHECKLIST.md) | Component handoff checklist |

## Figma

- [Main design — opt3](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886)
- [Layer + grid behaviour](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830)

## Repo layout

```text
matilda/
  assets/figma/     Export manifest + links
  assets/ui/        Raster/SVG from Figma handoff
  presets/          JSON patches
  plugin/           JUCE target (Phase 0 — evolves from gridwalker/)
cartesia/           Python engine prototype (isobar MIDI)
docs/cartesia-vst/  Spec + design docs
gridwalker/         Legacy sandbox (being superseded by matilda/plugin)
```

## Quick start (Python prototype)

```bash
python3 live_cartesia.py examples/cartesia/default.preset.json --dry-run
```

Engine schema aligns with `cartesia/model.py`. JUCE plugin: see **[plugin/README.md](plugin/README.md)** for build + Standalone/IAC/DAW testing.

## v1 scope

- Layer 1 playback first; 4 layers sequential (Option A)
- Movement dropdown per layer (Forward … Random Skip)
- Cell: gem pitch, octave offset, trigger prob, jitter, hard gate
- Quantise scale panel (tonic-relative min/max)
- Transport + clock in external chrome (see DESIGN.md)
