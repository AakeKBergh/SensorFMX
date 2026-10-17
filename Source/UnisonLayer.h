#pragma once
#include "HybridOscillator.h"

// One centre oscillator plus two quiet, symmetric wings. The centre remains present
// at every Width value so folding down to mono never relies on phase inversion.
class UnisonLayer {
public:
    void prepare(double rate) { for(auto& osc : oscillators) osc.prepare(rate); width.reset(rate,0.02); }
    void set(int wave,float shape,int harmonic,float character,float punch,float cents,float gain,float spread,bool immediate=false) {
        const float w=juce::jlimit(0.0f,1.0f,spread);
        const float detune=7.0f*std::pow(w,1.5f);
        if(immediate) width.setCurrentAndTargetValue(w); else width.setTargetValue(w);
        oscillators[0].set(wave,shape,harmonic,character,punch,cents,gain,immediate);
        oscillators[1].set(wave,shape,harmonic,character,punch,cents-detune,gain,immediate);
        oscillators[2].set(wave,shape,harmonic,character,punch,cents+detune,gain,immediate);
    }
    void noteOn(double hz) { for(auto& osc : oscillators) osc.noteOn(hz); }
    void setFrequency(double hz) { for(auto& osc : oscillators) osc.setFrequency(hz); }
    std::array<float,2> next() {
        const float w=width.getNextValue();
        const float centre=oscillators[0].next()*(1-0.45f*w);
        const float left=oscillators[1].next()*0.225f*w, right=oscillators[2].next()*0.225f*w;
        const float pan=0.9f*std::sqrt(w);
        return {centre+left*(1+pan)+right*(1-pan),centre+right*(1+pan)+left*(1-pan)};
    }
private:
    std::array<HybridOscillator,3> oscillators;
    juce::SmoothedValue<float> width;
};
