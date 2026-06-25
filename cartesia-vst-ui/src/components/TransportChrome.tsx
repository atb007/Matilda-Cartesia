import { useRef, useState, type CSSProperties } from "react";
import type { ReactNode } from "react";
import { useClickOutside } from "../hooks/useClickOutside";
import {
  CLOCK_DIVISIONS,
  PLAY_MODES,
  playModeLabel,
  type ClockDivisionId,
  type PlayModeId,
} from "../transportConfig";
import {
  GLASS_DD_CLOSE,
  GLASS_DD_ITEM_FS,
  GLASS_DD_ITEM_GAP,
  GLASS_DD_LINE_GAP,
  GLASS_DD_SCROLL_CLASS,
  glassHairline,
  glassPanelStyle,
} from "./glassDropdownStyle";

/**
 * M7 — Global Settings / transport chrome (Figma 4991:4644).
 * Play/pause gem · Play Mode · Clock — dropdown boxes with chevron step.
 */

const BASE_W = 439;
const BASE_H = 541;

const TITLE_FS = 18.875;
const TITLE_TRACK = 0.755;
const LABEL_FS = 24;
const VALUE_FS = 20;

/* Main Title block — Figma 4976:3577 (panel 4991:4644) */
const TITLE_FILIGREE_W = 383.087;
const TITLE_FILIGREE_H = 26.287;
const TITLE_FILIGREE_TOP_LEFT = 32.52;
const TITLE_FILIGREE_BOT_LEFT = 30.17;
const TITLE_FILIGREE_BOT_TOP = 61.05; /* inset 12.26% of panel — sits just below rule */
const TITLE_RULE_W = 399.41;
const TITLE_RULE_H = 31.312;
const TITLE_RULE_LEFT = 24.623;
const TITLE_RULE_TOP = 28.84;
const TITLE_TEXT_TOP = 27.66;
const TITLE_TEXT_H = 27.808;

const PLAY_MODE_TITLE_W = 264.18;

const COL_TOP = 127.55;
const COL_GAP = 28;
const ROW_GAP = 12;
const PLAY_SIZE = 156.37;
/* GlassBg 4992:5113 — same inset as Figma Rectangle inside playPause 4990:5897 */
const GLASS_INSET = { top: 7.66, right: 7.35, bottom: 7.82, left: 6.1 };
const PLAY_MODE_W = 277.5;
const CLOCK_W = 251.169;

type MenuId = "playMode" | "clock";

type Props = {
  scale?: number;
  playing?: boolean;
  onPlayingChange?: (playing: boolean) => void;
  playMode?: PlayModeId;
  onPlayModeChange?: (mode: PlayModeId) => void;
  clockDivision?: ClockDivisionId;
  onClockDivisionChange?: (id: ClockDivisionId) => void;
  dawSync?: boolean;
  onDawSyncChange?: (enabled: boolean) => void;
};

