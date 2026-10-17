#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
    inline static const juce::Colour amber{0xffc49965}, text{0xffd9d5cc}, muted{0xff90918b};
    CustomLookAndFeel() {
        setColour(juce::Slider::textBoxTextColourId,text);
        setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::transparentBlack);
        setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
        setColour(juce::TextEditor::highlightColourId,amber.withAlpha(0.25f));
        setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff202222));
        setColour(juce::ComboBox::textColourId,text);
        setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff484944));
        setColour(juce::ComboBox::arrowColourId,amber);
        setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff202222));
        setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff454039));
        setColour(juce::PopupMenu::textColourId,text);
    }
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float value,float begin,float end,juce::Slider& slider) override {
        const float radius=juce::jmin(w,h)*0.36f;
        const juce::Point<float> centre{static_cast<float>(x)+w*0.5f,static_cast<float>(y)+h*0.5f};
        const auto body=juce::Rectangle<float>(radius*2,radius*2).withCentre(centre);
        g.setColour(juce::Colours::black.withAlpha(0.45f)); g.fillEllipse(body.translated(0,3).expanded(3));
        g.setColour(amber.withAlpha(0.025f+0.07f*value)); g.fillEllipse(body.expanded(5));
        juce::Path track,lit;
        track.addCentredArc(centre.x,centre.y,radius+5,radius+5,0,begin,end,true);
        lit.addCentredArc(centre.x,centre.y,radius+5,radius+5,0,begin,begin+value*(end-begin),true);
        g.setColour(juce::Colour(0xff454641)); g.strokePath(track,juce::PathStrokeType(1));
        g.setColour(amber.withAlpha(slider.isEnabled() ? 0.3f+0.6f*value : 0.2f)); g.strokePath(lit,juce::PathStrokeType(1.5f));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff3c3e3b),centre.x,body.getY(),juce::Colour(0xff101312),centre.x,body.getBottom(),false));
        g.fillEllipse(body);
        for(int i=0;i<48;++i) {
            const float angle=juce::MathConstants<float>::twoPi*i/48.0f;
            const auto unit=juce::Point<float>(std::sin(angle),-std::cos(angle));
            g.setColour(i%2 ? juce::Colour(0xff101211) : juce::Colour(0xff565850));
            g.drawLine({centre+unit*(radius-3),centre+unit*(radius-0.8f)},0.7f);
        }
        g.setColour(juce::Colour(0xff111513)); g.fillEllipse(body.reduced(5));
        g.setColour(juce::Colour(0xff454941)); g.drawEllipse(body.reduced(5),0.7f);
        const float angle=begin+value*(end-begin);
        const auto unit=juce::Point<float>(std::sin(angle),-std::cos(angle));
        g.setColour(text); g.drawLine({centre+unit*(radius*0.48f),centre+unit*(radius*0.78f)},2.0f);
        g.setColour(amber.withAlpha(0.5f+value*0.5f)); g.fillEllipse(juce::Rectangle<float>(3,3).withCentre(centre+unit*(radius+5)));
    }
};
