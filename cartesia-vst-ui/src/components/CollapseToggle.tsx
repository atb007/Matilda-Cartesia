import { COLLAPSE_MS, ICON_SIZE } from "../heroLayout";

type Props = {
  left: number;
  top: number;
  collapsed: boolean;
  onToggle: () => void;
};

/**
 * Glass chevron button (Figma `icon` · 100×100).
 * Expanded → `>>`. Collapsed → `<<` (per-chevron scaleX flip, not parent rotate).
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
        transition: `left ${COLLAPSE_MS}ms cubic-bezier(0.4, 0, 0.2, 1), top ${COLLAPSE_MS}ms cubic-bezier(0.4, 0, 0.2, 1)`,
      }}
    >
      <img
        alt=""
        src="/assets/collapse-icon-outer.svg"
        style={{
          position: "absolute",
          inset: 0,
          width: ICON_SIZE,
          height: ICON_SIZE,
          objectFit: "contain",
        }}
      />
      <img
        alt=""
        src="/assets/collapse-icon-glass.svg"
        style={{
          position: "absolute",
          left: "5.88%",
          top: "5.88%",
          width: "88.24%",
          height: "88.24%",
          objectFit: "contain",
        }}
      />
      {/* Figma chevron slots — 5002:6416 expanded · 5002:6423 collapsed */}
      <div
        style={{
          position: "absolute",
          top: "8.82%",
          bottom: "8.82%",
          left: 0,
          right: 0,
        }}
      >
        <ChevronSlot side="left" flipped={collapsed} />
        <ChevronSlot side="right" flipped={collapsed} />
      </div>
    </button>
  );
}

function ChevronSlot({ side, flipped }: { side: "left" | "right"; flipped: boolean }) {
  const isLeft = side === "left";

  return (
    <div
      style={{
        position: "absolute",
        top: 0,
        bottom: 0,
        left: isLeft ? 0 : "17.65%",
        right: isLeft ? "17.65%" : 0,
        overflow: "hidden",
      }}
    >
      <div
        style={{
          position: "absolute",
          top: "29.15%",
          bottom: "29.17%",
          left: "37.48%",
          right: "37.48%",
          transform: flipped ? "scaleX(-1)" : "scaleX(1)",
        }}
      >
        <img
          alt=""
          src="/assets/collapse-chevron.svg"
          style={{ display: "block", width: "100%", height: "100%" }}
        />
      </div>
    </div>
  );
}
