/**
 * Aurora shader tuning — edit these values to fine-tune the hero sky effect.
 * Used by `AuroraShader.tsx` (WebGL) and mask gradient in `HeroCanvas.tsx`.
 */
export const AURORA = {
  /** Overall colour brightness multiplier. ↑ brighter · ↓ subtler */
  colorGain: 2.35,
  /** Overall opacity multiplier. ↑ more visible · ↓ more transparent */
  alphaGain: 0.82,
  /** < 1 = brighter mid-tones · > 1 = softer / mistier */
  intensityPow: 0.88,

  /** Layer weights inside the curtain */
  bands: 1.0,
  shimmer: 0.52,
  rays: 0.32,

  /** Horizontal wave on the curtain edge (0–0.1) */
  waveAmp1: 0.065,
  waveAmp2: 0.038,

  /** Curtain: strong when skyY is high (top of frame). Second value = horizon falloff */
  curtainTop: 0.95,
  curtainLow: 0.12,

  /**
   * Vertical fade (uv.y: 0 = top, 1 = bottom).
   * Fade starts at `skyFadeFrom`, fully gone by `skyFadeTo`.
   * ↑ skyFadeTo (e.g. 0.88) = aurora reaches lower · ↓ = stays in upper sky only
   */
  skyFadeFrom: 0.4,
  skyFadeTo: 0.82,

  /** Noise scale & drift — `drift` controls animation speed */
  noiseScaleX: 2.2,
  noiseScaleY: 1.3,
  drift: 0.05,

  /** RGB tint — green/teal left, violet/magenta right */
  green: [0.18, 0.95, 0.48] as const,
  teal: [0.1, 0.82, 0.92] as const,
  violet: [0.72, 0.24, 0.92] as const,
  magenta: [0.92, 0.36, 0.82] as const,

  /**
   * CSS mask on the aurora layer (HeroCanvas + fallback).
   * Percentages = top → bottom. Higher middle % = effect visible further down.
   */
  maskGradient:
    "linear-gradient(to bottom, rgba(0,0,0,1) 0%, rgba(0,0,0,1) 55%, rgba(0,0,0,0.72) 72%, rgba(0,0,0,0) 88%)",
} as const;
