# BlueARP → Matilda — future enhancements backlog

Reference: **BlueARP Operation Manual v2.2.9** (Oleg Mikheev / Graywolf).  
Matilda basic MIDI + GarageBand standalone testing is **done** (Jun 2026). This doc captures ideas to borrow later — not committed scope.

---

## Already in Matilda (test build)

| Area | Status |
|------|--------|
| Mono MIDI output (one note at a time) | ✅ |
| 4×4 grid + sequential layers + movement modes | ✅ |
| Trigger probability + jitter (engine) | ✅ |
| Master clock divisions (BlueARP `sync` set — 13 values) | ✅ |
| GarageBand transport via MIDI Start/Stop/Clock (Standalone + IAC) | ✅ |
| Quantise scale panel (tonic, min/max note window, scale list) | ✅ (functional UI) |

---

## DAW routing (plugin shipping)

BlueARP is a **VST/AU MIDI-FX** inserted before a synth. Matilda should ship the same for FL Studio, Ableton, Logic, Reaper.

| DAW | Pattern |
|-----|---------|
| **FL Studio** | Fruity Wrapper: BlueARP **output port** → synth **MIDI input port** (ports 11+). Or **Patcher**: BlueARP → generator inside patcher. |
| **Ableton Live** | 3-track chain: MIDI clips → BlueARP (Monitor In) → synth (MIDI From BlueARP). |
| **Reaper** | Synth track receives from BlueARP; synth direct keyboard input = None. |
| **Logic** | AU MIDI-FX slot on instrument track. |
| **GarageBand** | No third-party MIDI-FX → keep **Standalone → IAC** workflow. |

**Matilda action:** Build VST3 + AU MIDI effect; use JUCE `AudioPlayHead` for BPM/position in plugin mode. **One codebase** — Standalone for GarageBand + IAC; VST3/AU for FL Studio (Fruity Wrapper output port or Patcher), Ableton, Logic, Reaper. No fork required.

---

## Timing & transport (BlueARP Arp Engine)

| BlueARP control | Values / behaviour | Matilda idea |
|-----------------|-------------------|--------------|
| **sync** (step length) | 1/64 … 3/8 (13 divisions) | ✅ Done (`ClockDivisions.h`) |
| **gate time** | 1–125% of step length | Re-add staccato gates; host-tempo-aware |
| **swing** | −50% … +50% on even steps | Groove / shuffle layer |
| **restart on** | beat · key · 1st key | Align pattern to bar vs retrig on new note |
| **steps** | 0–64 (0 = MIDI thru) | Optional longer patterns; chain mode |
| **Host song position** | Steps locked to DAW timeline | Plugin mode: `getPpqPosition()` phase lock |

---

## Input filter (held-note / live performance)

| BlueARP control | Matilda idea |
|-----------------|--------------|
| **input range filter** + pass-through | Keyboard split across instances |
| **input range wrap** | Wrap out-of-range notes into window |
| **order algorithm** | by pitch, as played, chord normalize, … |
| **missing keys substitution** | cyclic / first / last / fixed when pattern needs more keys than held |
| **in quantize** | Capture held notes on 1/16, 1/8, 1/4, … |
| **arp latch** | Keep pattern after keys released (sustain pedal map) |

Matilda v1 policy: **mono input — lowest held note = root** (Cartesia). Chord modes explicitly out of scope unless product changes.

---

## Pitch & scale (BlueARP force-to-scale)

| BlueARP control | Matilda idea |
|-----------------|--------------|
| **force to scale: root** | Fixed vs detect from chord |
| **force to scale: scale** | Extended scale list (see engine) |
| **SCALE STEP bar** | Scale-degree transpose per step (not semitones) |
| **OCTAVE bar** | Per-step octave shift |
| **Output range wrap** | Clamp/wrap output to min…max window |

**Jitter fix (SPEC):** jitter should move in **scale-degree space**, then snap to quantised set — not raw semitone delta.

---

## Step programming (matrix / value bars)

| BlueARP value bar | Matilda mapping |
|-------------------|-----------------|
| **VELOCITY** | Per-cell `velocity` (partial — global default today) |
| **GATE TIME** | Per-step gate multiplier |
| **STEP TYPE** | Off · Normal · Rest · Tie · Chord |
| **KEY SELECT** | N/A for grid-first Matilda (fixed grid degrees) |
| **SCALE STEP / OCTAVE** | Could map to cell degree + `octave_offset` macros |

---

## Output filter & randomisation

| BlueARP control | Matilda idea |
|-----------------|--------------|
| **transp. oct / transpose** | Global transpose macros |
| **rand. velo / gate / start** | Output jitter macros (complement cell-level trigger/jitter) |

---

## Program chains & performance

| BlueARP feature | Matilda idea |
|-----------------|--------------|
| **Program chains** (up to 16) | Queue presets / layer sets for live switching |
| **chain quantize** | Switch chains on bar boundary |
| **bank/patch/volume on chain switch** | Hardware synth program change |
| **128 programs per bank** | JSON preset library |
| **Pattern shift** | Cyclic 1-step nudge to align with beat |
| **Page lock / auto scroll** | Long patterns (>16 steps) — Phase B |

---

## MIDI plumbing (Settings page)

| BlueARP setting | Matilda idea |
|-----------------|--------------|
| **prog.change / pitch bend / mod / aftertouch / CC** | ignore vs pass-through to downstream synth |
| **velocity scale** | Fine vs coarse velocity editing |
| **MIDI output port** (FL Studio) | Expose in plugin settings when standalone/port routing needed |

---

## Suggested implementation order

1. **VST3/AU MIDI-FX** + host playhead sync (FL Studio validation).
2. **Scale-quantized jitter** + any remaining scale engine gaps.
3. **Gate time %** + **swing** (timing feel).
4. **Restart on** modes + **input quantize** (live play).
5. **Step types** (Rest/Tie) + per-step gate/velocity bars.
6. **Program chains** + preset browser.
7. **CC pass-through** + output randomisation macros.

---

## References

- BlueARP manual PDF (project Case Studies folder)
- Matilda spec: `SPEC.md`
- GarageBand path: `matilda/plugin/README.md`
- Sandbox / routing PRD: `PRD-SANDBOX.md`
