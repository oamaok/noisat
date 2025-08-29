#include "PluginProcessor.h"
#include "PluginEditor.h"

DoubleIIR::DoubleIIR() {
    juce::NormalisableRange<float> expRange{};

    auto convertFrom0To1Function = [expRange](double currentRangeStart,
        double currentRangeEnd,
        double normalisedValue) mutable
        {
            expRange.start = (float)currentRangeStart;
            expRange.end = (float)currentRangeEnd;
            return currentRangeStart * std::exp(normalisedValue * std::log(currentRangeEnd / currentRangeStart));
        };

    auto convertTo0To1Function = [expRange](double currentRangeStart,
        double currentRangeEnd,
        double mappedValue) mutable
        {
            expRange.start = (float)currentRangeStart;
            expRange.end = (float)currentRangeEnd;
            return std::log(mappedValue / currentRangeStart) / std::log(currentRangeEnd / currentRangeStart);
        };

    auto snapToLegalValueFunction = [expRange](double currentRangeStart,
        double currentRangeEnd,
        double mappedValue) mutable
        {
            expRange.start = (float)currentRangeStart;
            expRange.end = (float)currentRangeEnd;
            return (double)expRange.snapToLegalValue((float)mappedValue);
        };

    juce::NormalisableRange<float> noiseQRange = {
        0.025f,
        40.0f,
        std::move(convertFrom0To1Function),
        std::move(convertTo0To1Function),
        std::move(snapToLegalValueFunction)
    };

    juce::NormalisableRange<float> noiseFreqRange = {
        4.0f,
        22000.0f,
        std::move(convertFrom0To1Function),
        std::move(convertTo0To1Function),
        std::move(snapToLegalValueFunction)
    };

    hpFreq = new juce::AudioParameterFloat("noiseHpFreq", "Noise Highpass Frequency", noiseFreqRange, 4.0f);
    hpQ = new juce::AudioParameterFloat("noiseHpQ", "Noise Highpass Q", noiseQRange, 1.0f);

    lpFreq = new juce::AudioParameterFloat("noiseLpFreq", "Noise Lowpass Frequency", noiseFreqRange, 22000.0f);
    lpQ = new juce::AudioParameterFloat("noiseLpQ", "Noise Lowpass Q", noiseQRange, 1.0f);

    hpFreq->addListener(this);
    hpQ->addListener(this);
    lpFreq->addListener(this);
    lpQ->addListener(this);
}

DoubleIIR::~DoubleIIR() {
    hpFreq->removeListener(this);
    hpQ->removeListener(this);
    lpFreq->removeListener(this);
    lpQ->removeListener(this);
}

void DoubleIIR::parameterValueChanged(int parameterIndex, float newValue) {
    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
    {
        cancelPendingUpdate();
        handleAsyncUpdate();
    }
    else
    {
        triggerAsyncUpdate();
    }

}

void DoubleIIR::handleAsyncUpdate() {
    lpCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(spec.sampleRate, lpFreq->get(), lpQ->get());
    hpCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, hpFreq->get(), hpQ->get());

    lpFilter.coefficients = lpCoeffs;
    hpFilter.coefficients = hpCoeffs;
}

float DoubleIIR::processSample(float sample) {
    sample = lpFilter.processSample(sample);
    sample = hpFilter.processSample(sample);
    return sample;
}

void DoubleIIR::prepare(juce::dsp::ProcessSpec sp) {
    spec = sp;
    hpFilter.prepare(sp);
    lpFilter.prepare(sp);

    handleAsyncUpdate();
}

void DoubleIIR::getMagnitude(const double* frequencies, double* magnitudes, size_t numSamples) {
    double* hpResponse = new double[numSamples];
    double* lpResponse = new double[numSamples];

    hpCoeffs.get()->getMagnitudeForFrequencyArray(frequencies, hpResponse, numSamples, spec.sampleRate);
    lpCoeffs.get()->getMagnitudeForFrequencyArray(frequencies, lpResponse, numSamples, spec.sampleRate);

    // TODO: Could be SIMDed...
    for (size_t i = 0; i < numSamples; i++) {
        magnitudes[i] = hpResponse[i] * lpResponse[i];
    }

    delete[] hpResponse;
    delete[] lpResponse;
}

