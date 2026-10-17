#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <cstdint>
class AirGenerator {
public:
    void prepare(double rate) {
        sampleRate=rate; highpass=std::exp(-juce::MathConstants<double>::twoPi*2800/rate);
        amount.reset(rate,0.02);
    }
    void set(float air,bool immediate=false) {
        if(immediate) amount.setCurrentAndTargetValue(air); else amount.setTargetValue(air);
    }
    void noteOn(int note) {
        seed=0x9e3779b9u ^ (static_cast<uint32_t>(note+1)*0x85ebca6bu);
        lastNoise=lastOutput=0; envelope=1;
    }
    float next() {
        // Independent, deterministic per-note noise; no shared generator or locks.
        seed^=seed<<13; seed^=seed>>17; seed^=seed<<5;
        const double noise=static_cast<double>(seed)/2147483648.0-1;
        const double hp=highpass*(lastOutput+noise-lastNoise);
        lastNoise=noise; lastOutput=hp;
        const float air=amount.getNextValue();
        const float out=static_cast<float>(hp*envelope)*0.28f*air;
        envelope*=std::exp(-1.0/(sampleRate*(0.065+0.09*air)));
        return out;
    }
private:
    double sampleRate=44100,highpass=0,lastNoise=0,lastOutput=0,envelope=0;
    uint32_t seed=1;
    juce::SmoothedValue<float> amount;
};
