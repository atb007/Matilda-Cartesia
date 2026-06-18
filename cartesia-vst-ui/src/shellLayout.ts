/**
 * Control shell layout — Figma 4976:4727 / 5002:7425 / GlassBg 5007:6100.
 * Glass corners are inset from the iron frame origin (GLASS_IN_FRAME).
 */

export const SHELL_W = 1405;
/** Figma Full Frame height (5002:7425). */
export const SHELL_H = 1737;
/** Legacy standalone height incl. frame bleed. */
export const SHELL_FRAME_H = 1765.4537353515625;

/** Integer px — matches shell-frame-vines-only.png (1405×1766) 1:1, no subpixel stretch. */
export const FRAME = {
  left: 0,
  top: -14,
  width: 1405,
  height: 1766,
} as const;

/** Glass bedding — Figma 5007:6100 · 1205×1407 */
export const GLASS = {
  left: 83.2177734375,
  top: 151,
  width: 1205,
  height: 1407,
} as const;

/** Glass position relative to iron frame top-left — corners anchor here. */
export const GLASS_IN_FRAME = {
  left: GLASS.left - FRAME.left,
  top: GLASS.top - FRAME.top,
  width: GLASS.width,
  height: GLASS.height,
} as const;
