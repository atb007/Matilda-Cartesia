import { useRef, useState, type CSSProperties, type ReactNode } from "react";
import { useClickOutside } from "../hooks/useClickOutside";
import {
  OCTAVES,
  PITCH_CLASSES,
  SCALES,
  formatOctaveNote,
  gemImageForScale,
  scaleDisplayName,
  type PitchClass,
  type ScaleId,
} from "../scaleConfig";
import {
  GLASS_DD_CLOSE,
  GLASS_DD_ITEM_FS,
  GLASS_DD_ITEM_GAP,
  GLASS_DD_LINE_GAP,
  GLASS_DD_SCROLL_CLASS,
  glassHairline,
  glassPanelStyle,
} from "./glassDropdownStyle";
import { GemSparks } from "./GemSparks";

/**
 * M6 — Quantise Scale panel (Figma 4976:3937).
 * Scale dropdown: Figma 4918:101473 (glass — same recipe as MovementMenu).
 *
 * Gem orb art is keyed by scale id in `scaleConfig.ts`; only Chromatic
 * is supplied for now — others fall back until more exports land.
 *
 * Min, Tonic, Max — glass dropdown; whole box hit target (chevrons step only).
 * Scale — `◄ ►` bar + glass dropdown; click outside dismisses any open menu.
 */

const BASE_W = 418;
const BASE_H = 598;

const FILIGREE_W = 383.2;
const FILIGREE_H = 26.73;

/* Title block — panel-absolute px (Figma 4976:4096 → 4976:3937) */
const TITLE_FILIGREE_LEFT = -9.88;
/* Title group y -4.997 + filigree y -4.997 → sits above rule, not on it */
const TITLE_FILIGREE_TOP  = -9.99;
const TITLE_RULE_LEFT     = 3.41;
const TITLE_RULE_TOP      = 19.28;
const TITLE_RULE_W        = 399.41;
const TITLE_RULE_H        = 31.31;
const TITLE_TEXT_TOP      = 19.11;
const TITLE_TEXT_H        = 27.808;
/* Bottom filigree hugs rule — Figma uses rotate(180) scaleX(-1), not scaleY(-1) */
const TITLE_FILIGREE_BOT_TOP = TITLE_RULE_TOP + TITLE_RULE_H + 2;

const TITLE_FS   = 18.875;
const TITLE_TRACK = 0.755;
const SCALE_BAR_FS = 20;
const SCALE_BAR_TRACK = 0.8;
const LABEL_FS = 24;
const NOTE_FS  = 18;
const TONIC_FS = 24;
/* Tonic pill — slightly larger than Figma base for hit target / balance */
const TONIC_PILL_PAD_Y = 7;
const TONIC_PILL_PAD_X = 18;
const TONIC_PILL_MIN_H = 37;
/* Figma 4976:5321 — fixed width so pill doesn't resize per pitch class */
const TONIC_PILL_W = 75;

type MenuId = "min" | "tonic" | "max" | "scale";

/* Figma slot 370×322 — exported PNG is 418×390; keep native aspect to avoid vertical squish */
const GEM_W = 370.057;
const GEM_SLOT_H = 321.92;
const GEM_H = GEM_W * (390 / 418);
const GEM_LEFT = 23.13;
const GEM_TOP  = 220.03 + (GEM_SLOT_H - GEM_H) / 2;

const MINMAX_TOP = 95;
const SCALE_BAR_TOP = 561;
const SCALE_BAR_W = 378;
const SCALE_BAR_H = 28;
const ARROW_W = 27.5;
const ARROW_H = 17.78;

type Props = {
  scale?: number;
  scaleId?: ScaleId;
  onScaleChange?: (id: ScaleId) => void;
  tonic?: PitchClass;
  onTonicChange?: (pitch: PitchClass) => void;
  minOctave?: number;
  maxOctave?: number;
  onMinOctaveChange?: (oct: number) => void;
  onMaxOctaveChange?: (oct: number) => void;
};

