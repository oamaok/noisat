#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ClippingCurve : public juce::Component, public juce::AudioProcessorParameter::Listener, public juce::AsyncUpdater {
public:
    ClippingCurve(NoisatAudioProcessor& audioProcessor);
    ~ClippingCurve();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override {};
    void handleAsyncUpdate() override;

    void paint(juce::Graphics& g) override;
private:
    NoisatAudioProcessor& audioProcessor;
};
