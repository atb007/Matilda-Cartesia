"""Cartesia / Matilda grid data model (v2)."""

from __future__ import annotations

import json
from copy import deepcopy
from dataclasses import asdict, dataclass, field
from typing import Any, Dict, List, Optional

from cartesia.movement import MovementMode, PathState


@dataclass
class Cell:
    degree: int = 0
    gate: bool = True
    velocity: int = 90
    octave_offset: int = 0
    trigger_armed: bool = False
    trigger_prob: float = 0.5
    jitter_armed: bool = False
    jitter_amount: float = 0.5

    # Legacy v1 fields (ignored on load if v2 keys present)
    gate_p: float = 1.0
    pitch_p: float = 1.0
    vel_p: float = 1.0

    def effective_trigger_prob(self) -> float:
        if not self.trigger_armed:
            return 1.0
        return max(0.0, min(1.0, self.trigger_prob))

    def to_dict(self) -> Dict[str, Any]:
        return {
            "degree": self.degree,
            "gate": self.gate,
            "velocity": self.velocity,
            "octave_offset": self.octave_offset,
            "trigger_armed": self.trigger_armed,
            "trigger_prob": self.trigger_prob,
            "jitter_armed": self.jitter_armed,
            "jitter_amount": self.jitter_amount,
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "Cell":
        trigger_armed = bool(data.get("trigger_armed", data.get("gate_p", 1.0) < 1.0))
        return cls(
            degree=int(data.get("degree", 0)),
            gate=bool(data.get("gate", True)),
            velocity=int(data.get("velocity", 90)),
            octave_offset=int(data.get("octave_offset", 0)),
            trigger_armed=trigger_armed,
            trigger_prob=float(data.get("trigger_prob", data.get("gate_p", 0.5))),
            jitter_armed=bool(data.get("jitter_armed", False)),
            jitter_amount=float(data.get("jitter_amount", 0.5)),
        )


def _empty_grid() -> List[List[Cell]]:
    return [[Cell() for _ in range(4)] for _ in range(4)]


@dataclass
class Layer:
    active: bool = False
    movement: MovementMode = MovementMode.FORWARD
    random_skip_prob: float = 0.0
    step_index: int = 0
    step_dir: int = 1
    cells: List[List[Cell]] = field(default_factory=_empty_grid)
    path: PathState = field(default_factory=PathState)

    def get(self, x: int, y: int) -> Cell:
        return self.cells[y][x]

    def sync_path_from_legacy(self) -> None:
        self.path.step_index = self.step_index
        self.path.direction = self.step_dir if self.step_dir in (-1, 1) else 1

    def to_dict(self) -> Dict[str, Any]:
        return {
            "active": self.active,
            "movement": self.movement.value,
            "random_skip_prob": self.random_skip_prob,
            "step_index": self.path.step_index,
            "step_dir": self.path.direction,
            "cells": [[c.to_dict() for c in row] for row in self.cells],
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "Layer":
        raw_cells = data.get("cells")
        if raw_cells is None:
            rows = _empty_grid()
        else:
            rows = [[Cell.from_dict(c) for c in row] for row in raw_cells]
            if len(rows) != 4 or any(len(r) != 4 for r in rows):
                rows = _empty_grid()
        movement_raw = data.get("movement", MovementMode.FORWARD.value)
        try:
            movement = MovementMode(movement_raw)
        except ValueError:
            movement = MovementMode.FORWARD
        layer = cls(
            active=bool(data.get("active", False)),
            movement=movement,
            random_skip_prob=float(data.get("random_skip_prob", 0.0)),
            step_index=int(data.get("step_index", 0)),
            step_dir=int(data.get("step_dir", 1)),
            cells=rows,
        )
        layer.path.step_index = layer.step_index
        layer.path.direction = layer.step_dir if layer.step_dir in (-1, 1) else 1
        return layer


@dataclass
class Patch:
    title: str = "Matilda patch"
    version: int = 2
    root: str = "C"
    mode: str = "major"
    quantize: bool = True
    min_octave: int = 1
    max_octave: int = 9
    master_division: float = 1.0 / 16.0
    play_mode: str = "note"
    play_on_transport: bool = False
    selected_layer: int = 0
    poly_voices: int = 1
    seed: Optional[int] = None
    layers: List[Layer] = field(default_factory=lambda: [Layer() for _ in range(4)])

    def to_dict(self) -> Dict[str, Any]:
        return {
            "title": self.title,
            "version": self.version,
            "root": self.root,
            "mode": self.mode,
            "quantize": self.quantize,
            "min_octave": self.min_octave,
            "max_octave": self.max_octave,
            "master_division": self.master_division,
            "play_mode": self.play_mode,
            "play_on_transport": self.play_on_transport,
            "selected_layer": self.selected_layer,
            "poly_voices": self.poly_voices,
            "seed": self.seed,
            "layers": [layer.to_dict() for layer in self.layers],
        }

    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> "Patch":
        version = int(data.get("version", 1))
        layers_data = data.get("layers") or []
        layers = [Layer.from_dict(item) for item in layers_data[:4]]
        while len(layers) < 4:
            layers.append(Layer())

        if version < 2:
            return _migrate_v1_patch(data, layers)

        if layers and not any(layer.active for layer in layers):
            layers[0].active = True

        return cls(
            title=data.get("title", "Matilda patch"),
            version=version,
            root=data.get("root", "C"),
            mode=data.get("mode", "major"),
            quantize=bool(data.get("quantize", True)),
            min_octave=int(data.get("min_octave", 1)),
            max_octave=int(data.get("max_octave", 9)),
            master_division=float(data.get("master_division", 1.0 / 16.0)),
            play_mode=str(data.get("play_mode", "note")),
            play_on_transport=bool(data.get("play_on_transport", False)),
            selected_layer=max(0, min(3, int(data.get("selected_layer", 0)))),
            poly_voices=max(1, min(4, int(data.get("poly_voices", 1)))),
            seed=data.get("seed"),
            layers=layers,
        )


def _migrate_v1_patch(data: Dict[str, Any], layers: List[Layer]) -> "Patch":
    """Upgrade legacy cartesia v1 preset shape."""
    layers[0].active = True
    movement_raw = data.get("movement", "forward")
    try:
        layers[0].movement = MovementMode(movement_raw)
    except ValueError:
        layers[0].movement = MovementMode.FORWARD

    return Patch(
        title=data.get("title", "Matilda patch"),
        version=2,
        root=data.get("root", "C"),
        mode=data.get("mode", "major"),
        quantize=bool(data.get("quantize", True)),
        min_octave=int(data.get("octave", 4)) - 3,
        max_octave=int(data.get("octave", 4)) + 5,
        master_division=float(data.get("master_division", 0.25)),
        layers=layers,
        seed=data.get("seed"),
    )


def load_patch(path: str) -> Patch:
    with open(path, encoding="utf-8") as fh:
        return Patch.from_dict(json.load(fh))


def save_patch(patch: Patch, path: str) -> None:
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(patch.to_dict(), fh, indent=2)


def demo_patch() -> Patch:
    from pathlib import Path

    default = Path(__file__).resolve().parents[1] / "matilda/presets/default.layer1.json"
    if default.is_file():
        return load_patch(str(default))

    patch = Patch(title="Matilda demo")
    patch.layers[0].active = True
    degrees = [
        [0, 2, 4, 5],
        [2, 4, 5, 7],
        [4, 5, 7, 9],
        [5, 7, 9, 11],
    ]
    for y in range(4):
        for x in range(4):
            cell = patch.layers[0].get(x, y)
            cell.degree = degrees[y][x]
            cell.gate = (x + y) % 3 != 0
    return patch


def clone_patch(patch: Patch) -> Patch:
    return Patch.from_dict(deepcopy(patch.to_dict()))
