#include "PluginProcessor.h"
#include "PluginEditor.h"
SensorFMXProcessor::SensorFMXProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),
      state(*this,nullptr,"SensorFMX",Parameters::layout()), masterParameter(state.getRawParameterValue("master")) {
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
    master.reset(rate,0.02); master.setCurrentAndTargetValue(masterParameter->load());
}
void SensorFMXProcessor::processBlock(juce::AudioBuffer<float>& buffer,juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();
    synth.renderNextBlock(buffer,midi,0,buffer.getNumSamples());
    master.setTargetValue(masterParameter->load());
    for (int i=0;i<buffer.getNumSamples();++i) {
        const float gain=master.getNextValue();
        for (int c=0;c<buffer.getNumChannels();++c) buffer.setSample(c,i,buffer.getSample(c,i)*gain);
    }
    midi.clear();
}
void SensorFMXProcessor::getStateInformation(juce::MemoryBlock& data) {
    auto saved=state.copyState(); saved.setProperty("schemaVersion",2,nullptr);
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
            put(prefix+"Character",std::cbrt(juce::jlimit(0.0f,1.0f,oldDepth/4.0f)));
            put(prefix+"Punch",0);
        }
        if(!get(prefix+"Harmonics").isValid() && get(prefix+"Ratio").isValid()) {
            const float oldRatio=static_cast<float>(get(prefix+"Ratio").getProperty("value"));
            int nearest=0;
            for(int i=1;i<9;++i) if(std::abs(HybridOscillator::ratioFor(i)-oldRatio)<std::abs(HybridOscillator::ratioFor(nearest)-oldRatio)) nearest=i;
            put(prefix+"Harmonics",static_cast<float>(nearest));
        }
    }
    merged.setProperty("schemaVersion",2,nullptr);
    state.replaceState(merged);
}
juce::AudioProcessorEditor* SensorFMXProcessor::createEditor() { return new SensorFMXEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SensorFMXProcessor(); }
