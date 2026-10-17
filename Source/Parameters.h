#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
namespace Parameters {
inline juce::AudioProcessorValueTreeState::ParameterLayout layout() {
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add = [&](const juce::String& id, const juce::String& name, float lo, float hi, float value, float skew = 1.0f) {
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id, 1}, name,
            juce::NormalisableRange<float>(lo, hi, 0.0f, skew), value));
    };
    for (auto prefix : {juce::String("a"), juce::String("b")}) {
        auto name = "OSC " + prefix.toUpperCase() + " ";
        p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{prefix+"Wave",1}, name+"Wave", juce::StringArray{"Sine","Triangle","Saw","Square"}, prefix=="a" ? 2 : 1));
        add(prefix+"Shape",name+"Shape",0,1,0.5f);
        p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{prefix+"Harmonics",1}, name+"Harmonics", juce::StringArray{"0.5","1","1.5","2","3","4","5","6","8"},3));
        add(prefix+"Character",name+"Character",0,1,prefix=="a" ? 0.12f : 0.07f);
        add(prefix+"Punch",name+"Punch",0,1,prefix=="a" ? 0.1f : 0.0f);
        add(prefix+"Fine",name+"Tune (cents)",-100,100,prefix=="a" ? 0.0f : -3.0f);
        add(prefix+"Level",name+"Level",0,1,prefix=="a" ? 0.7f : 0.3f);
    }
    add("width","Width",0,1,0.2f);
    add("air","Air",0,1,0.1f);
    add("velocity","Velocity",0,1,0.45f);
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"filterSlope",1},"Filter Slope",juce::StringArray{"LP12","LP24"},0));
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"chorus",1},"Chorus",juce::StringArray{"Off","I","II"},0));
    add("cutoff", "Cutoff (Hz)", 20, 20000, 7200, 0.25f);
    add("resonance", "Resonance (Q)", 0.7071f, 8, 0.8f, 0.5f);
    add("filterAmount", "Filter Envelope (octaves)", -8, 8, 1);
    for (auto prefix : {juce::String("amp"), juce::String("filter")}) {
        add(prefix+"Attack", prefix+" Attack (s)", 0.001f, 8, 0.01f, 0.3f);
        add(prefix+"Decay", prefix+" Decay (s)", 0.005f, 8, 0.3f, 0.3f);
        add(prefix+"Sustain", prefix+" Sustain", 0, 1, prefix == "amp" ? 0.8f : 0.3f);
        add(prefix+"Release", prefix+" Release (s)", 0.005f, 12, 0.4f, 0.3f);
    }
    add("master", "Master Volume", 0, 1, 0.7f);
    return p;
}
}
