#include "PluginEditor.h"
SensorFMXEditor::SensorFMXEditor(SensorFMXProcessor& p):AudioProcessorEditor(p),processor(p) {
    look.setColour(juce::Slider::thumbColourId,juce::Colour(0xffc7b899));
    look.setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffc7b899));
    look.setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff45494d));
    look.setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffe5e5df));
    look.setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
    setLookAndFeel(&look);
    for (auto prefix : {juce::String("a"),juce::String("b")}) {
        add(prefix+"Level","Level"); add(prefix+"Ratio","Ratio"); add(prefix+"FM","FM Amount");
        add(prefix+"Decay","FM Decay (s)"); add(prefix+"Fine","Fine (cents)");
    }
    add("cutoff","Cutoff (Hz)"); add("resonance","Resonance (Q)"); add("filterAmount","Env (octaves)");
    for (auto name : {"Attack","Decay","Sustain","Release"}) add(juce::String("filter")+name,juce::String(name)+(juce::String(name)=="Sustain" ? "" : " (s)"));
    for (auto name : {"Attack","Decay","Sustain","Release"}) add(juce::String("amp")+name,juce::String(name)+(juce::String(name)=="Sustain" ? "" : " (s)"));
    add("master","Volume");
    setSize(1050,660);
}
SensorFMXEditor::~SensorFMXEditor() { setLookAndFeel(nullptr); }
void SensorFMXEditor::add(const juce::String& id,const juce::String& title) {
    auto k=std::make_unique<Knob>();
    k->slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    k->slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,100,22);
    k->slider.setNumDecimalPlacesToDisplay(2);
    k->slider.setName(title);
    k->label.setText(title,juce::dontSendNotification); k->label.setJustificationType(juce::Justification::centred);
    k->label.setColour(juce::Label::textColourId,juce::Colour(0xffd5d7d8));
    addAndMakeVisible(k->slider); addAndMakeVisible(k->label);
    k->attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state,id,k->slider);
    const int decimals = id == "cutoff" ? 0 : (id.endsWith("Attack") || id.endsWith("Decay") || id.endsWith("Release") ? 3 : 2);
    k->slider.textFromValueFunction = [decimals](double value) { return juce::String(value,decimals); };
    k->slider.updateText();
    knobs.push_back(std::move(k));
}
void SensorFMXEditor::place(int first,int count,juce::Rectangle<int> area) {
    auto width=area.getWidth()/count;
    for(int i=0;i<count;++i) {
        auto cell=area.removeFromLeft(width).reduced(4,0);
        knobs[static_cast<size_t>(first+i)]->label.setBounds(cell.removeFromTop(24));
        knobs[static_cast<size_t>(first+i)]->slider.setBounds(cell);
    }
}
void SensorFMXEditor::resized() {
    place(0,5,{24,146,493,120}); place(5,5,{533,146,493,120});
    place(10,7,{24,330,1002,120}); place(17,4,{24,514,780,120}); place(21,1,{836,514,190,120});
}
void SensorFMXEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff202326));
    g.setColour(juce::Colour(0xffe6e5df));
    g.setFont(juce::FontOptions(34.0f)); g.drawText("SENSOR",26,20,150,42,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(34.0f,juce::Font::bold)); g.drawText("FMX",182,20,120,42,juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(12.0f)); g.setColour(juce::Colour(0xff9b9e9f));
    g.drawText("FM / SUBTRACTIVE HYBRID SYNTHESIZER",27,65,600,20,juce::Justification::centredLeft);
    auto panel=[&](juce::Rectangle<int> r,const juce::String& title) {
        g.setColour(juce::Colour(0xff2c3034)); g.fillRoundedRectangle(r.toFloat(),4);
        g.setColour(juce::Colour(0xff4a4e51)); g.drawHorizontalLine(r.getY()+35,static_cast<float>(r.getX()+14),static_cast<float>(r.getRight()-14));
        g.setColour(juce::Colour(0xffc7b899)); g.setFont(juce::FontOptions(13.0f,juce::Font::bold));
        g.drawText(title,r.reduced(16,4).removeFromTop(28),juce::Justification::centredLeft);
    };
    panel({16,104,509,176},"OSCILLATOR A"); panel({525,104,509,176},"OSCILLATOR B");
    panel({16,288,1018,176},"FILTER  /  ENVELOPE ADSR");
    panel({16,472,796,176},"AMPLIFIER  /  ADSR"); panel({820,472,214,176},"MASTER");
}
