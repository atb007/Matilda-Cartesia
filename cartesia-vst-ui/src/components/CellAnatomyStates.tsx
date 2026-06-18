import { useState, useCallback } from "react";
import { Led } from "./Led";
import { MiniKnob } from "./MiniKnob";
import { SequencerKnob } from "./SequencerKnob";
import { KNOB_COLORS, type KnobVariant } from "./knobColors";
import type { Cell } from "../engine";

const BASE_W         = 120;
const BASE_H         = 130;
const BASE_KNOB_SIZE = 74;
const BASE_KNOB_LEFT = 28;
const BASE_KNOB_TOP  = 28;
const BASE_MINI_SIZE = 24;
const BASE_MINI_GAP  = 10;

type Props = {
  variant?: KnobVariant;
  scale?: number;
  playhead?: boolean;
  cell: Cell;
  noteLabel: string;
  knobIndex: number;
  onCellChange: (patch: Partial<Cell>) => void;
};

export function CellAnatomyStates({
  variant = "orange",
  scale = 1,
  playhead = false,
  cell,
  noteLabel,
  knobIndex,
  onCellChange,
}: Props) {
  const W         = BASE_W         * scale;
  const H         = BASE_H         * scale;
  const KNOB_SIZE = BASE_KNOB_SIZE * scale;
  const KNOB_LEFT = BASE_KNOB_LEFT * scale;
  const KNOB_TOP  = BASE_KNOB_TOP  * scale;
  const KNOB_CX   = KNOB_LEFT + KNOB_SIZE / 2;
  const KNOB_CY   = KNOB_TOP  + KNOB_SIZE / 2;
  const MINI_SIZE = BASE_MINI_SIZE * scale;
  const MINI_GAP  = BASE_MINI_GAP  * scale;
  const MINI_X    = 0;
  const MINI_NP_Y = KNOB_CY - MINI_SIZE - MINI_GAP / 2;
  const MINI_JT_Y = KNOB_CY + MINI_GAP / 2;
  const [hovered, setHovered] = useState(false);

  const { gate } = cell;
  const npValue = cell.triggerArmed ? cell.triggerProb : null;
  const jtValue = cell.jitterArmed ? cell.jitterAmount : null;
  const colorConfig = KNOB_COLORS[variant];
  const showMiniKnobs = gate && (hovered || npValue !== null || jtValue !== null);

  const toggle = useCallback(() => onCellChange({ gate: !gate }), [gate, onCellChange]);
  const activateNP = useCallback(() => onCellChange({ triggerArmed: true, triggerProb: 0.5 }), [onCellChange]);
  const deactivateNP = useCallback(() => onCellChange({ triggerArmed: false }), [onCellChange]);
  const activateJT = useCallback(() => onCellChange({ jitterArmed: true, jitterAmount: 0.5 }), [onCellChange]);
  const deactivateJT = useCallback(() => onCellChange({ jitterArmed: false }), [onCellChange]);

  return (
    <div
      className="relative select-none"
      style={{ width: W, height: H }}
      onMouseEnter={() => setHovered(true)}
      onMouseLeave={() => setHovered(false)}
    >
      <p
        className="absolute font-extrabold whitespace-nowrap leading-none pointer-events-none"
        style={{
          fontFamily: "'Jost', sans-serif",
          fontSize: 16 * scale,
          top: 6,
          left: KNOB_CX - 20,
          width: 40,
          textAlign: "center",
          color: gate ? "#ffffff" : "#444",
          transition: "color 220ms ease",
        }}
      >
        {noteLabel}
      </p>

      <div style={{ position: "absolute", left: KNOB_LEFT, top: KNOB_TOP }}>
        <SequencerKnob
          defaultNoteIndex={knobIndex}
          size={KNOB_SIZE}
          active={gate}
          colorConfig={colorConfig}
          onToggle={toggle}
          onNoteChange={(_, idx) => onCellChange({ degree: idx })}
        />
      </div>

      <Led
        className="absolute"
        style={{ bottom: 20 * scale, right: 6 * scale, width: 18 * scale, height: 10 * scale }}
        lit={playhead && gate}
        litColor={colorConfig.ledColor}
      />

      {showMiniKnobs && (
        <>
          <div style={{ position: "absolute", left: MINI_X, top: MINI_NP_Y }}>
            <MiniKnob
              color="amber"
              label="Note Trigger"
              value={npValue}
              size={MINI_SIZE}
              onActivate={activateNP}
              onDeactivate={deactivateNP}
              onValueChange={v => onCellChange({ triggerArmed: true, triggerProb: v })}
            />
          </div>
          <div style={{ position: "absolute", left: MINI_X, top: MINI_JT_Y }}>
            <MiniKnob
              color="teal"
              label="Jitter"
              value={jtValue}
              size={MINI_SIZE}
              onActivate={activateJT}
              onDeactivate={deactivateJT}
              onValueChange={v => onCellChange({ jitterArmed: true, jitterAmount: v })}
            />
          </div>
        </>
      )}
    </div>
  );
}
