import { RASTER_SMOOTH } from "../rasterImageStyle";
import { FRAME } from "../shellLayout";

/**
 * Iron / vine frame — flat panel fill keyed out so glass bedding
 * shows through while ornate vines, crystals, and dividers stay on top.
 * @2x asset displayed 1:1 for mip-like downscale under plugin-frame scale.
 */
export function ShellFrameOverlay() {
  return (
    <img
      alt=""
      src="/assets/shell-frame-vines-only.png"
      srcSet="/assets/shell-frame-vines-only.png 1x, /assets/shell-frame-vines-only@2x.png 2x"
      width={FRAME.width}
      height={FRAME.height}
      style={{
        position: "absolute",
        left: FRAME.left,
        top: FRAME.top,
        width: FRAME.width,
        height: FRAME.height,
        zIndex: 2,
        pointerEvents: "none",
        ...RASTER_SMOOTH,
      }}
    />
  );
}