export function TransportChrome({
  scale = 1,
  playing: playingProp,
  onPlayingChange,
  playMode: playModeProp,
  onPlayModeChange,
  clockDivision: clockProp,
  onClockDivisionChange,
  dawSync: dawSyncProp,
  onDawSyncChange,
}: Props) {
  const [internalPlaying, setInternalPlaying] = useState(false);
  const [internalPlayMode, setInternalPlayMode] = useState<PlayModeId>("transport");
  const [internalClock, setInternalClock] = useState<ClockDivisionId>("1/16");
  const [internalDawSync, setInternalDawSync] = useState(true);
  const [openMenu, setOpenMenu] = useState<MenuId | null>(null);

  const playing = playingProp ?? internalPlaying;
  const playMode = playModeProp ?? internalPlayMode;
  const clockId = clockProp ?? internalClock;
  const dawSync = dawSyncProp ?? internalDawSync;

  const setPlaying = (v: boolean) => { setInternalPlaying(v); onPlayingChange?.(v); };
  const setPlayMode = (m: PlayModeId) => { setInternalPlayMode(m); onPlayModeChange?.(m); };
  const setClock = (id: ClockDivisionId) => { setInternalClock(id); onClockDivisionChange?.(id); };
  const setDawSync = (v: boolean) => { setInternalDawSync(v); onDawSyncChange?.(v); };

  const s = scale;
  const playModeIdx = PLAY_MODES.indexOf(playMode);
  const clockIdx = CLOCK_DIVISIONS.findIndex(d => d.id === clockId);

  const toggleMenu = (id: MenuId) => setOpenMenu(prev => (prev === id ? null : id));
  const closeMenu = () => setOpenMenu(null);

  const cyclePlayMode = (dir: 1 | -1) => {
    setPlayMode(PLAY_MODES[(playModeIdx + dir + PLAY_MODES.length) % PLAY_MODES.length]);
  };

  const cycleClock = (dir: 1 | -1) => {
    const next = (clockIdx + dir + CLOCK_DIVISIONS.length) % CLOCK_DIVISIONS.length;
    setClock(CLOCK_DIVISIONS[next].id);
  };

  const tealGlow = `0px ${2.5 * s}px ${1.5 * s}px #77a6ab, 0px 0px ${0.5 * s}px #10ffcf`;
  const colLeft = (BASE_W - PLAY_MODE_W) / 2;

  return (
    <div
      className="select-none"
      style={{ position: "relative", width: BASE_W * s, height: BASE_H * s, overflow: "visible" }}
    >
      {/* ── Title (Figma 4976:3577) ───────────────────────────────────────── */}
      <div
        style={{
          position: "absolute",
          left: TITLE_FILIGREE_TOP_LEFT * s,
          top: 0,
          width: TITLE_FILIGREE_W * s,
          height: TITLE_FILIGREE_H * s,
          pointerEvents: "none",
        }}
      >
        <img
          alt=""
          src="/assets/transport-title-filigree.svg"
          style={{ position: "absolute", inset: 0, width: "100%", height: "100%", display: "block" }}
        />
      </div>
      <div
        style={{
          position: "absolute",
          left: TITLE_FILIGREE_BOT_LEFT * s,
          top: TITLE_FILIGREE_BOT_TOP * s,
          width: TITLE_FILIGREE_W * s,
          height: TITLE_FILIGREE_H * s,
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          pointerEvents: "none",
        }}
      >
        <div
          style={{
            width: "100%",
            height: "100%",
            transform: "rotate(180deg) scaleX(-1)",
            transformOrigin: "center center",
          }}
        >
          <img
            alt=""
            src="/assets/transport-title-filigree.svg"
            style={{ position: "absolute", inset: 0, width: "100%", height: "100%", display: "block" }}
          />
        </div>
      </div>
      <div
        style={{
          position: "absolute",
          left: TITLE_RULE_LEFT * s,
          top: TITLE_RULE_TOP * s,
          width: TITLE_RULE_W * s,
          height: TITLE_RULE_H * s,
          pointerEvents: "none",
        }}
      >
        <div style={{ position: "absolute", top: "-0.92%", left: 0, right: 0, bottom: 0 }}>
          <img
            alt=""
            src="/assets/transport-title-rule.svg"
            style={{ display: "block", width: "100%", height: "100%" }}
          />
        </div>
      </div>
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
          Global Settings
        </p>
      </div>

      {/* ── Column: play · play mode · clock ─────────────────────────────── */}
      <div
        style={{
          position: "absolute",
          left: colLeft * s,
          top: COL_TOP * s,
          width: PLAY_MODE_W * s,
          display: "flex",
          flexDirection: "column",
          alignItems: "center",
          gap: COL_GAP * s,
        }}
      >
        <PlayPauseGem
          s={s}
          playing={playing}
          linked={dawSync}
          onToggle={() => setPlaying(!playing)}
        />

        {/* Play Mode */}
        <SectionBlock s={s} width={PLAY_MODE_W} titleVariant="playMode">
          <SettingDropdown
            s={s}
            width={PLAY_MODE_W}
            label={playModeLabel(playMode)}
            open={openMenu === "playMode"}
            onToggle={() => toggleMenu("playMode")}
            onClose={closeMenu}
            onStepUp={() => cyclePlayMode(1)}
            onStepDown={() => cyclePlayMode(-1)}
          >
            {PLAY_MODES.map(id => (
              <MenuItem
                key={id}
                s={s}
                label={playModeLabel(id)}
                selected={id === playMode}
                onPick={() => { setPlayMode(id); closeMenu(); }}
              />
            ))}
          </SettingDropdown>
        </SectionBlock>

        {/* Clock */}
        <SectionBlock s={s} width={CLOCK_W} titleVariant="clock">
          <SettingDropdown
            s={s}
            width={CLOCK_W}
            label={clockId}
            open={openMenu === "clock"}
            onToggle={() => toggleMenu("clock")}
            onClose={closeMenu}
            onStepUp={() => cycleClock(1)}
            onStepDown={() => cycleClock(-1)}
          >
            {CLOCK_DIVISIONS.map(d => (
              <MenuItem
                key={d.id}
                s={s}
                label={d.label}
                selected={d.id === clockId}
                onPick={() => { setClock(d.id); closeMenu(); }}
              />
            ))}
          </SettingDropdown>
        </SectionBlock>

        {/* DAW Sync — host transport follow (VST) */}
        <SectionBlock s={s} width={CLOCK_W} titleVariant="dawSync">
          <SyncToggleRow s={s} width={CLOCK_W} enabled={dawSync} onToggle={() => setDawSync(!dawSync)} />
        </SectionBlock>
      </div>
    </div>
  );
}

