#include "PluginProcessor.h"
#include "PluginEditor.h"
SensorFMXProcessor::SensorFMXProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),
      state(*this,nullptr,"SensorFMX",Parameters::layout()), masterParameter(state.getRawParameterValue("master")), chorusParameter(state.getRawParameterValue("chorus")) {
    for (int i=0;i<8;++i) synth.addVoice(new SynthVoice(state));
    synth.addSound(new SynthSound());
    // Avoid abrupt truncation of a stolen voice. Extra notes are ignored until a voice is free.
    synth.setNoteStealingEnabled(false);
    defaultState=state.copyState();
}
bool SensorFMXProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    return l.getMainInputChannelSet().isDisabled() &&
        (l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo() || l.getMainOutputChannelSet()==juce::AudioChannelSet::mono());
}
void SensorFMXProcessor::prepareToPlay(double rate,int block) {
    synth.setCurrentPlaybackSampleRate(rate);
    for (int i=0;i<synth.getNumVoices();++i) static_cast<SynthVoice*>(synth.getVoice(i))->prepare(rate,block);
    chorus.prepare(rate);
    master.reset(rate,0.02); master.setCurrentAndTargetValue(masterParameter->load());
}
void SensorFMXProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock(buffer,midi,0,buffer.getNumSamples());
    chorus.setMode(static_cast<int>(chorusParameter->load()));
    master.setTargetValue(masterParameter->load());
    for (int i=0;i<buffer.getNumSamples();++i) {
        const float gain=master.getNextValue();
        const bool mono=buffer.getNumChannels()==1;
        const auto stereo=chorus.process({buffer.getSample(0,i),buffer.getSample(mono ? 0 : 1,i)});
        if(mono) buffer.setSample(0,i,0.5f*(stereo[0]+stereo[1])*gain);
        else for(int c=0;c<2;++c) buffer.setSample(c,i,stereo[static_cast<size_t>(c)]*gain);
    }
    midi.clear();
}
void SensorFMXProcessor::getStateInformation(juce::MemoryBlock& data) {
    auto saved=state.copyState(); saved.setProperty("schemaVersion",3,nullptr);
    if (auto xml=saved.createXml()) copyXmlToBinary(*xml,data);
}
void SensorFMXProcessor::setStateInformation(const void* data,int size) {
    auto xml=getXmlFromBinary(data,size);
    if (!xml || !xml->hasTagName(state.state.getType())) return;
    const auto incoming=juce::ValueTree::fromXml(*xml);
    auto merged=defaultState.createCopy();
    auto get=[&](const juce::String& id) { return incoming.getChildWithProperty("id",id); };
    auto put=[&](const juce::String& id,float value) {
        if (!std::isfinite(value)) return;
        if(auto* p=state.getParameter(id)) {
            const auto range=p->getNormalisableRange();
            merged.getChildWithProperty("id",id).setProperty("value",range.snapToLegalValue(juce::jlimit(range.start,range.end,value)),nullptr);
        }
    };
    for(auto child : incoming) {
        const auto id=child.getProperty("id").toString();
        if(child.hasProperty("value")) put(id,static_cast<float>(child.getProperty("value")));
    }
    // Prototype v1: retain shared IDs and approximate the old sine FM patch.
    for(auto prefix : {juce::String("a"),juce::String("b")}) {
        if(!get(prefix+"Wave").isValid() && get(prefix+"FM").isValid()) {
            put(prefix+"Wave",0);
            const float oldDepth=static_cast<float>(get(prefix+"FM").getProperty("value"));
            put(prefix+"Character",std::pow(juce::jlimit(0.0f,1.0f,oldDepth/4.0f),0.25f));
            put(prefix+"Punch",0);
        }
        if(!get(prefix+"Harmonics").isValid() && get(prefix+"Ratio").isValid()) {
            const float oldRatio=static_cast<float>(get(prefix+"Ratio").getProperty("value"));
            int nearest=0;
            for(int i=1;i<9;++i) if(std::abs(HybridOscillator::ratioFor(i)-oldRatio)<std::abs(HybridOscillator::ratioFor(nearest)-oldRatio)) nearest=i;
            put(prefix+"Harmonics",static_cast<float>(nearest));
        }
    }
    const int version=static_cast<int>(incoming.getProperty("schemaVersion",1));
    if(version<3) {
        for(auto id : {"width","air","velocity","chorus","filterSlope"})
            if(!get(id).isValid()) put(id,juce::String(id)=="velocity" ? 1.0f : 0.0f);
        // Preserve v2 steady FM depth after the cubic-to-quartic rebalancing.
        if(version==2) for(auto prefix : {juce::String("a"),juce::String("b")})
            if(get(prefix+"Character").isValid()) put(prefix+"Character",std::pow(juce::jlimit(0.0f,1.0f,static_cast<float>(get(prefix+"Character").getProperty("value"))),0.75f));
    }
    merged.setProperty("schemaVersion",3,nullptr);
    state.replaceState(merged);
}
juce::AudioProcessorEditor* SensorFMXProcessor::createEditor() { return new SensorFMXEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SensorFMXProcessor(); }
