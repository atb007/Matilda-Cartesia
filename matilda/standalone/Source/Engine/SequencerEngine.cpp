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

int midiBaseOctave(const PatchState& patch) {
    return patch.minOctave + 1;
}

int minMidiInWindow(const PatchState& patch, int root) {
    return (patch.minOctave + 1) * 12 + root;
}

int maxMidiInWindow(const PatchState& patch, int root) {
    return juce::jmin(127, (patch.maxOctave + 1) * 12 + root + 11);
}

bool isPitchClassInScale(int pitchClass, int root, const int* scale, int scaleCount) {
    const int rel = (pitchClass - root + 12) % 12;
    for (int i = 0; i < scaleCount; ++i) {
        if (scale[i] == rel)
            return true;
    }
    return false;
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

int SequencerEngine::computeCandidateMidi(const CellState& cell) const {
    const int root = noteNumberFromRoot(patch_.root);
    const int baseOct = midiBaseOctave(patch_);

    if (!patch_.quantize || patch_.mode.equalsIgnoreCase("chromatic")) {
        const int semitone = ((cell.degree % 12) + 12) % 12;
        return juce::jlimit(0, 127, (baseOct + cell.octaveOffset) * 12 + root + semitone);
    }

    int scaleCount = 7;
    const int* scale = scaleIntervalsForMode(patch_.mode, scaleCount);
    const int scaleIdx = ((cell.degree % scaleCount) + scaleCount) % scaleCount;
    const int pitch = root + scale[scaleIdx];
    const int carry = pitch / 12;
    const int semi = pitch % 12;
    return juce::jlimit(0, 127, (baseOct + cell.octaveOffset + carry) * 12 + semi);
}

int SequencerEngine::resolveMidiNote(const CellState& cell) const {
    const auto midis = quantisedMidisInWindow();
    if (midis.empty())
        return 60;
    return midis[static_cast<size_t>(quantisedNoteIndex(cell))];
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
    const int minMidi = minMidiInWindow(patch_, root);
    const int maxMidi = maxMidiInWindow(patch_, root);
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
    const int next = juce::jlimit(0, static_cast<int>(midis.size()) - 1, idx + delta);
    if (next == idx)
        return;

    assignCellFromMidi(cell, midis[static_cast<size_t>(next)]);
}

int SequencerEngine::quantisedNoteIndex(const CellState& cell) const {
    const auto midis = quantisedMidisInWindow();
    if (midis.empty())
        return 0;

    int candidate = computeCandidateMidi(cell);
    candidate = juce::jlimit(midis.front(), midis.back(), candidate);

    auto it = std::lower_bound(midis.begin(), midis.end(), candidate);
    if (it != midis.end() && *it == candidate)
        return static_cast<int>(std::distance(midis.begin(), it));

    if (it == midis.end())
        return static_cast<int>(midis.size()) - 1;
    if (it == midis.begin())
        return 0;

    const int hi = static_cast<int>(std::distance(midis.begin(), it));
    const int lo = hi - 1;
    return (candidate - midis[static_cast<size_t>(lo)]) <= (midis[static_cast<size_t>(hi)] - candidate) ? lo : hi;
}

void SequencerEngine::requantizeAllCells() {
    const auto midis = quantisedMidisInWindow();
    if (midis.empty())
        return;

    for (auto& layer : patch_.layers) {
        for (auto& row : layer.cells) {
            for (auto& cell : row)
                assignCellFromMidi(cell, midis[static_cast<size_t>(quantisedNoteIndex(cell))]);
        }
    }
}

int SequencerEngine::quantisedNoteCount() const {
    return static_cast<int>(quantisedMidisInWindow().size());
}

float SequencerEngine::knobVisualPosition(const CellState& cell) const {
    const auto midis = quantisedMidisInWindow();
    if (midis.size() <= 1)
        return 0.f;

    const int idx = quantisedNoteIndex(cell);
    return 11.f * static_cast<float>(idx) / static_cast<float>(midis.size() - 1);
}

void SequencerEngine::setQuantisedNoteIndex(CellState& cell, int index) const {
    const auto midis = quantisedMidisInWindow();
    if (midis.empty())
        return;

    index = juce::jlimit(0, static_cast<int>(midis.size()) - 1, index);
    assignCellFromMidi(cell, midis[static_cast<size_t>(index)]);
}

std::vector<int> SequencerEngine::quantisedMidisInWindow() const {
    std::vector<int> out;
    const int root = noteNumberFromRoot(patch_.root);
    const int minMidi = minMidiInWindow(patch_, root);
    const int maxMidi = maxMidiInWindow(patch_, root);

    if (!patch_.quantize || patch_.mode.equalsIgnoreCase("chromatic")) {
        for (int m = minMidi; m <= maxMidi; ++m)
            out.push_back(m);
        return out;
    }

    int scaleCount = 7;
    const int* scale = scaleIntervalsForMode(patch_.mode, scaleCount);
    for (int m = minMidi; m <= maxMidi; ++m) {
        if (isPitchClassInScale(m % 12, root, scale, scaleCount))
            out.push_back(m);
    }

    return out;
}

void SequencerEngine::assignCellFromMidi(CellState& cell, int midi) const {
    midi = juce::jlimit(0, 127, midi);
    const int root = noteNumberFromRoot(patch_.root);
    const int baseOct = midiBaseOctave(patch_);

    if (!patch_.quantize || patch_.mode.equalsIgnoreCase("chromatic")) {
        const int noteOct = midi / 12;
        cell.octaveOffset = noteOct - baseOct;
        cell.degree = ((midi % 12) - root + 12) % 12;
        return;
    }

    int scaleCount = 7;
    const int* scale = scaleIntervalsForMode(patch_.mode, scaleCount);
    const int noteOct = midi / 12;
    const int notePc = midi % 12;

    for (int d = 0; d < scaleCount; ++d) {
        const int pitch = root + scale[d];
        const int carry = pitch / 12;
        const int semi = pitch % 12;
        if (semi != notePc)
            continue;

        const int octOff = noteOct - baseOct - carry;
        if (octOff < 0)
            continue;

        cell.degree = d;
        cell.octaveOffset = octOff;
        return;
    }

    cell.degree = 0;
    cell.octaveOffset = juce::jmax(0, noteOct - baseOct);
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
