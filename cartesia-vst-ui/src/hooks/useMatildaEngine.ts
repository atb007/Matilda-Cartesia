import { useCallback, useEffect, useRef, useState } from "react";
import type { MovementMode } from "../components/MovementMenu";
import type { ClockDivisionId } from "../transportConfig";
import { CLOCK_DIVISIONS } from "../transportConfig";
import type { PitchClass, ScaleId } from "../scaleConfig";
import {
  CartesiaEngine,
  engineMovementToUi,
  patchFromJson,
  syncPatchFromUI,
  uiMovementToEngine,
  type Cell,
  type Patch,
} from "../engine";
import defaultPreset from "../presets/default.layer1.json";

const DEFAULT_BPM = 120;

function loadDefaultPatch(): Patch {
  return patchFromJson(defaultPreset as Parameters<typeof patchFromJson>[0]);
}

function clockBeats(id: ClockDivisionId): number {
  return CLOCK_DIVISIONS.find(d => d.id === id)?.beats ?? 0.25;
}

export type MatildaEngineState = {
  patch: Patch;
  playing: boolean;
  playheadStep: number;
  playingLayer: number;
  movement: MovementMode;
  scaleId: ScaleId;
  tonic: PitchClass;
  minOctave: number;
  maxOctave: number;
  clockDivision: ClockDivisionId;
  setPlaying: (v: boolean) => void;
  setSelectedLayer: (layer: number) => void;
  toggleLayer: (layer: number) => void;
  setMovement: (mode: MovementMode) => void;
  setScaleId: (id: ScaleId) => void;
  setTonic: (pitch: PitchClass) => void;
  setMinOctave: (oct: number) => void;
  setMaxOctave: (oct: number) => void;
  setClockDivision: (id: ClockDivisionId) => void;
  patchCell: (layer: number, index: number, patch: Partial<Cell>) => void;
  layerCellsFlat: (layer: number) => Cell[];
};

export function useMatildaEngine(): MatildaEngineState {
  const [patch, setPatch] = useState<Patch>(loadDefaultPatch);
  const engineRef = useRef<CartesiaEngine | null>(null);
  if (!engineRef.current) engineRef.current = new CartesiaEngine(patch);

  const [playing, setPlayingState] = useState(false);
  const [playheadStep, setPlayheadStep] = useState(-1);
  const [playingLayer, setPlayingLayer] = useState(-1);
  const [clockDivision, setClockDivisionState] = useState<ClockDivisionId>("1/16");

  const selected = patch.selectedLayer;
  const movement = engineMovementToUi(patch.layers[selected].movement);

  useEffect(() => {
    syncPatchFromUI(engineRef.current!, patch);
  }, [patch]);

  const setPlaying = useCallback((v: boolean) => {
    if (v) {
      engineRef.current!.reset();
      const firstActive = patch.layers.findIndex(l => l.active);
      setPlayheadStep(0);
      setPlayingLayer(firstActive >= 0 ? firstActive : 0);
    } else {
      setPlayheadStep(-1);
      setPlayingLayer(-1);
    }
    setPlayingState(v);
  }, [patch.layers]);

  useEffect(() => {
    if (!playing) return;

    const beats = clockBeats(clockDivision);
    const msPerStep = (60 / DEFAULT_BPM) * beats * 1000;

    const id = window.setInterval(() => {
      const engine = engineRef.current!;
      engine.tick();
      setPlayheadStep(engine.lastStepIndex);
      setPlayingLayer(engine.lastPlayingLayer);
    }, msPerStep);

    return () => window.clearInterval(id);
  }, [playing, clockDivision]);

  const updatePatch = useCallback((fn: (p: Patch) => void) => {
    setPatch(prev => {
      const next = structuredClone(prev);
      fn(next);
      return next;
    });
  }, []);

  const setSelectedLayer = useCallback((layer: number) => {
    updatePatch(p => { p.selectedLayer = layer; });
  }, [updatePatch]);

  const toggleLayer = useCallback((layer: number) => {
    if (layer === 0) return;
    updatePatch(p => {
      p.layers[layer].active = !p.layers[layer].active;
      if (!p.layers[p.selectedLayer].active) {
        const fallback = p.layers.findIndex(l => l.active);
        if (fallback >= 0) p.selectedLayer = fallback;
      }
    });
  }, [updatePatch]);

  const setMovement = useCallback((mode: MovementMode) => {
    updatePatch(p => {
      p.layers[p.selectedLayer].movement = uiMovementToEngine(mode);
    });
  }, [updatePatch]);

  const setScaleId = useCallback((id: ScaleId) => {
    updatePatch(p => { p.mode = id; });
  }, [updatePatch]);

  const setTonic = useCallback((pitch: PitchClass) => {
    updatePatch(p => { p.root = pitch; });
  }, [updatePatch]);

  const setMinOctave = useCallback((oct: number) => {
    updatePatch(p => { p.minOctave = oct; });
  }, [updatePatch]);

  const setMaxOctave = useCallback((oct: number) => {
    updatePatch(p => { p.maxOctave = oct; });
  }, [updatePatch]);

  const setClockDivision = useCallback((id: ClockDivisionId) => {
    setClockDivisionState(id);
    const beats = clockBeats(id);
    updatePatch(p => { p.masterDivision = beats / 4; });
  }, [updatePatch]);

  const patchCell = useCallback((layer: number, index: number, cellPatch: Partial<Cell>) => {
    updatePatch(p => {
      const x = index % 4;
      const y = Math.floor(index / 4);
      Object.assign(p.layers[layer].cells[y][x], cellPatch);
    });
  }, [updatePatch]);

  const layerCellsFlat = useCallback((layer: number): Cell[] => {
    const cells: Cell[] = [];
    for (let y = 0; y < 4; y++)
      for (let x = 0; x < 4; x++)
        cells.push(patch.layers[layer].cells[y][x]);
    return cells;
  }, [patch.layers]);

  return {
    patch,
    playing,
    playheadStep,
    playingLayer,
    movement,
    scaleId: patch.mode as ScaleId,
    tonic: patch.root as PitchClass,
    minOctave: patch.minOctave,
    maxOctave: patch.maxOctave,
    clockDivision,
    setPlaying,
    setSelectedLayer,
    toggleLayer,
    setMovement,
    setScaleId,
    setTonic,
    setMinOctave,
    setMaxOctave,
    setClockDivision,
    patchCell,
    layerCellsFlat,
  };
}
