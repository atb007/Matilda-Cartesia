import { useState, type CSSProperties, type ReactNode } from "react";
import { useHoverTip } from "./useHoverTip";

type Props = {
  disabled?: boolean;
  onClick?: () => void;
  style?: CSSProperties;
  children: ReactNode;
};

/** Play/pause gem — press scale only (no hover tooltip). */
export function ClickableGemButton({ disabled, onClick, style, children }: Props) {
  const [pressed, setPressed] = useState(false);

  return (
    <button
      type="button"
      disabled={disabled}
      onClick={disabled ? undefined : onClick}
      className="select-none icon-press-btn"
      style={{
        position: "relative",
        padding: 0,
        border: "none",
        background: "transparent",
        cursor: disabled ? "default" : "pointer",
        lineHeight: 0,
        overflow: "visible",
        transform: pressed && !disabled ? "scale(0.94)" : "scale(1)",
        transition: "transform 120ms ease-out",
        ...style,
      }}
      onMouseDown={() => { if (!disabled) setPressed(true); }}
      onMouseUp={() => setPressed(false)}
      onMouseLeave={() => setPressed(false)}
    >
      {children}
    </button>
  );
}
