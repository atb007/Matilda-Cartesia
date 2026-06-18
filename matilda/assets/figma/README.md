# Figma — Matilda / Cartesia v1.0

**File:** [AdMaker-CMS](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS)

## Key frames

| Frame | Node ID | URL |
|-------|---------|-----|
| Main UI (`opt3`) | `4919:97886` | [open](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4919-97886) |
| Layer + grid behaviour | `4922:103830` | [open](https://www.figma.com/design/jdsiHSEmMSTHUkDlgKSiod/AdMaker-CMS?node-id=4922-103830) |
| 1 layer activated | `4919:98982` | child of opt3 |
| 4×4 Grid | `4922:103831` | `Cell Anatomy States` instances |
| Layer overview mini grid | `4919:99118` | `top cell` |
| Movement dropdown | `4919:99373` | Presets list: forward, reverse, ping-pong, pendulum, random |
| Quantise Scale | `4919:99226` | Min / Tonic / Max + scale picker |
| External transport | `4919:98600` | playPause, Clock 1/16, Play mode Note |

## Figma components → code

| Figma component | Code id |
|-----------------|---------|
| `Cell Anatomy States` | `Cell/Gem` |
| `Inactive Cell` | `Cell/Gem/GateOff` |
| `image 74` (mini dot) | `LayerOverview/Dot` |
| `All four activated` | `LayerSelector/CrystalsRow` |

## Export rules

- Cell gem: SVG + `@2x` PNG (gate on, gate off, playhead, selected)
- Crystal layer icons: SVG ×4 accent colors
- Hero frame + metallic borders: PNG `@2x` WebP optional
- Prob ring: SVG arc template (`prob-ring.svg`)

Place exports under `matilda/assets/ui/`.
