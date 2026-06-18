import { useRef, useState } from "react";
import { useClickOutside } from "../hooks/useClickOutside";

/**
 * M3 — Movement menu (Figma 4957:103346) + dropdown (Figma 4960:3435).
 *
 * Bar:      grunge BgTexture strip + filigree vine ornaments above/below,
 *           ◄ ► arrows cycle modes, Asimovian label with teal neon shadow.
 * Dropdown: rounded dark sheet, Kode Mono Bold uppercase items, hairline rules.
 *
 * Everything is sized off BASE_* × scale so the module vector-scales cleanly.
 */

export const MOVEMENT_MODES = [
  "forward",
  "reverse",
  "ping-pong",
  "pendulum",
  "random",
  "random skip",
] as const;
export type MovementMode = (typeof MOVEMENT_MODES)[number];

/* ── Base geometry (Figma px) ────────────────────────────────────────────── */
const BASE_W          = 514;
const BASE_H          = 89;
const FILIGREE_W      = 383.2;
const FILIGREE_H      = 26.73;
const TEXTURE_W       = 354.88;
const TEXTURE_H       = 31.31;
const TEXTURE_Y       = 29.3;
const ARROW_W         = 27.5;
const ARROW_H         = 17.78;
const ARROW_ROW_Y     = 33.96;
const ARROW_INSET_X   = 56.36;
const FONT_SIZE       = 18.875;
const TRACKING        = 0.755;

/* Dropdown base geometry */
const DD_W        = 316;
const DD_RADIUS   = 24;
const DD_PAD_Y    = 22;
const DD_ITEM_FS  = 16;
const DD_ITEM_GAP = 11;
const DD_LINE_GAP = 15;
const DD_CLOSE    = 18;

type Props = {
  value?: MovementMode;
  onChange?: (mode: MovementMode) => void;
  /** Uniform scale factor — all dimensions multiply by this. Default 1. */
  scale?: number;
};

