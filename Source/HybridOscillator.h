#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

// Familiar carrier waveforms with a phase-continuous sine modulator.
// Shape is waveform shaping; Character is steady coloration; Punch is transient only.
class HybridOscillator {
public:
    enum Wave { sine, triangle, saw, square };
    inline static constexpr std::array<float,9> harmonicRatios {0.5f,1,1.5f,2,3,4,5,6,8};
    static float ratioFor(int index) { return harmonicRatios[static_cast<size_t>(juce::jlimit(0,8,index))]; }
    static float characterDepth(float value) { return 4.0f * value * value * value * value; }
    void prepare(double rate) {
        sampleRate=rate;
        for(auto* s : {&shape,&character,&punch,&fine,&level,&ratio}) s->reset(rate,0.02);
        for(auto& weight : waveWeights) weight.reset(rate,0.01);
    }
    void set(int wave, float sh, int harmonic, float ch, float pu, float cents, float gain, bool immediate=false) {
        const int selected=juce::jlimit(0,3,wave);
        for(int i=0;i<4;++i) {
            auto& weight=waveWeights[static_cast<size_t>(i)];
            const float target=i==selected ? 1.0f : 0.0f;
            if(immediate) weight.setCurrentAndTargetValue(target); else weight.setTargetValue(target);
        }
        auto put=[immediate](auto& s,float v) { if(immediate) s.setCurrentAndTargetValue(v); else s.setTargetValue(v); };
        put(shape,juce::jlimit(0.0f,1.0f,sh)); put(character,juce::jlimit(0.0f,1.0f,ch));
        put(punch,juce::jlimit(0.0f,1.0f,pu)); put(fine,cents); put(level,gain); put(ratio,ratioFor(harmonic));
    }
    void noteOn(double hz) { frequency=hz; carrier=modulator=0; transient=1; }
    void setFrequency(double hz) { frequency=hz; }
    float next() {
        const double dt=juce::jlimit(0.0000001,0.45,frequency*std::exp2(fine.getNextValue()/1200.0)/sampleRate);
        const float p=punch.getNextValue();
        const double depth=characterDepth(character.getNextValue()) + 3.0*p*p*transient;
        const double phase=wrap(carrier + std::sin(juce::MathConstants<double>::twoPi*modulator)*depth/juce::MathConstants<double>::twoPi);
        const float sh=shape.getNextValue();
        double out=0;
        for(int i=0;i<4;++i) {
            const float weight=waveWeights[static_cast<size_t>(i)].getNextValue();
            if(weight>0) out+=waveform(i,phase,dt,sh)*weight;
        }
        carrier=wrap(carrier+dt); modulator=wrap(modulator+dt*ratio.getNextValue());
        // Higher Punch adds a little more body: 35–140 ms time constant.
        transient*=std::exp(-1.0/(sampleRate*(0.035+0.105*p)));
        return static_cast<float>(out)*level.getNextValue();
    }
private:
    static double wrap(double p) { return p-std::floor(p); }
    static double blep(double t,double dt) {
        if(t<dt) { t/=dt; return t+t-t*t-1; }
        if(t>1-dt) { t=(t-1)/dt; return t*t+t+t+1; }
        return 0;
    }
    static double waveform(int wave,double t,double dt,float shapeValue) {
        const double bipolar=2.0*shapeValue-1.0;
        switch(wave) {
            case sine: {
                const double x=std::sin(juce::MathConstants<double>::twoPi*t);
                // Centre is a pure sine; either side adds a subtle second harmonic.
                return (x+0.22*bipolar*std::sin(juce::MathConstants<double>::twoPi*2*t))/(1+0.22*std::abs(bipolar));
            }
            case triangle: {
                const double peak=0.5+0.3*bipolar;
                return t<peak ? -1+2*t/peak : 1-2*(t-peak)/(1-peak);
            }
            case saw: {
                const double x=2*t-1-blep(t,dt);
                const double drive=1+2.5*std::abs(bipolar);
                // Centre is a conventional ramp; Shape rounds its edges.
                return std::abs(bipolar)<0.001 ? x : std::tanh(drive*x)/std::tanh(drive);
            }
            case square: {
                const double width=0.5+0.4*bipolar;
                const double x=(t<width ? 1.0 : -1.0)+blep(t,dt)-blep(wrap(t-width),dt);
                return (x-(2*width-1))/(1+std::abs(2*width-1)); // Remove pulse-width DC.
            }
        }
        return 0;
    }
    double sampleRate=44100,frequency=440,carrier=0,modulator=0,transient=0;
    juce::SmoothedValue<float> shape,character,punch,fine,level,ratio;
    std::array<juce::SmoothedValue<float>,4> waveWeights;
};
