import { CellAnatomyStates } from "./CellAnatomyStates";
import type { Cell, Patch } from "../engine";
import { degreeToKnobIndex, noteLabelForCell } from "../engine";
import type { KnobVariant } from "./knobColors";

/**
 * M2 — 4×4 gem grid (Figma 4922:103831).
 * 16 cells of a single color variant = one layer.
 *
 * Step index 0…15 maps row-major: top-left → right → next row (SPEC.md).
 */
const CELL_BASE_W = 120;
const CELL_BASE_H = 130;

type Props = {
  variant?: KnobVariant;
  scale?: number;
  cellWidth?: number;
  cellHeight?: number;
  colGap?: number;
  rowGap?: number;
  playheadStep?: number;
  cells: Cell[];
  patch: Pick<Patch, "root" | "mode" | "quantize" | "minOctave" | "maxOctave">;
  onCellChange: (index: number, patch: Partial<Cell>) => void;
};

export function Grid4x4({
  variant = "orange",
  scale = 1,
  cellWidth,
  cellHeight,
  colGap = 0,
  rowGap = 0,
  playheadStep = -1,
  cells,
  patch,
  onCellChange,
}: Props) {
  const slotW = cellWidth ?? CELL_BASE_W * scale;
  const slotH = cellHeight ?? CELL_BASE_H * scale;
  const sx = slotW / CELL_BASE_W;
  const sy = slotH / CELL_BASE_H;

  return (
    <div
      style={{
        display: "grid",
        gridTemplateColumns: `repeat(4, ${slotW}px)`,
        gridTemplateRows: `repeat(4, ${slotH}px)`,
        columnGap: colGap,
        rowGap: rowGap,
      }}
    >
      {cells.map((cell, i) => (
        <div
          key={i}
          style={{
            position: "relative",
            width: slotW,
            height: slotH,
            overflow: "visible",
          }}
        >
          <div
            style={{
              position: "absolute",
              left: "50%",
              top: "50%",
              width: CELL_BASE_W,
              height: CELL_BASE_H,
              transform: `translate(-50%, -50%) scale(${sx}, ${sy})`,
              transformOrigin: "center center",
            }}
          >
            <CellAnatomyStates
              variant={variant}
              scale={1}
              playhead={i === playheadStep}
              cell={cell}
              noteLabel={noteLabelForCell(cell, patch as Patch)}
              knobIndex={degreeToKnobIndex(cell.degree)}
              onCellChange={patchCell => onCellChange(i, patchCell)}
            />
          </div>
        </div>
      ))}
    </div>
  );
}
