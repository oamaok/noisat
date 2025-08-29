#pragma once

#include <JuceHeader.h>

class Panel : public juce::Component {
public:
    Panel(juce::BorderSize<float> margin = juce::BorderSize<float>(5.0f));
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getLocalBounds();
    void setTitle(juce::String title);

private:
    juce::String title;
    juce::BorderSize<float> margin;
};
