#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>

template <typename Emitter> class ControlPointListener {
public:
    virtual ~ControlPointListener() = default;
    virtual void controlPointValueChanged(Emitter*) = 0;
    virtual void controlPointDragStarted(Emitter*) {}
    virtual void controlPointDragEnded(Emitter*) {}
};

class ControlPoint : public juce::Component {
public:
    ControlPoint();

    void setXValue(float);
    void setYValue(float);

    bool hitTest(int x, int y) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    void paint(juce::Graphics& g) override;

    void addListener(ControlPointListener<ControlPoint>* listener);
    void removeListener(ControlPointListener<ControlPoint>* listener);

    juce::Point<float> position;

private:
    bool isDragged;
    juce::ListenerList<ControlPointListener<ControlPoint>> listeners;
    juce::Point<float> startPosition;
};

class ControlPointAttachment : private ControlPointListener<ControlPoint> {
public:
    ControlPointAttachment(
        ControlPoint& cp, juce::AudioParameterFloat& xParam,
        juce::AudioParameterFloat& yParam
    );
    ControlPointAttachment(const ControlPointAttachment&) = delete;
    ~ControlPointAttachment();

private:
    void controlPointValueChanged(ControlPoint*) override;
    void controlPointDragStarted(ControlPoint*) override {
        xParamAttachment.beginGesture();
        yParamAttachment.beginGesture();
    }
    void controlPointDragEnded(ControlPoint*) override {
        xParamAttachment.endGesture();
        yParamAttachment.endGesture();
    }

    void setXValue(float);
    void setYValue(float);
    ControlPoint& controlPoint;
    juce::AudioParameterFloat& xParam;
    juce::AudioParameterFloat& yParam;

    juce::ParameterAttachment xParamAttachment;
    juce::ParameterAttachment yParamAttachment;

    std::atomic<float> lastXValue{ 0.0f };
    std::atomic<float> lastYValue{ 0.0f };

    bool ignoreCallbacks;
};

class NoiseColorEditor : public juce::Component {
public:
    NoiseColorEditor(NoisatAudioProcessor&);
    ~NoiseColorEditor();

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    NoisatAudioProcessor& audioProcessor;
    ControlPoint hpControl;
    ControlPointAttachment hpControlAttch;
    ControlPoint lpControl;
    ControlPointAttachment lpControlAttch;

    ControlPoint* currentControlPoint;
};
