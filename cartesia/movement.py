"""Path movement through 16 cells (row-major 4x4)."""

from __future__ import annotations

import random
from dataclasses import dataclass, field
from enum import Enum
from typing import List, Optional


class MovementMode(str, Enum):
    FORWARD = "forward"
    REVERSE = "reverse"
    PING_PONG = "ping_pong"
    PENDULUM = "pendulum"
    RANDOM = "random"
    RANDOM_SKIP = "random_skip"

    # Legacy aliases (v1 presets / migration)
    SCAN = "forward"
    SCAN_PINGPONG = "ping_pong"
    FREE = "forward"
    X_ONLY = "forward"
    Y_ONLY = "forward"
    RANDOM_WALK = "random"


@dataclass
class PathState:
    step_index: int = 0
    direction: int = 1
    at_end_hold: int = 0
    random_bag: List[int] = field(default_factory=list)
    random_bag_pos: int = 0

    def xy(self) -> tuple[int, int]:
        idx = max(0, min(15, self.step_index))
        return idx % 4, idx // 4


def _reshuffle_bag(state: PathState) -> None:
    state.random_bag = list(range(16))
    random.shuffle(state.random_bag)
    state.random_bag_pos = 0


def advance_path(
    mode: MovementMode,
    state: PathState,
    *,
    random_skip_prob: float = 0.0,
    rng: Optional[random.Random] = None,
) -> int:
    """Advance to next step index; returns new index 0..15."""
    r = rng or random

    if mode == MovementMode.FORWARD:
        state.step_index = (state.step_index + 1) % 16
        return state.step_index

    if mode == MovementMode.REVERSE:
        state.step_index = (state.step_index - 1) % 16
        return state.step_index

    if mode in (MovementMode.PING_PONG, MovementMode.PENDULUM):
        hold_needed = 2 if mode == MovementMode.PING_PONG else 1
        if state.at_end_hold > 0:
            state.at_end_hold -= 1
            return state.step_index
        next_idx = state.step_index + state.direction
        if next_idx >= 15:
            state.step_index = 15
            state.direction = -1
            state.at_end_hold = hold_needed - 1
        elif next_idx <= 0:
            state.step_index = 0
            state.direction = 1
            state.at_end_hold = hold_needed - 1
        else:
            state.step_index = next_idx
        return state.step_index

    if mode == MovementMode.RANDOM:
        if not state.random_bag or state.random_bag_pos >= len(state.random_bag):
            _reshuffle_bag(state)
        state.step_index = state.random_bag[state.random_bag_pos]
        state.random_bag_pos += 1
        return state.step_index

    if mode == MovementMode.RANDOM_SKIP:
        for _ in range(16):
            candidate = (state.step_index + 1) % 16
            if r.random() >= max(0.0, min(1.0, random_skip_prob)):
                state.step_index = candidate
                break
            state.step_index = candidate
        return state.step_index

    state.step_index = (state.step_index + 1) % 16
    return state.step_index
