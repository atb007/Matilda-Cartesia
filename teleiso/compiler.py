"""
Teleiso — Teletype-inspired lines that compile to isobar patterns.

Not Teletype firmware or syntax; same *shape*: one short line → one musical idea.
Isobar stays the engine. Drop to Python via teleiso.run_python() when needed.
"""

from __future__ import annotations

import random
from dataclasses import dataclass, field
from typing import Any, Dict, List, Optional, Union

from isobar import (
    Key,
    PConcatenate,
    PDegree,
    PEuclidean,
    PLoop,
    PMapEnumerated,
    PBrown,
    PPingPong,
    PSeries,
    PSequence,
    Pattern,
    Timeline,
)

from teleiso.patterns import (
    PHoldOrMuteOnRest,
    make_nudge_mutator,
    make_phrase_mutator,
    scale_offsets,
)


@dataclass
class CompiledPatch:
    tempo: int = 120
    key: Key = field(default_factory=lambda: Key("C", "major"))
    octave: int = 4
    gate: float = 0.92
    degree: Optional[Pattern] = None
    note: Optional[Pattern] = None
    duration: Union[float, Pattern] = 0.25
    amplitude: Optional[Pattern] = None
    active: Optional[Pattern] = None
    title: str = "Teleiso patch"
    midi_port: str = "IAC Driver Bus 1"
    use_virtual_midi: bool = False
    seed: Optional[int] = None

    def schedule_dict(self) -> Dict[str, Any]:
        payload: Dict[str, Any] = {
            "duration": self.duration,
            "gate": self.gate,
        }
        if self.note is not None:
            payload["note"] = self.note
        else:
            payload["degree"] = self.degree
            payload["key"] = self.key
            payload["octave"] = self.octave
        if self.amplitude is not None:
            payload["amplitude"] = self.amplitude
        if self.active is not None:
            payload["active"] = self.active
        return payload


class CompileError(Exception):
    pass


class _State:
    def __init__(self):
        self.tempo = 120
        self.root = "C"
        self.mode = "major"
        self.octave = 4
        self.gate = 0.92
        self.title = "Teleiso patch"
        self.midi_port = "IAC Driver Bus 1"
        self.use_virtual_midi = False
        self.seed: Optional[int] = None

        self.degrees: List[int] = []
        self.degree_pattern: Optional[Pattern] = None
        self.note_pattern: Optional[Pattern] = None

        self.rhythm_stack: List[Pattern] = []
        self.rhythm_pattern: Optional[Pattern] = None

        self.velocity_pattern: Optional[Pattern] = None
        self.duration: Union[float, Pattern] = 0.25

        self.use_active_gate = False
        self.hold_prob: Optional[float] = None
        self.pingpong = False
        self.mutate: Optional[tuple] = None  # (phrase_len, max_prob)
        self.nudge_prob: Optional[float] = None

        self._loop_pending = False


def _nums(tokens: List[str]) -> List[float]:
    out: List[float] = []
    for tok in tokens:
        try:
            if "." in tok:
                out.append(float(tok))
            else:
                out.append(float(int(tok)))
        except ValueError as exc:
            raise CompileError(f"expected number, got {tok!r}") from exc
    return out


def _strip_comment(line: str) -> str:
    if "#" in line:
        line = line.split("#", 1)[0]
    return line.strip()


def compile(source: Union[str, List[str]]) -> CompiledPatch:
    """Parse Teleiso source and return a CompiledPatch (no MIDI yet)."""
    if isinstance(source, str):
        lines = source.splitlines()
    else:
        lines = list(source)

    st = _State()

    for lineno, raw in enumerate(lines, start=1):
        line = _strip_comment(raw)
        if not line:
            continue
        tokens = line.split()
        op = tokens[0].upper()
        args = tokens[1:]

        try:
            _dispatch(st, op, args)
        except CompileError:
            raise
        except Exception as exc:
            raise CompileError(f"line {lineno}: {line!r} — {exc}") from exc

    return _finalize(st)


