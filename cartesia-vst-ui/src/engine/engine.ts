/** Matilda v2 stepping engine — port of cartesia/engine.py */

import { advancePath, pathXY } from "./movement";
import type { Cell, Layer, Patch } from "./model";
import { mapDegree, scaleOffsets } from "./pitch";

export type StepEvent = {
  degree: number | null;
  velocity: number | null;
  x: number;
  y: number;
  layerIdx: number;
  stepIndex: number;
  fired: boolean;
};

/** Merge UI patch edits without clobbering live path / scheduler state. */
export function syncPatchFromUI(engine: CartesiaEngine, next: Patch): void {
  const ep = engine.patch;
  ep.title = next.title;
  ep.root = next.root;
  ep.mode = next.mode;
  ep.quantize = next.quantize;
  ep.minOctave = next.minOctave;
  ep.maxOctave = next.maxOctave;
  ep.masterDivision = next.masterDivision;
  ep.playMode = next.playMode;
  ep.playOnTransport = next.playOnTransport;
  ep.selectedLayer = next.selectedLayer;
  ep.polyVoices = next.polyVoices;
  ep.seed = next.seed;

  for (let i = 0; i < 4; i++) {
    ep.layers[i].active = next.layers[i].active;
    ep.layers[i].movement = next.layers[i].movement;
    ep.layers[i].randomSkipProb = next.layers[i].randomSkipProb;
    for (let y = 0; y < 4; y++)
      for (let x = 0; x < 4; x++)
        Object.assign(ep.layers[i].cells[y][x], next.layers[i].cells[y][x]);
  }

  engine.reconcilePlayingLayer();
}

export class CartesiaEngine {
  static readonly STEPS_PER_LAYER_PASS = 16;

  patch: Patch;
  masterTick = 0;
  lastStepIndex = -1;
  lastPlayingLayer = 0;
  lastFired = false;

  /** Pinned layer index (0…3) — only changes after a 16-step pass or forced skip. */
  private playingLayerIdx = 0;
  private stepsOnLayer = 0;
  private rngState: number;

  constructor(patch: Patch) {
    this.patch = patch;
    this.rngState = patch.seed ?? Date.now() & 0xffff;
    this.playingLayerIdx = CartesiaEngine.firstActiveLayer(patch);
  }

  static firstActiveLayer(patch: Patch): number {
    const idx = patch.layers.findIndex(l => l.active);
    return idx >= 0 ? idx : 0;
  }

  reset(): void {
    this.masterTick = 0;
    this.stepsOnLayer = 0;
    this.playingLayerIdx = CartesiaEngine.firstActiveLayer(this.patch);
    this.lastStepIndex = -1;
    this.lastFired = false;
    for (const layer of this.patch.layers) {
      layer.path.stepIndex = 0;
      layer.path.direction = 1;
      layer.path.atEndHold = 0;
      layer.path.randomBag = [];
      layer.path.randomBagPos = 0;
      layer.stepIndex = 0;
      layer.stepDir = 1;
    }
  }

  /** If the pinned layer was deactivated mid-play, skip to the next active layer. */
  reconcilePlayingLayer(): void {
    if (this.patch.layers[this.playingLayerIdx]?.active) return;
    this.playingLayerIdx = this.findNextActiveLayer(this.playingLayerIdx);
    this.stepsOnLayer = 0;
  }

  /** Next active layer in fixed order 0 → 1 → 2 → 3, wrapping. */
  findNextActiveLayer(fromLayer: number): number {
    for (let i = 1; i <= 4; i++) {
      const idx = (fromLayer + i) % 4;
      if (this.patch.layers[idx].active) return idx;
    }
    return 0;
  }

  private playingLayerIndex(): number {
    return this.playingLayerIdx;
  }

  private rng(): number {
    let t = (this.rngState += 0x6d2b79f5);
    t = Math.imul(t ^ (t >>> 15), t | 1);
    t ^= t + Math.imul(t ^ (t >>> 7), t | 61);
    return ((t ^ (t >>> 14)) >>> 0) / 4294967296;
  }

  private chance(prob: number): boolean {
    return this.rng() < Math.max(0, Math.min(1, prob));
  }

  private effectiveTriggerProb(cell: Cell): number {
    return cell.triggerArmed ? Math.max(0, Math.min(1, cell.triggerProb)) : 1;
  }

  private applyJitter(semitone: number, cell: Cell): number {
    if (!cell.jitterArmed || cell.jitterAmount <= 0) return semitone;
    const span = this.patch.quantize
      ? Math.max(1, scaleOffsets(this.patch.mode).length)
      : 12;
    const maxDelta = Math.max(1, Math.round(cell.jitterAmount * span));
    const delta = Math.floor(this.rng() * (2 * maxDelta + 1)) - maxDelta;
    return semitone + delta;
  }

  private resolveCell(layer: Layer, x: number, y: number): { fired: boolean; degree: number | null; velocity: number | null } {
    const cell = layer.cells[y][x];
    if (!cell.gate) return { fired: false, degree: null, velocity: null };
    if (!this.chance(this.effectiveTriggerProb(cell))) {
      return { fired: false, degree: null, velocity: null };
    }
    let semitone = mapDegree(cell.degree, this.patch);
    semitone += cell.octaveOffset * 12;
    semitone = this.applyJitter(semitone, cell);
    return { fired: true, degree: semitone, velocity: cell.velocity };
  }

  private advanceLayerPath(layer: Layer): void {
    advancePath(layer.movement, layer.path, {
      randomSkipProb: layer.randomSkipProb,
      rng: () => this.rng(),
    });
    layer.stepIndex = layer.path.stepIndex;
    layer.stepDir = layer.path.direction;
  }

  private maybeSwitchLayer(): void {
    this.stepsOnLayer += 1;
    if (this.stepsOnLayer < CartesiaEngine.STEPS_PER_LAYER_PASS) return;
    this.stepsOnLayer = 0;
    this.playingLayerIdx = this.findNextActiveLayer(this.playingLayerIdx);
  }

  tick(): StepEvent {
    this.masterTick += 1;
    this.reconcilePlayingLayer();
    const layerIdx = this.playingLayerIndex();
    const layer = this.patch.layers[layerIdx];
    const stepIndex = layer.path.stepIndex;
    const [x, y] = pathXY(layer.path);

    const { fired, degree, velocity } = this.resolveCell(layer, x, y);

    this.lastStepIndex = stepIndex;
    this.lastPlayingLayer = layerIdx;
    this.lastFired = fired;

    this.advanceLayerPath(layer);
    this.maybeSwitchLayer();

    return { degree, velocity, x, y, layerIdx, stepIndex, fired };
  }

  playhead(): { layerIdx: number; stepIndex: number } {
    const layerIdx = this.playingLayerIndex();
    return { layerIdx, stepIndex: this.patch.layers[layerIdx].path.stepIndex };
  }
}
