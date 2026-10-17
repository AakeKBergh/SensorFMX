#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <array>

// Adapted from AakeKBergh/SensorFlux, Source/InstrumentLook.h
// Reference commit: 1e67934601b3aacd980b504b20b285921c9def06
class SensorLookAndFeel : public juce::LookAndFeel_V4 {
    std::array<juce::Path,4> glyphs;
    const std::array<juce::String,4> waveNames{{"SINE","TRI","SAW","SQR"}};
public:
    SensorLookAndFeel(){
        for(int i=0;i<=24;++i){float x=float(i)/24,y=.5f-float(std::sin(i*6.283185307179586/24))*.4f;if(i==0)glyphs[0].startNewSubPath(x,y);else glyphs[0].lineTo(x,y);}
        glyphs[1].startNewSubPath(0,1);glyphs[1].lineTo(.5f,0);glyphs[1].lineTo(1,1);
        glyphs[2].startNewSubPath(0,1);glyphs[2].lineTo(1,0);glyphs[2].lineTo(1,1);
        glyphs[3].startNewSubPath(0,1);glyphs[3].lineTo(0,0);glyphs[3].lineTo(.5f,0);glyphs[3].lineTo(.5f,1);glyphs[3].lineTo(1,1);glyphs[3].lineTo(1,0);
        setColour(juce::Label::textColourId,juce::Colour(0xffc3c4c6));
        setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffd6d7da));
        setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        setColour(juce::TextButton::textColourOffId,juce::Colour(0xffc8c9cb));
        setColour(juce::TextButton::textColourOnId,juce::Colour(0xffdf686d));
    }
    juce::Label* createSliderTextBox(juce::Slider& slider) override {
        auto* box=juce::LookAndFeel_V4::createSliderTextBox(slider);
        box->setColour(juce::Label::outlineColourId,juce::Colours::transparentBlack);
        box->setColour(juce::Label::backgroundColourId,juce::Colours::transparentBlack);
        box->setColour(juce::TextEditor::outlineColourId,juce::Colour(0xff444648));
        box->setColour(juce::TextEditor::focusedOutlineColourId,juce::Colour(0xffbf535c));
        return box;
    }
    void drawLabel(juce::Graphics& g,juce::Label& label) override {
        if(dynamic_cast<juce::Slider*>(label.getParentComponent())==nullptr) {
            juce::LookAndFeel_V4::drawLabel(g,label);return;
        }
        if(label.isBeingEdited()) return;
        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(label.isEnabled()?1.0f:0.5f));
        g.setFont(getLabelFont(label));
        g.drawFittedText(label.getText(),label.getLocalBounds(),label.getJustificationType(),1);
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,float start,float end,juce::Slider& slider) override {
        const float radius=std::min(w,h)*.36f,cx=x+w*.5f,cy=y+h*.5f;
        const auto bounds=juce::Rectangle<float>(cx-radius,cy-radius,2*radius,2*radius);
        for(int i=0;i<=14;++i){const float a=start+(end-start)*i/14;
            g.setColour(juce::Colour(0xff626366));g.fillEllipse(cx+std::sin(a)*(radius+8)-.7f,cy-std::cos(a)*(radius+8)-.7f,1.4f,1.4f);}
        const bool large=slider.getProperties().getWithDefault("sensorProminent",false) || slider.getName()=="FLUX" || slider.getName()=="Output";
        g.setColour(juce::Colours::black.withAlpha(.16f));g.fillEllipse(bounds.translated(2,5).expanded(5));
        g.setColour(juce::Colours::black.withAlpha(.55f));g.fillEllipse(bounds.translated(1,3).expanded(2));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff7b7d80),cx-radius,cy-radius,juce::Colour(0xff070809),cx+radius,cy+radius,false));g.fillEllipse(bounds.expanded(1));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3e4043),cx-radius*.35f,cy-radius*.45f,juce::Colour(0xff111214),cx+radius,cy+radius,true));g.fillEllipse(bounds.reduced(1.5f));
        g.setColour(juce::Colour(0xff090a0b));g.drawEllipse(bounds.reduced(3),1);
        juce::Path rim;rim.addCentredArc(cx,cy,radius-2,radius-2,0,3.8f,5.8f,true);g.setColour(juce::Colours::white.withAlpha(.20f));g.strokePath(rim,juce::PathStrokeType(.9f));
        const float a=start+value*(end-start);
        g.setColour(juce::Colours::black.withAlpha(.8f));g.drawLine(cx+std::sin(a)*radius*.42f+1,cy-std::cos(a)*radius*.42f+1,cx+std::sin(a)*radius*.88f+1,cy-std::cos(a)*radius*.88f+1,3);
        g.setColour(juce::Colour(0xffe5e6e8));g.drawLine(cx+std::sin(a)*radius*.42f,cy-std::cos(a)*radius*.42f,cx+std::sin(a)*radius*.88f,cy-std::cos(a)*radius*.88f,2);
        if(large){juce::Path arc;arc.addCentredArc(cx,cy,radius+5,radius+5,0,start,a,true);g.setColour(juce::Colour(0xffdcdddf));g.strokePath(arc,juce::PathStrokeType(2));
            if(value>0){juce::Path tip;tip.addCentredArc(cx,cy,radius+5,radius+5,0,std::max(start,a-.14f),a,true);g.setColour(juce::Colour(0xffdc5a62));g.strokePath(tip,juce::PathStrokeType(2.3f));}}

    }
    void drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down) override {
        auto r=b.getLocalBounds().toFloat().reduced(1);
        g.setColour(juce::Colours::black.withAlpha(.5f));g.fillRoundedRectangle(r.translated(0,2),3);
        g.setGradientFill(juce::ColourGradient(juce::Colour(down?0xff171819:0xff2b2d2f),0,0,juce::Colour(0xff151617),0,r.getBottom(),false));g.fillRoundedRectangle(r,3);
        g.setColour(juce::Colour(over?0xff68696b:0xff444648));g.drawRoundedRectangle(r,3,1);
        g.setColour(juce::Colours::white.withAlpha(down?.025f:.10f));g.drawLine(r.getX()+4,r.getY()+1,r.getRight()-4,r.getY()+1,.6f);
        if(b.getToggleState() && b.getComponentID().startsWith("wave")){g.setColour(juce::Colour(0xffbf535c));g.fillRoundedRectangle(r.getCentreX()-9,r.getBottom()-3,18,1.5f,.6f);}
    }
    void drawButtonText(juce::Graphics& g,juce::TextButton& b,bool over,bool down) override {
        const bool wave=b.getComponentID().startsWith("wave");
        if(b.getComponentID().startsWith("chorus") || b.getComponentID().startsWith("reverb")){
            const bool active=b.isEnabled() && b.getToggleState() && b.getButtonText()!="OFF";
            auto textBounds=b.getLocalBounds().reduced(18,0);
            const float x=10,y=b.getHeight()*.5f;
            g.setColour(juce::Colour(active?0xfff2666d:0xff454749));g.fillEllipse(x-2,y-2,4,4);
            if(active){g.setColour(juce::Colour(0xffec5962).withAlpha(.12f));g.fillEllipse(x-6,y-6,12,12);}
            g.setColour(juce::Colour(active?0xffdf686d:0xffc8c9cc));g.setFont(11);
            g.drawText(b.getButtonText(),textBounds,juce::Justification::centred);return;
        }
        if(!wave){juce::LookAndFeel_V4::drawButtonText(g,b,over,down);return;}
        auto r=b.getLocalBounds().toFloat();const float cx=r.getCentreX(),y=5,w=21,h=14;
        const int index=b.getComponentID().getTrailingIntValue();
        g.setColour(juce::Colour(b.getToggleState()?0xffdf686d:0xffd0d1d4));g.strokePath(glyphs[size_t(index)],juce::PathStrokeType(1.2f),juce::AffineTransform::scale(w,h).translated(cx-w*.5f,y));
        g.setFont(8);g.drawText(waveNames[size_t(index)],0,23,b.getWidth(),12,juce::Justification::centred);
    }
    void drawToggleButton(juce::Graphics& g,juce::ToggleButton& b,bool over,bool down) override {
        const float cx=b.getWidth()*.5f,cy=b.getHeight()*.63f;
        if(b.getToggleState()){g.setColour(juce::Colour(0xffd85c62).withAlpha(.12f));g.fillEllipse(cx-9,cy-32,18,18);}
        g.setColour(juce::Colour(b.getToggleState()?0xffe1666d:0xff454749));g.fillEllipse(cx-2.5f,cy-25,5,5);
        g.setColour(juce::Colours::black);g.fillEllipse(cx-15,cy-11,30,30);
        g.setGradientFill(juce::ColourGradient(juce::Colour(over||down?0xff424448:0xff323437),cx,cy-10,juce::Colour(0xff17181a),cx,cy+17,false));g.fillEllipse(cx-12,cy-9,24,25);
        g.setColour(juce::Colour(0xffc3c4c6));g.setFont(10);g.drawText(b.getButtonText().toUpperCase(),0,0,b.getWidth(),18,juce::Justification::centred);
    }
};
