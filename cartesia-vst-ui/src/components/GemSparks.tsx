import { useEffect, useRef, useState } from "react";
import type { ScaleGemPalette } from "../scaleConfig";
import { scaleGemColors, type ScaleId } from "../scaleConfig";

/**
 * Sparse tribal streak sparks — varied arcs from left & right,
 * passing behind the gem then sweeping in front before fading.
 */

type Vec2 = { x: number; y: number };

type Streak = {
  id: number;
  p0: Vec2;
  p1: Vec2;
  p2: Vec2;
  duration: number;
  length: number;
  thickness: number;
  /** 0…1 shifts bezier tension for unique paths */
  tension: number;
};

type Props = {
  scaleId: ScaleId;
  width: number;
  height: number;
  panelScale?: number;
};

const MIN_GAP_MS = 700;
const MAX_GAP_MS = 1800;
const MAX_ALIVE = 7;

function quadPoint(p0: Vec2, p1: Vec2, p2: Vec2, t: number): Vec2 {
  const u = 1 - t;
  return {
    x: u * u * p0.x + 2 * u * t * p1.x + t * t * p2.x,
    y: u * u * p0.y + 2 * u * t * p1.y + t * t * p2.y,
  };
}

function quadTangent(p0: Vec2, p1: Vec2, p2: Vec2, t: number): Vec2 {
  const u = 1 - t;
  return {
    x: 2 * u * (p1.x - p0.x) + 2 * t * (p2.x - p1.x),
    y: 2 * u * (p1.y - p0.y) + 2 * t * (p2.y - p1.y),
  };
}

function makeStreak(id: number, side: "left" | "right", w: number, h: number, ps: number): Streak {
  const cx = w / 2;
  const cy = h / 2;
  const spreadY = h * 0.38;
  const y0 = cy + (Math.random() - 0.5) * spreadY;
  const y2 = cy + (Math.random() - 0.5) * spreadY * 0.9;
  const arch = (Math.random() - 0.5) * h * 0.35;
  const pull = side === "left" ? 0.28 + Math.random() * 0.22 : 0.72 - Math.random() * 0.22;

  const p0: Vec2 =
    side === "left"
      ? { x: -w * 0.08, y: y0 }
      : { x: w * 1.08, y: y0 };
  const p2: Vec2 =
    side === "left"
      ? { x: w * (0.92 + Math.random() * 0.2), y: y2 }
      : { x: w * (0.08 - Math.random() * 0.2), y: y2 };
  const p1: Vec2 = {
    x: cx + (pull - 0.5) * w * 0.35,
    y: cy + arch,
  };

  return {
    id,
    p0,
    p1,
    p2,
    duration: 0.85 + Math.random() * 0.7,
    length: (28 + Math.random() * 36) * ps,
    thickness: (1.4 + Math.random() * 1.6) * ps,
    tension: Math.random(),
  };
}

export function GemSparks({ scaleId, width, height, panelScale = 1 }: Props) {
  const [streaks, setStreaks] = useState<Streak[]>([]);
  const idRef = useRef(0);
  const colors = scaleGemColors(scaleId);

  useEffect(() => {
    setStreaks([]);

    const add = (streak: Streak) => {
      setStreaks(prev => [...prev.slice(-(MAX_ALIVE - 1)), streak]);
      window.setTimeout(
        () => setStreaks(prev => prev.filter(s => s.id !== streak.id)),
        streak.duration * 1000 + 120,
      );
    };

    const spawn = (forceSide?: "left" | "right") => {
      const id = ++idRef.current;
      const side = forceSide ?? (Math.random() < 0.5 ? "left" : "right");
      add(makeStreak(id, side, width, height, panelScale));
    };

    const spawnPair = () => {
      spawn("left");
      window.setTimeout(() => spawn("right"), 40 + Math.random() * 120);
    };

    let timer = 0;
    const schedule = () => {
      const gap = MIN_GAP_MS + Math.random() * (MAX_GAP_MS - MIN_GAP_MS);
      timer = window.setTimeout(() => {
        const roll = Math.random();
        if (roll < 0.5) spawnPair();
        else if (roll < 0.72) {
          spawn();
          window.setTimeout(spawn, 60 + Math.random() * 100);
        } else spawn();
        schedule();
      }, gap);
    };

    spawnPair();
    const kickoff = window.setTimeout(spawn, 200);
    schedule();

    return () => {
      clearTimeout(timer);
      clearTimeout(kickoff);
    };
  }, [scaleId, width, height, panelScale]);

  const layerStyle = {
    position: "absolute" as const,
    inset: 0,
    overflow: "visible" as const,
    pointerEvents: "none" as const,
  };

  return (
    <>
      <div aria-hidden style={{ ...layerStyle, zIndex: 1 }}>
        {streaks.map(s => (
          <StreakSprite key={`b-${s.id}`} streak={s} colors={colors} layer="behind" />
        ))}
      </div>
      <div aria-hidden style={{ ...layerStyle, zIndex: 3 }}>
        {streaks.map(s => (
          <StreakSprite key={`f-${s.id}`} streak={s} colors={colors} layer="front" />
        ))}
      </div>
    </>
  );
}