const glassInsetStyle = (): CSSProperties => ({
  position: "absolute",
  top: `${GLASS_INSET.top}%`,
  right: `${GLASS_INSET.right}%`,
  bottom: `${GLASS_INSET.bottom}%`,
  left: `${GLASS_INSET.left}%`,
});

/** GlassBg + border deco + centered Vector play/stop — Figma 4992:5113 / 4990:6150 */
function PlayPauseGem({
  s,
  playing,
  linked = false,
  onToggle,
}: {
  s: number;
  playing: boolean;
  linked?: boolean;
  onToggle: () => void;
}) {
  return (
    <button
      type="button"
      onClick={onToggle}
      title={linked ? "DAW Sync on — follows host transport in plugin" : playing ? "Stop" : "Play"}
      style={{
        position: "relative",
        width: PLAY_SIZE * s,
        height: PLAY_SIZE * s,
        padding: 0,
        border: "none",
        background: "transparent",
        cursor: "pointer",
        lineHeight: 0,
      }}
    >
      <div style={{ ...glassInsetStyle(), pointerEvents: "none" }}>
        <img
          alt=""
          src="/assets/transport-glass-bg.png"
          style={{
            position: "absolute",
            inset: 0,
            width: "100%",
            height: "100%",
            objectFit: "fill",
            display: "block",
          }}
        />
      </div>
      <img
        alt=""
        src="/assets/transport-play-frame.png"
        style={{
          position: "absolute",
          inset: 0,
          width: "100%",
          height: "100%",
          objectFit: "contain",
          pointerEvents: "none",
        }}
      />
      <div style={{ ...glassInsetStyle(), pointerEvents: "none" }}>
        <img
          alt=""
          src={playing ? "/assets/transport-stop-icon.png" : "/assets/transport-play-icon.png"}
          style={{
            position: "absolute",
            left: "50%",
            top: "50%",
            transform: "translate(-50%, -50%)",
            width: playing ? "47.3%" : "42.9%",
            height: playing ? "48.4%" : "54.5%",
            objectFit: "contain",
            objectPosition: "center",
          }}
        />
      </div>
    </button>
  );
}

function SectionBlock({
  s,
  width,
  titleVariant,
  children,
}: {
  s: number;
  width: number;
  titleVariant: "playMode" | "clock" | "dawSync";
  children: ReactNode;
}) {
  if (titleVariant === "playMode") {
    return (
      <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: ROW_GAP * s, width: width * s }}>
        {/* Figma 4976:3838 — left ornament + divider + label + right ornament/divider */}
        <div
          style={{
            display: "flex",
            alignItems: "center",
            gap: 4.5 * s,
            width: PLAY_MODE_TITLE_W * s,
          }}
        >
          <div style={{ display: "flex", alignItems: "center", gap: 6 * s, flex: 1, minWidth: 0 }}>
            <img alt="" src="/assets/transport-playmode-ornament-left.svg" style={{ width: 38.32 * s, height: 20 * s, flexShrink: 0 }} />
            <img
              alt=""
              src="/assets/transport-playmode-divider.svg"
              style={{
                width: 28.52 * s,
                height: 3.35 * s,
                flexShrink: 0,
                transform: "scaleY(-1) rotate(180deg)",
              }}
            />
            <span style={{ fontFamily: "'Supermercado One', cursive", fontSize: LABEL_FS * s, color: "rgba(255,255,255,0.7)", whiteSpace: "nowrap" }}>
              Play Mode
            </span>
          </div>
          <img
            alt=""
            src="/assets/transport-playmode-ornament-right.svg"
            style={{ width: 72.84 * s, height: 20 * s, flexShrink: 0, transform: "scaleY(-1) rotate(180deg)" }}
          />
        </div>
        {children}
      </div>
    );
  }

  return (
    <div style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: ROW_GAP * s, width: width * s }}>
      <div style={{ display: "flex", alignItems: "center", justifyContent: "space-between", width: "100%" }}>
        <img alt="" src="/assets/transport-ornament-clock-left.svg" style={{ width: 88.32 * s, height: 20 * s }} />
        <span style={{ fontFamily: "'Supermercado One', cursive", fontSize: LABEL_FS * s, color: "rgba(255,255,255,0.7)", whiteSpace: "nowrap" }}>
          {titleVariant === "dawSync" ? "DAW Sync" : "Clock"}
        </span>
        <img alt="" src="/assets/transport-ornament-clock-right.svg" style={{ width: 88.32 * s, height: 20 * s, transform: "scaleY(-1) rotate(180deg)" }} />
      </div>
      {children}
    </div>
  );
}

