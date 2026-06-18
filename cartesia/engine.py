"""Matilda v2 stepping engine — sequential layers + path movement."""

from __future__ import annotations

import random
from dataclasses import dataclass
from typing import List, Optional, Tuple

from isobar import Key

from cartesia.model import Layer, Patch
from cartesia.movement import MovementMode, advance_path


SCALE_OFFSETS = {
    "major": [0, 2, 4, 5, 7, 9, 11],
    "minor": [0, 2, 3, 5, 7, 8, 10],
    "dorian": [0, 2, 3, 5, 7, 9, 10],
    "phrygian": [0, 1, 3, 5, 7, 8, 10],
    "lydian": [0, 2, 4, 6, 7, 9, 11],
    "mixolydian": [0, 2, 4, 5, 7, 9, 10],
    "locrian": [0, 1, 3, 5, 6, 8, 10],
    "harmonic_minor": [0, 2, 3, 5, 7, 8, 11],
    "melodic_minor": [0, 2, 3, 5, 7, 9, 11],
    "pentatonic": [0, 2, 4, 7, 9],
    "pentatonic_minor": [0, 3, 5, 7, 10],
    "blues": [0, 3, 5, 6, 7, 10],
    "ionian": [0, 2, 4, 5, 7, 9, 11],
    "aeolian": [0, 2, 3, 5, 7, 8, 10],
}


@dataclass
class StepEvent:
    degree: Optional[int]
    velocity: Optional[int]
    x: int
    y: int
    z: int
    fired: bool
    note: Optional[str] = None


class CartesiaEngine:
    """v2 engine: one path step per master tick; layers play sequentially."""

    STEPS_PER_LAYER_PASS = 16

    def __init__(self, patch: Patch):
        self.patch = patch
        self.key = Key(patch.root, patch.mode)
        mode_key = patch.mode.lower()
        self.scale = SCALE_OFFSETS.get(mode_key, SCALE_OFFSETS["major"])
        self.master_tick = 0
        self._playing_layer_idx = self._first_active_layer()
        self._steps_on_layer = 0
        self._cached_primary: Optional[StepEvent] = None
        self._fields_read = 0
        self._rng = random.Random(patch.seed)

    def _first_active_layer(self) -> int:
        for i, layer in enumerate(self.patch.layers):
            if layer.active:
                return i
        return 0

    def _find_next_active_layer(self, from_layer: int) -> int:
        for i in range(1, 5):
            idx = (from_layer + i) % 4
            if self.patch.layers[idx].active:
                return idx
        return 0

    def _reconcile_playing_layer(self) -> None:
        if self.patch.layers[self._playing_layer_idx].active:
            return
        self._playing_layer_idx = self._find_next_active_layer(self._playing_layer_idx)
        self._steps_on_layer = 0

    def current(self) -> StepEvent:
        if self._cached_primary is None:
            self._do_step()
        return self._cached_primary

    def mark_field_read(self) -> None:
        self._fields_read += 1
        if self._fields_read >= 3:
            self._fields_read = 0
            self._cached_primary = None

    def reset(self) -> None:
        self.master_tick = 0
        self._playing_layer_idx = self._first_active_layer()
        self._steps_on_layer = 0
        for layer in self.patch.layers:
            layer.path.step_index = 0
            layer.path.direction = 1
            layer.path.at_end_hold = 0
            layer.path.random_bag.clear()
            layer.path.random_bag_pos = 0
            layer.step_index = 0
            layer.step_dir = 1
        self._cached_primary = None

    def _playing_layer_index(self) -> int:
        return self._playing_layer_idx

    def _chance(self, prob: float) -> bool:
        return self._rng.random() < max(0.0, min(1.0, prob))

    def _map_degree(self, raw: int) -> int:
        if self.patch.quantize:
            span = max(1, len(self.scale))
            return self.scale[raw % span]
        return raw

    def _apply_jitter(self, semitone: int, cell) -> int:
        if not cell.jitter_armed or cell.jitter_amount <= 0:
            return semitone
        span = max(1, len(self.scale)) if self.patch.quantize else 12
        max_delta = max(1, int(round(cell.jitter_amount * span)))
        delta = self._rng.randint(-max_delta, max_delta)
        return semitone + delta

    def _resolve_cell(self, layer: Layer, x: int, y: int) -> Tuple[Optional[int], Optional[int], bool]:
        cell = layer.get(x, y)
        if not cell.gate:
            return None, None, False
        if not self._chance(cell.effective_trigger_prob()):
            return None, None, False

        semitone = self._map_degree(cell.degree)
        semitone += cell.octave_offset * 12
        semitone = self._apply_jitter(semitone, cell)
        return semitone, cell.velocity, True

    def _advance_layer_path(self, layer: Layer) -> None:
        advance_path(
            layer.movement,
            layer.path,
            random_skip_prob=layer.random_skip_prob,
            rng=self._rng,
        )
        layer.step_index = layer.path.step_index
        layer.step_dir = layer.path.direction

    def _maybe_switch_layer(self) -> None:
        self._steps_on_layer += 1
        if self._steps_on_layer < self.STEPS_PER_LAYER_PASS:
            return
        self._steps_on_layer = 0
        self._playing_layer_idx = self._find_next_active_layer(self._playing_layer_idx)

    def step(self) -> List[StepEvent]:
        self._do_step()
        return [self._cached_primary]

    def _do_step(self) -> List[StepEvent]:
        self.master_tick += 1
        self._reconcile_playing_layer()
        layer_idx = self._playing_layer_index()
        layer = self.patch.layers[layer_idx]
        x, y = layer.path.xy()

        degree, velocity, fired = self._resolve_cell(layer, x, y)

        self._advance_layer_path(layer)
        self._maybe_switch_layer()

        event = StepEvent(
            degree=degree,
            velocity=velocity,
            x=x,
            y=y,
            z=layer_idx,
            fired=fired,
        )
        self._cached_primary = event
        return [event]

    def preview_steps(self, n: int = 16) -> List[StepEvent]:
        clone = CartesiaEngine(self.patch)
        clone.reset()
        out: List[StepEvent] = []
        for _ in range(n):
            out.append(clone._do_step()[0])
        return out

    def playhead(self) -> Tuple[int, int, int]:
        layer_idx = self._playing_layer_index()
        layer = self.patch.layers[layer_idx]
        x, y = layer.path.xy()
        return x, y, layer_idx
