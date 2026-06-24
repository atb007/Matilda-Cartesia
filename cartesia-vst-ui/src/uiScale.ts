/** kPreviewScale (0.52) × factor; default 0.9. Corner / edge drag 0.7…1.0. */
export const BASE_SCALE = 0.52;
export const UI_SCALE_MIN = 0.7;
export const UI_SCALE_MAX = 1.0;
export const UI_SCALE_DEFAULT = 0.9;

export type UiGrip =
  | "topLeft"
  | "top"
  | "topRight"
  | "left"
  | "right"
  | "bottomLeft"
  | "bottom"
  | "bottomRight";

export const UI_GRIPS: UiGrip[] = [
  "topLeft",
  "top",
  "topRight",
  "left",
  "right",
  "bottomLeft",
  "bottom",
  "bottomRight",
];

export function isCornerGrip(grip: UiGrip): boolean {
  return grip === "topLeft" || grip === "topRight" || grip === "bottomLeft" || grip === "bottomRight";
}

export function effectiveScale(uiScaleFactor: number): number {
  const clamped = Math.max(UI_SCALE_MIN, Math.min(UI_SCALE_MAX, uiScaleFactor));
  return BASE_SCALE * clamped;
}

export function referenceViewportWidth100(viewportDesignW: number): number {
  return viewportDesignW * BASE_SCALE;
}

export function referenceViewportHeight100(viewportDesignH: number): number {
  return viewportDesignH * BASE_SCALE;
}

export function uiScaleFactorFromGripDrag(
  grip: UiGrip,
  deltaX: number,
  deltaY: number,
  startWidth: number,
  startHeight: number,
  refWidth100: number,
  refHeight100: number,
): number {
  const clamp = (factor: number) => Math.max(UI_SCALE_MIN, Math.min(UI_SCALE_MAX, factor));

  switch (grip) {
    case "topLeft": {
      const fw = (startWidth - deltaX) / refWidth100;
      const fh = (startHeight - deltaY) / refHeight100;
      return clamp((fw + fh) * 0.5);
    }
    case "topRight": {
      const fw = (startWidth + deltaX) / refWidth100;
      const fh = (startHeight - deltaY) / refHeight100;
      return clamp((fw + fh) * 0.5);
    }
    case "bottomLeft": {
      const fw = (startWidth - deltaX) / refWidth100;
      const fh = (startHeight + deltaY) / refHeight100;
      return clamp((fw + fh) * 0.5);
    }
    case "bottomRight": {
      const fw = (startWidth + deltaX) / refWidth100;
      const fh = (startHeight + deltaY) / refHeight100;
      return clamp((fw + fh) * 0.5);
    }
    case "top":
      return clamp((startHeight - deltaY) / refHeight100);
    case "bottom":
      return clamp((startHeight + deltaY) / refHeight100);
    case "left":
      return clamp((startWidth - deltaX) / refWidth100);
    case "right":
      return clamp((startWidth + deltaX) / refWidth100);
  }
}

/** @deprecated use UiGrip */
export type UiCorner = "topLeft" | "topRight" | "bottomLeft" | "bottomRight";
