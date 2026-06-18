import type { CSSProperties } from "react";
import type { Cell } from "../cellState";
import { rowMajorFromMiniGridIndex } from "../cellState";
import type { KnobVariant } from "./knobColors";
import { KNOB_COLORS, KNOB_VARIANT_ORDER } from "./knobColors";

/**
 * M5 — Layer overview mini-grid (Figma 4957:102758, states 4957:102757 / 4957:104400).
 *
 * 4 layers × 16 cells draped on a rope-net frame, plus one activation toggle
 * per layer in the top row.
 *
 * Cell index in LAYER_CELLS is **column-major** (down each column, then next column).
 * The main 4×4 grid playhead uses **row-major** (left → right, then next row).
 * `miniGridIndexFromRowMajorStep` bridges the two.
 *
 * Assets from Figma 4914:99077 (Cell States — Mini grid) and 4910:97550 (Inactive Cell).
 */

/* ── Base geometry (Figma px, frame 609.565 × 301.43) ────────────────────── */
const BASE_W = 609.565;
const BASE_H = 301.43;
const CELL   = 30;

const CELL_INACTIVE = "/assets/minigrid-inactive.png";
/** Visual centre of inactive export sits ~2.5px below gem centre at scale 1. */
const INACTIVE_Y_NUDGE = -2.5;

/* Rope-net frame: aspect 518/241, full width, centre offset +4.29px */
const FRAME_H   = BASE_W * (241 / 518);
const FRAME_TOP = BASE_H / 2 + 4.29 - FRAME_H / 2;

/* Cell positions per layer — [leftPct of frame width, topPx] from Figma */
const LAYER_CELLS: [number, number][][] = [
  [
    [4.52, 47],  [4.45, 100],  [4.45, 156],  [4.45, 211],
    [10.19, 69], [10.19, 120], [10.19, 175], [10.19, 231],
    [15.44, 90], [15.44, 140], [15.44, 198], [15.44, 250],
    [21.18, 110],[21.18, 165], [21.18, 218], [21.35, 269],
  ],
  [
    [27.86, 47.34],  [27.86, 99.91],  [27.86, 156.17], [27.86, 211.49],
    [33.43, 70.4],   [33.43, 122.96], [33.43, 179.22], [33.43, 234.56],
    [39.01, 86.07],  [39.01, 138.64], [39.01, 194.9],  [39.01, 250.24],
    [43.98, 107.28], [43.98, 159.85], [43.98, 216.1],  [43.98, 271.43],
  ],
  [
    [51.96, 47.34],  [51.96, 100.83], [51.96, 158.01], [51.96, 211.49],
    [57.38, 70.4],   [57.38, 121.13], [57.38, 178.3],  [57.38, 231.78],
    [62.2, 86.07],   [62.2, 139.57],  [62.2, 196.74],  [62.2, 250.24],
    [68.07, 107.28], [68.07, 160.78], [68.07, 217.95], [68.07, 271.43],
  ],
  [
    [75.45, 47.34],  [75.45, 100.83], [75.75, 158.01], [75.75, 211.49],
    [80.87, 70.4],   [80.87, 121.13], [80.87, 178.3],  [80.87, 234.56],
    [86.3, 87.92],   [86.3, 139.57],  [86.3, 198.59],  [86.3, 250.24],
    [91.72, 107.28], [91.72, 160.78], [91.72, 217.95], [91.72, 271.43],
  ],
];

const TOGGLES: [number, number][] = [
  [12.49, 6],
  [35.46, 3],
  [58.75, 3],
  [82.05, 3],
];

const CELL_OFF: Record<KnobVariant, string> = {
  orange: "/assets/minigrid-orange-off.png",
  red:    "/assets/minigrid-red-off.png",
  green:  "/assets/minigrid-green-off.png",
  blue:   "/assets/minigrid-blue-off.png",
};
const CELL_ON: Record<KnobVariant, string> = {
  orange: "/assets/minigrid-orange-on.png",
  red:    "/assets/minigrid-red-on.png",
  green:  "/assets/minigrid-green-on.png",
  blue:   "/assets/minigrid-blue-on.png",
};

/** Main 4×4 row-major step → mini-grid column-major cell index. */
export function miniGridIndexFromRowMajorStep(step: number): number {
  const row = Math.floor(step / 4);
  const col = step % 4;
  return col * 4 + row;
}

function gemImgStyle(): CSSProperties {
  return {
    position: "absolute",
    inset: 0,
    width: "100%",
    height: "100%",
    objectFit: "cover",
    display: "block",
    pointerEvents: "none",
  };
}

function inactiveSocketStyle(s: number): CSSProperties {
  return {
    position: "absolute",
    inset: 0,
    width: "100%",
    height: "100%",
    objectFit: "contain",
    objectPosition: `center calc(50% + ${INACTIVE_Y_NUDGE * s}px)`,
    display: "block",
    pointerEvents: "none",
  };
}

type Props = {
  scale?: number;
  activeLayers: boolean[];
  selectedLayer: number;
  playheadStep?: number;
  playingLayer?: number;
  /** Per-layer cell state — gate drives mini-grid playhead lighting. */
  layerCells: Cell[][];
  onToggleLayer?: (layer: number) => void;
  onSelectLayer?: (layer: number) => void;
};

