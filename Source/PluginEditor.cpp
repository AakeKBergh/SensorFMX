#include "PluginEditor.h"
#include "SensorStyle.h"
SensorFMXEditor::SensorFMXEditor(SensorFMXProcessor& p)
    : AudioProcessorEditor(p),processor(p),waveA(p.state,"aWave"),waveB(p.state,"bWave"),
      slope(p.state,"filterSlope",{"LP12","LP24"},{0,1},false),
      chorus(p.state,"chorus",{"CHORUS I","CHORUS II"},{1,2},true),tooltips(this,700) {
    setLookAndFeel(&look);
    addAndMakeVisible(waveA); addAndMakeVisible(waveB); addAndMakeVisible(slope); addAndMakeVisible(chorus);
    for(auto prefix : {juce::String("a"),juce::String("b")}) {
        add(prefix+"Shape","SHAPE"); add(prefix+"Harmonics","HARMONICS"); add(prefix+"Character","CHARACTER");
        add(prefix+"Punch","PUNCH"); add(prefix+"Fine","TUNE"); add(prefix+"Level","LEVEL");
    }
    add("width","WIDTH"); add("air","AIR");
    add("cutoff","CUTOFF"); add("resonance","RESONANCE"); add("filterAmount","ENV AMOUNT");
    for(auto name : {"Attack","Decay","Sustain","Release"}) add(juce::String("filter")+name,juce::String(name).toUpperCase());
    for(auto name : {"Attack","Decay","Sustain","Release"}) add(juce::String("amp")+name,juce::String(name).toUpperCase());
    add("velocity","VELOCITY"); add("master","MASTER");
    setSize(1180,760);
}
SensorFMXEditor::~SensorFMXEditor() { setLookAndFeel(nullptr); }
void SensorFMXEditor::add(const juce::String& id,const juce::String& title) {
    auto knob=std::make_unique<CameraKnob>(processor.state,id,title);
    addAndMakeVisible(*knob); knobs.push_back(std::move(knob));
}
void SensorFMXEditor::place(int first,int count,juce::Rectangle<int> area) {
    const int width=area.getWidth()/count;
    for(int i=0;i<count;++i) knobs[static_cast<size_t>(first+i)]->setBounds(area.removeFromLeft(width).reduced(6,0));
}
void SensorFMXEditor::resized() {
    waveA.setBounds(82,110,464,38); waveB.setBounds(672,110,464,38);
    place(0,6,{26,150,548,103});
    place(6,6,{606,150,548,103});
    place(12,2,{30,300,350,114});
    slope.setBounds(430,339,210,34); chorus.setBounds(740,339,390,34);
    // Cutoff is deliberately larger than the other filter controls.
    knobs[14]->setBounds(24,452,184,132);
    place(15,2,{214,469,264,115}); place(17,4,{514,469,636,115});
    place(21,4,{24,628,720,116});
    knobs[25]->setBounds(758,628,158,116); knobs[26]->setBounds(966,628,184,116);

}
void SensorFMXEditor::paint(juce::Graphics& g) {
    SensorStyle::panel(g,getWidth(),getHeight());
    SensorStyle::branding(g,"FMX","FM / SUBTRACTIVE HYBRID SYNTHESIZER");
    g.setFont(juce::FontOptions(10.0f));g.setColour(SensorStyle::secondary);
    g.drawText("POLYPHONIC / 08",870,28,264,22,juce::Justification::centredRight);
    g.drawText("WAVE",30,112,48,30,juce::Justification::centredLeft);
    g.drawText("WAVE",620,112,48,30,juce::Justification::centredLeft);
    g.setColour(SensorStyle::rule);
    for(int y : {80,266,424,590}) g.drawHorizontalLine(y,28,1152);
    for(auto line : {juce::Rectangle<int>(590,94,1,160),{402,300,1,110},{690,308,1,94},{496,464,1,120},{944,624,1,120}}) g.fillRect(line);
    const auto title=[&](int x,int y,const juce::String& text) { SensorStyle::heading(g,text,x,y,480); };
    title(30,84,"OSCILLATOR A");title(620,84,"OSCILLATOR B");
    title(30,272,"TONE / SPACE");title(430,302,"FILTER SLOPE");title(748,302,"CHORUS");
    title(30,430,"FILTER");title(526,430,"FILTER ENVELOPE");title(30,596,"AMP / PERFORMANCE");
    g.setFont(juce::FontOptions(10.0f));g.setColour(SensorStyle::secondary);
    g.drawText("PRESS ACTIVE MODE TO BYPASS",748,378,390,20,juce::Justification::centredLeft);
}
