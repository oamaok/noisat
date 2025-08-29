#include "Panel.h"
#include "FontManager.h"

Panel::Panel(juce::BorderSize<float> m) : margin(m) {}

void Panel::setTitle(juce::String t) {
    title = t;
    repaint();
}

juce::Rectangle<int> Panel::getLocalBounds() {
    juce::BorderSize<float> padding{ 8.0f };

    auto bounds = Component::getLocalBounds();
    return padding.subtractedFrom(margin.subtractedFrom(bounds.toFloat()))
        .withTrimmedTop(title.length() ? 20 : 0)
        .toNearestInt();
}

void Panel::paint(juce::Graphics& g) {
    float cornerRounding = 5.0f;

    auto bounds = Component::getLocalBounds();
    auto panelSize = margin.subtractedFrom(bounds.toFloat());

    // First fill the "dropshadow"
    g.setColour(juce::Colour::fromRGB(0x66, 0x66, 0x66));
    g.fillRoundedRectangle(panelSize.translated(0, 2).toFloat(), cornerRounding);

    // Main background
    g.setColour(juce::Colour::fromRGB(0x44, 0x44, 0x44));
    g.fillRoundedRectangle(panelSize.toFloat(), cornerRounding);

    if (title.length()) {
        juce::Path titleClip;
        titleClip.addRoundedRectangle(panelSize, cornerRounding);
        auto titleArea = panelSize.removeFromTop(20);

        g.reduceClipRegion(titleClip);
        g.setColour(juce::Colour::fromRGB(0xe8, 0x5d, 0x00));
        g.fillRect(titleArea);

        juce::BorderSize<float> padding{ 4.0f, 8.0f, 4.0f, 4.0f };
        g.setFont(FontManager::getFont("GemunuLibre-Bold", 12.0f));
        g.setColour(juce::Colour::fromRGB(0x00, 0x00, 0x00));
        g.drawText(title.toUpperCase(), padding.subtractedFrom(titleArea).translated(0.0f, 1.5f), juce::Justification::topLeft);
        g.setColour(juce::Colour::fromRGB(0xff, 0xff, 0xff));
        g.drawText(title.toUpperCase(), padding.subtractedFrom(titleArea), juce::Justification::topLeft);

    }
}
