import { GLASS } from "../shellLayout";
import { SHELL_GLASS_BG } from "./shellGlassStyle";

type Props = {
  /** Use Figma raster export instead of live CSS gradients. */
  useRaster?: boolean;
};

/**
 * Glass panel fill — Figma 5007:6100 · shell-absolute coords.
 * Sits behind the vine frame overlay; flat frame fill is keyed out in the raster.
 */
export function ShellGlassBedding({ useRaster = false }: Props) {
  if (useRaster) {
    return (
      <img
        alt=""
        src="/assets/shell-glass-bedding.png"
        style={{
          position: "absolute",
          left: GLASS.left,
          top: GLASS.top,
          width: GLASS.width,
          height: GLASS.height,
          zIndex: 1,
          pointerEvents: "none",
        }}
      />
    );
  }

  return (
    <div
      style={{
        position: "absolute",
        left: GLASS.left,
        top: GLASS.top,
        width: GLASS.width,
        height: GLASS.height,
        zIndex: 1,
        pointerEvents: "none",
        overflow: "hidden",
        ...SHELL_GLASS_BG,
      }}
    />
  );
}
