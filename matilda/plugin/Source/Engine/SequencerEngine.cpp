#include "SequencerEngine.h"

#include "ScaleConfig.h"

#include <algorithm>
#include <vector>

namespace matilda {

namespace {
int noteNumberFromRoot(const juce::String& root) {
    static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    for (int i = 0; i < 12; ++i)
        if (root.equalsIgnoreCase(names[i]))
            return i;
    return 0;
}
} // namespace

SequencerEngine::SequencerEngine(PatchState& patch) : patch_(patch) {
    if (!patch_.layers[0].active)
        patch_.layers[0].active = true;
    for (int i = 0; i < kLayerCount; ++i) {
        if (patch_.layers[static_cast<size_t>(i)].active) {
            playingLayerIdx_ = i;
            break;
        }
    }
}

void SequencerEngine::reset() {
    masterTick_ = 0;
    stepsOnLayer_ = 0;
    lastStepIndex_ = -1;
    lastFired_ = false;
    for (int i = 0; i < kLayerCount; ++i) {
        if (patch_.layers[static_cast<size_t>(i)].active) {
            playingLayerIdx_ = i;
            break;
        }
    }
    for (auto& layer : patch_.layers) {
        layer.stepIndex = 0;
        layer.stepDir = 1;
        layer.stepsOnLayer = 0;
        layer.pathHold = 0;
        layer.randomBag.clear();
        layer.randomBagPos = 0;
    }
}

std::pair<int, int> SequencerEngine::indexToXY(int index) {
    index = juce::jlimit(0, 15, index);
    return {index % kGridSize, index / kGridSize};
}

int SequencerEngine::findNextActiveLayer(int fromLayer) const {
    for (int i = 1; i <= kLayerCount; ++i) {
        const int idx = (fromLayer + i) % kLayerCount;
        if (patch_.layers[static_cast<size_t>(idx)].active)
            return idx;
    }
    return 0;
}

void SequencerEngine::reconcilePlayingLayer() {
    if (patch_.layers[static_cast<size_t>(playingLayerIdx_)].active)
        return;
    playingLayerIdx_ = findNextActiveLayer(playingLayerIdx_);
    stepsOnLayer_ = 0;
}

int SequencerEngine::playheadX() const {
    const auto [x, y] = indexToXY(patch_.layers[static_cast<size_t>(playingLayerIdx_)].stepIndex);
    juce::ignoreUnused(y);
    return x;
}

int SequencerEngine::playheadY() const {
    const auto [x, y] = indexToXY(patch_.layers[static_cast<size_t>(playingLayerIdx_)].stepIndex);
    juce::ignoreUnused(x);
    return y;
}

int SequencerEngine::playingLayer() const { return playingLayerIdx_; }

int SequencerEngine::currentStepIndex() const {
    return patch_.layers[static_cast<size_t>(playingLayerIdx_)].stepIndex;
}

CellState& SequencerEngine::cell(int layer, int x, int y) {
    return patch_.layers[static_cast<size_t>(layer)].cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
}

const CellState& SequencerEngine::cell(int layer, int x, int y) const {
    return patch_.layers[static_cast<size_t>(layer)].cells[static_cast<size_t>(y)][static_cast<size_t>(x)];
}

bool SequencerEngine::rollTrigger(const CellState& cell) const {
    if (!cell.triggerArmed)
        return true;
    const float p = juce::jlimit(0.0f, 1.0f, cell.triggerProb);
    if (p <= 0.0f)
        return false;
    if (p >= 1.0f)
        return true;
    return rng_.nextFloat() < p;
}

int SequencerEngine::resolveMidiNote(const CellState& cell) const {
    const int root = noteNumberFromRoot(patch_.root);
    const int baseOct = patch_.minOctave + 2;

    if (!patch_.quantize || patch_.mode.equalsIgnoreCase("chromatic")) {
        const int semitone = cell.degree % 12;
        return juce::jlimit(0, 127, (baseOct + cell.octaveOffset) * 12 + root + semitone);
    }

    int scaleCount = 7;
    const int* scale = scaleIntervalsForMode(patch_.mode, scaleCount);
    const int scaleIdx = ((cell.degree % scaleCount) + scaleCount) % scaleCount;
    // Match cartesia / GridWalker: scale degree wraps within octave; octave from octaveOffset only.
    return juce::jlimit(0, 127, (baseOct + cell.octaveOffset) * 12 + root + scale[scaleIdx]);
}

int SequencerEngine::resolveFiredMidiNote(const CellState& cell) {
    const int midi = resolveMidiNote(cell);
    if (!cell.jitterArmed || cell.jitterAmount <= 0.f)
        return midi;

    if (!patch_.quantize || patch_.mode.equalsIgnoreCase("chromatic")) {
        const int maxDelta = juce::jmax(1, juce::roundToInt(cell.jitterAmount * 12.f));
        const int delta = rng_.nextInt(maxDelta * 2 + 1) - maxDelta;
        return juce::jlimit(0, 127, midi + delta);
    }

    const auto midis = quantisedMidisInWindow();
    if (midis.empty())
        return midi;

    auto it = std::find(midis.begin(), midis.end(), midi);
    const int idx = it != midis.end() ? static_cast<int>(std::distance(midis.begin(), it)) : 0;

    int scaleCount = 7;
    scaleIntervalsForMode(patch_.mode, scaleCount);
    const int maxDelta = juce::jmax(1, juce::roundToInt(cell.jitterAmount * static_cast<float>(scaleCount)));
    const int delta = rng_.nextInt(maxDelta * 2 + 1) - maxDelta;
    const int newIdx = juce::jlimit(0, static_cast<int>(midis.size()) - 1, idx + delta);
    return midis[static_cast<size_t>(newIdx)];
}

juce::String SequencerEngine::noteLabel(int layer, int x, int y) const {
    const int midi = resolveMidiNote(cell(layer, x, y));
    const int root = noteNumberFromRoot(patch_.root);
    const int minMidi = (patch_.minOctave + 2) * 12 + root;
    const int maxMidi = (patch_.maxOctave + 2) * 12 + root + 11;
    if (midi < minMidi || midi > maxMidi)
        return juce::String::fromUTF8("\xe2\x80\x94"); // em dash

    static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    const int n = juce::jlimit(0, 127, midi);
    return juce::String(names[n % 12]) + juce::String(n / 12 - 1);
}

void SequencerEngine::bumpCellDegree(CellState& cell, int delta) {
    const auto midis = quantisedMidisInWindow();
    if (midis.empty()) {
        cell.degree = juce::jmax(0, cell.degree + delta);
        return;
    }

    const int idx = quantisedNoteIndex(cell);
    const int next = (idx + delta + static_cast<int>(midis.size())) % static_cast<int>(midis.size());
    assignCellFromMidi(cell, midis[static_cast<size_t>(next)]);
}

int SequencerEngine::quantisedNoteIndex(const CellState& cell) const {
    const auto midis = quantisedMidisInWindow();
    if (midis.empty())
        return 0;

    const int current = resolveMidiNote(cell);
    auto it = std::find(midis.begin(), midis.end(), current);
    if (it != midis.end())
        return static_cast<int>(std::distance(midis.begin(), it));

    int nearest = 0;
    int bestDist = 128;
    for (int i = 0; i < static_cast<int>(midis.size()); ++i) {
        const int d = std::abs(midis[static_cast<size_t>(i)] - current);
        if (d < bestDist) {
            bestDist = d;
            nearest = i;
        }
    }
    return nearest;
}

void SequencerEngine::setQuantisedNoteIndex(CellState& cell, int index) const {
    const auto midis = quantisedMidisInWindow();
    if (midis.empty())
        return;

    index = juce::jlimit(0, static_cast<int>(midis.size()) - 1, index);
    assignCellFromMidi(cell, midis[static_cast<size_t>(index)]);
}

int SequencerEngine::knobVisualIndex(const CellState& cell) const {
    const auto midis = quantisedMidisInWindow();
    if (midis.size() <= 1)
        return 0;

    const int idx = quantisedNoteIndex(cell);
    return juce::jlimit(0, 11,
                        juce::roundToInt(11.0f * static_cast<float>(idx)
                                         / static_cast<float>(midis.size() - 1)));
}

std::vector<int> SequencerEngine::quantisedMidisInWindow() const {
    std::vector<int> out;
    const int root = noteNumberFromRoot(patch_.root);
    const int minMidi = (patch_.minOctave + 2) * 12 + root;
    const int maxMidi = (patch_.maxOctave + 2) * 12 + root + 11;

    if (!patch_.quantize || patch_.mode.equalsIgnoreCase("chromatic")) {
        for (int m = minMidi; m <= maxMidi; ++m)
            out.push_back(m);
        return out;
    }

    int scaleCount = 7;
    const int* scale = scaleIntervalsForMode(patch_.mode, scaleCount);
    for (int oct = patch_.minOctave + 2; oct <= patch_.maxOctave + 2; ++oct) {
        for (int i = 0; i < scaleCount; ++i) {
            const int m = juce::jlimit(0, 127, oct * 12 + root + scale[i]);
            if (m >= minMidi && m <= maxMidi)
                out.push_back(m);
        }
    }

    std::sort(out.begin(), out.end());
    out.erase(std::unique(out.begin(), out.end()), out.end());
    return out;
}

void SequencerEngine::assignCellFromMidi(CellState& cell, int midi) const {
    midi = juce::jlimit(0, 127, midi);
    const int root = noteNumberFromRoot(patch_.root);
    const int baseOct = patch_.minOctave + 2;

    if (!patch_.quantize || patch_.mode.equalsIgnoreCase("chromatic")) {
        const int noteOct = midi / 12;
        cell.octaveOffset = noteOct - baseOct;
        cell.degree = ((midi % 12) - root + 12) % 12;
        return;
    }

    int scaleCount = 7;
    const int* scale = scaleIntervalsForMode(patch_.mode, scaleCount);
    const int noteOct = midi / 12;
    const int noteSemi = ((midi % 12) - root + 12) % 12;

    int degreeIndex = 0;
    for (int i = 0; i < scaleCount; ++i) {
        if (scale[i] == noteSemi) {
            degreeIndex = i;
            break;
        }
    }

    cell.degree = degreeIndex;
    cell.octaveOffset = noteOct - baseOct;
}

void SequencerEngine::advancePath(LayerState& layer) {
    switch (layer.movement) {
        case MovementMode::Forward:
            layer.stepIndex = (layer.stepIndex + 1) % 16;
            break;
        case MovementMode::Reverse:
            layer.stepIndex = (layer.stepIndex + 15) % 16;
            break;
        case MovementMode::PingPong:
        case MovementMode::Pendulum: {
            const int holdNeeded = layer.movement == MovementMode::PingPong ? 2 : 1;
            if (layer.pathHold > 0) {
                --layer.pathHold;
                break;
            }
            int next = layer.stepIndex + layer.stepDir;
            if (next >= 15) {
                layer.stepIndex = 15;
                layer.stepDir = -1;
                layer.pathHold = holdNeeded - 1;
            } else if (next <= 0) {
                layer.stepIndex = 0;
                layer.stepDir = 1;
                layer.pathHold = holdNeeded - 1;
            } else {
                layer.stepIndex = next;
            }
            break;
        }
        case MovementMode::Random:
            if (layer.randomBag.empty() || layer.randomBagPos >= static_cast<int>(layer.randomBag.size())) {
                layer.randomBag.resize(16);
                for (int i = 0; i < 16; ++i)
                    layer.randomBag[static_cast<size_t>(i)] = i;
                for (int i = 15; i > 0; --i) {
                    const int j = rng_.nextInt(i + 1);
                    std::swap(layer.randomBag[static_cast<size_t>(i)], layer.randomBag[static_cast<size_t>(j)]);
                }
                layer.randomBagPos = 0;
            }
            layer.stepIndex = layer.randomBag[static_cast<size_t>(layer.randomBagPos++)];
            break;
        case MovementMode::RandomSkip:
            for (int i = 0; i < 16; ++i) {
                const int candidate = (layer.stepIndex + 1) % 16;
                if (rng_.nextFloat() >= juce::jlimit(0.0f, 1.0f, layer.randomSkipProb)) {
                    layer.stepIndex = candidate;
                    break;
                }
                layer.stepIndex = candidate;
            }
            break;
    }
}

void SequencerEngine::maybeSwitchLayer() {
    ++stepsOnLayer_;
    if (stepsOnLayer_ < kStepsPerLayerPass)
        return;
    stepsOnLayer_ = 0;
    playingLayerIdx_ = findNextActiveLayer(playingLayerIdx_);
}

void SequencerEngine::tick() {
    ++masterTick_;
    reconcilePlayingLayer();
    const int layerIdx = playingLayerIdx_;
    auto& layer = patch_.layers[static_cast<size_t>(layerIdx)];
    const int stepBefore = layer.stepIndex;
    const auto [x, y] = indexToXY(stepBefore);
    auto& c = cell(layerIdx, x, y);

    lastStepIndex_ = stepBefore;
    lastPlayingLayer_ = layerIdx;
    lastFired_ = c.gate && rollTrigger(c);

    advancePath(layer);
    maybeSwitchLayer();
}

} // namespace matilda
