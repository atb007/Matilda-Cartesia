"""Shared isobar pattern helpers used by the Teleiso compiler."""

from __future__ import annotations

import random
from typing import Callable, Iterable, List, Optional

from isobar import Pattern


MAJOR_OFFSETS = [0, 2, 4, 5, 7, 9, 11]
MINOR_OFFSETS = [0, 2, 3, 5, 7, 8, 10]


def scale_offsets(root: str, mode: str) -> List[int]:
    mode = mode.lower()
    if mode in ("major", "ionian"):
        return MAJOR_OFFSETS[:]
    if mode in ("minor", "aeolian", "natural minor"):
        return MINOR_OFFSETS[:]
    raise ValueError(f"unsupported mode: {mode!r} (use major or minor)")


class PHoldOrMuteOnRest(Pattern):
    """On Euclidean rests, mute or re-trigger the previous scale degree."""

    def __init__(
        self,
        degrees: Pattern,
        rhythm: Pattern,
        hold_prob: float = 0.5,
        initial_degree: Optional[int] = None,
    ):
        self.degrees = degrees
        self.rhythm = rhythm
        self.hold_prob = hold_prob
        self.last_degree = initial_degree
        self.last_was_hold = False

    def __repr__(self):
        return f"PHoldOrMuteOnRest({self.degrees!r}, {self.rhythm!r})"

    def __next__(self):
        degree = Pattern.value(self.degrees)
        hit = Pattern.value(self.rhythm)

        if hit:
            if degree is not None:
                self.last_degree = degree
            self.last_was_hold = False
            return self.last_degree

        if random.random() < self.hold_prob:
            self.last_was_hold = True
            return self.last_degree

        self.last_was_hold = False
        return None


def make_phrase_mutator(
    phrase_len: int,
    max_prob: float,
    choices: Iterable[int],
) -> Callable[[int, Optional[int]], Optional[int]]:
    pool = list(choices)

    def mutate(step: int, degree: Optional[int]) -> Optional[int]:
        if degree is None:
            return None
        pos = step % phrase_len
        prob = 0.06 + (pos / max(phrase_len - 1, 1)) * max_prob
        if random.random() < prob:
            return random.choice(pool)
        return degree

    return mutate


def make_nudge_mutator(
    phrase_len: int,
    nudge_prob: float,
    choices: Iterable[int],
) -> Callable[[int, Optional[int]], Optional[int]]:
    pool = list(choices)

    def nudge(step: int, degree: Optional[int]) -> Optional[int]:
        if degree is None:
            return None
        if step % phrase_len != phrase_len - 1:
            return degree
        if random.random() > nudge_prob:
            return degree
        try:
            idx = pool.index(degree)
        except ValueError:
            return degree
        delta = random.choice([-1, 1])
        idx = max(0, min(len(pool) - 1, idx + delta))
        return pool[idx]

    return nudge
