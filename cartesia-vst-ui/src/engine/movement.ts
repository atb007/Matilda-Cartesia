/** Path movement through 16 cells (row-major 4×4). Port of cartesia/movement.py */

export const ENGINE_MOVEMENT_MODES = [
  "forward",
  "reverse",
  "ping_pong",
  "pendulum",
  "random",
  "random_skip",
] as const;

export type EngineMovementMode = (typeof ENGINE_MOVEMENT_MODES)[number];

export type PathState = {
  stepIndex: number;
  direction: 1 | -1;
  atEndHold: number;
  randomBag: number[];
  randomBagPos: number;
};

export function createPathState(): PathState {
  return {
    stepIndex: 0,
    direction: 1,
    atEndHold: 0,
    randomBag: [],
    randomBagPos: 0,
  };
}

export function pathXY(state: PathState): [number, number] {
  const idx = Math.max(0, Math.min(15, state.stepIndex));
  return [idx % 4, Math.floor(idx / 4)];
}

function reshuffleBag(state: PathState, rng: () => number): void {
  state.randomBag = Array.from({ length: 16 }, (_, i) => i);
  for (let i = 15; i > 0; i--) {
    const j = Math.floor(rng() * (i + 1));
    [state.randomBag[i], state.randomBag[j]] = [state.randomBag[j], state.randomBag[i]];
  }
  state.randomBagPos = 0;
}

export function advancePath(
  mode: EngineMovementMode,
  state: PathState,
  opts: { randomSkipProb?: number; rng?: () => number } = {},
): number {
  const r = opts.rng ?? Math.random;
  const skipProb = Math.max(0, Math.min(1, opts.randomSkipProb ?? 0));

  switch (mode) {
    case "forward":
      state.stepIndex = (state.stepIndex + 1) % 16;
      break;
    case "reverse":
      state.stepIndex = (state.stepIndex + 15) % 16;
      break;
    case "ping_pong":
    case "pendulum": {
      const holdNeeded = mode === "ping_pong" ? 2 : 1;
      if (state.atEndHold > 0) {
        state.atEndHold -= 1;
        break;
      }
      const next = state.stepIndex + state.direction;
      if (next >= 15) {
        state.stepIndex = 15;
        state.direction = -1;
        state.atEndHold = holdNeeded - 1;
      } else if (next <= 0) {
        state.stepIndex = 0;
        state.direction = 1;
        state.atEndHold = holdNeeded - 1;
      } else {
        state.stepIndex = next;
      }
      break;
    }
    case "random":
      if (!state.randomBag.length || state.randomBagPos >= state.randomBag.length) {
        reshuffleBag(state, r);
      }
      state.stepIndex = state.randomBag[state.randomBagPos];
      state.randomBagPos += 1;
      break;
    case "random_skip":
      for (let i = 0; i < 16; i++) {
        const candidate = (state.stepIndex + 1) % 16;
        if (r() >= skipProb) {
          state.stepIndex = candidate;
          break;
        }
        state.stepIndex = candidate;
      }
      break;
  }
  return state.stepIndex;
}
