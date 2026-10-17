#include "PluginProcessor.h"
#include "PluginEditor.h"
SensorFMXProcessor::SensorFMXProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output",juce::AudioChannelSet::stereo(),true)),
      state(*this,nullptr,"SensorFMX",Parameters::layout()), masterParameter(state.getRawParameterValue("master")) {
    for (int i=0;i<8;++i) synth.addVoice(new SynthVoice(state));
    synth.addSound(new SynthSound());
    // Avoid abrupt truncation of a stolen voice. Extra notes are ignored until a voice is free.
    synth.setNoteStealingEnabled(false);
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
    if (auto xml=state.copyState().createXml()) copyXmlToBinary(*xml,data);
}
void SensorFMXProcessor::setStateInformation(const void* data,int size) {
    if (auto xml=getXmlFromBinary(data,size))
        if (xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessorEditor* SensorFMXProcessor::createEditor() { return new SensorFMXEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new SensorFMXProcessor(); }
