# Matilda (Cartesia VST) — documentation index

**Product:** Matilda · Cartesia v1.0  
**Code:** [`matilda/`](../../matilda/) · [`cartesia/`](../../cartesia/) (Python engine) · [`cartesia-vst-ui/`](../../cartesia-vst-ui/) (React UI prototype)

## Status

| Phase | State |
|-------|-------|
| **M1–M8b** UI prototype | ✅ Complete (Jun 11, 2026) |
| **M9** Engine / JUCE link | ✅ Layers + quantise + transport in JUCE |
| **FL Studio integration** | ✅ Virtual-port routing validated (Jul 2026) — see [MILESTONES.md](./MILESTONES.md#integration-milestones-jul-2026) |
| **Standalone Jul 2026** | ✅ **Frozen Jul 15, 2026** — polyphony · step scroll · clipboard · presets · frost · Rive v3 · playhead sync |
| **Rive hero animation** | ✅ v3 Metal + Windows D3D offscreen **validated v1.0.17** — [rulebook](../../matilda/standalone/docs/RIVE_ANIMATION_RULEBOOK.md) · [milestone](./MILESTONES.md#-windows-rive-hero-validated--aug-12-2026) |
| **Plugin / Windows VST3 port** | ✅ Source port Jul 16, 2026 · Rive hero Windows-validated Aug 2026 — [SPEC status](./SPEC.md#windows-vst3-port-status-postv1011) |

## Documents

| Doc | Description |
|-----|-------------|
| [SPEC.md](./SPEC.md) | Product + engine spec (source of truth) |
| [DESIGN.md](./DESIGN.md) | UI layout, cell interactions, hero/collapse behaviour |
| [MILESTONES.md](./MILESTONES.md) | Module-by-module UI build log + host + standalone Jul 2026 |
| [ARCHITECTURE.md](./ARCHITECTURE.md) | Repo layout, phases, UI ↔ engine binding |
| [FIGMA-CHECKLIST.md](./FIGMA-CHECKLIST.md) | Figma component handoff + shipped node map |
| [PRD-SANDBOX.md](./PRD-SANDBOX.md) | Legacy GridWalker sandbox PRD (historical) |
| [Standalone README](../../matilda/standalone/README.md) | Build + DAW wiring + Jul 2026 feature table |
| [VERSIONING.md](../../matilda/plugin/releases/VERSIONING.md) | When to bump MAJOR / MINOR / PATCH for Matilda releases |

## Figma

- [Main UI](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886)
- [Layer + grid behaviour](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830)
- [Polyphony crown](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5171-102837)
- [Presets bar](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5193-102814)
- [Expanded plugin window](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5002-6446)
- [Collapsed plugin window](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=5002-6447)

## Quick test

**Python engine (dry run):**

```bash
python3 live_cartesia.py matilda/presets/default.layer1.json --dry-run
```

**UI prototype (dev server):**

```bash
cd cartesia-vst-ui && npm install && npm run dev
```

Open the local URL (e.g. `http://localhost:5173/`) — renders `MatildaPluginFrame` at default user scale (0.52 × 0.9). Drag corners/edges to resize 0.7…1.0.