Clipper::Clipper() {
    threshold = new juce::AudioParameterFloat("clipThres", "Clipping Threshold", 0.1f, 1.0f, 1.0f);
    knee = new juce::AudioParameterFloat("clipKnee", "Clipping Knee", 0.0f, 1.0f, 1.0f);
    ratio = new juce::AudioParameterFloat("clipRatio", "Clipping Ratio", 1.0f, 40.0f, 1.0f);
}

float Clipper::evaluate(float sample) {
    auto kneeValue = knee->get();
    auto ratioValue = ratio->get();
    auto thresValue = threshold->get();

    if (sample <= thresValue) return sample;
    sample = (sample - thresValue) / (1 - thresValue);
    return (sample * std::exp(-kneeValue * sample) / ratioValue) * (1 - thresValue) + thresValue;
}

//==============================================================================
NoisatAudioProcessor::NoisatAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    noiseThres = new juce::AudioParameterFloat("noiseThres", "Noise Threshold", 0.01f, 1.0f, 0.5f);

    preGain = new juce::AudioParameterFloat("preGain", "Pre-Gain", 0.0f, 5.0f, 1.0f);
    postGain = new juce::AudioParameterFloat("postGain", "Post-Gain", 0.0f, 5.0f, 1.0f);
    dryWet = new juce::AudioParameterFloat("dryWet", "Mix", 0.0f, 1.0f, 1.0f);

    addParameter(noiseEq.hpQ);
    addParameter(noiseEq.hpFreq);
    addParameter(noiseEq.lpQ);
    addParameter(noiseEq.lpFreq);

    addParameter(clipper.threshold);
    addParameter(clipper.knee);
    addParameter(clipper.ratio);

    addParameter(noiseThres);
    addParameter(preGain);
    addParameter(postGain);
    addParameter(dryWet);
}

NoisatAudioProcessor::~NoisatAudioProcessor()
{
}

//==============================================================================
const juce::String NoisatAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool NoisatAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool NoisatAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool NoisatAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double NoisatAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int NoisatAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int NoisatAudioProcessor::getCurrentProgram()
{
    return 0;
}

void NoisatAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String NoisatAudioProcessor::getProgramName (int index)
{
    return {};
}

void NoisatAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void NoisatAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getNumInputChannels();

    noiseEq.prepare(spec);
}

void NoisatAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool NoisatAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


void NoisatAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto noiseThreshold = noiseThres->get();
    auto preGainFactor = preGain->get();
    auto postGainFactor = postGain->get();
    auto dryWetFactor = dryWet->get();

    // TODO: Make member of the class to avoid unnecessary allocations?
    // TODO: Stereo noise?
    std::vector<float> noiseBuf = std::vector<float>(buffer.getNumSamples());

    for (int i = 0; i < buffer.getNumSamples(); i++) {
        float noise = noiseGen.nextFloat();
        noise = noiseEq.processSample(noise);
        noiseBuf[i] = noise;
    }

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        if (totalNumOutputChannels < channel) break;

        float* output = buffer.getWritePointer(channel);
        const float* input = buffer.getReadPointer(channel);

        for (auto i = 0; i < buffer.getNumSamples(); i++) {
            float sample = input[i] * preGainFactor;
            float clipped = clipper.evaluate(sample);

            float amountClipped = std::abs(clipped - sample);
            if (amountClipped > noiseThreshold) {
                clipped += std::copysignf(amountClipped - noiseThreshold, clipped) * noiseBuf[i];
            }

            output[i] = sample * dryWetFactor + (1.0f - dryWetFactor) * clipped * postGainFactor;
        }
    }
}

//==============================================================================
bool NoisatAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* NoisatAudioProcessor::createEditor()
{
    return new NoisatAudioProcessorEditor (*this);
}

//==============================================================================
void NoisatAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void NoisatAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NoisatAudioProcessor();
}
