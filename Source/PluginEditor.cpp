#include "PluginEditor.h"
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
    waveA.setBounds(82,112,464,30); waveB.setBounds(672,112,464,30);
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
    g.fillAll(juce::Colour(0xff242725));
    // Almost invisible horizontal machining lines; deterministic and image-free.
    for(int y=0;y<getHeight();y+=3) {
        g.setColour(juce::Colours::black.withAlpha(y%9==0 ? 0.07f : 0.025f));
        g.drawHorizontalLine(y,0,static_cast<float>(getWidth()));
    }
    g.setColour(CustomLookAndFeel::text);
    g.setFont(juce::FontOptions(32.0f)); g.drawText("SENSOR",28,10,141,40,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(32.0f,juce::Font::bold)); g.drawText("FMX",169,10,100,40,juce::Justification::centredLeft);
    g.setColour(CustomLookAndFeel::muted);
    g.setFont(juce::Font(juce::FontOptions(10.0f)).withExtraKerningFactor(0.13f));
    g.drawText("FM / SUBTRACTIVE HYBRID SYNTHESIZER",30,49,600,20,juce::Justification::centredLeft);
    g.drawText("POLYPHONIC  /  08",870,28,264,22,juce::Justification::centredRight);
    auto line=[&](int y,int left,int right) {
        g.setColour(juce::Colours::black.withAlpha(0.65f)); g.drawHorizontalLine(y,static_cast<float>(left),static_cast<float>(right));
        g.setColour(juce::Colours::white.withAlpha(0.055f)); g.drawHorizontalLine(y+1,static_cast<float>(left),static_cast<float>(right));
    };
    g.setFont(juce::FontOptions(10.0f)); g.setColour(CustomLookAndFeel::muted);
    g.drawText("WAVE",30,112,48,30,juce::Justification::centredLeft);
    g.drawText("WAVE",620,112,48,30,juce::Justification::centredLeft);
    for(int y : {78,266,424,590}) line(y,28,1152);
    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.drawVerticalLine(590,94,250); g.drawVerticalLine(402,300,410);
    g.drawVerticalLine(690,308,402); g.drawVerticalLine(496,464,580); g.drawVerticalLine(944,624,744);
    auto title=[&](int x,int y,const juce::String& text) {
        g.setColour(CustomLookAndFeel::amber.withAlpha(0.9f));
        g.setFont(juce::Font(juce::FontOptions(12.0f,juce::Font::bold)).withExtraKerningFactor(0.12f));
        g.drawText(text,x,y,480,24,juce::Justification::centredLeft);
    };
    title(30,86,"OSC A"); title(620,86,"OSC B");
    title(30,274,"TONE / SPACE");
    title(430,304,"FILTER SLOPE"); title(748,304,"CHORUS");
    title(30,432,"FILTER"); title(526,432,"FILTER ENVELOPE");
    title(30,598,"AMP / PERFORMANCE");
    g.setFont(juce::FontOptions(10.0f)); g.setColour(CustomLookAndFeel::muted);
    g.drawText("PRESS ACTIVE MODE TO BYPASS",748,378,390,20,juce::Justification::centredLeft);
    for(auto point : {juce::Point<float>{12,12},{1168,12},{12,748},{1168,748}}) {
        g.setColour(juce::Colour(0xff101311)); g.fillEllipse(point.x-3,point.y-3,6,6);
        g.setColour(juce::Colour(0xff474a43)); g.drawLine(point.x-1.5f,point.y,point.x+1.5f,point.y,0.7f);
    }
}
