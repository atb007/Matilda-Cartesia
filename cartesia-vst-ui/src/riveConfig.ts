/** Rive hero — matilda-cartesia-v3.riv (Artboard + data bind). See matilda/standalone/docs/RIVE_ANIMATION_RULEBOOK.md */
export const RIVE_SRC = "/assets/matilda-cartesia-v3.riv";
export const RIVE_ARTBOARD = "Artboard";
export const RIVE_VIEW_MODEL = "HairStreaksTrimControl";
/** Boolean data bind — true while transport is playing. */
export const RIVE_PLAY_BOOLEAN = "streakVisible";
export const RIVE_BODY_STREAK_BOOLEAN = "bodyStreak";
export const RIVE_BODY_GLOW_BOOLEAN = "bodyGlow";
export const RIVE_FACE_GLOW_VIS_BOOLEAN = "faceGlowVis";
/** Boolean — true while playing AND polyphony on AND ≥2 active layers. */
export const RIVE_FACE_STREAK_VIS_BOOLEAN = "faceStreakVis";

/** Design-px nudge inside mask — Rive artboard framing vs static PNG. */
export const RIVE_PORTRAIT_OFFSET_X = 20;
export const RIVE_PORTRAIT_OFFSET_Y = 0;
/** Slightly larger than static portrait height multiplier (1.1634). */
export const RIVE_PORTRAIT_HEIGHT_SCALE = 1.2;
/** Uniform scale on portrait width/height (1 = design size). */
export const RIVE_PORTRAIT_CONTENT_SCALE = 0.88;

/** Optional macOS Metal GPU nudge — mirrored in matilda/standalone/Source/Rive/RiveHeroConfig.h */
export const RIVE_METAL_RENDER_PAN_X = 0;
export const RIVE_METAL_RENDER_PAN_Y = 0;