export function ScalePanel({
  scale = 1,
  scaleId: scaleIdProp,
  onScaleChange,
  tonic: tonicProp,
  onTonicChange,
  minOctave: minOctProp,
  maxOctave: maxOctProp,
  onMinOctaveChange,
  onMaxOctaveChange,
}: Props) {
  const [internalScale, setInternalScale] = useState<ScaleId>("chromatic");
  const [internalTonic, setInternalTonic] = useState<PitchClass>("C#");
  const [internalMinOct, setInternalMinOct] = useState(1);
  const [internalMaxOct, setInternalMaxOct] = useState(9);
  const [openMenu, setOpenMenu] = useState<MenuId | null>(null);

  const scaleId = scaleIdProp ?? internalScale;
  const tonic   = tonicProp ?? internalTonic;
  const minOct  = minOctProp ?? internalMinOct;
  const maxOct  = maxOctProp ?? internalMaxOct;

  const setScaleId = (id: ScaleId) => { setInternalScale(id); onScaleChange?.(id); };
  const setTonic = (p: PitchClass) => { setInternalTonic(p); onTonicChange?.(p); };
  const setMinOct = (o: number) => { setInternalMinOct(o); onMinOctaveChange?.(o); };
  const setMaxOct = (o: number) => { setInternalMaxOct(o); onMaxOctaveChange?.(o); };

  const s = scale;
  const scaleIdx = SCALES.indexOf(scaleId);

  const cycleScale = (dir: 1 | -1) => {
    setScaleId(SCALES[(scaleIdx + dir + SCALES.length) % SCALES.length]);
  };

  const tonicIdx = PITCH_CLASSES.indexOf(tonic);

  const scaleMenuRef = useRef<HTMLDivElement>(null);

  const toggleMenu = (id: MenuId) => setOpenMenu(prev => (prev === id ? null : id));
  const closeMenu = () => setOpenMenu(null);

  useClickOutside(scaleMenuRef, openMenu === "scale", closeMenu);

  const cycleTonic = (dir: 1 | -1) => {
    setTonic(PITCH_CLASSES[(tonicIdx + dir + 12) % 12]);
  };

  const tealGlow = `0px ${2.5 * s}px ${1.5 * s}px #77a6ab, 0px 0px ${0.5 * s}px #10ffcf`;

  return (
    <div
      className="select-none"
      style={{ position: "relative", width: BASE_W * s, height: BASE_H * s, overflow: "visible" }}
    >
      {/* ── Title filigree + rule + label (Figma 4976:4096) ───────────────── */}
      <img
        alt=""
        src="/assets/scale-title-filigree-top.svg"
        style={{
          position: "absolute",
          left: TITLE_FILIGREE_LEFT * s,
          top: TITLE_FILIGREE_TOP * s,
          width: FILIGREE_W * s,
          height: FILIGREE_H * s,
          pointerEvents: "none",
        }}
      />
      <img
        alt=""
        src="/assets/scale-title-filigree-top.svg"
        style={{
          position: "absolute",
          left: (TITLE_FILIGREE_LEFT + 2.35) * s,
          top: TITLE_FILIGREE_BOT_TOP * s,
          width: FILIGREE_W * s,
          height: FILIGREE_H * s,
          transform: "rotate(180deg) scaleX(-1)",
          transformOrigin: "center center",
          pointerEvents: "none",
        }}
      />
      <img
        alt=""
        src="/assets/scale-title-rule.svg"
        style={{
          position: "absolute",
          left: TITLE_RULE_LEFT * s,
          top: TITLE_RULE_TOP * s,
          width: TITLE_RULE_W * s,
          height: TITLE_RULE_H * s,
          pointerEvents: "none",
        }}
      />
      <div
        style={{
          position: "absolute",
          left: 0,
          top: TITLE_TEXT_TOP * s,
          width: BASE_W * s,
          height: TITLE_TEXT_H * s,
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          pointerEvents: "none",
        }}
      >
        <p
          style={{
            margin: 0,
            fontFamily: "'Asimovian', sans-serif",
            fontSize: TITLE_FS * s,
            letterSpacing: TITLE_TRACK * s,
            color: "#ffffff",
            textShadow: tealGlow,
            whiteSpace: "nowrap",
            lineHeight: 1,
          }}
        >
          Quantise Scale
        </p>
      </div>

      {/* ── Min / Tonic / Max row ─────────────────────────────────────────── */}
      <div style={{ position: "absolute", left: 4 * s, top: MINMAX_TOP * s, width: 394 * s }}>
        {/* Label row */}
        <div
          style={{
            display: "flex",
            alignItems: "center",
            justifyContent: "space-between",
            width: "100%",
            marginBottom: 11 * s,
          }}
        >
          <img alt="" src="/assets/scale-minmax-ornament-left.svg" style={{ width: 61.22 * s, height: 19.94 * s }} />
          <span style={{ fontFamily: "'Supermercado One', cursive", fontSize: LABEL_FS * s, color: "rgba(255,255,255,0.7)" }}>Min</span>
          <DividerLine s={s} />
          <span style={{ fontFamily: "'Supermercado One', cursive", fontSize: LABEL_FS * s, color: "rgba(255,255,255,0.7)" }}>Tonic</span>
          <DividerLine s={s} />
          <span style={{ fontFamily: "'Supermercado One', cursive", fontSize: LABEL_FS * s, color: "rgba(255,255,255,0.7)" }}>Max</span>
          <img alt="" src="/assets/scale-minmax-ornament-right.svg" style={{ width: 61.22 * s, height: 19.94 * s, transform: "scaleX(-1)" }} />
        </div>

        {/* Pickers row — Min / Tonic / Max glass dropdowns + chevron step */}
        <div style={{ display: "flex", alignItems: "center", justifyContent: "center", gap: 11 * s }}>
          <PickerDropdown
            s={s}
            variant="box"
            label={formatOctaveNote(tonic, minOct)}
            fontSize={NOTE_FS}
            open={openMenu === "min"}
            onToggle={() => toggleMenu("min")}
            onClose={closeMenu}
            menuWidth={128}
            onStepUp={() => setMinOct(Math.min(maxOct - 1, minOct + 1))}
            onStepDown={() => setMinOct(Math.max(0, minOct - 1))}
          >
            {OCTAVES.filter(o => o < maxOct).map(oct => (
              <DropdownItem
                key={oct}
                s={s}
                label={formatOctaveNote(tonic, oct)}
                selected={oct === minOct}
                onPick={() => { setMinOct(oct); closeMenu(); }}
              />
            ))}
          </PickerDropdown>

          <img alt="" src="/assets/scale-connector-left.svg" style={{ width: 23.59 * s, height: 2 * s }} />

          <PickerDropdown
            s={s}
            variant="pill"
            label={tonic}
            open={openMenu === "tonic"}
            onToggle={() => toggleMenu("tonic")}
            onClose={closeMenu}
            menuWidth={100}
            onStepUp={() => cycleTonic(1)}
            onStepDown={() => cycleTonic(-1)}
          >
            {PITCH_CLASSES.map(p => (
              <DropdownItem
                key={p}
                s={s}
                label={p}
                selected={p === tonic}
                onPick={() => { setTonic(p); closeMenu(); }}
              />
            ))}
          </PickerDropdown>

          <img alt="" src="/assets/scale-connector-right.svg" style={{ width: 23.59 * s, height: 2 * s }} />

          <PickerDropdown
            s={s}
            variant="box"
            label={formatOctaveNote(tonic, maxOct)}
            fontSize={NOTE_FS}
            open={openMenu === "max"}
            onToggle={() => toggleMenu("max")}
            onClose={closeMenu}
            menuWidth={128}
            onStepUp={() => setMaxOct(Math.min(9, maxOct + 1))}
            onStepDown={() => setMaxOct(Math.max(minOct + 1, maxOct - 1))}
          >
            {OCTAVES.filter(o => o > minOct).map(oct => (
              <DropdownItem
                key={oct}
                s={s}
                label={formatOctaveNote(tonic, oct)}
                selected={oct === maxOct}
                onPick={() => { setMaxOct(oct); closeMenu(); }}
              />
            ))}
          </PickerDropdown>
        </div>
      </div>

      {/* ── Scale gem orb + sparse orbital sparks ─────────────────────────── */}
      <div
        style={{
          position: "absolute",
          left: GEM_LEFT * s,
          top: GEM_TOP * s,
          width: GEM_W * s,
          height: GEM_H * s,
          overflow: "visible",
          pointerEvents: "none",
          isolation: "isolate",
        }}
      >
        <GemSparks
          scaleId={scaleId}
          width={GEM_W * s}
          height={GEM_H * s}
          panelScale={s}
        />
        <img
          alt=""
          src={gemImageForScale(scaleId)}
          style={{
            position: "relative",
            zIndex: 2,
            width: "100%",
            height: "100%",
            objectFit: "contain",
            filter: "drop-shadow(-10px 5px 34px rgba(0,0,0,0.5))",
          }}
        />
      </div>

      {/* ── Scale selector bar ◄ Chromatic ► + dropdown ───────────────────── */}
      <div
        ref={scaleMenuRef}
        style={{
          position: "absolute",
          left: 19.78 * s,
          top: SCALE_BAR_TOP * s,
          width: SCALE_BAR_W * s,
        }}
      >
        <div
          style={{
            width: "100%",
            height: SCALE_BAR_H * s,
            display: "flex",
            alignItems: "center",
            justifyContent: "space-between",
            padding: "5px",
          }}
        >
          <ScaleArrow s={s} dir="left" onClick={() => cycleScale(-1)} />
          <button
            type="button"
            onClick={() => toggleMenu("scale")}
            title="Scale"
            style={{
              flex: 1,
              alignSelf: "stretch",
              display: "flex",
              alignItems: "center",
              justifyContent: "center",
              background: "none",
              border: "none",
              cursor: "pointer",
              padding: `2px ${10 * s}px`,
              fontFamily: "'Kode Mono', monospace",
              fontWeight: 600,
              fontSize: SCALE_BAR_FS * s,
              letterSpacing: SCALE_BAR_TRACK * s,
              color: "#ffffff",
              textShadow: tealGlow,
              whiteSpace: "nowrap",
              lineHeight: 1,
              minWidth: 0,
            }}
          >
            {scaleDisplayName(scaleId)}
          </button>
          <ScaleArrow s={s} dir="right" onClick={() => cycleScale(1)} />
        </div>

        {openMenu === "scale" && (
          <div
            style={{
              position: "absolute",
              left: "50%",
              top: (SCALE_BAR_H + 4) * s,
              transform: "translateX(-50%)",
              ...glassPanelStyle(s),
            }}
          >
          <button
            onClick={closeMenu}
            title="Close"
            style={{
              position: "absolute",
              top: 20 * s,
              right: 17 * s,
              width: GLASS_DD_CLOSE * s,
              height: GLASS_DD_CLOSE * s,
              background: "none",
              border: "none",
              cursor: "pointer",
              padding: 0,
            }}
          >
            <svg width={GLASS_DD_CLOSE * s} height={GLASS_DD_CLOSE * s} viewBox="0 0 18 18" fill="none">
              <path d="M4 4L14 14M14 4L4 14" stroke="rgba(255,255,255,0.85)" strokeWidth="1.6" strokeLinecap="round" />
            </svg>
          </button>

          <div style={{ display: "flex", flexDirection: "column", gap: GLASS_DD_ITEM_GAP * s, width: "83%", margin: "0 auto" }}>
            {SCALES.map(id => (
              <div key={id} style={{ display: "flex", flexDirection: "column", gap: GLASS_DD_LINE_GAP * s }}>
                <button
                  onClick={() => { setScaleId(id); closeMenu(); }}
                  style={{
                    background: "none",
                    border: "none",
                    cursor: "pointer",
                    padding: 0,
                    fontFamily: "'Kode Mono', monospace",
                    fontWeight: 700,
                    fontSize: GLASS_DD_ITEM_FS * s,
                    textTransform: "uppercase",
                    textAlign: "center",
                    color: id === scaleId ? "#ffffff" : "rgba(255,255,255,0.65)",
                    textShadow: id === scaleId
                      ? `0 ${1 * s}px ${3 * s}px rgba(0,0,0,0.7), 0 0 ${6 * s}px rgba(16,255,207,0.45)`
                      : `0 ${1 * s}px ${3 * s}px rgba(0,0,0,0.7)`,
                    transition: "color 150ms ease",
                    lineHeight: "normal",
                  }}
                >
                  {id}
                </button>
                <div style={glassHairline()} />
              </div>
            ))}
          </div>
        </div>
        )}
      </div>
    </div>
  );
}