export function LayerMiniGrid({
  scale = 1,
  activeLayers,
  selectedLayer,
  playheadStep = -1,
  playingLayer = -1,
  layerCells,
  onToggleLayer,
  onSelectLayer,
}: Props) {
  const s = scale;
  const px = (pct: number) => (pct / 100) * BASE_W * s;
  const slot = CELL * s;

  const miniPlayheadIndex =
    playheadStep >= 0 ? miniGridIndexFromRowMajorStep(playheadStep) : -1;

  const bounds = LAYER_CELLS.map(cells => {
    const xs = cells.map(([l]) => px(l));
    const ys = cells.map(([, t]) => t * s);
    return {
      x: Math.min(...xs),
      y: Math.min(...ys),
      w: Math.max(...xs) - Math.min(...xs) + slot,
      h: Math.max(...ys) - Math.min(...ys) + slot,
    };
  });

  return (
    <div
      className="select-none"
      style={{ position: "relative", width: BASE_W * s, height: BASE_H * s }}
    >
      <img
        alt=""
        src="/assets/minigrid-frame.png"
        style={{
          position: "absolute",
          left: 0,
          top: FRAME_TOP * s,
          width: BASE_W * s,
          height: FRAME_H * s,
          pointerEvents: "none",
        }}
      />

      {LAYER_CELLS.map((cells, layer) => {
        const variant = KNOB_VARIANT_ORDER[layer];
        const active = activeLayers[layer];
        const isPlayingLayer = layer === playingLayer && miniPlayheadIndex >= 0;

        return (
          <div key={variant}>
            {cells.map(([leftPct, topPx], i) => {
              const isPlayhead = isPlayingLayer && active && i === miniPlayheadIndex;
              const gateOn = layerCells[layer]?.[rowMajorFromMiniGridIndex(i)]?.gate ?? true;
              const showPlayheadLit = isPlayhead && gateOn;
              return (
                <div
                  key={i}
                  style={{
                    position: "absolute",
                    left: px(leftPct),
                    top: topPx * s,
                    width: slot,
                    height: slot,
                    pointerEvents: "none",
                    overflow: "visible",
                  }}
                >
                  <img
                    alt=""
                    src={CELL_INACTIVE}
                    style={{
                      ...inactiveSocketStyle(s),
                      opacity: active ? 0 : 1,
                      transition: "opacity 220ms ease",
                    }}
                  />

                  {/* Colored gem — off by default, on at playhead */}
                  <img
                    alt=""
                    src={showPlayheadLit ? CELL_ON[variant] : CELL_OFF[variant]}
                    style={{
                      ...gemImgStyle(),
                      opacity: active ? 1 : 0,
                      transition: "opacity 220ms ease",
                      filter: showPlayheadLit
                        ? `drop-shadow(0 0 ${5 * s}px ${KNOB_COLORS[variant].ledColor})`
                        : "none",
                    }}
                  />
                </div>
              );
            })}
          </div>
        );
      })}

      {bounds.map((b, layer) => {
        const variant = KNOB_VARIANT_ORDER[layer];
        const active = activeLayers[layer];
        return (
          <div
            key={variant}
            title={active ? `Edit layer ${layer + 1}` : `Layer ${layer + 1} (inactive)`}
            onClick={() => active && onSelectLayer?.(layer)}
            style={{
              position: "absolute",
              left: b.x - 4 * s,
              top: b.y - 4 * s,
              width: b.w + 8 * s,
              height: b.h + 8 * s,
              cursor: active ? "pointer" : "default",
            }}
          />
        );
      })}

      {TOGGLES.map(([leftPct, topPx], layer) => {
        const variant = KNOB_VARIANT_ORDER[layer];
        const active = activeLayers[layer];
        const selected = layer === selectedLayer;
        const locked = layer === 0;
        return (
          <button
            key={variant}
            title={
              locked
                ? "Layer 1 is always active"
                : active
                  ? `Deactivate layer ${layer + 1}`
                  : `Activate layer ${layer + 1}`
            }
            onClick={() => !locked && onToggleLayer?.(layer)}
            style={{
              position: "absolute",
              left: px(leftPct),
              top: topPx * s,
              width: slot,
              height: slot,
              background: "none",
              border: "none",
              padding: 0,
              cursor: locked ? "default" : "pointer",
            }}
          >
            <img
              alt=""
              src={CELL_INACTIVE}
              style={{
                ...inactiveSocketStyle(s),
                opacity: active ? 0 : 1,
                transition: "opacity 220ms ease",
              }}
            />
            <img
              alt=""
              src={active && selected ? CELL_ON[variant] : CELL_OFF[variant]}
              style={{
                ...gemImgStyle(),
                opacity: active ? 1 : 0,
                transition: "opacity 220ms ease",
                filter: selected && active
                  ? `drop-shadow(0 0 ${5 * s}px ${KNOB_COLORS[variant].ledColor})`
                  : "none",
              }}
            />
          </button>
        );
      })}
    </div>
  );
}
