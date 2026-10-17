#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
// Shared SENSOR-family panel and typography tokens, taken from FLUX's editor.
namespace SensorStyle {
inline const juce::Colour panelTop{0xff1a1b1d},panelBottom{0xff0c0d0f},rule{0xff353638};
inline const juce::Colour ink{0xffe0e1e3},secondary{0xff8e9094},headingInk{0xffd4d5d7};
inline void panel(juce::Graphics& g,int width,int height) {
    g.setGradientFill(juce::ColourGradient(panelTop,0,0,panelBottom,0,static_cast<float>(height),false));
    g.fillRect(0,0,width,height);
}
inline void heading(juce::Graphics& g,const juce::String& title,int x,int y,int width) {
    g.setFont(juce::Font(juce::FontOptions(12.0f)).withExtraKerningFactor(0.16f));
    g.setColour(headingInk);g.drawText(title,x,y,width,24,juce::Justification::centredLeft);
}
inline void branding(juce::Graphics& g,const juce::String& model,const juce::String& subtitle) {
    g.setColour(ink);g.setFont(juce::Font(juce::FontOptions(29.0f)).withExtraKerningFactor(0.2f));
    g.drawText("SENSOR "+model,28,14,650,38,juce::Justification::centredLeft);
    g.setColour(secondary);g.setFont(juce::Font(juce::FontOptions(10.0f)).withExtraKerningFactor(0.16f));
    g.drawText(subtitle,30,56,650,18,juce::Justification::centredLeft);
}
}
