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
| [RIVE_ANIMATION_RULEBOOK.md](standalone/docs/RIVE_ANIMATION_RULEBOOK.md) | Rive v3 bindings + Metal/Windows presentation |
| [releases/VERSIONING.md](plugin/releases/VERSIONING.md) | When to bump MAJOR / MINOR / PATCH |

### Current focus (Jul 2026)

- **Done:** UI shell (M1–M8b), VST3/AU + Standalone builds, beat-quantized start, scale-quantised gem knobs
- **Done:** **FL Studio + virtual MIDI ports** — Standalone two-port wiring (clock + notes) and VST3 direct MIDI-out (v1.0.8+) validated on **FL Studio 20.0.1**
- **Frozen Jul 15, 2026:** polyphony crown + engine · step-count vine · frosted glass · mini-grid clipboard (reset = gate on) · presets bar · Rive v3 + Metal wordmark · `faceStreakVis` ← polyphony ∧ ≥2 layers · playhead UI = last fired step — see [MILESTONES freeze](../docs/cartesia-vst/MILESTONES.md#standalone-freeze--jul-15-2026)
- **Ported Jul 16, 2026:** the frozen set is merged into `matilda/plugin/` for Windows VST3.
- **Windows Rive hero validated Aug 2026 (v1.0.17):** D3D offscreen→image + framing parity — [milestone](../docs/cartesia-vst/MILESTONES.md#-windows-rive-hero-validated--aug-12-2026)
- **GarageBand:** Standalone + IAC + manual BPM (no host tempo sync)
- **Next:** random gen · remaining Windows VST3 host soak
- Full matrix: [MILESTONES.md](../docs/cartesia-vst/MILESTONES.md)

## Figma

- [Main design — opt3](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886)
- [Layer + grid behaviour](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830)
- [Polyphony crown — glowPolyphony](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5171-102837)

## Repo layout

```text
matilda/
  assets/figma/     Export manifest + links
  assets/ui/        Raster/SVG from Figma handoff (incl. stepscroll/, polyphony/)
  presets/          JSON patches
  plugin/           JUCE VST3 + AU (evolves with DAW integration)
  standalone/       JUCE Standalone only (stable audio base + UI QOL + Jul 2026 features)
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

- 4 layers: sequential by default; **polyphony** optional (standalone)
- Movement dropdown per layer (Forward … Random Skip)
- Per-layer **step count** 1…16 (standalone vine scroll)
- Cell: gem pitch, octave offset, trigger prob, jitter, hard gate
- Layer overview: activate, select, **right-click copy/paste/reset** (standalone)
- **Presets** bar + user library (standalone)
- Quantise scale panel (tonic-relative min/max)
- Transport + clock in external chrome (see DESIGN.md)
