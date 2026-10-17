#pragma once
#include "Parameters.h"
#include "SynthVoice.h"
class SensorFMXProcessor : public juce::AudioProcessor {
public:
    SensorFMXProcessor();
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "SENSOR FMX"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 12; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Init"; }
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState state;
private:
    juce::Synthesiser synth;
    juce::SmoothedValue<float> master;
    std::atomic<float>* masterParameter;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SensorFMXProcessor)
};
