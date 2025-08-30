#include "NoiseColorEditor.h"

ControlPoint::ControlPoint() : isDragged(false) {}

void ControlPoint::addListener(ControlPointListener<ControlPoint>* listener) {
    listeners.add(listener);
}

void ControlPoint::removeListener(ControlPointListener<ControlPoint>* listener) {
    listeners.remove(listener);
}

void ControlPoint::setXValue(float value) {
    jassert(value >= 0.0f);
    jassert(value <= 1.0f);
    position.x = value;
    repaint();
}

void ControlPoint::setYValue(float value) {
    jassert(value >= 0.0f);
    jassert(value <= 1.0f);
    position.y = value;
    repaint();
}

bool ControlPoint::hitTest(int x, int y) {
    auto bounds = getLocalBounds().toFloat();
    return juce::Point<int>(x, y).toFloat().getDistanceFrom(bounds.getRelativePoint<float>(position.x, position.y)) < 5.0f;
}

void ControlPoint::mouseDown(const juce::MouseEvent& event) {
    startPosition = position;
    isDragged = true;
    listeners.call([this](ControlPointListener<ControlPoint>& l) { l.controlPointDragStarted(this); });
}

void ControlPoint::mouseUp(const juce::MouseEvent& event) {
    if (isDragged) {
        listeners.call([this](ControlPointListener<ControlPoint>& l) { l.controlPointDragEnded(this); });
    }

    isDragged = false;
}

void ControlPoint::mouseDrag(const juce::MouseEvent& event) {
    if (!isDragged) return;
    auto bounds = getLocalBounds().toFloat();

    auto nextPos = startPosition + event.getOffsetFromDragStart().toFloat().transformedBy(juce::AffineTransform::scale(1.0f / bounds.getWidth(), 1.0f / bounds.getHeight()));

    setXValue(nextPos.x);
    setYValue(nextPos.y);

    listeners.call([this](ControlPointListener<ControlPoint>& l) { l.controlPointValueChanged(this); });
}

void ControlPoint::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    juce::Path cpPath;
    cpPath.addEllipse(position.x * bounds.getWidth() - 5.0f, position.y * bounds.getHeight() - 5.0f, 10.0f, 10.0f);

    g.setColour(juce::Colour::fromRGB(0x44, 0x44, 0x44));
    g.fillPath(cpPath);

    g.setColour(juce::Colour::fromRGB(0x88, 0x88, 0x88));
    g.strokePath(cpPath, juce::PathStrokeType(2.0f));
}

ControlPointAttachment::ControlPointAttachment(ControlPoint& cp, juce::AudioParameterFloat& xp, juce::AudioParameterFloat& yp)
    : controlPoint(cp),
    xParam(xp),
    yParam(yp),
    xParamAttachment(xParam, [this](float f) { setXValue(xParam.convertTo0to1(f)); }, nullptr),
    yParamAttachment(yParam, [this](float f) { setYValue(1.0f - yParam.convertTo0to1(f)); }, nullptr),
    ignoreCallbacks(false)
{
    xParamAttachment.sendInitialUpdate();
    yParamAttachment.sendInitialUpdate();
    cp.addListener(this);
}

ControlPointAttachment::~ControlPointAttachment() {
    controlPoint.removeListener(this);
}

void ControlPointAttachment::setXValue(float value) {
    const juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
    controlPoint.setXValue(value);
}

void ControlPointAttachment::setYValue(float value) {
    const juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
    controlPoint.setYValue(value);
}

void ControlPointAttachment::controlPointValueChanged(ControlPoint* cp) {
    if (!ignoreCallbacks) {
        xParamAttachment.setValueAsPartOfGesture(xParam.convertFrom0to1(cp->position.x));
        yParamAttachment.setValueAsPartOfGesture(yParam.convertFrom0to1(1.0f - cp->position.y));
    }
}

NoiseColorEditor::NoiseColorEditor(NoisatAudioProcessor& p) : audioProcessor(p), currentControlPoint(nullptr) {
    auto lpControlPoint = new ControlPoint();
    auto hpControlPoint = new ControlPoint();

    controlPoints.push_back(lpControlPoint);
    controlPoints.push_back(hpControlPoint);
    controlPointAttachments.push_back(new ControlPointAttachment(*lpControlPoint, *p.noiseEq.lpFreq, *p.noiseEq.lpQ));
    controlPointAttachments.push_back(new ControlPointAttachment(*hpControlPoint, *p.noiseEq.hpFreq, *p.noiseEq.hpQ));

    for (auto& controlPoint : controlPoints) {
        addAndMakeVisible(controlPoint);
    }
}

NoiseColorEditor::~NoiseColorEditor() {}

void NoiseColorEditor::resized() {
    for (auto& controlPoint : controlPoints) {
        controlPoint->setBounds(getLocalBounds().withTrimmedBottom(2));
    }
}

void NoiseColorEditor::paint(juce::Graphics& g) {
    g.setColour(juce::Colour::fromRGB(0x66, 0x66, 0x66));
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.0);

    auto bounds = getLocalBounds().withTrimmedBottom(2);

    juce::Path clippedBounds;
    clippedBounds.addRoundedRectangle(bounds, 5.0f);
    g.reduceClipRegion(clippedBounds);

    g.fillAll(juce::Colour::fromRGB(0x11, 0x11, 0x11));

    {
        int eqWidth = bounds.getWidth();

        std::vector<double> freqs(eqWidth);
        std::vector<double> magnitudes(eqWidth);

        for (int i = 0; i < eqWidth; i++) {
            freqs[i] = audioProcessor.noiseEq.hpFreq->convertFrom0to1((float)i / (float) eqWidth);
        }
        audioProcessor.noiseEq.getMagnitude(freqs.data(), magnitudes.data(), eqWidth);
        // TODO: Draw freqency gridlines


        juce::Path eqCurve;
        for (int i = 0; i < eqWidth; i++) {
            float x = (float)i;
            float db = std::log((float)magnitudes[i]) / std::log(10.0f) * 0.5f;
            float y = (float)bounds.getHeight() - (db + 1.0f) * 0.5f * (float)bounds.getHeight();

            if (i == 0) {
                eqCurve.startNewSubPath(x, y);
            } else {
                eqCurve.lineTo(x, y);
            }
        }

        juce::Path eqCurveBackground(eqCurve);
        eqCurveBackground.lineTo(bounds.getWidth(), bounds.getHeight());
        eqCurveBackground.lineTo(0, bounds.getHeight());
        eqCurveBackground.closeSubPath();
        g.setColour(juce::Colour::fromRGB(0x46, 0x1c, 0x00));
        g.fillPath(eqCurveBackground);

        g.setColour(juce::Colour::fromRGB(0xe8, 0x5d, 0x00));
        g.strokePath(eqCurve, juce::PathStrokeType(2.0f));
    }
}