function SyncToggleRow({
  s,
  width,
  enabled,
  onToggle,
}: {
  s: number;
  width: number;
  enabled: boolean;
  onToggle: () => void;
}) {
  return (
    <button
      type="button"
      onClick={onToggle}
      style={{
        position: "relative",
        width: width * s,
        height: (VALUE_FS + 12) * s,
        padding: 0,
        border: "none",
        background: "transparent",
        cursor: "pointer",
      }}
    >
      <div
        style={{
          ...glassPanelStyle(s),
          position: "absolute",
          inset: 0,
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          borderRadius: 12 * s,
        }}
      >
        <span
          style={{
            fontFamily: "'Kode Mono', monospace",
            fontWeight: 700,
            fontSize: VALUE_FS * s,
            color: "#fff",
          }}
        >
          {enabled ? "On" : "Off"}
        </span>
      </div>
    </button>
  );
}

function SettingDropdown({
  s,
  width,
  label,
  open,
  onToggle,
  onClose,
  onStepUp,
  onStepDown,
  children,
}: {
  s: number;
  width: number;
  label: string;
  open: boolean;
  onToggle: () => void;
  onClose: () => void;
  onStepUp: () => void;
  onStepDown: () => void;
  children: ReactNode;
}) {
  const rootRef = useRef<HTMLDivElement>(null);
  useClickOutside(rootRef, open, onClose);

  const shell: CSSProperties = {
    display: "flex",
    alignItems: "center",
    gap: 15 * s,
    width: "100%",
    padding: `${6 * s}px ${15 * s}px ${6 * s}px ${39 * s}px`,
    borderRadius: 12 * s,
    border: `${1.5 * s}px solid #cfeff3`,
    background: "rgba(184,184,184,0.1)",
    overflow: "hidden",
  };

  return (
    <div ref={rootRef} style={{ position: "relative", width: width * s }}>
      <div style={shell}>
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
            padding: 0,
            fontFamily: "'Kode Mono', monospace",
            fontWeight: 600,
            fontSize: VALUE_FS * s,
            color: "#fff",
            lineHeight: 1,
          }}
        >
          {label}
        </button>
        <ChevronPair s={s} onUp={onStepUp} onDown={onStepDown} />
      </div>
      {open && (
        <div
          className={GLASS_DD_SCROLL_CLASS}
          style={{
            position: "absolute",
            left: "50%",
            top: `calc(100% + ${6 * s}px)`,
            transform: "translateX(-50%)",
            ...glassPanelStyle(s),
            width: Math.min(220, width) * s,
            padding: `${14 * s}px 0`,
            zIndex: 50,
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

function ChevronPair({ s, onUp, onDown }: { s: number; onUp: () => void; onDown: () => void }) {
  const btn: CSSProperties = {
    background: "none",
    border: "none",
    cursor: "pointer",
    padding: 0,
    lineHeight: 0,
    flexShrink: 0,
  };
  const tri = (flipY: boolean) => (
    <svg width={9 * s} height={4.5 * s} viewBox="0 0 6 3" style={flipY ? { transform: "scaleY(-1)" } : undefined}>
      <polygon points="3,0 6,3 0,3" fill="#9c9c9c" />
    </svg>
  );
  return (
    <div style={{ display: "flex", flexDirection: "column", gap: 4 * s, width: 9 * s, justifyContent: "center" }}>
      <button type="button" onClick={e => { e.stopPropagation(); onUp(); }} style={btn} aria-label="Increase">
        {tri(false)}
      </button>
      <button type="button" onClick={e => { e.stopPropagation(); onDown(); }} style={btn} aria-label="Decrease">
        {tri(true)}
      </button>
    </div>
  );
}

function MenuItem({
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