function DividerLine({ s }: { s: number }) {
  return (
    <div style={{ width: 39 * s, height: 2 * s, background: "linear-gradient(90deg, transparent, rgba(255,255,255,0.35), transparent)" }} />
  );
}

/** Figma 4976:3988 — top chevron normal, bottom chevron `-scale-y-100`. */
function ChevronStack({ s, onUp, onDown }: { s: number; onUp: () => void; onDown: () => void }) {
  const tri = (flipY: boolean) => (
    <svg width={6 * s} height={3 * s} viewBox="0 0 6 3" style={flipY ? { transform: "scaleY(-1)" } : undefined}>
      <polygon points="3,0 6,3 0,3" fill="rgba(255,255,255,0.85)" />
    </svg>
  );
  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 5 * s }}>
      <button type="button" onClick={e => { e.stopPropagation(); onUp(); }} style={chevronBtn} aria-label="Increase">
        {tri(false)}
      </button>
      <button type="button" onClick={e => { e.stopPropagation(); onDown(); }} style={chevronBtn} aria-label="Decrease">
        {tri(true)}
      </button>
    </div>
  );
}

const chevronBtn: CSSProperties = {
  background: "none",
  border: "none",
  cursor: "pointer",
  padding: 0,
  lineHeight: 0,
};

