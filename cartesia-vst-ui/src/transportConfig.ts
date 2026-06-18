/** Master clock divisions — maps to `master_division` in Patch v2. */
export const CLOCK_DIVISIONS = [
  { id: "1/4", label: "1/4", beats: 1 },
  { id: "1/8", label: "1/8", beats: 0.5 },
  { id: "1/16", label: "1/16", beats: 0.25 },
  { id: "1/32", label: "1/32", beats: 0.125 },
] as const;

export type ClockDivisionId = (typeof CLOCK_DIVISIONS)[number]["id"];

/** v1 play-mode policy — Figma default is Transport; Note reserved for held-note root. */
export const PLAY_MODES = ["transport", "note"] as const;
export type PlayModeId = (typeof PLAY_MODES)[number];

export function playModeLabel(id: PlayModeId): string {
  return id === "transport" ? "Transport" : "Note";
}
