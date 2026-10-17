#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <vector>
// Two feed-forward fractional delay lines, with slow quadrature modulation.
// All storage is allocated in prepare; bypass and mode changes ramp smoothly.
class SimpleChorus {
public:
    void prepare(double rate) {
        sampleRate=rate;
        for(auto& line : delay) line.assign(static_cast<size_t>(std::ceil(rate*0.05))+4,0);
        write=0; phase=0;
        mix.reset(rate,0.025); depth.reset(rate,0.05); speed.reset(rate,0.05);
        mix.setCurrentAndTargetValue(0); depth.setCurrentAndTargetValue(0.0006f); speed.setCurrentAndTargetValue(0.18f);
    }
    void setMode(int mode) {
        mix.setTargetValue(mode==0 ? 0.0f : (mode==1 ? 0.18f : 0.34f));
        depth.setTargetValue(mode==2 ? 0.0014f : 0.0006f);
        speed.setTargetValue(mode==2 ? 0.24f : 0.18f);
    }
    std::array<float,2> process(std::array<float,2> input) {
        const float wet=mix.getNextValue(), excursion=depth.getNextValue();
        const double increment=speed.getNextValue()/sampleRate;
        for(size_t c=0;c<2;++c) {
            delay[c][write]=input[c];
            const double seconds=0.012+excursion*std::sin(juce::MathConstants<double>::twoPi*(phase+c*0.25));
            double position=static_cast<double>(write)-seconds*sampleRate;
            if(position<0) position+=delay[c].size();
            const auto left=static_cast<size_t>(position), right=(left+1)%delay[c].size();
            const float fraction=static_cast<float>(position-left);
            const float delayed=delay[c][left]+(delay[c][right]-delay[c][left])*fraction;
            input[c]=input[c]*(1-wet)+delayed*wet;
        }
        write=(write+1)%delay[0].size(); phase+=increment; phase-=std::floor(phase);
        return input;
    }
private:
    double sampleRate=44100,phase=0;
    size_t write=0;
    std::array<std::vector<float>,2> delay;
    juce::SmoothedValue<float> mix,depth,speed;
};