def _dispatch(st: _State, op: str, args: List[str]) -> None:
    if op == "TEMPO":
        st.tempo = int(_nums(args)[0])
    elif op == "KEY":
        if len(args) < 2:
            raise CompileError("KEY needs root and mode, e.g. KEY C major")
        st.root, st.mode = args[0], " ".join(args[1:])
    elif op == "OCT":
        st.octave = int(_nums(args)[0])
    elif op == "GATE":
        st.gate = float(_nums(args)[0])
    elif op == "LEGATO":
        st.gate = 1.0
    elif op == "SEED":
        st.seed = int(_nums(args)[0])
    elif op == "TITLE":
        st.title = " ".join(args) or st.title
    elif op == "OUT":
        if not args:
            raise CompileError("OUT needs IAC or VIRTUAL")
        target = args[0].upper()
        if target == "IAC":
            st.use_virtual_midi = False
        elif target == "VIRTUAL":
            st.use_virtual_midi = True
        else:
            st.midi_port = " ".join(args)
    elif op == "DEG":
        st.degrees = [int(n) for n in _nums(args)]
        st.degree_pattern = PSequence(st.degrees, 1)
        st.pingpong = False
        st.mutate = None
        st.nudge_prob = None
    elif op == "PINGPONG":
        st.pingpong = True
    elif op == "MUTATE":
        nums = _nums(args)
        phrase_len = int(nums[0])
        max_prob = float(nums[1]) if len(nums) > 1 else 0.78
        st.mutate = (phrase_len, max_prob)
    elif op == "NUDGE":
        st.nudge_prob = float(_nums(args)[0])
    elif op == "ER":
        nums = _nums(args)
        if len(nums) < 2:
            raise CompileError("ER needs hits and steps, e.g. ER 5 16")
        hits, steps = int(nums[0]), int(nums[1])
        phase = int(nums[2]) if len(nums) > 2 else 0
        st.rhythm_stack.append(PEuclidean(hits, steps, phase=phase))
    elif op == "CONCAT":
        if not st.rhythm_stack:
            raise CompileError("CONCAT with empty rhythm stack")
        st.rhythm_pattern = (
            st.rhythm_stack[0]
            if len(st.rhythm_stack) == 1
            else PConcatenate(st.rhythm_stack)
        )
        st.rhythm_stack = []
    elif op == "VEL":
        values = [int(n) for n in _nums(args)]
        st.velocity_pattern = PSequence(values, 1)
    elif op == "BROWN":
        nums = _nums(args)
        lo, hi = int(nums[0]), int(nums[1])
        if st.velocity_pattern is None:
            st.velocity_pattern = PBrown(0, 1, lo, hi)
        else:
            st.velocity_pattern = st.velocity_pattern + PBrown(0, 1, lo, hi)
    elif op == "DUR":
        if not args:
            raise CompileError("DUR needs a beat value or ALT …")
        if args[0].upper() == "ALT":
            nums = _nums(args[1:])
            if len(nums) != 4:
                raise CompileError("DUR ALT beat1 count1 beat2 count2")
            b1, c1, b2, c2 = nums
            st.duration = PSequence([float(b1)] * int(c1) + [float(b2)] * int(c2), 1)
        else:
            st.duration = float(_nums(args)[0])
    elif op == "ACTIVE":
        st.use_active_gate = True
    elif op == "HOLD":
        st.hold_prob = float(_nums(args)[0])
    elif op == "LOOP":
        st._loop_pending = True
        _apply_loop(st)
    elif op == "ARP":
        nums = _nums(args)
        if len(nums) < 3:
            raise CompileError("ARP needs start step increment count, e.g. ARP 0 2 4")
        start, step, count = int(nums[0]), int(nums[1]), int(nums[2])
        series = PSeries(start, step, count)
        key = Key(st.root, st.mode)
        st.note_pattern = PDegree(series, key) + 60
    elif op == "RUN":
        pass
    else:
        raise CompileError(f"unknown op {op!r}")