function PickerDropdown({
  s,
  variant,
  label,
  fontSize = NOTE_FS,
  open,
  onToggle,
  onClose,
  menuWidth,
  onStepUp,
  onStepDown,
  children,
}: {
  s: number;
  variant: "box" | "pill";
  label: string;
  fontSize?: number;
  open: boolean;
  onToggle: () => void;
  onClose: () => void;
  menuWidth: number;
  onStepUp: () => void;
  onStepDown: () => void;
  children: ReactNode;
}) {
  const shellStyle: CSSProperties =
    variant === "pill"
      ? {
          display: "flex",
          width: TONIC_PILL_W * s,
          minHeight: TONIC_PILL_MIN_H * s,
          alignItems: "center",
          gap: 10 * s,
          borderRadius: 31 * s,
          background: "rgba(221, 216, 216, 0.09)",
          boxShadow:
            `inset 0 ${-2 * s}px ${3 * s}px 0 rgba(0, 0, 0, 0.31), inset 0 ${4 * s}px ${3 * s}px 0 rgba(0, 0, 0, 0.25)`,
          overflow: "hidden",
          flexShrink: 0,
        }
      : {
          display: "flex",
          alignItems: "center",
          gap: 10 * s,
          minHeight: 31 * s,
          borderRadius: 8 * s,
          border: "1px solid #cfeff3",
          background: "rgba(184,184,184,0.1)",
          overflow: "hidden",
        };

  const labelFs = variant === "pill" ? TONIC_FS : fontSize;
  const togglePad =
    variant === "pill"
      ? `${TONIC_PILL_PAD_Y * s}px ${TONIC_PILL_PAD_X * s}px`
      : `${4 * s}px ${8 * s}px`;
  const rootRef = useRef<HTMLDivElement>(null);
  useClickOutside(rootRef, open, onClose);

  return (
    <div ref={rootRef} style={{ position: "relative" }}>
      <div style={shellStyle}>
        <button
          type="button"
          onClick={onToggle}
          title="Open menu"
          style={{
            flex: 1,
            alignSelf: "stretch",
            display: "flex",
            alignItems: "center",
            justifyContent: "center",
            minWidth: 0,
            background: "none",
            border: "none",
            cursor: "pointer",
            padding: togglePad,
            fontFamily: "'Jost', sans-serif",
            fontWeight: 600,
            fontSize: labelFs * s,
            color: "#fff",
            whiteSpace: "nowrap",
            lineHeight: 1,
          }}
        >
          {label}
        </button>
        <div style={{ flexShrink: 0, paddingRight: variant === "pill" ? TONIC_PILL_PAD_X * s : 8 * s }}>
          <ChevronStack s={s} onUp={onStepUp} onDown={onStepDown} />
        </div>
      </div>
      {open && (
        <div
          className={GLASS_DD_SCROLL_CLASS}
          style={{
            position: "absolute",
            left: "50%",
            top: 35 * s,
            transform: "translateX(-50%)",
            ...glassPanelStyle(s),
            width: menuWidth * s,
            padding: `${14 * s}px 0`,
            maxHeight: 220 * s,
            overflowY: "auto",
          }}
        >
          <button
            onClick={onClose}
            title="Close"
            style={{
              position: "absolute",
              top: 10 * s,
              right: 10 * s,
              width: GLASS_DD_CLOSE * s,
              height: GLASS_DD_CLOSE * s,
              background: "none",
              border: "none",
              cursor: "pointer",
              padding: 0,
              zIndex: 1,
            }}
          >
            <svg width={GLASS_DD_CLOSE * s} height={GLASS_DD_CLOSE * s} viewBox="0 0 18 18" fill="none">
              <path d="M4 4L14 14M14 4L4 14" stroke="rgba(255,255,255,0.85)" strokeWidth="1.6" strokeLinecap="round" />
            </svg>
          </button>
          <div style={{ display: "flex", flexDirection: "column", gap: GLASS_DD_ITEM_GAP * s, width: "86%", margin: "0 auto" }}>
            {children}
          </div>
        </div>
      )}
    </div>
  );
}

