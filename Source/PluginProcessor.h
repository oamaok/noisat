#pragma once

#include <JuceHeader.h>

class DoubleIIR : public juce::AudioProcessorParameter::Listener,
                  public juce::AsyncUpdater {
public:
    using Param = juce::AudioParameterFloat;
    DoubleIIR();
    ~DoubleIIR();

    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(
        int parameterIndex, bool gestureIsStarting
    ) override {};
    void handleAsyncUpdate() override;

    void prepare(juce::dsp::ProcessSpec spec);
    float processSample(float sample);

    void getMagnitude(
        const double* frequencies, double* magnitudes, size_t numSamples
    );

    juce::AudioParameterFloat* hpFreq;
    juce::AudioParameterFloat* hpQ;

    juce::AudioParameterFloat* lpFreq;
    juce::AudioParameterFloat* lpQ;

    juce::dsp::IIR::Filter<float> hpFilter;
    juce::dsp::IIR::Filter<float> lpFilter;

private:
    juce::dsp::ProcessSpec spec;

    juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>
        hpCoeffs;
    juce::ReferenceCountedObjectPtr<juce::dsp::IIR::Coefficients<float>>
        lpCoeffs;
};

class Clipper {
public:
    Clipper();
    float evaluate(float sample);

    juce::AudioParameterFloat* threshold;
    juce::AudioParameterFloat* knee;
    juce::AudioParameterFloat* ratio;
};

class NoisatAudioProcessor : public juce::AudioProcessor {
public:
    NoisatAudioProcessor();
    ~NoisatAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioParameterFloat* noiseThres;

    juce::AudioParameterFloat* preGain;
    juce::AudioParameterFloat* postGain;
    juce::AudioParameterFloat* dryWet;

    DoubleIIR noiseEq;
    Clipper clipper;

private:
    juce::Random noiseGen;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    const size_t oversamplingFactor = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoisatAudioProcessor)
};
