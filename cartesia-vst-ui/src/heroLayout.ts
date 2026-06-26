/**
 * M8b — Hero canvas layout (Figma 5002:6446 expanded · 5002:6447 collapsed).
 * Collapse: viewport clips from the left; shell stays right-pinned (static on screen).
 */

export const FRAME_H = 1805;
export const EXPANDED_W = 2376;
export const COLLAPSED_W = 1515;
/** Extra width revealed on the left when expanded. */
export const HERO_PANEL_W = EXPANDED_W - COLLAPSED_W;

export const HERO_MAIN_LEFT = 66;

export const SHELL_W = 1405;
export const SHELL_TOP = 50;
export const SHELL_RIGHT_GUTTER = 85;
/** Fixed in expanded coords — right-pinned in both states via viewport clip. */
export const SHELL_LEFT = EXPANDED_W - SHELL_W - SHELL_RIGHT_GUTTER;

/** Figma 100 px − 30 % → 70 px. */
export const ICON_SIZE = 70;
/** Symmetric inset from hero / viewport edges (Figma 17 px inside 66 px main gutter). */
export const ICON_INSET = 17;
/** Expanded — top-left inside hero MainFrame. */
export const ICON_EXPANDED = { left: HERO_MAIN_LEFT + ICON_INSET, top: HERO_MAIN_LEFT + ICON_INSET };
/** Expanded — top-right (mirror of collapse chevron). */
export const DAW_SYNC_EXPANDED_LEFT = EXPANDED_W - (HERO_MAIN_LEFT + ICON_INSET) - ICON_SIZE;
/** Collapsed — inset inside control shell frame (shell-absolute). */
export const ICON_COLLAPSED_INSET = { left: ICON_INSET, top: ICON_INSET };

export const HERO = {
  mainW: 2310,
  mainH: 1805,
  bg: { left: 0, top: 7, width: 2310, height: 1798 },
  mask: { left: -463.39, top: -217.05, width: 1853.47, height: 2048.29 },
  portrait: { left: -895.32, top: -47.48, width: 1905.46, height: 1716.41 },
  label: { left: 109, top: 1449, width: 480 },
} as const;

export const COLLAPSE_MS = 380;

/** Shift expanded canvas left so the right edge stays fixed while viewport narrows. */
export function viewportContentOffset(viewportW: number): number {
  return viewportW - EXPANDED_W;
}
