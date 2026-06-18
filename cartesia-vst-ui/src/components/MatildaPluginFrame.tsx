import { useState } from "react";
import {
  COLLAPSED_W,
  COLLAPSE_MS,
  EXPANDED_W,
  FRAME_H,
  ICON_COLLAPSED_INSET,
  ICON_EXPANDED,
  SHELL_LEFT,
  SHELL_TOP,
  viewportContentOffset,
} from "../heroLayout";
import { CollapseToggle } from "./CollapseToggle";
import { HeroCanvas } from "./HeroCanvas";
import { MatildaShell } from "./MatildaShell";

type Props = {
  scale?: number;
};

/**
 * M8b — Full plugin window: hero canvas + collapsible left panel + M8 control shell.
 * Collapse clips from the left; shell stays right-pinned (no shell left animation).
 */
export function MatildaPluginFrame({ scale = 0.52 }: Props) {
  const [collapsed, setCollapsed] = useState(false);

  const viewportW = collapsed ? COLLAPSED_W : EXPANDED_W;
  const contentOffset = viewportContentOffset(viewportW);
  const iconLeft = collapsed
    ? SHELL_LEFT + ICON_COLLAPSED_INSET.left
    : ICON_EXPANDED.left;
  const iconTop = collapsed
    ? SHELL_TOP + ICON_COLLAPSED_INSET.top
    : ICON_EXPANDED.top;
  const ease = `${COLLAPSE_MS}ms cubic-bezier(0.4, 0, 0.2, 1)`;

  return (
    <div
      className="select-none"
      style={{
        position: "relative",
        width: viewportW * scale,
        height: FRAME_H * scale,
        overflow: "hidden",
        transition: `width ${ease}`,
      }}
    >
      <div
        style={{
          width: EXPANDED_W * scale,
          height: FRAME_H * scale,
          marginLeft: contentOffset * scale,
          transition: `margin-left ${ease}`,
        }}
      >
        <div
          className="plugin-frame-native"
          style={{
            position: "relative",
            width: EXPANDED_W,
            height: FRAME_H,
            transform: `scale(${scale})`,
            transformOrigin: "top left",
          }}
        >
          <HeroCanvas />

          <div
            style={{
              position: "absolute",
              left: SHELL_LEFT,
              top: SHELL_TOP,
              zIndex: 2,
            }}
          >
            <MatildaShell scale={1} embedded />
          </div>

          <CollapseToggle
            left={iconLeft}
            top={iconTop}
            collapsed={collapsed}
            onToggle={() => setCollapsed(c => !c)}
          />
        </div>
      </div>
    </div>
  );
}