function StreakSprite({
  streak,
  colors,
  layer,
}: {
  streak: Streak;
  colors: ScaleGemPalette;
  layer: "behind" | "front";
}) {
  const ref = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const start = performance.now();
    const duration = streak.duration * 1000;
    let raf = 0;

    const tick = (now: number) => {
      const t = Math.min(1, (now - start) / duration);
      const eased = t < 0.5 ? 2 * t * t : 1 - (-2 * t + 2) ** 2 / 2;
      const pos = quadPoint(streak.p0, streak.p1, streak.p2, eased);
      const tan = quadTangent(streak.p0, streak.p1, streak.p2, eased);
      const angle = Math.atan2(tan.y, tan.x);

      const behind = eased < 0.42 || pos.y < streak.p1.y - streak.thickness * 2;
      const onLayer = layer === "behind" ? behind : !behind;
      const fadeIn = Math.min(1, eased * 5);
      const fadeOut = eased > 0.78 ? Math.max(0, 1 - (eased - 0.78) / 0.22) : 1;
      const life = fadeIn * fadeOut * Math.sin(Math.min(eased * 1.4, 1) * Math.PI * 0.95);

      const opacity = onLayer
        ? layer === "behind"
          ? life * 0.5
          : life * 0.88
        : 0;
      const stretch = 0.7 + life * 0.5 + streak.tension * 0.15;

      const el = ref.current;
      if (el) {
        el.style.left = `${pos.x}px`;
        el.style.top = `${pos.y}px`;
        el.style.opacity = String(opacity);
        el.style.transform = `translate(-50%, -50%) rotate(${angle}rad) scaleX(${stretch})`;
        el.style.filter = layer === "behind" ? "blur(1px)" : "none";
      }

      if (t < 1) raf = requestAnimationFrame(tick);
    };

    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [streak, layer]);

  return (
    <div
      ref={ref}
      style={{
        position: "absolute",
        left: streak.p0.x,
        top: streak.p0.y,
        width: streak.length,
        height: streak.thickness,
        opacity: 0,
        pointerEvents: "none",
        willChange: "transform, opacity, left, top",
      }}
    >
      {/* outer glow streak */}
      <div
        style={{
          position: "absolute",
          inset: `-${streak.thickness * 0.8}px -4px`,
          borderRadius: streak.thickness * 2,
          background: `linear-gradient(90deg, transparent 0%, ${colors.glow}55 18%, ${colors.core}99 50%, ${colors.glow}55 82%, transparent 100%)`,
        }}
      />
      {/* bright tribal core */}
      <div
        style={{
          position: "absolute",
          left: "12%",
          right: "18%",
          top: "22%",
          bottom: "22%",
          borderRadius: 1,
          background: `linear-gradient(90deg, transparent, ${colors.hot}ee 35%, #ffffff 50%, ${colors.hot}ee 65%, transparent)`,
          boxShadow: `0 0 ${streak.thickness * 4}px ${colors.glow}`,
        }}
      />
      {/* forked tip — tribal flare */}
      <div
        style={{
          position: "absolute",
          right: "-2px",
          top: "50%",
          width: streak.thickness * 2.2,
          height: streak.thickness * 0.5,
          transform: "translateY(-80%) rotate(-18deg)",
          borderRadius: 1,
          background: `linear-gradient(90deg, ${colors.core}, transparent)`,
          opacity: 0.7,
        }}
      />
      <div
        style={{
          position: "absolute",
          right: "-2px",
          top: "50%",
          width: streak.thickness * 2.2,
          height: streak.thickness * 0.5,
          transform: "translateY(20%) rotate(18deg)",
          borderRadius: 1,
          background: `linear-gradient(90deg, ${colors.core}, transparent)`,
          opacity: 0.7,
        }}
      />
    </div>
  );
}
