import { useCallback, useEffect, useRef, useState } from "react";
import { type KnobColorConfig, KNOB_COLORS } from "./knobColors";

export const NOTES = ["C1","C2","C3","C4","C5","C6","C7","C8","C9","C10","C11","C12"];

/**
 * Rotary knob using Figma sphere/ring image layers.
 *
 * active=true  → active image set, indicator tick visible
 * active=false → off image set cross-fades in, tick hidden
 * click (no drag) → fires onToggle
 *
 * Range: C1 at 7 o'clock (−135°) → C12 at 5 o'clock (+135°)
 */

export type SequencerKnobProps = {
  defaultNoteIndex?: number;
  size?: number;
  active?: boolean;
  colorConfig?: KnobColorConfig;
  onToggle?: () => void;
  onNoteChange?: (note: string, index: number) => void;
};

export function SequencerKnob({
  defaultNoteIndex = 2,
  size = 74,
  active = true,
  colorConfig = KNOB_COLORS.orange,
  onToggle,
  onNoteChange,
}: SequencerKnobProps) {
  const [noteIndex, setNoteIndex] = useState(defaultNoteIndex);
  const dragRef = useRef<{ startY: number; startIndex: number; moved: boolean } | null>(null);

  useEffect(() => {
    setNoteIndex(defaultNoteIndex);
  }, [defaultNoteIndex]);

  const indicatorAngle = -135 + (noteIndex / 11) * 270;
  const clamp = (i: number) => Math.max(0, Math.min(11, i));

  const updateIndex = useCallback((i: number) => {
    const c = clamp(i);
    setNoteIndex(c);
    onNoteChange?.(NOTES[c], c);
  }, [onNoteChange]);

  const onMouseDown = useCallback((e: React.MouseEvent) => {
    e.preventDefault();
    dragRef.current = { startY: e.clientY, startIndex: noteIndex, moved: false };
  }, [noteIndex]);

  useEffect(() => {
    const onMove = (e: MouseEvent) => {
      if (!dragRef.current) return;
      const dy = dragRef.current.startY - e.clientY;
      if (Math.abs(dy) > 3) {
        dragRef.current.moved = true;
        if (active) updateIndex(dragRef.current.startIndex + Math.round(dy / 14));
      }
    };
    const onUp = (e: MouseEvent) => {
      if (!dragRef.current) return;
      if (!dragRef.current.moved && Math.abs(e.clientY - dragRef.current.startY) < 4) {
        onToggle?.();
      }
      dragRef.current = null;
    };
    window.addEventListener("mousemove", onMove);
    window.addEventListener("mouseup", onUp);
    return () => {
      window.removeEventListener("mousemove", onMove);
      window.removeEventListener("mouseup", onUp);
    };
  }, [active, updateIndex, onToggle]);

  const onWheel = useCallback((e: React.WheelEvent) => {
    if (!active) return;
    e.preventDefault();
    updateIndex(noteIndex + (e.deltaY < 0 ? 1 : -1));
  }, [active, noteIndex, updateIndex]);

  // ── SVG indicator geometry ──────────────────────────────────
  const cx = size / 2;
  const cy = size / 2;
  const sphereR = size * 0.424;
  const innerR  = sphereR * 0.55;
  const outerR  = sphereR * 0.90;
  const rad     = (indicatorAngle * Math.PI) / 180;
  const ix1 = cx + Math.sin(rad) * innerR;
  const iy1 = cy - Math.cos(rad) * innerR;
  const ix2 = cx + Math.sin(rad) * outerR;
  const iy2 = cy - Math.cos(rad) * outerR;
  const gradId = `tick-${colorConfig.variant}-${size}`;

  // ── Layer renderer (reused for active + off sets) ───────────
  const renderLayers = (imgs: KnobColorConfig["active"], opacity: number) => (
    <div
      style={{
        position: "absolute",
        inset: 0,
        opacity,
        transition: "opacity 220ms ease",
        pointerEvents: "none",
      }}
    >
      {/* Sphere body */}
      <div style={{ position: "absolute", inset: "7.6%" }}>
        <img alt="" src={imgs.sphere} style={{ position: "absolute", inset: "-3.56%", width: "107.12%", height: "107.12%", maxWidth: "none", display: "block" }} />
      </div>
      {/* Gloss (coloured inner circle) */}
      {imgs.glossCss ? (
        <div style={{ position: "absolute", inset: "25%", borderRadius: "50%", ...imgs.glossCss }} />
      ) : (
        <div style={{ position: "absolute", inset: "25%" }}>
          <img alt="" src={imgs.gloss} style={{ position: "absolute", inset: 0, width: "100%", height: "100%", display: "block" }} />
        </div>
      )}
      {/* Soft-light ambient glow */}
      {!imgs.glossCss && (
        <div style={{ position: "absolute", inset: "25%", mixBlendMode: "soft-light" }}>
          <img alt="" src={imgs.softLight} style={{ position: "absolute", top: "-158%", right: "-158%", bottom: "-158%", left: "-158%", width: "416%", height: "416%", maxWidth: "none", display: "block" }} />
        </div>
      )}
    </div>
  );

  return (
    <div
      className="select-none"
      style={{ width: size, height: size, position: "relative", cursor: active ? "ns-resize" : "pointer" }}
      onMouseDown={onMouseDown}
      onWheel={onWheel}
    >
      <div style={{ position: "absolute", inset: 0, borderRadius: "50%", overflow: "hidden" }}>
        {/* Ring — rendered once; greyed out in off state via CSS filter */}
        <div
          style={{
            position: "absolute",
            inset: "1.6%",
            filter: active ? "none" : "grayscale(1) brightness(0.55)",
            transition: "filter 220ms ease",
            pointerEvents: "none",
          }}
        >
          <img
            alt=""
            src={colorConfig.active.ring}
            style={{ position: "absolute", inset: "-3.13%", width: "106.26%", height: "106.26%", maxWidth: "none", display: "block" }}
          />
        </div>

        {/* Active layer — fades out when inactive */}
        {renderLayers(colorConfig.active, active ? 1 : 0)}

        {/* Off layer — fades in when inactive */}
        {renderLayers(colorConfig.off, active ? 0 : 1)}

        {/* Inset shadow overlay for off state depth */}
        <div
          style={{
            position: "absolute",
            inset: "7.6% 7.6% 7.6% 7.6%",
            borderRadius: "50%",
            opacity: active ? 0 : 1,
            transition: "opacity 220ms ease",
            boxShadow: "inset 0 12px 12px 0 rgba(0,0,0,0.85)",
            pointerEvents: "none",
          }}
        />

        {/* Indicator tick — only when active */}
        <svg
          width={size}
          height={size}
          viewBox={`0 0 ${size} ${size}`}
          style={{
            position: "absolute",
            inset: 0,
            display: "block",
            opacity: active ? 1 : 0,
            transition: "opacity 220ms ease",
          }}
        >
          <defs>
            <linearGradient id={gradId} gradientUnits="userSpaceOnUse" x1={ix1} y1={iy1} x2={ix2} y2={iy2}>
              <stop offset="0%"   stopColor={colorConfig.tickFrom} />
              <stop offset="100%" stopColor={colorConfig.tickTo} />
            </linearGradient>
          </defs>
          <line
            x1={ix1} y1={iy1} x2={ix2} y2={iy2}
            stroke={`url(#${gradId})`}
            strokeWidth="6"
            strokeLinecap="round"
          />
        </svg>
      </div>
    </div>
  );
}

export function noteFromIndex(i: number) {
  return NOTES[Math.max(0, Math.min(11, i))];
}
