import { COLLAPSE_MS, ICON_SIZE } from "../heroLayout";
import { PressableIconButton } from "./PressableIconButton";

type Props = {
  left: number;
  top: number;
  collapsed: boolean;
  onToggle: () => void;
};

/**
 * Glass chevron button — Figma `icon` · 70×70 (100×100 @2x PNG).
 */
export function CollapseToggle({ left, top, collapsed, onToggle }: Props) {
  const label = collapsed ? "Expand hero panel" : "Collapse hero panel";
  const ease = `${COLLAPSE_MS}ms cubic-bezier(0.4, 0, 0.2, 1)`;

  return (
    <PressableIconButton
      left={left}
      top={top}
      size={ICON_SIZE}
      label={label}
      onClick={onToggle}
      transition={`left ${ease}, top ${ease}`}
      ariaExpanded={!collapsed}
    >
      <img
        alt=""
        src={
          collapsed
            ? "/assets/collapse-toggle-collapsed@2x.png"
            : "/assets/collapse-toggle-expanded@2x.png"
        }
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
