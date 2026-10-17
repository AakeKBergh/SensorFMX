#include "PluginEditor.h"
SensorFMXEditor::SensorFMXEditor(SensorFMXProcessor& p)
    : AudioProcessorEditor(p),processor(p),waveA(p.state,"aWave"),waveB(p.state,"bWave"),tooltips(this,700) {
    setLookAndFeel(&look);
    addAndMakeVisible(waveA); addAndMakeVisible(waveB);
    for(auto prefix : {juce::String("a"),juce::String("b")}) {
        add(prefix+"Shape","SHAPE"); add(prefix+"Harmonics","HARMONICS"); add(prefix+"Character","CHARACTER");
        add(prefix+"Punch","PUNCH"); add(prefix+"Fine","TUNE"); add(prefix+"Level","LEVEL");
    }
    add("cutoff","CUTOFF"); add("resonance","RESONANCE"); add("filterAmount","ENV AMOUNT");
    for(auto name : {"Attack","Decay","Sustain","Release"}) add(juce::String("filter")+name,juce::String(name).toUpperCase());
    for(auto name : {"Attack","Decay","Sustain","Release"}) add(juce::String("amp")+name,juce::String(name).toUpperCase());
    add("master","VOLUME");
    setSize(1120,800);
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
    waveA.setBounds(88,148,432,30); waveB.setBounds(640,148,432,30);
    place(0,3,{28,192,512,104}); place(3,3,{28,306,512,104});
    place(6,3,{580,192,512,104}); place(9,3,{580,306,512,104});
    place(12,3,{28,478,450,122}); place(15,4,{500,478,592,122});
    place(19,4,{28,660,760,122}); place(23,1,{866,660,210,122});
}
void SensorFMXEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff242725));
    // Almost invisible horizontal machining lines; deterministic and image-free.
    for(int y=0;y<getHeight();y+=3) {
        g.setColour(juce::Colours::black.withAlpha(y%9==0 ? 0.07f : 0.025f));
        g.drawHorizontalLine(y,0,static_cast<float>(getWidth()));
    }
    g.setColour(CustomLookAndFeel::text);
    g.setFont(juce::FontOptions(32.0f)); g.drawText("SENSOR",28,23,141,40,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(32.0f,juce::Font::bold)); g.drawText("FMX",169,23,100,40,juce::Justification::centredLeft);
    g.setColour(CustomLookAndFeel::muted);
    g.setFont(juce::Font(juce::FontOptions(10.0f)).withExtraKerningFactor(0.13f));
    g.drawText("FM / SUBTRACTIVE HYBRID SYNTHESIZER",30,66,600,20,juce::Justification::centredLeft);
    g.drawText("POLYPHONIC  /  08",810,40,264,22,juce::Justification::centredRight);
    auto line=[&](int y,int left,int right) {
        g.setColour(juce::Colours::black.withAlpha(0.65f)); g.drawHorizontalLine(y,static_cast<float>(left),static_cast<float>(right));
        g.setColour(juce::Colours::white.withAlpha(0.055f)); g.drawHorizontalLine(y+1,static_cast<float>(left),static_cast<float>(right));
    };
    g.setFont(juce::FontOptions(10.0f)); g.setColour(CustomLookAndFeel::muted);
    g.drawText("WAVE",32,148,48,30,juce::Justification::centredLeft);
    g.drawText("WAVE",584,148,48,30,juce::Justification::centredLeft);
    line(102,28,1092); line(432,28,1092); line(616,28,1092);
    g.setColour(juce::Colours::black.withAlpha(0.45f)); g.drawVerticalLine(560,120,410); g.drawVerticalLine(842,638,782);
    g.drawVerticalLine(487,478,596);
    auto title=[&](int x,int y,const juce::String& text) {
        g.setColour(CustomLookAndFeel::amber.withAlpha(0.9f));
        g.setFont(juce::Font(juce::FontOptions(12.0f,juce::Font::bold)).withExtraKerningFactor(0.12f));
        g.drawText(text,x,y,480,24,juce::Justification::centredLeft);
    };
    title(32,114,"OSCILLATOR A"); title(584,114,"OSCILLATOR B");
    title(32,444,"FILTER"); title(512,444,"FILTER ENVELOPE");
    title(32,630,"AMPLIFIER"); title(878,630,"MASTER");
    for(auto point : {juce::Point<float>{12,12},{1108,12},{12,788},{1108,788}}) {
        g.setColour(juce::Colour(0xff101311)); g.fillEllipse(point.x-3,point.y-3,6,6);
        g.setColour(juce::Colour(0xff474a43)); g.drawLine(point.x-1.5f,point.y,point.x+1.5f,point.y,0.7f);
    }
}
