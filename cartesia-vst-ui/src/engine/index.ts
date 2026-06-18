/** UI movement labels ↔ engine movement ids. */

import type { MovementMode } from "../components/MovementMenu";
import type { EngineMovementMode } from "./movement";

const UI_TO_ENGINE: Record<MovementMode, EngineMovementMode> = {
  forward: "forward",
  reverse: "reverse",
  "ping-pong": "ping_pong",
  pendulum: "pendulum",
  random: "random",
  "random skip": "random_skip",
};

const ENGINE_TO_UI: Record<EngineMovementMode, MovementMode> = {
  forward: "forward",
  reverse: "reverse",
  ping_pong: "ping-pong",
  pendulum: "pendulum",
  random: "random",
  random_skip: "random skip",
};

export function uiMovementToEngine(mode: MovementMode): EngineMovementMode {
  return UI_TO_ENGINE[mode];
}

export function engineMovementToUi(mode: EngineMovementMode): MovementMode {
  return ENGINE_TO_UI[mode];
}

export { CartesiaEngine, syncPatchFromUI } from "./engine";
export type { StepEvent } from "./engine";
export type { Cell, Layer, Patch } from "./model";
export {
  cellAtIndex,
  defaultCell,
  defaultPatch,
  flattenLayerCells,
  patchFromJson,
  setCellAtIndex,
} from "./model";
export {
  degreeToKnobIndex,
  noteLabelForCell,
  patchModeFromScaleId,
  resolveMidi,
  scaleIdFromPatchMode,
} from "./pitch";
