import { COLLAPSE_MS, ICON_SIZE } from "../heroLayout";

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
  return (
    <button
      type="button"
      aria-label={collapsed ? "Expand hero panel" : "Collapse hero panel"}
      aria-expanded={!collapsed}
      onClick={onToggle}
      className="select-none"
      style={{
        position: "absolute",
        left,
        top,
        width: ICON_SIZE,
        height: ICON_SIZE,
        zIndex: 10,
        padding: 0,
        border: "none",
        background: "transparent",
        cursor: "pointer",
        overflow: "hidden",
        borderRadius: "50%",
        transition: `left ${COLLAPSE_MS}ms cubic-bezier(0.4, 0, 0.2, 1), top ${COLLAPSE_MS}ms cubic-bezier(0.4, 0, 0.2, 1)`,
      }}
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
    </button>
  );
}
