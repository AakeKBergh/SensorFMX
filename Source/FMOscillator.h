#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
// A sine carrier phase-modulated by a sine modulator; depth is in radians.
class FMOscillator {
public:
    void prepare(double rate) {
        sampleRate = rate;
        for (auto* s : {&level, &ratio, &depth, &fine, &decay}) s->reset(rate, 0.02);
    }
    void set(float l, float r, float d, float f, float seconds, bool immediate = false) {
        auto put = [immediate](auto& s, float v) { if (immediate) s.setCurrentAndTargetValue(v); else s.setTargetValue(v); };
        put(level,l); put(ratio,r); put(depth,d); put(fine,f); put(decay,seconds);
    }
    void noteOn(double hz) { frequency = hz; carrier = modulator = 0; envelope = 1; }
    void setFrequency(double hz) { frequency = hz; }
    float next() {
        const double hz = frequency * std::exp2(fine.getNextValue() / 1200.0);
        const double r = ratio.getNextValue();
        const float out = static_cast<float>(std::sin(carrier + std::sin(modulator) * depth.getNextValue() * envelope));
        const double twoPi = juce::MathConstants<double>::twoPi;
        carrier = std::fmod(carrier + twoPi * hz / sampleRate, twoPi);
        modulator = std::fmod(modulator + twoPi * hz * r / sampleRate, twoPi);
        envelope *= std::exp(-1.0 / (sampleRate * decay.getNextValue()));
        return out * level.getNextValue();
    }
private:
    double sampleRate = 44100, frequency = 440, carrier = 0, modulator = 0, envelope = 0;
    juce::SmoothedValue<float> level, ratio, depth, fine, decay;
};
