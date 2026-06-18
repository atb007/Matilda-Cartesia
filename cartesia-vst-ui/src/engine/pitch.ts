/** Scale-aware pitch resolution — port of cartesia/engine.py + SPEC note pipeline. */

import type { Cell, Patch } from "./model";
import type { ScaleId } from "../scaleConfig";

const SCALE_OFFSETS: Record<string, number[]> = {
  chromatic: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
  major: [0, 2, 4, 5, 7, 9, 11],
  minor: [0, 2, 3, 5, 7, 8, 10],
  dorian: [0, 2, 3, 5, 7, 9, 10],
  phrygian: [0, 1, 3, 5, 7, 8, 10],
  lydian: [0, 2, 4, 6, 7, 9, 11],
  mixolydian: [0, 2, 4, 5, 7, 9, 10],
  locrian: [0, 1, 3, 5, 6, 8, 10],
  harmonic_minor: [0, 2, 3, 5, 7, 8, 11],
  melodic_minor: [0, 2, 3, 5, 7, 9, 11],
  pentatonic: [0, 2, 4, 7, 9],
  pentatonic_minor: [0, 3, 5, 7, 10],
  blues: [0, 3, 5, 6, 7, 10],
  ionian: [0, 2, 4, 5, 7, 9, 11],
  aeolian: [0, 2, 3, 5, 7, 8, 10],
};

const PITCH_NAMES = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"];

export function rootSemitone(root: string): number {
  const idx = PITCH_NAMES.findIndex(n => n.toLowerCase() === root.toLowerCase());
  return idx >= 0 ? idx : 0;
}

export function scaleOffsets(mode: string): number[] {
  return SCALE_OFFSETS[mode.toLowerCase()] ?? SCALE_OFFSETS.major;
}

/** Map scale degree index → semitone offset within octave. */
export function mapDegree(raw: number, patch: Patch): number {
  if (!patch.quantize) return raw % 12;
  const scale = scaleOffsets(patch.mode);
  return scale[((raw % scale.length) + scale.length) % scale.length];
}

/** Full MIDI note number for a cell (before jitter). */
export function resolveMidi(cell: Cell, patch: Patch): number {
  const semitone = mapDegree(cell.degree, patch);
  const root = rootSemitone(patch.root);
  const baseOct = patch.minOctave + 2;
  const midi = (baseOct + cell.octaveOffset) * 12 + root + semitone;
  return Math.max(0, Math.min(127, midi));
}

export function midiToLabel(midi: number): string {
  const n = Math.max(0, Math.min(127, midi));
  return `${PITCH_NAMES[n % 12]}${Math.floor(n / 12) - 1}`;
}

/** Display label for a cell's degree within the active quantise window. */
export function noteLabelForCell(cell: Cell, patch: Patch): string {
  const midi = resolveMidi(cell, patch);
  const minMidi = (patch.minOctave + 2) * 12 + rootSemitone(patch.root);
  const maxMidi = (patch.maxOctave + 2) * 12 + rootSemitone(patch.root) + 11;
  if (midi < minMidi || midi > maxMidi) return "—";
  return midiToLabel(midi);
}

/** Knob rotation index 0…11 from degree (visual only). */
export function degreeToKnobIndex(degree: number): number {
  return Math.max(0, Math.min(11, degree % 12));
}

/** UI scale id ↔ patch mode string. */
export function patchModeFromScaleId(id: ScaleId): string {
  return id;
}

export function scaleIdFromPatchMode(mode: string): ScaleId {
  const m = mode.toLowerCase() as ScaleId;
  if ((SCALES as readonly string[]).includes(m))
    return m;
  if (m === "ionian") return "major";
  if (m === "aeolian") return "minor";
  return "major";
}
