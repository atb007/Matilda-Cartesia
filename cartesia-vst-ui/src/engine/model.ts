/** Patch v2 schema — port of cartesia/model.py */

import type { EngineMovementMode } from "./movement";
import { createPathState, type PathState } from "./movement";

export type Cell = {
  degree: number;
  gate: boolean;
  velocity: number;
  octaveOffset: number;
  triggerArmed: boolean;
  triggerProb: number;
  jitterArmed: boolean;
  jitterAmount: number;
};

export function defaultCell(degree = 0): Cell {
  return {
    degree,
    gate: true,
    velocity: 90,
    octaveOffset: 0,
    triggerArmed: false,
    triggerProb: 0.5,
    jitterArmed: false,
    jitterAmount: 0.5,
  };
}

function emptyGrid(): Cell[][] {
  return Array.from({ length: 4 }, () =>
    Array.from({ length: 4 }, () => defaultCell()),
  );
}

export type Layer = {
  active: boolean;
  movement: EngineMovementMode;
  randomSkipProb: number;
  stepIndex: number;
  stepDir: 1 | -1;
  cells: Cell[][];
  path: PathState;
};

export function defaultLayer(active = false): Layer {
  return {
    active,
    movement: "forward",
    randomSkipProb: 0,
    stepIndex: 0,
    stepDir: 1,
    cells: emptyGrid(),
    path: createPathState(),
  };
}

export type Patch = {
  title: string;
  version: number;
  root: string;
  mode: string;
  quantize: boolean;
  minOctave: number;
  maxOctave: number;
  masterDivision: number;
  playMode: string;
  playOnTransport: boolean;
  selectedLayer: number;
  polyVoices: number;
  seed: number | null;
  layers: Layer[];
};

export function defaultPatch(): Patch {
  const layers = Array.from({ length: 4 }, (_, i) => defaultLayer(i === 0));
  return {
    title: "Matilda patch",
    version: 2,
    root: "C",
    mode: "major",
    quantize: true,
    minOctave: 1,
    maxOctave: 9,
    masterDivision: 1 / 16,
    playMode: "transport",
    playOnTransport: false,
    selectedLayer: 0,
    polyVoices: 1,
    seed: null,
    layers,
  };
}

/** Row-major flat index 0…15 for UI grids. */
export function cellAtIndex(layer: Layer, index: number): Cell {
  const x = index % 4;
  const y = Math.floor(index / 4);
  return layer.cells[y][x];
}

export function setCellAtIndex(layer: Layer, index: number, cell: Cell): void {
  const x = index % 4;
  const y = Math.floor(index / 4);
  layer.cells[y][x] = cell;
}

export function flattenLayerCells(layer: Layer): Cell[] {
  const out: Cell[] = [];
  for (let y = 0; y < 4; y++)
    for (let x = 0; x < 4; x++)
      out.push(layer.cells[y][x]);
  return out;
}

type RawCell = Partial<{
  degree: number;
  gate: boolean;
  velocity: number;
  octave_offset: number;
  trigger_armed: boolean;
  trigger_prob: number;
  jitter_armed: boolean;
  jitter_amount: number;
}>;

function cellFromRaw(data: RawCell): Cell {
  return {
    degree: data.degree ?? 0,
    gate: data.gate ?? true,
    velocity: data.velocity ?? 90,
    octaveOffset: data.octave_offset ?? 0,
    triggerArmed: data.trigger_armed ?? false,
    triggerProb: data.trigger_prob ?? 0.5,
    jitterArmed: data.jitter_armed ?? false,
    jitterAmount: data.jitter_amount ?? 0.5,
  };
}

type RawLayer = Partial<{
  active: boolean;
  movement: string;
  random_skip_prob: number;
  step_index: number;
  step_dir: number;
  cells: RawCell[][];
}>;

function layerFromRaw(data: RawLayer): Layer {
  let cells = emptyGrid();
  if (data.cells?.length === 4 && data.cells.every(r => r.length === 4)) {
    cells = data.cells.map(row => row.map(cellFromRaw));
  }
  const movement = (data.movement ?? "forward") as EngineMovementMode;
  const layer: Layer = {
    active: data.active ?? false,
    movement,
    randomSkipProb: data.random_skip_prob ?? 0,
    stepIndex: data.step_index ?? 0,
    stepDir: (data.step_dir === -1 ? -1 : 1) as 1 | -1,
    cells,
    path: createPathState(),
  };
  layer.path.stepIndex = layer.stepIndex;
  layer.path.direction = layer.stepDir;
  return layer;
}

type RawPatch = Partial<{
  title: string;
  version: number;
  root: string;
  mode: string;
  quantize: boolean;
  min_octave: number;
  max_octave: number;
  master_division: number;
  play_mode: string;
  play_on_transport: boolean;
  selected_layer: number;
  poly_voices: number;
  seed: number | null;
  layers: RawLayer[];
}>;

export function patchFromJson(data: RawPatch): Patch {
  const layers = (data.layers ?? []).slice(0, 4).map(layerFromRaw);
  while (layers.length < 4) layers.push(defaultLayer());

  if (!layers.some(l => l.active)) layers[0].active = true;

  return {
    title: data.title ?? "Matilda patch",
    version: data.version ?? 2,
    root: data.root ?? "C",
    mode: data.mode ?? "major",
    quantize: data.quantize ?? true,
    minOctave: data.min_octave ?? 1,
    maxOctave: data.max_octave ?? 9,
    masterDivision: data.master_division ?? 1 / 16,
    playMode: data.play_mode ?? "transport",
    playOnTransport: data.play_on_transport ?? false,
    selectedLayer: Math.max(0, Math.min(3, data.selected_layer ?? 0)),
    polyVoices: Math.max(1, Math.min(4, data.poly_voices ?? 1)),
    seed: data.seed ?? null,
    layers,
  };
}
