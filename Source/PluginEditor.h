#pragma once
#include "PluginProcessor.h"
#include "CameraKnob.h"
#include "WaveSelector.h"
class SensorFMXEditor : public juce::AudioProcessorEditor {
public:
    explicit SensorFMXEditor(SensorFMXProcessor&);
    ~SensorFMXEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void add(const juce::String&,const juce::String&);
    void place(int first,int count,juce::Rectangle<int>);
    SensorFMXProcessor& processor;
    CustomLookAndFeel look;
    std::vector<std::unique_ptr<CameraKnob>> knobs;
    WaveSelector waveA,waveB;
    juce::TooltipWindow tooltips;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SensorFMXEditor)
};
