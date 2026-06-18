import type { CSSProperties } from "react";

/**
 * Smooth bilinear downscale for static PNG overlays.
 * Avoid crisp-edges / pixelated — those alias badly under CSS transform scale.
 */
export const RASTER_SMOOTH: CSSProperties = {
  imageRendering: "auto",
  WebkitBackfaceVisibility: "hidden",
  backfaceVisibility: "hidden",
};
