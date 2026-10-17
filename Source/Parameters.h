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
        add(prefix+"Level", name+"Level", 0, 1, prefix == "a" ? 0.7f : 0.4f);
        add(prefix+"Ratio", name+"Ratio", 0.25f, 16, 2, 0.5f);
        add(prefix+"FM", name+"FM Amount", 0, 12, 2);
        add(prefix+"Decay", name+"FM Decay (s)", 0.005f, 8, 0.8f, 0.35f);
        add(prefix+"Fine", name+"Fine (cents)", -100, 100, 0);
    }
    add("cutoff", "Cutoff (Hz)", 20, 20000, 6000, 0.25f);
    add("resonance", "Resonance (Q)", 0.7071f, 8, 0.8f, 0.5f);
    add("filterAmount", "Filter Envelope (octaves)", -8, 8, 2);
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