function DropdownItem({
  s, label, selected, onPick,
}: { s: number; label: string; selected: boolean; onPick: () => void }) {
  return (
    <div style={{ display: "flex", flexDirection: "column", gap: GLASS_DD_LINE_GAP * s }}>
      <button
        type="button"
        onClick={onPick}
        style={{
          background: "none",
          border: "none",
          cursor: "pointer",
          padding: 0,
          fontFamily: "'Kode Mono', monospace",
          fontWeight: 700,
          fontSize: GLASS_DD_ITEM_FS * s,
          textAlign: "center",
          color: selected ? "#ffffff" : "rgba(255,255,255,0.65)",
          textShadow: selected
            ? `0 ${1 * s}px ${3 * s}px rgba(0,0,0,0.7), 0 0 ${6 * s}px rgba(16,255,207,0.45)`
            : `0 ${1 * s}px ${3 * s}px rgba(0,0,0,0.7)`,
          lineHeight: "normal",
        }}
      >
        {label}
      </button>
      <div style={glassHairline()} />
    </div>
  );
}

/** ◄ ► triangles — same geometry as MovementMenu (points outward left/right). */
function ScaleArrow({ s, dir, onClick }: { s: number; dir: "left" | "right"; onClick: () => void }) {
  const [hover, setHover] = useState(false);
  const gradId = `scale-arrow-${dir}`;
  return (
    <button
      onClick={onClick}
      onMouseEnter={() => setHover(true)}
      onMouseLeave={() => setHover(false)}
      title={dir === "left" ? "Previous scale" : "Next scale"}
      style={{
        width: ARROW_W * s,
        height: ARROW_H * s,
        background: "none",
        border: "none",
        cursor: "pointer",
        padding: 0,
        filter: hover ? "brightness(1.35)" : "none",
        transition: "filter 150ms ease",
      }}
    >
      <svg width={ARROW_W * s} height={ARROW_H * s} viewBox="0 0 27.5 17.78" fill="none">
        <defs>
          <linearGradient id={gradId} x1="0" y1="0" x2="0" y2="1">
            <stop offset="0%" stopColor="#ececec" />
            <stop offset="100%" stopColor="#8d8d8d" />
          </linearGradient>
        </defs>
        <polygon
          points={dir === "left" ? "27.5,0 27.5,17.78 0,8.89" : "0,0 0,17.78 27.5,8.89"}
          fill={`url(#${gradId})`}
        />
      </svg>
    </button>
  );
}
