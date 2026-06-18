import type { CSSProperties } from "react";

/** Figma 4996:5119 / 5007:6100 — radial highlight + vertical fade to transparent. */
export const SHELL_GLASS_BG: CSSProperties = {
  backgroundImage: [
    "url(/assets/shell-glass-radial.svg)",
    "linear-gradient(180deg, rgb(27, 27, 27) 37.943%, rgba(50, 50, 50, 0.744) 63.166%, rgba(115, 115, 115, 0) 100%)",
  ].join(", "),
  backgroundSize: "100% 100%, 100% 100%",
  backgroundRepeat: "no-repeat, no-repeat",
};
