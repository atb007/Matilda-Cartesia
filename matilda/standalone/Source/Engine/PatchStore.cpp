#include "PatchStore.h"
#include "BinaryData.h"

namespace matilda {

namespace {

juce::var cellToVar(const CellState& c) {
    auto* o = new juce::DynamicObject();
    o->setProperty("degree", c.degree);
    o->setProperty("gate", c.gate);
    o->setProperty("velocity", c.velocity);
    o->setProperty("octave_offset", c.octaveOffset);
    o->setProperty("trigger_armed", c.triggerArmed);
    o->setProperty("trigger_prob", static_cast<double>(c.triggerProb));
    o->setProperty("jitter_armed", c.jitterArmed);
    o->setProperty("jitter_amount", static_cast<double>(c.jitterAmount));
    return juce::var(o);
}

bool cellFromVar(const juce::var& v, CellState& c) {
    if (!v.isObject())
        return false;
    c.degree = static_cast<int>(v.getProperty("degree", 0));
    c.gate = static_cast<bool>(v.getProperty("gate", true));
    c.velocity = static_cast<int>(v.getProperty("velocity", 90));
    c.octaveOffset = static_cast<int>(v.getProperty("octave_offset", 0));
    c.triggerArmed = static_cast<bool>(v.getProperty("trigger_armed", false));
    c.triggerProb = static_cast<float>(static_cast<double>(v.getProperty("trigger_prob", 0.5)));
    c.jitterArmed = static_cast<bool>(v.getProperty("jitter_armed", false));
    c.jitterAmount = static_cast<float>(static_cast<double>(v.getProperty("jitter_amount", 0.5)));
    return true;
}

juce::var layerToVar(const LayerState& layer) {
    auto* o = new juce::DynamicObject();
    o->setProperty("active", layer.active);
    o->setProperty("movement", PatchStore::movementToString(layer.movement));
    o->setProperty("random_skip_prob", static_cast<double>(layer.randomSkipProb));

    juce::Array<juce::var> rows;
    for (int y = 0; y < kGridSize; ++y) {
        juce::Array<juce::var> row;
        for (int x = 0; x < kGridSize; ++x)
            row.add(cellToVar(layer.cells[static_cast<size_t>(y)][static_cast<size_t>(x)]));
        rows.add(row);
    }
    o->setProperty("cells", rows);
    return juce::var(o);
}

bool layerFromVar(const juce::var& v, LayerState& layer) {
    if (!v.isObject())
        return false;
    layer.active = static_cast<bool>(v.getProperty("active", false));
    layer.movement = PatchStore::movementFromString(v.getProperty("movement", "forward").toString());
    layer.randomSkipProb = static_cast<float>(static_cast<double>(v.getProperty("random_skip_prob", 0.0)));

    const auto cells = v.getProperty("cells", {});
    if (cells.isArray()) {
        const auto* rows = cells.getArray();
        for (int y = 0; y < kGridSize && y < rows->size(); ++y) {
            const auto row = rows->getReference(y);
            if (!row.isArray())
                continue;
            const auto* cols = row.getArray();
            for (int x = 0; x < kGridSize && x < cols->size(); ++x)
                cellFromVar(cols->getReference(x), layer.cells[static_cast<size_t>(y)][static_cast<size_t>(x)]);
        }
    }
    return true;
}

} // namespace

juce::String PatchStore::movementToString(MovementMode mode) {
    switch (mode) {
        case MovementMode::Forward:    return "forward";
        case MovementMode::Reverse:    return "reverse";
        case MovementMode::PingPong:   return "ping_pong";
        case MovementMode::Pendulum:   return "pendulum";
        case MovementMode::Random:     return "random";
        case MovementMode::RandomSkip: return "random_skip";
    }
    return "forward";
}

MovementMode PatchStore::movementFromString(const juce::String& s) {
    const auto m = s.toLowerCase();
    if (m == "reverse")     return MovementMode::Reverse;
    if (m == "ping_pong" || m == "ping-pong") return MovementMode::PingPong;
    if (m == "pendulum")    return MovementMode::Pendulum;
    if (m == "random")      return MovementMode::Random;
    if (m == "random_skip" || m == "random skip") return MovementMode::RandomSkip;
    return MovementMode::Forward;
}

juce::String PatchStore::patchToJson(const PatchState& patch) {
    auto* root = new juce::DynamicObject();
    root->setProperty("title", "Matilda patch");
    root->setProperty("version", 2);
    root->setProperty("root", patch.root);
    root->setProperty("mode", patch.mode);
    root->setProperty("quantize", patch.quantize);
    root->setProperty("min_octave", patch.minOctave);
    root->setProperty("max_octave", patch.maxOctave);
    root->setProperty("master_division", patch.masterDivision);
    root->setProperty("play_mode", patch.playMode == PlayMode::Note ? "note" : "transport");
    root->setProperty("play_on_transport", false);
    root->setProperty("selected_layer", patch.selectedLayer);
    root->setProperty("poly_voices", 1);

    juce::Array<juce::var> layers;
    for (const auto& layer : patch.layers)
        layers.add(layerToVar(layer));
    root->setProperty("layers", layers);

    return juce::JSON::toString(juce::var(root), true);
}

bool PatchStore::patchFromJson(const juce::String& json, PatchState& out) {
    const auto parsed = juce::JSON::parse(json);
    if (!parsed.isObject())
        return false;

    out.root = parsed.getProperty("root", "C").toString();
    out.mode = parsed.getProperty("mode", "major").toString().toLowerCase();
    out.quantize = static_cast<bool>(parsed.getProperty("quantize", true));
    out.minOctave = static_cast<int>(parsed.getProperty("min_octave", 1));
    out.maxOctave = static_cast<int>(parsed.getProperty("max_octave", 9));
    out.masterDivision = static_cast<double>(parsed.getProperty("master_division", 1.0 / 16.0));
    out.selectedLayer = juce::jlimit(0, kLayerCount - 1,
                                     static_cast<int>(parsed.getProperty("selected_layer", 0)));

    const auto playModeStr = parsed.getProperty("play_mode", "note").toString().toLowerCase();
    out.playMode = playModeStr == "transport" ? PlayMode::Transport : PlayMode::Note;

    const auto layers = parsed.getProperty("layers", {});
    if (layers.isArray()) {
        const auto* arr = layers.getArray();
        for (int i = 0; i < kLayerCount && i < arr->size(); ++i)
            layerFromVar(arr->getReference(i), out.layers[static_cast<size_t>(i)]);
    }

    if (!out.layers[0].active)
        out.layers[0].active = true;

    return true;
}

bool PatchStore::loadFromFile(const juce::File& file, PatchState& out) {
    if (!file.existsAsFile())
        return false;
    return patchFromJson(file.loadFileAsString(), out);
}

bool PatchStore::loadDefaultPreset(PatchState& out) {
    if (loadFromFile(juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                         .getParentDirectory()
                         .getChildFile("default.layer1.json"),
                     out))
        return true;

    const juce::String embedded(
        juce::String::fromUTF8(reinterpret_cast<const char*>(BinaryData::default_layer1_json),
                              BinaryData::default_layer1_jsonSize));
    return patchFromJson(embedded, out);
}

} // namespace matilda