def _apply_loop(st: _State) -> None:
    if st.degree_pattern is not None and st._loop_pending:
        pat = st.degree_pattern
        if st.pingpong:
            pat = PPingPong(pat)
            st.pingpong = False
        offsets = scale_offsets(st.root, st.mode)
        if st.mutate:
            phrase_len, max_prob = st.mutate
            fn = make_phrase_mutator(phrase_len, max_prob, offsets)
            pat = PMapEnumerated(pat, fn)
            st.mutate = None
        elif st.nudge_prob is not None:
            phrase_len = len(st.degrees) or 8
            fn = make_nudge_mutator(phrase_len, st.nudge_prob, offsets)
            pat = PMapEnumerated(pat, fn)
            st.nudge_prob = None
        st.degree_pattern = PLoop(pat)
    if st.rhythm_pattern is not None:
        st.rhythm_pattern = PLoop(st.rhythm_pattern)
    elif st.rhythm_stack:
        merged = (
            st.rhythm_stack[0]
            if len(st.rhythm_stack) == 1
            else PConcatenate(st.rhythm_stack)
        )
        st.rhythm_pattern = PLoop(merged)
        st.rhythm_stack = []
    if st.velocity_pattern is not None:
        st.velocity_pattern = PLoop(st.velocity_pattern)
    if isinstance(st.duration, Pattern):
        st.duration = PLoop(st.duration)
    if st.note_pattern is not None:
        pat = st.note_pattern
        if st.pingpong:
            pat = PPingPong(pat)
            st.pingpong = False
        st.note_pattern = PLoop(pat)
    st._loop_pending = False


def _finalize(st: _State) -> CompiledPatch:
    if st.seed is not None:
        random.seed(st.seed)

    key = Key(st.root, st.mode)
    offsets = scale_offsets(st.root, st.mode)

    degree = st.degree_pattern
    active = None

    if st.hold_prob is not None:
        if degree is None or st.rhythm_pattern is None:
            raise CompileError("HOLD needs DEG … LOOP and ER … CONCAT LOOP first")
        initial = st.degrees[0] if st.degrees else offsets[0]
        degree = PHoldOrMuteOnRest(
            degree,
            st.rhythm_pattern,
            hold_prob=st.hold_prob,
            initial_degree=initial,
        )
    elif st.use_active_gate:
        if degree is None or st.rhythm_pattern is None:
            raise CompileError("ACTIVE needs DEG … LOOP and ER … CONCAT LOOP first")
        active = st.rhythm_pattern

    if degree is None and st.note_pattern is None:
        raise CompileError("patch needs DEG or ARP")

    return CompiledPatch(
        tempo=st.tempo,
        key=key,
        octave=st.octave,
        gate=st.gate,
        degree=degree,
        note=st.note_pattern,
        duration=st.duration,
        amplitude=st.velocity_pattern,
        active=active,
        title=st.title,
        midi_port=st.midi_port,
        use_virtual_midi=st.use_virtual_midi,
        seed=st.seed,
    )


def compile_file(path: str) -> CompiledPatch:
    with open(path, encoding="utf-8") as fh:
        return compile(fh.read())


def preview(pattern: Pattern, n: int = 16) -> List[Any]:
    values = []
    for _ in range(n):
        try:
            values.append(Pattern.value(pattern))
        except StopIteration:
            values.append(None)
    return values


def run_python(build_fn, tempo: int = 120, **schedule_kw) -> CompiledPatch:
    """Escape hatch: pass any isobar build function unchanged."""
    built = build_fn()
    if isinstance(built, CompiledPatch):
        return built
    if isinstance(built, dict):
        return CompiledPatch(tempo=tempo, **built, **schedule_kw)
    raise TypeError("build_fn must return CompiledPatch or dict")


def run_timeline(patch: CompiledPatch, output_device=None) -> Timeline:
    timeline = Timeline(patch.tempo, output_device=output_device)
    timeline.schedule(patch.schedule_dict())
    return timeline
