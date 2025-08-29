#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "NoiseColorEditor.h"
#include "Panel.h"
#include "ClippingCurve.h"

struct GeneralControlsPanel : public Panel {
public:
    GeneralControlsPanel(NoisatAudioProcessor&);
    void resized() override;
private:
    juce::Slider preGain;
    juce::SliderParameterAttachment preGainAttch;

    juce::Slider postGain;
    juce::SliderParameterAttachment postGainAttch;

    juce::Slider dryWet;
    juce::SliderParameterAttachment dryWetAttch;
};

struct ClipControlPanel : public Panel {
public:
    ClipControlPanel(NoisatAudioProcessor&);
    void resized() override;
private:
    ClippingCurve clippingCurve;

    juce::Slider clipThres;
    juce::SliderParameterAttachment clipThresAttch;

    juce::Slider clipKnee;
    juce::SliderParameterAttachment clipKneeAttch;

    juce::Slider clipRatio;
    juce::SliderParameterAttachment clipRatioAttch;

    juce::Slider noiseThreshold;
    juce::SliderParameterAttachment noiseThresholdAttch;
};

struct NoiseControlPanel : public Panel {
public:
    NoiseControlPanel(NoisatAudioProcessor&);
    void resized() override;
private:
    NoiseColorEditor noiseColorEditor;

    juce::Slider noiseHpFreq;
    juce::SliderParameterAttachment noiseHpFreqAttch;

    juce::Slider noiseHpQ;
    juce::SliderParameterAttachment noiseHpQAttch;

    juce::Slider noiseLpFreq;
    juce::SliderParameterAttachment noiseLpFreqAttch;

    juce::Slider noiseLpQ;
    juce::SliderParameterAttachment noiseLpQAttch;
};

class NoisatAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    NoisatAudioProcessorEditor (NoisatAudioProcessor&);
    ~NoisatAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    NoisatAudioProcessor& audioProcessor;

    GeneralControlsPanel generalControlsPanel;
    ClipControlPanel clipControlPanel;
    NoiseControlPanel noiseControlPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoisatAudioProcessorEditor)
};
