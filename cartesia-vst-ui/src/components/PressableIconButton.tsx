import { useState, type CSSProperties, type ReactNode } from "react";
import { useHoverTip } from "./useHoverTip";

type Props = {
  left: number;
  top: number;
  size: number;
  label: string;
  onClick: () => void;
  zIndex?: number;
  transition?: string;
  disabled?: boolean;
  ariaExpanded?: boolean;
  children: ReactNode;
};

/**
 * Circular canvas icon — pointer cursor, delayed hover tip, subtle press scale.
 */
export function PressableIconButton({
  left,
  top,
  size,
  label,
  onClick,
  zIndex = 10,
  transition,
  disabled = false,
  ariaExpanded,
  children,
}: Props) {
  const [pressed, setPressed] = useState(false);
  const tip = useHoverTip();

  const shell: CSSProperties = {
    position: "absolute",
    left,
    top,
    width: size,
    height: size,
    zIndex,
    padding: 0,
    border: "none",
    background: "transparent",
    cursor: disabled ? "default" : "pointer",
    overflow: "visible",
    borderRadius: "50%",
    transform: pressed && !disabled ? "scale(0.92)" : "scale(1)",
    transition: transition
      ? `${transition}, transform 120ms ease-out`
      : "transform 120ms ease-out",
  };

  return (
    <button
      type="button"
      aria-label={label}
      aria-expanded={ariaExpanded}
      disabled={disabled}
      onClick={disabled ? undefined : onClick}
      className="select-none icon-press-btn"
      style={shell}
      onMouseDown={() => { if (!disabled) setPressed(true); }}
      onMouseUp={() => setPressed(false)}
      onMouseLeave={() => { setPressed(false); tip.onLeave(); }}
      onMouseEnter={tip.onEnter}
    >
      <span
        style={{
          position: "absolute",
          inset: 0,
          overflow: "hidden",
          borderRadius: "50%",
          pointerEvents: "none",
        }}
      >
        {children}
      </span>
      {tip.visible && <span className="icon-hover-tip">{label}</span>}
    </button>
  );
}
