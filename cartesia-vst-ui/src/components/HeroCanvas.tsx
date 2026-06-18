import { AURORA } from "../auroraConfig";
import { FRAME_H, HERO, HERO_MAIN_LEFT } from "../heroLayout";
import { AuroraShader } from "./AuroraShader";

/**
 * Starfield bg, masked Matilda portrait, wordmark.
 * Clipped by the collapsing viewport — no independent slide animation.
 */
export function HeroCanvas() {
  const portraitInMaskLeft = HERO.portrait.left - HERO.mask.left;
  const portraitInMaskTop = HERO.portrait.top - HERO.mask.top;

  return (
    <>
      {/* Static starfield + forest raster + aurora (same MainFrame bounds) */}
      <div
        style={{
          position: "absolute",
          left: HERO_MAIN_LEFT,
          top: HERO.bg.top,
          width: HERO.bg.width,
          height: HERO.bg.height,
          overflow: "hidden",
          pointerEvents: "none",
          zIndex: 0,
        }}
      >
        <img
          alt=""
          src="/assets/hero-bg-m8b.png"
          style={{
            position: "absolute",
            width: "107.58%",
            height: "103.2%",
            left: 0,
            top: "-1.74%",
            maxWidth: "none",
          }}
        />
        <AuroraShader
          style={{
            WebkitMaskImage: AURORA.maskGradient,
            maskImage: AURORA.maskGradient,
          }}
        />
      </div>

      <div
        style={{
          position: "absolute",
          left: HERO_MAIN_LEFT,
          top: 0,
          width: HERO.mainW,
          height: FRAME_H,
          overflow: "hidden",
          pointerEvents: "none",
          zIndex: 1,
        }}
      >
        <div
          style={{
            position: "absolute",
            left: HERO.mask.left,
            top: HERO.mask.top,
            width: HERO.mask.width,
            height: HERO.mask.height,
            overflow: "hidden",
            WebkitMaskImage: "url(/assets/matilda-mask-alpha.svg)",
            maskImage: "url(/assets/matilda-mask-alpha.svg)",
            WebkitMaskSize: "100% 100%",
            maskSize: "100% 100%",
            WebkitMaskRepeat: "no-repeat",
            maskRepeat: "no-repeat",
          }}
        >
          <img
            alt=""
            src="/assets/matilda-portrait-v2.png"
            style={{
              position: "absolute",
              left: portraitInMaskLeft,
              top: portraitInMaskTop,
              width: HERO.portrait.width,
              height: HERO.portrait.height * 1.1634,
              maxWidth: "none",
            }}
          />
        </div>

        <div
          style={{
            position: "absolute",
            left: HERO.label.left,
            top: HERO.label.top,
            width: HERO.label.width,
            textAlign: "right",
            fontFamily: "'Jacquard 24', serif",
            lineHeight: 1,
          }}
        >
          <p style={{ margin: 0, fontSize: 180, color: "#fff", height: 120.7 }}>
            Matilda
          </p>
          <p style={{ margin: "21px 0 0", fontSize: 60, color: "#df90e5", height: 60 }}>
            Cartesia - v1.0
          </p>
        </div>
      </div>
    </>
  );
}
