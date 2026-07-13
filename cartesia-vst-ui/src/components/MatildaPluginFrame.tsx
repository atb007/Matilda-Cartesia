import { useState } from "react";
import {
  COLLAPSED_W,
  COLLAPSE_MS,
  DAW_SYNC_EXPANDED_LEFT,
  EXPANDED_W,
  FRAME_H,
  ICON_COLLAPSED_INSET,
  ICON_EXPANDED,
  SHELL_LEFT,
  SHELL_TOP,
  viewportContentOffset,
} from "../heroLayout";
import { effectiveScale, UI_SCALE_DEFAULT } from "../uiScale";
import { CollapseToggle } from "./CollapseToggle";
import { DawSyncToggle } from "./DawSyncToggle";
import { enableRiveHero } from "../featureFlags";
import { useMatildaEngine, type MatildaEngineState } from "../hooks/useMatildaEngine";
import { HeroCanvas } from "./HeroCanvas";
import { MatildaShell } from "./MatildaShell";
import { UiResizeGrips } from "./UiResizeGrip";

type Props = {
  /** Override user scale (dev only). Omit to enable corner resize 70%…100%. */
  scale?: number;
  resizable?: boolean;
};

type FrameContentProps = Props & {
  riveHero: boolean;
  engine?: MatildaEngineState;
};

/**
 * M8b — Full plugin window: hero canvas + collapsible left panel + M8 control shell.
 */
function MatildaPluginFrameContent({
  scale: scaleOverride,
  resizable = scaleOverride == null,
  riveHero,
  engine,
}: FrameContentProps) {
  const [collapsed, setCollapsed] = useState(false);
  const [dawSync, setDawSync] = useState(true);
  const [uiScaleFactor, setUiScaleFactor] = useState(UI_SCALE_DEFAULT);

  const scale = scaleOverride ?? effectiveScale(uiScaleFactor);

  const viewportW = collapsed ? COLLAPSED_W : EXPANDED_W;
  const viewportPxW = viewportW * scale;
  const viewportPxH = FRAME_H * scale;
  const contentOffset = viewportContentOffset(viewportW);
  const iconLeft = collapsed
    ? SHELL_LEFT + ICON_COLLAPSED_INSET.left
    : ICON_EXPANDED.left;
  const iconTop = collapsed
    ? SHELL_TOP + ICON_COLLAPSED_INSET.top
    : ICON_EXPANDED.top;
  const ease = `${COLLAPSE_MS}ms cubic-bezier(0.4, 0, 0.2, 1)`;

  const playing = riveHero && engine ? engine.playing : false;
  const shellDawSync = riveHero && engine ? engine.dawSync : dawSync;
  const onDawSyncChange = riveHero && engine ? engine.setDawSync : setDawSync;

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
          <HeroCanvas playing={riveHero ? playing : undefined} />

          <div
            style={{
              position: "absolute",
              left: SHELL_LEFT,
              top: SHELL_TOP,
              zIndex: 2,
            }}
          >
            <MatildaShell
              scale={1}
              embedded
              dawSync={shellDawSync}
              engine={riveHero ? engine : undefined}
            />
          </div>

          <CollapseToggle
            left={iconLeft}
            top={iconTop}
            collapsed={collapsed}
            onToggle={() => setCollapsed(c => !c)}
          />

          <DawSyncToggle
            left={DAW_SYNC_EXPANDED_LEFT}
            top={ICON_EXPANDED.top}
            enabled={shellDawSync}
            onToggle={onDawSyncChange}
          />
        </div>
      </div>

      {resizable && (
        <UiResizeGrips
          viewportDesignW={viewportW}
          viewportDesignH={FRAME_H}
          currentWidth={viewportPxW}
          currentHeight={viewportPxH}
          onScaleChange={setUiScaleFactor}
        />
      )}
    </div>
  );
}

function MatildaPluginFrameWithRive(props: Props) {
  const engine = useMatildaEngine();
  return <MatildaPluginFrameContent {...props} riveHero engine={engine} />;
}

export function MatildaPluginFrame(props: Props) {
  const riveHero = enableRiveHero();
  if (riveHero)
    return <MatildaPluginFrameWithRive {...props} />;
  return <MatildaPluginFrameContent {...props} riveHero={false} />;
}
