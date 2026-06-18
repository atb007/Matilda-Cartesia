/**
 * Cell knob color variants — sourced from Figma component 4789:104854.
 *
 * The ring and sphere-body images are SHARED across all 4 colors and both
 * states.  Only the gloss (coloured inner circle) and soft-light (ambient glow)
 * differ per color/state.
 *
 * Variants:  orange | red | green | blue
 * States:    active (state=true) | off (state=false)
 */

import type { CSSProperties } from "react";

const BASE = "https://www.figma.com/api/mcp/asset";
const img  = (uuid: string) => `${BASE}/${uuid}`;

export type KnobVariant = "orange" | "red" | "green" | "blue";

// ── Shared layers (identical across all variants + states) ───────────────────
const RING   = img("47fafdc8-5130-411b-8064-1b75801c2bc3"); // imgEllipse42976
const SPHERE = img("e9983ad3-c308-42f7-bad4-3a1142e3a8c5"); // imgEllipse42977

// ── Types ────────────────────────────────────────────────────────────────────
export type KnobImages = {
  ring:      string;
  sphere:    string;
  gloss:     string;
  softLight: string;
  /**
   * Optional: when present, replaces gloss img + soft-light img with a single
   * CSS div.  Useful for one-off overrides without Figma assets.
   */
  glossCss?: CSSProperties;
};

export type KnobColorConfig = {
  variant:  KnobVariant;
  label:    string;
  active:   KnobImages;
  off:      KnobImages;
  /** Indicator tick gradient (all variants share the same orange tick). */
  tickFrom: string;
  tickTo:   string;
  /** LED glow color when the playhead passes over the cell. */
  ledColor: string;
};

// ── Color configs ────────────────────────────────────────────────────────────

export const KNOB_COLORS: Record<KnobVariant, KnobColorConfig> = {

  orange: {
    variant: "orange",
    label:   "Orange",
    active: {
      ring:      RING,
      sphere:    SPHERE,
      gloss:     img("6c8a4030-fb87-4a7d-98ff-1d5c636f552e"), // imgEllipse42978
      softLight: img("4840cf53-2f03-4b3d-8dae-03132a0e7f46"), // imgEllipse42980
    },
    off: {
      ring:      RING,
      sphere:    SPHERE,
      gloss:     img("83542422-28e9-4410-b69a-7c541c2c734d"), // imgEllipse42981
      softLight: img("0167c7d6-7c1c-403d-b125-f9fcf77c70ad"), // imgEllipse42982
    },
    tickFrom: "#B75600",
    tickTo:   "#512600",
    ledColor: "#FFA040",
  },

  red: {
    variant: "red",
    label:   "Red",
    active: {
      ring:      RING,
      sphere:    SPHERE,
      gloss:     img("38e46a23-1a4d-4b90-b584-cab2edc1a5d4"), // imgEllipse42983
      softLight: img("c2639394-9213-4632-a1b5-c4988bf96495"), // imgEllipse42984
    },
    off: {
      ring:      RING,
      sphere:    SPHERE,
      gloss:     img("dca2d6d5-e88c-4875-905e-d795880ca6d8"), // imgEllipse42985
      softLight: img("c2639394-9213-4632-a1b5-c4988bf96495"), // imgEllipse42984 (same for both states)
    },
    tickFrom: "#B75600",
    tickTo:   "#512600",
    ledColor: "#FF5545",
  },

  green: {
    variant: "green",
    label:   "Green",
    active: {
      ring:      RING,
      sphere:    SPHERE,
      gloss:     img("a97c95d7-4b79-43ab-aba9-f763ca412509"), // imgEllipse42986
      softLight: img("e850de22-5f49-4845-be93-054f0200f96e"), // imgEllipse42987
    },
    off: {
      ring:      RING,
      sphere:    SPHERE,
      gloss:     img("f99ee0cc-cd11-4565-b91d-1b4f3fb6e8a7"), // imgEllipse42988
      softLight: img("e850de22-5f49-4845-be93-054f0200f96e"), // imgEllipse42987 (same for both states)
    },
    tickFrom: "#B75600",
    tickTo:   "#512600",
    ledColor: "#50E080",
  },

  blue: {
    variant: "blue",
    label:   "Blue",
    active: {
      ring:      RING,
      sphere:    SPHERE,
      gloss:     img("57e386d9-b252-49a8-9ed3-3e83f29d522e"), // imgEllipse42989
      softLight: img("99ef9eab-51f0-4571-a7a2-6a4108092f07"), // imgEllipse42990
    },
    off: {
      ring:      RING,
      sphere:    SPHERE,
      gloss:     img("9b3680b5-bdd2-438f-9051-1ecfc615e176"), // imgEllipse42991
      softLight: img("809c0f97-9099-485d-b31f-7517107653eb"), // imgEllipse42992
    },
    tickFrom: "#B75600",
    tickTo:   "#512600",
    ledColor: "#50B0FF",
  },

};

export const KNOB_VARIANT_ORDER: KnobVariant[] = ["orange", "red", "green", "blue"];
