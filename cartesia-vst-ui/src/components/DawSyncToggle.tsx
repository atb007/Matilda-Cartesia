import { ICON_SIZE } from "../heroLayout";
import { PressableIconButton } from "./PressableIconButton";

type Props = {
  /** Viewport-relative left (design px). */
  left: number;
  top: number;
  enabled: boolean;
  onToggle: (enabled: boolean) => void;
};

/**
 * Top-right DAW sync — Figma 5108:109841 · DawSyncOn/Off @2x PNG (272 → 70 design px).
 */
export function DawSyncToggle({ left, top, enabled, onToggle }: Props) {
  const label = enabled ? "DAW sync is on" : "DAW sync is off";

  return (
    <PressableIconButton
      left={left}
      top={top}
      size={ICON_SIZE}
      label={label}
      onClick={() => onToggle(!enabled)}
    >
      <img
        alt=""
        src={enabled ? "/assets/DawSyncOn.png" : "/assets/DawSyncOff.png"}
        style={{
          position: "absolute",
          inset: 0,
          width: ICON_SIZE,
          height: ICON_SIZE,
          objectFit: "contain",
        }}
      />
    </PressableIconButton>
  );
}
