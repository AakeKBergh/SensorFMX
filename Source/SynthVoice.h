#pragma once
#include "HybridOscillator.h"
#include "SynthSound.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
class SynthVoice : public juce::SynthesiserVoice {
public:
    explicit SynthVoice(juce::AudioProcessorValueTreeState& state) {
        for (auto prefix : {juce::String("a"), juce::String("b")})
            for (auto suffix : {"Wave", "Shape", "Harmonics", "Character", "Punch", "Fine", "Level"}) params.push_back(state.getRawParameterValue(prefix+suffix));
        for (auto id : {"cutoff","resonance","filterAmount","ampAttack","ampDecay","ampSustain","ampRelease","filterAttack","filterDecay","filterSustain","filterRelease"})
            params.push_back(state.getRawParameterValue(id));
    }
    void prepare(double rate, int block) {
        a.prepare(rate); b.prepare(rate); amp.setSampleRate(rate); env.setSampleRate(rate);
        filter.prepare({rate, static_cast<juce::uint32>(block), 1});
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        cutoff.reset(rate, 0.02); resonance.reset(rate,0.02); amount.reset(rate,0.02);
        sampleRate = rate;
    }
    bool canPlaySound(juce::SynthesiserSound* s) override { return dynamic_cast<SynthSound*>(s) != nullptr; }
    void startNote(int note, float vel, juce::SynthesiserSound*, int wheel) override {
        baseFrequency = juce::MidiMessage::getMidiNoteInHertz(note); velocity = vel;
        update(true); pitchWheelMoved(wheel); a.noteOn(baseFrequency * bend); b.noteOn(baseFrequency * bend);
        filter.reset(); amp.reset(); env.reset(); amp.noteOn(); env.noteOn();
    }
    void stopNote(float, bool tail) override {
        amp.noteOff(); env.noteOff();
        if (!tail) { amp.reset(); env.reset(); clearCurrentNote(); }
    }
    void pitchWheelMoved(int value) override {
        bend = std::exp2((value - 8192) / 8192.0 * 2.0 / 12.0);
        a.setFrequency(baseFrequency * bend); b.setFrequency(baseFrequency * bend);
    }
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>& output, int start, int count) override {
        if (!isVoiceActive()) return;
        update(false);
        for (int i=0; i<count; ++i) {
            float hz = cutoff.getNextValue() * std::exp2(env.getNextSample() * amount.getNextValue());
            filter.setCutoffFrequency(juce::jlimit(20.0f, static_cast<float>(sampleRate*0.45), hz));
            filter.setResonance(resonance.getNextValue());
            const float sample = filter.processSample(0, 0.5f*(a.next()+b.next())) * amp.getNextSample() * velocity * 0.22f;
            for (int ch=0; ch<output.getNumChannels(); ++ch) output.addSample(ch,start+i,sample);
            if (!amp.isActive()) { clearCurrentNote(); break; }
        }
    }
private:
    float v(int i) const { return params[static_cast<size_t>(i)]->load(); }
    void update(bool immediate) {
        a.set(static_cast<int>(v(0)),v(1),static_cast<int>(v(2)),v(3),v(4),v(5),v(6),immediate);
        b.set(static_cast<int>(v(7)),v(8),static_cast<int>(v(9)),v(10),v(11),v(12),v(13),immediate);
        auto put = [immediate](auto& s,float value) { if (immediate) s.setCurrentAndTargetValue(value); else s.setTargetValue(value); };
        put(cutoff,v(14)); put(resonance,v(15)); put(amount,v(16));
        auto envelope = [](juce::ADSR& e, const juce::ADSR::Parameters& next, bool force) {
            const auto& old = e.getParameters();
            if (force || old.attack != next.attack || old.decay != next.decay || old.sustain != next.sustain || old.release != next.release)
                e.setParameters(next);
        };
        envelope(amp,{v(17),v(18),v(19),v(20)},immediate);
        envelope(env,{v(21),v(22),v(23),v(24)},immediate);
    }
    std::vector<std::atomic<float>*> params;
    HybridOscillator a,b;
    juce::ADSR amp,env;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::SmoothedValue<float> cutoff,resonance,amount;
    double sampleRate=44100, baseFrequency=440, bend=1;
    float velocity=0;
};
