#include "NoisatLookAndFeel.h"

#include "FontManager.h"

NoisatLookAndFeel::NoisatLookAndFeel() {
    knob = juce::PNGImageFormat::loadFrom(
        BinaryData::Knob_png, BinaryData::Knob_pngSize
    );
}

void NoisatLookAndFeel::drawRotarySlider(
    juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle,
    juce::Slider& slider
) {
    auto bounds = juce::Rectangle(x, y, width, height);
    float titleHeight = 12.0f;

    {
        float totalRotation = rotaryEndAngle - rotaryStartAngle;
        float size = (float)std::min(width, height) - titleHeight;
        g.drawImageTransformed(
            knob,
            juce::AffineTransform::scale(size / 110.0f)
                .translated(-size * 0.5f, -size * 0.5f)
                .rotated(rotaryStartAngle + totalRotation * sliderPos)
                .translated(size * 0.5f + titleHeight * 0.5f, size * 0.5f)
        );
    }

    {
        auto titleBounds = bounds.toFloat().removeFromBottom(titleHeight);
        g.setFont(FontManager::getFont("GemunuLibre-Light", 12.0f));
        g.setColour(juce::Colour::fromRGB(0x00, 0x00, 0x00));
        g.drawText(
            slider.getTitle().toUpperCase(),
            titleBounds.translated(0.0f, 1.5f),
            juce::Justification::centred
        );
        g.setColour(juce::Colour::fromRGB(0xff, 0xff, 0xff));
        g.drawText(
            slider.getTitle().toUpperCase(),
            titleBounds,
            juce::Justification::centred
        );
    }
}