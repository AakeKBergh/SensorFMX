#pragma once
#include "PluginProcessor.h"
class SensorFMXEditor : public juce::AudioProcessorEditor {
public:
    explicit SensorFMXEditor(SensorFMXProcessor&);
    ~SensorFMXEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    struct Knob {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };
    void add(const juce::String&,const juce::String&);
    void place(int first,int count,juce::Rectangle<int>);
    SensorFMXProcessor& processor;
    juce::LookAndFeel_V4 look;
    std::vector<std::unique_ptr<Knob>> knobs;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SensorFMXEditor)
};
