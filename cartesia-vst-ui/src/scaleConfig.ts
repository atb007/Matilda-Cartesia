/**
 * Scale ids — extend `SCALE_GEM_IMAGES` as art is exported per scale.
 *
 * Engine hookup (M9): scale + tonic + min/max define the quantised pitch set.
 * Grid cells store scale degrees; knob note labels resolve through this config
 * and clamp to the min…max octave window.
 */
export const SCALES = [
  "chromatic",
  "major",
  "minor",
  "dorian",
  "phrygian",
  "lydian",
  "mixolydian",
  "locrian",
  "harmonic_minor",
  "melodic_minor",
  "pentatonic",
  "pentatonic_minor",
  "blues",
] as const;

export type ScaleId = (typeof SCALES)[number];

export const PITCH_CLASSES = [
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B",
] as const;

export type PitchClass = (typeof PITCH_CLASSES)[number];

/** Gem orb art per scale — `public/assets/scale-gem-{id}.png` */
export const SCALE_GEM_IMAGES: Record<ScaleId, string> = {
  chromatic: "/assets/scale-gem-chromatic.png",
  major: "/assets/scale-gem-major.png",
  minor: "/assets/scale-gem-minor.png",
  dorian: "/assets/scale-gem-dorian.png",
  phrygian: "/assets/scale-gem-phrygian.png",
  lydian: "/assets/scale-gem-lydian.png",
  mixolydian: "/assets/scale-gem-mixolydian.png",
  locrian: "/assets/scale-gem-locrian.png",
  harmonic_minor: "/assets/scale-gem-harmonic_minor.png",
  melodic_minor: "/assets/scale-gem-melodic_minor.png",
  pentatonic: "/assets/scale-gem-pentatonic.png",
  pentatonic_minor: "/assets/scale-gem-pentatonic_minor.png",
  blues: "/assets/scale-gem-blues.png",
};

export function gemImageForScale(scale: ScaleId): string {
  return SCALE_GEM_IMAGES[scale];
}

/** Spark / glow palette per scale gem — extend as art lands. */
export type ScaleGemPalette = { core: string; glow: string; hot: string };

export const SCALE_GEM_COLORS: Record<ScaleId, ScaleGemPalette> = {
  chromatic:  { core: "#7ec8ff", glow: "#5a48e8", hot: "#e8f4ff" },
  major:      { core: "#ffc860", glow: "#e87820", hot: "#fff4d8" },
  minor:      { core: "#88b8ff", glow: "#3858c8", hot: "#d8e8ff" },
  dorian:     { core: "#78d8c8", glow: "#289878", hot: "#d8fff0" },
  phrygian:   { core: "#ff8868", glow: "#c83828", hot: "#ffe0d8" },
  lydian:     { core: "#c8a0ff", glow: "#7040d8", hot: "#f0e0ff" },
  mixolydian: { core: "#ffb870", glow: "#d87820", hot: "#fff0d8" },
  locrian:    { core: "#a898b8", glow: "#584868", hot: "#e8e0f0" },
  harmonic_minor: { core: "#9090ff", glow: "#4040c0", hot: "#e0e0ff" },
  melodic_minor:  { core: "#80a0ff", glow: "#3060d0", hot: "#d8e8ff" },
  pentatonic: { core: "#70e8b0", glow: "#28a868", hot: "#d8ffe8" },
  pentatonic_minor: { core: "#68c890", glow: "#208858", hot: "#d0ffe0" },
  blues:      { core: "#6898d0", glow: "#2858a0", hot: "#d8e8ff" },
};

export function scaleGemColors(scale: ScaleId): ScaleGemPalette {
  return SCALE_GEM_COLORS[scale];
}

export function scaleDisplayName(id: ScaleId): string {
  const names: Record<ScaleId, string> = {
    chromatic: "Chromatic",
    major: "Major",
    minor: "Minor",
    dorian: "Dorian",
    phrygian: "Phrygian",
    lydian: "Lydian",
    mixolydian: "Mixolydian",
    locrian: "Locrian",
    harmonic_minor: "Harmonic minor",
    melodic_minor: "Melodic minor",
    pentatonic: "Pentatonic",
    pentatonic_minor: "Pentatonic minor",
    blues: "Blues",
  };
  return names[id];
}

export const OCTAVES = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9] as const;

export function formatOctaveNote(pitch: PitchClass, octave: number): string {
  return `${pitch}${octave}`;
}
