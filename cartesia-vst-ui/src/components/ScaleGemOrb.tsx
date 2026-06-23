import { useEffect, useRef, useState } from "react";
import { gemImageForScale, type ScaleId } from "../scaleConfig";
import { GemSparks } from "./GemSparks";

/** motion tokens — FIGMA-CHECKLIST.md */
const OUT_MS = 160;
const IN_MS = 220;
const FLOAT_MS = 3200;
const FLOAT_PX = 3;
const HOVER_FLOAT_MULT = 1.35;

type TransitionPhase = "idle" | "out" | "in";

type Props = {
  scaleId: ScaleId;
  width: number;
  height: number;
  panelScale?: number;
};

function easeIn(t: number) {
  return t * t;
}

function easeOut(t: number) {
  return 1 - (1 - t) * (1 - t);
}

export function ScaleGemOrb({ scaleId, width, height, panelScale = 1 }: Props) {
  const [displayedScale, setDisplayedScale] = useState(scaleId);
  const [phase, setPhase] = useState<TransitionPhase>("idle");
  const [phaseStart, setPhaseStart] = useState(0);
  const [hovered, setHovered] = useState(false);
  const [floatY, setFloatY] = useState(0);
  const [visualScale, setVisualScale] = useState(1);
  const [visualAlpha, setVisualAlpha] = useState(1);
  const rafRef = useRef(0);
  const floatStartRef = useRef(performance.now());

  useEffect(() => {
    if (scaleId === displayedScale)
      return;
    setPhase("out");
    setPhaseStart(performance.now());
  }, [scaleId, displayedScale]);

  useEffect(() => {
    const tick = (now: number) => {
      const floatAmp = FLOAT_PX * panelScale * (hovered ? HOVER_FLOAT_MULT : 1);
      const floatT = ((now - floatStartRef.current) / FLOAT_MS) * Math.PI * 2;
      setFloatY(Math.sin(floatT) * floatAmp);

      if (phase === "out") {
        const t = Math.min(1, (now - phaseStart) / OUT_MS);
        const e = easeIn(t);
        setVisualScale(1 - e * 0.28);
        setVisualAlpha(1 - e);
        if (t >= 1) {
          setDisplayedScale(scaleId);
          setPhase("in");
          setPhaseStart(now);
        }
      } else if (phase === "in") {
        const t = Math.min(1, (now - phaseStart) / IN_MS);
        const e = easeOut(t);
        setVisualScale(0.88 + e * 0.12);
        setVisualAlpha(e);
        if (t >= 1) {
          setPhase("idle");
          setVisualScale(1);
          setVisualAlpha(1);
        }
      } else {
        setVisualScale(1);
        setVisualAlpha(1);
      }

      rafRef.current = requestAnimationFrame(tick);
    };

    rafRef.current = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(rafRef.current);
  }, [phase, phaseStart, scaleId, panelScale, hovered]);

  const sparksScaleId = phase === "out" ? displayedScale : scaleId;

  return (
    <div
      style={{
        position: "relative",
        width,
        height,
        overflow: "visible",
        isolation: "isolate",
        cursor: "default",
      }}
      onMouseEnter={() => setHovered(true)}
      onMouseLeave={() => setHovered(false)}
    >
      <GemSparks
        scaleId={sparksScaleId}
        width={width}
        height={height}
        panelScale={panelScale}
      />
      <div
        style={{
          position: "absolute",
          inset: 0,
          zIndex: 2,
          transform: `translateY(${floatY}px) scale(${visualScale})`,
          opacity: visualAlpha,
          transformOrigin: "center center",
          willChange: "transform, opacity",
          pointerEvents: "none",
        }}
      >
        <img
          alt=""
          src={gemImageForScale(displayedScale)}
          style={{
            width: "100%",
            height: "100%",
            objectFit: "contain",
            filter: hovered
              ? "drop-shadow(-10px 6px 38px rgba(0,0,0,0.55))"
              : "drop-shadow(-10px 5px 34px rgba(0,0,0,0.5))",
            transition: "filter 200ms ease",
          }}
        />
      </div>
    </div>
  );
}
