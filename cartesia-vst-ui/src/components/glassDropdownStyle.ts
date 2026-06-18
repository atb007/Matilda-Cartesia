import type { CSSProperties } from "react";

/** Shared glass dropdown dimensions (Figma 4960:3435 / 4918:101473). */
export const GLASS_DD_W        = 316;
export const GLASS_DD_RADIUS   = 24;
export const GLASS_DD_PAD_Y    = 22;
export const GLASS_DD_ITEM_FS  = 16;
export const GLASS_DD_ITEM_GAP = 11;
export const GLASS_DD_LINE_GAP = 15;
export const GLASS_DD_CLOSE    = 18;
export const GLASS_DD_SCROLL_CLASS = "glass-dd-scroll";

/**
 * Figma glass material — Light -45° @ 80%, Refraction 80, Depth 85,
 * Dispersion 50, Frost 20. Same recipe as MovementMenu dropdown.
 */
export function glassPanelStyle(s: number): CSSProperties {
  return {
    width: GLASS_DD_W * s,
    borderRadius: GLASS_DD_RADIUS * s,
    background:
      "linear-gradient(135deg, rgba(255,255,255,0.16) 0%, rgba(255,255,255,0.05) 35%, rgba(80,80,80,0.04) 65%, rgba(0,0,0,0.12) 100%)",
    backdropFilter: "blur(8px) saturate(1.2)",
    WebkitBackdropFilter: "blur(8px) saturate(1.2)",
    border: "1px solid rgba(255,255,255,0.2)",
    boxShadow: [
      "0 16px 48px rgba(0,0,0,0.45)",
      "inset 0 1.5px 1px rgba(255,255,255,0.35)",
      "inset 1.5px 0 1px rgba(255,255,255,0.14)",
      "inset 0 -2.5px 4px rgba(0,0,0,0.4)",
      "inset -1.5px 0 3px rgba(0,0,0,0.22)",
      "inset 0 0 14px rgba(255,255,255,0.05)",
      "inset 2px 2px 6px rgba(130,215,255,0.07)",
      "inset -2px -2px 6px rgba(255,160,110,0.07)",
    ].join(", "),
    padding: `${GLASS_DD_PAD_Y * s}px 0`,
    zIndex: 50,
    overflow: "hidden",
  };
}

export function glassHairline(): CSSProperties {
  return {
    height: 1,
    width: "100%",
    background: "linear-gradient(90deg, transparent 0%, rgba(255,255,255,0.22) 50%, transparent 100%)",
  };
}
