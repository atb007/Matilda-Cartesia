import { useRef } from "react";
import {
  referenceViewportHeight100,
  referenceViewportWidth100,
  type UiGrip,
  uiScaleFactorFromGripDrag,
} from "../uiScale";

const CORNER = 22;
const EDGE = 8;

type Props = {
  grip: UiGrip;
  viewportDesignW: number;
  viewportDesignH: number;
  currentWidth: number;
  currentHeight: number;
  onScaleChange: (factor: number) => void;
};

type GripLayout = {
  style: React.CSSProperties;
  cursor: string;
  flipX: boolean;
  flipY: boolean;
  edge: boolean;
  horizontal: boolean;
};

function gripLayout(grip: UiGrip, width: number, height: number): GripLayout {
  const isCorner =
    grip === "topLeft" || grip === "topRight" || grip === "bottomLeft" || grip === "bottomRight";
  const base = { position: "absolute" as const, zIndex: isCorner ? 51 : 49, touchAction: "none" as const };

  switch (grip) {
    case "topLeft":
      return {
        style: { ...base, top: 0, left: 0, width: CORNER, height: CORNER },
        cursor: "nwse-resize",
        flipX: true,
        flipY: true,
        edge: false,
        horizontal: false,
      };
    case "top":
      return {
        style: { ...base, top: 0, left: CORNER, width: width - CORNER * 2, height: EDGE },
        cursor: "ns-resize",
        flipX: false,
        flipY: false,
        edge: true,
        horizontal: true,
      };
    case "topRight":
      return {
        style: { ...base, top: 0, right: 0, width: CORNER, height: CORNER },
        cursor: "nesw-resize",
        flipX: false,
        flipY: true,
        edge: false,
        horizontal: false,
      };
    case "left":
      return {
        style: { ...base, top: CORNER, left: 0, width: EDGE, height: height - CORNER * 2 },
        cursor: "ew-resize",
        flipX: false,
        flipY: false,
        edge: true,
        horizontal: false,
      };
    case "right":
      return {
        style: { ...base, top: CORNER, right: 0, width: EDGE, height: height - CORNER * 2 },
        cursor: "ew-resize",
        flipX: false,
        flipY: false,
        edge: true,
        horizontal: false,
      };
    case "bottomLeft":
      return {
        style: { ...base, bottom: 0, left: 0, width: CORNER, height: CORNER },
        cursor: "nesw-resize",
        flipX: true,
        flipY: false,
        edge: false,
        horizontal: false,
      };
    case "bottom":
      return {
        style: { ...base, bottom: 0, left: CORNER, width: width - CORNER * 2, height: EDGE },
        cursor: "ns-resize",
        flipX: false,
        flipY: false,
        edge: true,
        horizontal: true,
      };
    case "bottomRight":
      return {
        style: { ...base, bottom: 0, right: 0, width: CORNER, height: CORNER },
        cursor: "nwse-resize",
        flipX: false,
        flipY: false,
        edge: false,
        horizontal: false,
      };
  }
}

function GripIcon({ layout }: { layout: GripLayout }) {
  if (layout.edge) {
    return (
      <svg
        width="100%"
        height="100%"
        viewBox="0 0 40 8"
        preserveAspectRatio="xMidYMid meet"
        style={{ display: "block", pointerEvents: "none", opacity: 0.25 }}
        aria-hidden
      >
        {layout.horizontal ? (
          [-1, 0, 1].map(i => (
            <line key={i} x1={20 + i * 6} y1={3} x2={20 + i * 6} y2={5} stroke="white" strokeWidth="1.2" />
          ))
        ) : (
          [-1, 0, 1].map(i => (
            <line key={i} x1={3} y1={4 + i * 6} x2={5} y2={4 + i * 6} stroke="white" strokeWidth="1.2" />
          ))
        )}
      </svg>
    );
  }

  const sx = layout.flipX ? -1 : 1;
  const sy = layout.flipY ? -1 : 1;

  return (
    <svg
      width="22"
      height="22"
      viewBox="0 0 22 22"
      style={{
        display: "block",
        pointerEvents: "none",
        opacity: 0.25,
        transform: `scale(${sx}, ${sy})`,
        transformOrigin: "center",
      }}
      aria-hidden
    >
      {[0, 1, 2].map(i => (
        <line
          key={i}
          x1={12 + i * 4}
          y1={20}
          x2={20}
          y2={12 + i * 4}
          stroke="white"
          strokeWidth="1.2"
        />
      ))}
    </svg>
  );
}

export function UiResizeGrip({
  grip,
  viewportDesignW,
  viewportDesignH,
  currentWidth,
  currentHeight,
  onScaleChange,
}: Props) {
  const dragRef = useRef<{
    startX: number;
    startY: number;
    startWidth: number;
    startHeight: number;
  } | null>(null);

  const layout = gripLayout(grip, currentWidth, currentHeight);

  const onPointerDown = (e: React.PointerEvent<HTMLDivElement>) => {
    e.preventDefault();
    e.currentTarget.setPointerCapture(e.pointerId);
    dragRef.current = {
      startX: e.clientX,
      startY: e.clientY,
      startWidth: currentWidth,
      startHeight: currentHeight,
    };
  };

  const onPointerMove = (e: React.PointerEvent<HTMLDivElement>) => {
    if (!dragRef.current) return;
    const deltaX = e.clientX - dragRef.current.startX;
    const deltaY = e.clientY - dragRef.current.startY;
    onScaleChange(
      uiScaleFactorFromGripDrag(
        grip,
        deltaX,
        deltaY,
        dragRef.current.startWidth,
        dragRef.current.startHeight,
        referenceViewportWidth100(viewportDesignW),
        referenceViewportHeight100(viewportDesignH),
      ),
    );
  };

  const onPointerUp = (e: React.PointerEvent<HTMLDivElement>) => {
    if (dragRef.current) {
      e.currentTarget.releasePointerCapture(e.pointerId);
      dragRef.current = null;
    }
  };

  return (
    <div
      aria-label={`Resize plugin from ${grip}`}
      onPointerDown={onPointerDown}
      onPointerMove={onPointerMove}
      onPointerUp={onPointerUp}
      onPointerCancel={onPointerUp}
      style={{ ...layout.style, cursor: layout.cursor }}
    >
      <GripIcon layout={layout} />
    </div>
  );
}

export function UiResizeGrips(props: Omit<Props, "grip">) {
  return (
    <>
      {(["topLeft", "top", "topRight", "left", "right", "bottomLeft", "bottom", "bottomRight"] as const).map(
        grip => (
          <UiResizeGrip key={grip} grip={grip} {...props} />
        ),
      )}
    </>
  );
}
