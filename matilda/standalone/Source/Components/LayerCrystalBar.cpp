#include "LayerCrystalBar.h"
#include "BinaryData.h"

LayerCrystalBar::LayerCrystalBar(matilda::PatchState& patch, MatildaLookAndFeel& laf)
    : patch_(patch), laf_(laf) {
    setOpaque(false);
    crystalImg_ = juce::ImageCache::getFromMemory(BinaryData::crystalamber_png, BinaryData::crystalamber_pngSize);
    refresh();
}

std::vector<int> LayerCrystalBar::activeLayerIndices() const {
    std::vector<int> out;
    for (int i = 0; i < matilda::kLayerCount; ++i)
        if (patch_.layers[static_cast<size_t>(i)].active)
            out.push_back(i);
    return out;
}

void LayerCrystalBar::refresh() {
    repaint();
}

int LayerCrystalBar::hitTestCrystal(juce::Point<int> p) const {
    for (int i = 0; i < matilda::kLayerCount; ++i)
        if (crystalHits_[static_cast<size_t>(i)].contains(p))
            return i;
    return -1;
}

void LayerCrystalBar::paint(juce::Graphics& g) {
    const auto active = activeLayerIndices();
    if (active.empty())
        return;

    auto area = getLocalBounds().toFloat().reduced(4.f, 8.f);
    const float crystalSize = juce::jmin(area.getHeight() * 1.6f, static_cast<float>(getHeight()) * 0.95f);

    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawLine(area.getX() + area.getWidth() * 0.08f, area.getCentreY(), area.getRight() - area.getWidth() * 0.08f,
               area.getCentreY(), 1.f);

    const float spacing = area.getWidth() / static_cast<float>(active.size());
    float cx = area.getX() + spacing * 0.5f;

    for (int layer : active) {
        const bool selected = layer == patch_.selectedLayer;
        auto crystalBounds = juce::Rectangle<float>(crystalSize, crystalSize).withCentre({ cx, area.getCentreY() });
        crystalHits_[static_cast<size_t>(layer)] = crystalBounds.toNearestInt();

        if (!crystalImg_.isNull()) {
            g.setColour(juce::Colours::white.withAlpha(selected ? 1.f : 0.55f));
            g.drawImage(crystalImg_, crystalBounds, juce::RectanglePlacement::centred);
        }

        if (selected) {
            g.setColour(laf_.accentOrange.withAlpha(0.5f));
            g.drawEllipse(crystalBounds.expanded(3.f), 2.f);
        }

        cx += spacing;
    }
}

void LayerCrystalBar::mouseDown(const juce::MouseEvent& e) {
    const int layer = hitTestCrystal(e.getPosition());
    if (layer < 0)
        return;

    patch_.selectedLayer = layer;
    if (onLayerSelected)
        onLayerSelected(layer);
    repaint();
}
