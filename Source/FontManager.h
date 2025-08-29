#pragma once

#include <JuceHeader.h>

class FontManager {
public:
    static juce::Font getFont(const char* fontName, float fontSize);
private:
    FontManager();
    ~FontManager();
    static FontManager* instance;
    static FontManager* getInstance();
    std::unordered_map<juce::String, juce::FontOptions> typefaceCache;
};
