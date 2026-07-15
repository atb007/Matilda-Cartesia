#include "PresetLibrary.h"
#include "PatchStore.h"

namespace matilda {

juce::File PresetLibrary::presetsDirectory() {
    auto dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                   .getChildFile("IdeasLab")
                   .getChildFile("Matilda")
                   .getChildFile("presets");
    return dir;
}

void PresetLibrary::ensureSeeded() {
    auto dir = presetsDirectory();
    if (!dir.exists())
        dir.createDirectory();

    const auto initFile = fileForName(kInitName);
    if (initFile.existsAsFile())
        return;

    PatchState seed;
    if (!PatchStore::loadDefaultPreset(seed))
        return;
    (void) saveToFile(initFile, seed);
}

juce::String PresetLibrary::sanitizeName(const juce::String& name) {
    auto n = name.trim();
    if (n.endsWithChar('*'))
        n = n.dropLastCharacters(1).trim();
    n = n.replaceCharacters("\\/:*?\"<>|", "_________");
    if (n.isEmpty())
        n = kInitName;
    return n;
}

juce::String PresetLibrary::displayNameFromFile(const juce::File& file) {
    return file.getFileNameWithoutExtension();
}

juce::File PresetLibrary::fileForName(const juce::String& name) {
    return presetsDirectory().getChildFile(sanitizeName(name) + ".json");
}

juce::StringArray PresetLibrary::listPresetNames() {
    ensureSeeded();
    juce::StringArray names;
    for (const auto& entry :
         presetsDirectory().findChildFiles(juce::File::findFiles, false, "*.json")) {
        names.add(displayNameFromFile(entry));
    }
    names.sort(true);
    // Keep Init first when present.
    const int initIdx = names.indexOf(kInitName);
    if (initIdx > 0) {
        names.remove(initIdx);
        names.insert(0, kInitName);
    }
    return names;
}

bool PresetLibrary::loadNamed(const juce::String& name, PatchState& out) {
    return PatchStore::loadFromFile(fileForName(name), out);
}

bool PresetLibrary::saveToFile(const juce::File& file, const PatchState& patch) {
    if (file.getFullPathName().isEmpty())
        return false;
    file.getParentDirectory().createDirectory();
    return file.replaceWithText(PatchStore::patchToJson(patch));
}

bool PresetLibrary::saveNamed(const juce::String& name, const PatchState& patch) {
    return saveToFile(fileForName(name), patch);
}

} // namespace matilda
