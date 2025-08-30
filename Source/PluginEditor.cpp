#include "PluginEditor.h"

#include "FontManager.h"
#include "NoisatLookAndFeel.h"
#include "PluginProcessor.h"

GeneralControlsPanel::GeneralControlsPanel(NoisatAudioProcessor& audioProcessor)
    : preGainAttch(*audioProcessor.preGain, preGain),
      postGainAttch(*audioProcessor.postGain, postGain),
      dryWetAttch(*audioProcessor.dryWet, dryWet) {
    auto knobs = std::vector<std::pair<const char*, juce::Slider*>>();

    knobs.push_back(std::make_pair("Pre", &preGain));
    knobs.push_back(std::make_pair("Post", &postGain));
    knobs.push_back(std::make_pair("Mix", &dryWet));

    for (auto i = 0; i < knobs.size(); i++) {
        auto knob = knobs[i];
        auto slider = knob.second;

        slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider->setPopupDisplayEnabled(true, true, this);
        slider->setTitle(knob.first);

        addAndMakeVisible(slider);
    }
}
void GeneralControlsPanel::resized() {
    auto area = getLocalBounds();

    {
        preGain.setBounds(area.removeFromTop(40));
        dryWet.setBounds(area.removeFromBottom(40));
        postGain.setBounds(area.removeFromTop(40));
    }
}

ClipControlPanel::ClipControlPanel(NoisatAudioProcessor& audioProcessor)
    : clipThresAttch(*audioProcessor.clipper.threshold, clipThres),
      clipKneeAttch(*audioProcessor.clipper.knee, clipKnee),
      clipRatioAttch(*audioProcessor.clipper.ratio, clipRatio),
      noiseThresholdAttch(*audioProcessor.noiseThres, noiseThreshold),
      clippingCurve(audioProcessor) {
    setTitle("Clipping");

    auto knobs = std::vector<std::pair<const char*, juce::Slider*>>();

    knobs.push_back(std::make_pair("Thres", &clipThres));
    knobs.push_back(std::make_pair("Ratio", &clipRatio));
    knobs.push_back(std::make_pair("Knee", &clipKnee));
    knobs.push_back(std::make_pair("Noise lvl", &noiseThreshold));

    for (auto i = 0; i < knobs.size(); i++) {
        auto knob = knobs[i];
        auto slider = knob.second;

        slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider->setPopupDisplayEnabled(true, true, this);
        slider->setTitle(knob.first);

        addAndMakeVisible(slider);
    }

    addAndMakeVisible(clippingCurve);
}

void ClipControlPanel::resized() {
    auto area = getLocalBounds();

    {
        auto knobColumn = area.removeFromLeft(50);
        clipThres.setBounds(knobColumn.removeFromTop(50));
        clipRatio.setBounds(knobColumn.removeFromBottom(50));
    }

    {
        auto knobColumn = area.removeFromLeft(50);
        clipKnee.setBounds(knobColumn.removeFromTop(50));
        noiseThreshold.setBounds(knobColumn.removeFromBottom(50));
    }

    clippingCurve.setBounds(area);
}

NoiseControlPanel::NoiseControlPanel(NoisatAudioProcessor& audioProcessor)
    : noiseColorEditor(audioProcessor),
      noiseHpFreqAttch(*audioProcessor.noiseEq.hpFreq, noiseHpFreq),
      noiseHpQAttch(*audioProcessor.noiseEq.hpQ, noiseHpQ),
      noiseLpFreqAttch(*audioProcessor.noiseEq.lpFreq, noiseLpFreq),
      noiseLpQAttch(*audioProcessor.noiseEq.lpQ, noiseLpQ) {
    setTitle("Noise color");

    auto knobs = std::vector<std::pair<const char*, juce::Slider*>>();

    knobs.push_back(std::make_pair("Freq", &noiseLpFreq));
    knobs.push_back(std::make_pair("Q", &noiseLpQ));
    knobs.push_back(std::make_pair("Freq", &noiseHpFreq));
    knobs.push_back(std::make_pair("Q", &noiseHpQ));

    for (auto i = 0; i < knobs.size(); i++) {
        auto knob = knobs[i];
        auto slider = knob.second;

        slider->setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider->setPopupDisplayEnabled(true, true, this);
        slider->setTitle(knob.first);

        addAndMakeVisible(slider);
    }

    addAndMakeVisible(noiseColorEditor);
}

void NoiseControlPanel::resized() {
    auto area = getLocalBounds();

    {
        auto hpControlArea = area.removeFromLeft(50);
        noiseHpFreq.setBounds(hpControlArea.removeFromTop(50));
        noiseHpQ.setBounds(hpControlArea.removeFromBottom(50));
    }

    {
        auto lpControlArea = area.removeFromRight(50);
        noiseLpFreq.setBounds(lpControlArea.removeFromTop(50));
        noiseLpQ.setBounds(lpControlArea.removeFromBottom(50));
    }

    noiseColorEditor.setBounds(area);
}

NoisatAudioProcessorEditor::NoisatAudioProcessorEditor(NoisatAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), generalControlsPanel(p),
      noiseControlPanel(p), clipControlPanel(p) {
    setLookAndFeel(new NoisatLookAndFeel());

    addAndMakeVisible(generalControlsPanel);
    addAndMakeVisible(clipControlPanel);
    addAndMakeVisible(noiseControlPanel);

    setSize(600, 150);
}

NoisatAudioProcessorEditor::~NoisatAudioProcessorEditor() {}

void NoisatAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour::fromRGB(0x11, 0x11, 0x11));
}

void NoisatAudioProcessorEditor::resized() {
    auto area = getLocalBounds();

    generalControlsPanel.setBounds(area.removeFromLeft(80));
    noiseControlPanel.setBounds(area.removeFromLeft(250));
    clipControlPanel.setBounds(area.removeFromLeft(240));
}
