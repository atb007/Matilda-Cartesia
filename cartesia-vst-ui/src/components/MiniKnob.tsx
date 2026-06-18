import { useCallback, useEffect, useRef, useState } from "react";

/**
 * Small arc-indicator knob.
 *
 * Behaviour:
 *  - Inactive (value=null): renders a grey dormant dot.
 *  - Active  (value 0–1):   partial arc filled to current value.
 *  - Click while inactive:  calls onActivate (parent sets value to 0.5).
 *  - Vertical drag while active: adjusts value (up = more, down = less).
 */

const COLORS = {
  teal:  { track: "#1a5c42", fill: "#2dca8c", dot: "#58f0b8", ring: "#1e8c62" },
  amber: { track: "#5c3a0a", fill: "#f0920a", dot: "#ffc04a", ring: "#c0720a" },
};

// Arc geometry — same range as main knob (270° travel)
const MIN_DEG = -135;
const MAX_DEG =  135;

function arcPath(
  cx: number,
  cy: number,
  r: number,
  fromDeg: number,
  toDeg: number,
): string {
  const rad = (d: number) => (d * Math.PI) / 180;
  // Convert "degrees from 12 o'clock, clockwise-positive" → SVG coords
  const sx = cx + Math.sin(rad(fromDeg)) * r;
  const sy = cy - Math.cos(rad(fromDeg)) * r;
  const ex = cx + Math.sin(rad(toDeg)) * r;
  const ey = cy - Math.cos(rad(toDeg)) * r;
  const sweep = toDeg - fromDeg;
  const large = sweep > 180 ? 1 : 0;
  return `M ${sx} ${sy} A ${r} ${r} 0 ${large} 1 ${ex} ${ey}`;
}

export type MiniKnobProps = {
  color: "teal" | "amber";
  /** Tooltip label when inactive (e.g. "Note Trigger", "Jitter"). */
  label: string;
  /** null = inactive (grey). 0–1 = active with arc fill. */
  value: number | null;
  size?: number;
  onActivate?: () => void;
  onDeactivate?: () => void;
  onValueChange?: (v: number) => void;
};

export function MiniKnob({
  color,
  label,
  value,
  size = 24,
  onActivate,
  onDeactivate,
  onValueChange,
}: MiniKnobProps) {
  const c = COLORS[color];
  const isActive = value !== null;
  const cx = size / 2;
  const cy = size / 2;
  const trackR = size / 2 - 2.5;   // arc track radius
  const dotR   = size * 0.15;       // center indicator dot

  const currentAngle = isActive
    ? MIN_DEG + (value as number) * (MAX_DEG - MIN_DEG)
    : MIN_DEG;

  const [hovered, setHovered] = useState(false);
  const [dragging, setDragging] = useState(false);

  /* ── Drag ─────────────────────────────────────────────────── */
  const dragRef = useRef<{ startY: number; startValue: number } | null>(null);

  const handleMouseDown = useCallback(
    (e: React.MouseEvent) => {
      if (!isActive) return; // click-only when inactive; let onClick handle it
      e.preventDefault();
      e.stopPropagation();
      dragRef.current = { startY: e.clientY, startValue: value as number };
      setDragging(true);
    },
    [isActive, value],
  );

  useEffect(() => {
    const onMove = (e: MouseEvent) => {
      if (!dragRef.current || !isActive) return;
      const dy = dragRef.current.startY - e.clientY; // up = more
      const delta = dy / 80; // 80px drag = full range
      const next = Math.max(0, Math.min(1, dragRef.current.startValue + delta));
      onValueChange?.(next);
    };
    const onUp = () => {
      dragRef.current = null;
      setDragging(false);
    };
    window.addEventListener("mousemove", onMove);
    window.addEventListener("mouseup", onUp);
    return () => {
      window.removeEventListener("mousemove", onMove);
      window.removeEventListener("mouseup", onUp);
    };
  }, [isActive, onValueChange]);

  const showTooltip = hovered || dragging;
  const tooltipText =
    isActive && value !== null
      ? `${Math.round(value * 100)}%`
      : label;

  const handleClick = useCallback(
    (e: React.MouseEvent) => {
      e.stopPropagation();
      if (!isActive) onActivate?.();
      else onDeactivate?.();
    },
    [isActive, onActivate, onDeactivate],
  );

  return (
    <div
      style={{ position: "relative", width: size, height: size }}
      onMouseEnter={() => setHovered(true)}
      onMouseLeave={() => setHovered(false)}
    >
      {showTooltip && (
        <div
          style={{
            position: "absolute",
            left: size + 6,
            top: "50%",
            transform: "translateY(-50%)",
            padding: "3px 7px",
            borderRadius: 4,
            background: "rgba(12, 14, 18, 0.92)",
            border: "1px solid rgba(207, 239, 243, 0.25)",
            fontFamily: "'Jost', sans-serif",
            fontSize: 11,
            fontWeight: 600,
            color: "#fff",
            whiteSpace: "nowrap",
            pointerEvents: "none",
            zIndex: 20,
            lineHeight: 1.2,
          }}
        >
          {tooltipText}
        </div>
      )}
      <svg
        width={size}
        height={size}
        viewBox={`0 0 ${size} ${size}`}
        className="select-none"
        style={{ display: "block", cursor: isActive ? "ns-resize" : "pointer", overflow: "visible" }}
        onClick={handleClick}
        onMouseDown={handleMouseDown}
      >
      <defs>
        <radialGradient id={`mk-bg-${color}`} cx="40%" cy="35%" r="65%">
          <stop offset="0%"  stopColor={isActive ? "#2a2a2a" : "#303030"} />
          <stop offset="100%" stopColor="#111" />
        </radialGradient>
      </defs>

      {/* Base circle */}
      <circle
        cx={cx} cy={cy} r={trackR}
        fill={`url(#mk-bg-${color})`}
        stroke={isActive ? c.ring : "#404040"}
        strokeWidth="1.5"
      />

      {/* Track groove (full 270° arc) */}
      <path
        d={arcPath(cx, cy, trackR, MIN_DEG, MAX_DEG)}
        fill="none"
        stroke={isActive ? c.track : "#2a2a2a"}
        strokeWidth="2"
        strokeLinecap="round"
      />

      {/* Value fill arc */}
      {isActive && value! > 0.01 && (
        <path
          d={arcPath(cx, cy, trackR, MIN_DEG, currentAngle)}
          fill="none"
          stroke={c.fill}
          strokeWidth="2.5"
          strokeLinecap="round"
        />
      )}

      {/* Center dot */}
      <circle
        cx={cx} cy={cy} r={dotR}
        fill={isActive ? c.dot : "#555"}
      />

      {/* Tick at current value tip */}
      {isActive && (
        <circle
          cx={cx + Math.sin((currentAngle * Math.PI) / 180) * trackR}
          cy={cy - Math.cos((currentAngle * Math.PI) / 180) * trackR}
          r="2"
          fill={c.dot}
        />
      )}
      </svg>
    </div>
  );
}
