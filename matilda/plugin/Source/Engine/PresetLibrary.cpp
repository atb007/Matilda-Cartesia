#include "PresetLibrary.h"
#include "PatchStore.h"

namespace matilda {

namespace {

#if JUCE_MAC
/** Older builds used JUCE userApplicationDataDirectory which is ~/Library on this JUCE. */
juce::File legacyPresetsDirectory() {
    return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
        .getChildFile("Library")
        .getChildFile("IdeasLab")
        .getChildFile("Matilda")
        .getChildFile("presets");
}
#endif

void migrateLegacyPresetsIfNeeded(const juce::File& destDir) {
#if JUCE_MAC
    const auto legacyDir = legacyPresetsDirectory();
    if (!legacyDir.isDirectory())
        return;
    if (legacyDir.getFullPathName() == destDir.getFullPathName())
        return;

    const auto marker = destDir.getParentDirectory().getChildFile(".migrated_from_library_ideaslab");
    if (marker.existsAsFile())
        return;

    for (const auto& src :
         legacyDir.findChildFiles(juce::File::findFiles, false, "*.json")) {
        const auto dest = destDir.getChildFile(src.getFileName());
        // Prefer the user's existing library file when both exist (except fill gaps).
        if (!dest.existsAsFile())
            (void) src.copyFileTo(dest);
    }
    (void) marker.replaceWithText("1\n");
#else
    juce::ignoreUnused(destDir);
#endif
}

} // namespace

juce::File PresetLibrary::presetsDirectory() {
#if JUCE_MAC
    // Explicit Application Support — JUCE's userApplicationDataDirectory is ~/Library here,
    // which previously hid presets from the documented path.
    return juce::File::getSpecialLocation(juce::File::userHomeDirectory)
        .getChildFile("Library")
        .getChildFile("Application Support")
        .getChildFile("IdeasLab")
        .getChildFile("Matilda")
        .getChildFile("presets");
#else
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("IdeasLab")
        .getChildFile("Matilda")
        .getChildFile("presets");
#endif
}

void PresetLibrary::ensureSeeded() {
    auto dir = presetsDirectory();
    if (!dir.exists())
        dir.createDirectory();

    migrateLegacyPresetsIfNeeded(dir);

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
