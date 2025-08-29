#pragma once

#include <JuceHeader.h>

class NoisatLookAndFeel : public juce::LookAndFeel_V4
{
public:
	NoisatLookAndFeel();
	void drawRotarySlider(
		juce::Graphics& g,
		int x,
		int y,
		int width,
		int height,
		float sliderPos,
		const float rotaryStartAngle,
		const float rotaryEndAngle,
		juce::Slider&
	) override;
private:
	juce::Image knob;
};
