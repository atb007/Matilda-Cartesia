import type { CSSProperties } from "react";

/**
 * Figma gray pill indicator (4896:98133).
 * Two states:
 *  - idle: static gray pill
 *  - lit:  neon glow in the layer color — playhead is passing over this cell
 *
 * The lit visual is a separate always-mounted overlay that cross-fades with
 * OPACITY ONLY (GPU-composited) — animating background/box-shadow directly
 * causes visible snapping/jitter since gradients can't be interpolated.
 */
type LedProps = {
  className?: string;
  style?: CSSProperties;
  /** True while the playhead/note is passing over this cell. */
  lit?: boolean;
  /** Glow color when lit (per layer variant). */
  litColor?: string;
};

export function Led({ className, style, lit = false, litColor = "#ffa040" }: LedProps) {
  return (
    <div className={className} style={style}>
      <div
        style={{
          width: "100%",
          height: "100%",
          position: "relative",
          borderRadius: 6.4,
          overflow: "visible",
        }}
      >
        {/* Base gray pill (constant) */}
        <div
          style={{
            position: "absolute",
            inset: 0,
            background: "#8f8f8f",
            borderRadius: 6.4,
          }}
        />
        {/* Inset shading (constant) */}
        <div
          style={{
            position: "absolute",
            inset: 0,
            borderRadius: 6.4,
            boxShadow:
              "inset 0px -3.2px 3.2px 0px rgba(0,0,0,0.25), inset 0px 3.2px 3.2px 0px rgba(0,0,0,0.25)",
          }}
        />
        {/* Lit overlay — white-hot neon core + tight halo; opacity-only crossfade */}
        <div
          style={{
            position: "absolute",
            inset: 0,
            borderRadius: 6.4,
            background: `radial-gradient(ellipse at 50% 45%, #ffffff 0%, ${litColor} 55%, ${litColor} 100%)`,
            boxShadow: `inset 0px -3.2px 3.2px 0px rgba(0,0,0,0.1), inset 0px 3.2px 3.2px 0px rgba(0,0,0,0.1), 0 0 2px 0.5px #ffffffcc, 0 0 4px 1px ${litColor}, 0 0 7px 1px ${litColor}55`,
            opacity: lit ? 1 : 0,
            transition: "opacity 140ms ease-out",
            willChange: "opacity",
            pointerEvents: "none",
          }}
        />
      </div>
    </div>
  );
}