export function MovementMenu({ value, onChange, scale = 1 }: Props) {
  const [internal, setInternal] = useState<MovementMode>("forward");
  const [open, setOpen]         = useState(false);
  const menuRef                 = useRef<HTMLDivElement>(null);

  useClickOutside(menuRef, open, () => setOpen(false));

  const mode = value ?? internal;
  const setMode = (m: MovementMode) => {
    setInternal(m);
    onChange?.(m);
  };

  const cycle = (dir: 1 | -1) => {
    const i = MOVEMENT_MODES.indexOf(mode);
    setMode(MOVEMENT_MODES[(i + dir + MOVEMENT_MODES.length) % MOVEMENT_MODES.length]);
  };

  const s = scale;

  return (
    <div
      ref={menuRef}
      className="select-none"
      style={{ position: "relative", width: BASE_W * s, height: BASE_H * s }}
    >
      {/* ── Filigree top ─────────────────────────────────────────────────── */}
      <img
        alt=""
        src="/assets/movement-filigree-top.svg"
        style={{
          position: "absolute",
          left: (BASE_W - FILIGREE_W) / 2 * s,
          top: 0,
          width: FILIGREE_W * s,
          height: FILIGREE_H * s,
          pointerEvents: "none",
        }}
      />

      {/* ── Filigree bottom (vertically flipped) ─────────────────────────── */}
      <img
        alt=""
        src="/assets/movement-filigree-bottom.svg"
        style={{
          position: "absolute",
          left: (BASE_W - FILIGREE_W) / 2 * s,
          bottom: 0,
          width: FILIGREE_W * s,
          height: FILIGREE_H * s,
          transform: "scaleY(-1)",
          pointerEvents: "none",
        }}
      />

      {/* ── BgTexture strip ──────────────────────────────────────────────── */}
      <img
        alt=""
        src="/assets/movement-bgtexture.svg"
        style={{
          position: "absolute",
          left: (BASE_W - TEXTURE_W) / 2 * s,
          top: TEXTURE_Y * s,
          width: TEXTURE_W * s,
          height: TEXTURE_H * s,
          pointerEvents: "none",
        }}
      />

      {/* ── ◄ arrow ──────────────────────────────────────────────────────── */}
      <ArrowButton
        dir="left"
        x={ARROW_INSET_X * s}
        y={ARROW_ROW_Y * s}
        w={ARROW_W * s}
        h={ARROW_H * s}
        onClick={() => cycle(-1)}
      />

      {/* ── ► arrow ──────────────────────────────────────────────────────── */}
      <ArrowButton
        dir="right"
        x={(BASE_W - ARROW_INSET_X - ARROW_W) * s}
        y={ARROW_ROW_Y * s}
        w={ARROW_W * s}
        h={ARROW_H * s}
        onClick={() => cycle(1)}
      />

      {/* ── Mode label — Asimovian, teal neon shadow ─────────────────────── */}
      <button
        onClick={() => setOpen(v => !v)}
        title="Movement mode"
        style={{
          position: "absolute",
          left: "50%",
          top: (TEXTURE_Y + TEXTURE_H / 2) * s,
          transform: "translate(-50%, -50%)",
          background: "none",
          border: "none",
          cursor: "pointer",
          padding: `${2 * s}px ${10 * s}px`,
          fontFamily: "'Asimovian', sans-serif",
          fontSize: FONT_SIZE * s,
          letterSpacing: TRACKING * s,
          color: "#ffffff",
          textShadow: `0px ${2.5 * s}px ${1.5 * s}px #77a6ab, 0px 0px ${0.5 * s}px #10ffcf`,
          whiteSpace: "nowrap",
          lineHeight: 1,
        }}
      >
        {mode === "random skip" ? "Random Skip" : mode.charAt(0).toUpperCase() + mode.slice(1)}
      </button>

      {/* ── Dropdown (Figma 4960:3435) ───────────────────────────────────── */}
      {open && (
        <div
          style={{
            position: "absolute",
            left: "50%",
            top: `calc(100% + ${4 * s}px)`,
            transform: "translateX(-50%)",
            width: DD_W * s,
            borderRadius: DD_RADIUS * s,
            /*
             * Glass material — Figma params: Light -45° @ 80%, Refraction 80,
             * Depth 85, Dispersion 50, Frost 20, Splay 0.
             *   Frost 20      → light blur (8px), content stays readable through it
             *   Light -45°    → sheen gradient + bright top/left edges (135deg light)
             *   Depth 85      → pronounced bevel: dark bottom/right inset
             *   Refraction 80 → soft inner glow across the slab
             *   Dispersion 50 → faint chromatic fringe (cool top-left, warm bottom-right)
             */
            background:
              "linear-gradient(135deg, rgba(255,255,255,0.16) 0%, rgba(255,255,255,0.05) 35%, rgba(80,80,80,0.04) 65%, rgba(0,0,0,0.12) 100%)",
            backdropFilter: "blur(8px) saturate(1.2)",
            WebkitBackdropFilter: "blur(8px) saturate(1.2)",
            border: "1px solid rgba(255,255,255,0.2)",
            boxShadow: [
              "0 16px 48px rgba(0,0,0,0.45)",                  // drop shadow
              "inset 0 1.5px 1px rgba(255,255,255,0.35)",      // light edge — top
              "inset 1.5px 0 1px rgba(255,255,255,0.14)",      // light edge — left
              "inset 0 -2.5px 4px rgba(0,0,0,0.4)",            // depth — bottom
              "inset -1.5px 0 3px rgba(0,0,0,0.22)",           // depth — right
              "inset 0 0 14px rgba(255,255,255,0.05)",         // refraction inner glow
              "inset 2px 2px 6px rgba(130,215,255,0.07)",      // dispersion — cool fringe
              "inset -2px -2px 6px rgba(255,160,110,0.07)",    // dispersion — warm fringe
            ].join(", "),
            padding: `${DD_PAD_Y * s}px 0`,
            zIndex: 50,
            overflow: "hidden",
          }}
        >
          {/* Close icon */}
          <button
            onClick={() => setOpen(false)}
            title="Close"
            style={{
              position: "absolute",
              top: 20 * s,
              right: 17 * s,
              width: DD_CLOSE * s,
              height: DD_CLOSE * s,
              background: "none",
              border: "none",
              cursor: "pointer",
              padding: 0,
            }}
          >
            <svg width={DD_CLOSE * s} height={DD_CLOSE * s} viewBox="0 0 18 18" fill="none">
              <path d="M4 4L14 14M14 4L4 14" stroke="rgba(255,255,255,0.85)" strokeWidth="1.6" strokeLinecap="round" />
            </svg>
          </button>

          <div style={{ display: "flex", flexDirection: "column", gap: DD_ITEM_GAP * s, width: "83%", margin: "0 auto" }}>
            {MOVEMENT_MODES.map(m => (
              <div key={m} style={{ display: "flex", flexDirection: "column", gap: DD_LINE_GAP * s }}>
                <button
                  onClick={() => { setMode(m); setOpen(false); }}
                  style={{
                    background: "none",
                    border: "none",
                    cursor: "pointer",
                    padding: 0,
                    fontFamily: "'Kode Mono', monospace",
                    fontWeight: 700,
                    fontSize: DD_ITEM_FS * s,
                    textTransform: "uppercase",
                    textAlign: "center",
                    color: m === mode ? "#ffffff" : "rgba(255,255,255,0.65)",
                    textShadow: m === mode
                      ? `0 ${1 * s}px ${3 * s}px rgba(0,0,0,0.7), 0 0 ${6 * s}px rgba(16,255,207,0.45)`
                      : `0 ${1 * s}px ${3 * s}px rgba(0,0,0,0.7)`,
                    transition: "color 150ms ease",
                    lineHeight: "normal",
                  }}
                >
                  {m}
                </button>
                {/* Hairline rule */}
                <div
                  style={{
                    height: 1,
                    width: "100%",
                    background: "linear-gradient(90deg, transparent 0%, rgba(255,255,255,0.22) 50%, transparent 100%)",
                  }}
                />
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
}

/** Silvery ◄ ► triangle — pure SVG so it vector-scales. */
function ArrowButton({
  dir, x, y, w, h, onClick,
}: { dir: "left" | "right"; x: number; y: number; w: number; h: number; onClick: () => void }) {
  const [hover, setHover] = useState(false);
  return (
    <button
      onClick={onClick}
      onMouseEnter={() => setHover(true)}
      onMouseLeave={() => setHover(false)}
      title={dir === "left" ? "Previous mode" : "Next mode"}
      style={{
        position: "absolute",
        left: x,
        top: y,
        width: w,
        height: h,
        background: "none",
        border: "none",
        cursor: "pointer",
        padding: 0,
        filter: hover ? "brightness(1.35)" : "none",
        transition: "filter 150ms ease",
      }}
    >
      <svg width={w} height={h} viewBox="0 0 27.5 17.78" fill="none">
        <defs>
          <linearGradient id={`arrow-${dir}`} x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor="#ececec" />
            <stop offset="100%" stopColor="#8d8d8d" />
          </linearGradient>
        </defs>
        <polygon
          points={dir === "left" ? "27.5,0 27.5,17.78 0,8.89" : "0,0 0,17.78 27.5,8.89"}
          fill={`url(#arrow-${dir})`}
        />
      </svg>
    </button>
  );
}
