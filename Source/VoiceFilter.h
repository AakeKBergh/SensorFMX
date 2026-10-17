#pragma once
#include <array>
#include <juce_dsp/juce_dsp.h>
class VoiceFilter {
public:
    void prepare(double rate,int block) {
        sampleRate=rate;
        for(auto* f : {&first,&second}) {
            f->prepare({rate,static_cast<juce::uint32>(block),2});
            f->setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        }
        slope.reset(rate,0.02);
    }
    void reset() { first.reset(); second.reset(); }
    void setSlope(bool steep,bool immediate=false) {
        if(immediate) slope.setCurrentAndTargetValue(steep ? 1.0f : 0.0f);
        else slope.setTargetValue(steep ? 1.0f : 0.0f);
    }
    void set(float cutoff,float resonance) {
        cutoff=juce::jlimit(20.0f,static_cast<float>(sampleRate*0.45),cutoff);
        first.setCutoffFrequency(cutoff); second.setCutoffFrequency(cutoff);
        first.setResonance(resonance); second.setResonance(0.70710678f);
    }
    std::array<float,2> process(std::array<float,2> input) {
        const float blend=slope.getNextValue();
        for(size_t c=0;c<2;++c) {
            const float lp12=first.processSample(static_cast<int>(c),input[c]);
            const float lp24=second.processSample(static_cast<int>(c),lp12);
            input[c]=lp12+(lp24-lp12)*blend;
        }
        return input;
    }
private:
    double sampleRate=44100;
    juce::dsp::StateVariableTPTFilter<float> first,second;
    juce::SmoothedValue<float> slope;
};
