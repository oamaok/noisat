#include "ClippingCurve.h"

ClippingCurve::ClippingCurve(NoisatAudioProcessor& audioProcessor) : audioProcessor(audioProcessor) {
    audioProcessor.clipper.knee->addListener(this);
    audioProcessor.clipper.ratio->addListener(this);
    audioProcessor.clipper.threshold->addListener(this);
}

ClippingCurve::~ClippingCurve() {

}

void ClippingCurve::parameterValueChanged(int parameterIndex, float newValue) {
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

void ClippingCurve::handleAsyncUpdate() {
    repaint();
}

void ClippingCurve::paint(juce::Graphics& g) {
    g.setColour(juce::Colour::fromRGB(0x66, 0x66, 0x66));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0);

    auto bounds = getLocalBounds().withTrimmedBottom(2);

    juce::Path clippedBounds;
    clippedBounds.addRoundedRectangle(bounds, 5.0f);
    g.reduceClipRegion(clippedBounds);

    g.fillAll(juce::Colour::fromRGB(0x11, 0x11, 0x11));

    float height = (float)bounds.getHeight();
    float width = (float)bounds.getWidth();

    juce::Path clipCurve;
    for (int i = 0; i < bounds.getWidth(); i++) {
        float x = (float)i;
        float y = height - audioProcessor.clipper.evaluate(x / width) * height;

        if (i == 0) {
            clipCurve.startNewSubPath(x, y);
        }
        else {
            clipCurve.lineTo(x, y);
        }
    }

    g.setColour(juce::Colour::fromRGB(0xe8, 0x5d, 0x00));
    g.strokePath(clipCurve, juce::PathStrokeType(2.0f));
}