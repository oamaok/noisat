#include "FontManager.h"

FontManager* FontManager::instance = nullptr;

FontManager::FontManager() {
    jassert(!instance);

    typefaceCache.insert({
        "GemunuLibre-Light",
        juce::FontOptions(juce::Typeface::createSystemTypefaceFor(
            BinaryData::GemunuLibreLight_ttf,
            BinaryData::GemunuLibreLight_ttfSize
        ))
    });

    typefaceCache.insert({
        "GemunuLibre-Bold",
        juce::FontOptions(juce::Typeface::createSystemTypefaceFor(
            BinaryData::GemunuLibreBold_ttf,
            BinaryData::GemunuLibreBold_ttfSize
        ))
    });
}

FontManager::~FontManager() {
    typefaceCache.clear();
}

FontManager* FontManager::getInstance() {
    if (!instance) instance = new FontManager();
    return instance;
}

juce::Font FontManager::getFont(const char* fontName, float fontSize) {
    auto& fontOptions = getInstance()->typefaceCache[fontName];

    return juce::Font(fontOptions.withHeight(fontSize).withKerningFactor(0.04f));
}