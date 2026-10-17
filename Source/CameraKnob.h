#pragma once
#include "CustomLookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>
class CameraKnob : public juce::Component {
public:
    CameraKnob(juce::AudioProcessorValueTreeState& state,const juce::String& id,const juce::String& title) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,104,21);
        slider.setName(title); slider.setTitle(title);
        if(id.endsWith("Shape")) slider.setTooltip("Reshape the waveform. Centre is neutral; Square adjusts pulse width.");
        if(id.endsWith("Harmonics")) slider.setTooltip("Choose a musical harmonic relationship for the coloration. Try 1, 2 or 3 first.");
        if(id.endsWith("Character")) slider.setTooltip("Add harmonic richness. Lower settings stay gentle; high settings become experimental.");
        if(id.endsWith("Punch")) slider.setTooltip("Add a short bright attack. Useful for plucks and bass definition.");
        if(id.endsWith("Fine")) slider.setTooltip("Fine tuning in cents. Detune the oscillators slightly for a wider sound.");
        label.setText(title,juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(11.0f)); label.setColour(juce::Label::textColourId,CustomLookAndFeel::muted);
        addAndMakeVisible(slider); addAndMakeVisible(label);
        attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(state,id,slider);
        slider.textFromValueFunction=[id](double v) {
            if(id.endsWith("Harmonics")) return juce::StringArray{"0.5","1","1.5","2","3","4","5","6","8"}[juce::jlimit(0,8,juce::roundToInt(v))];
            if(id=="cutoff") return v<1000 ? juce::String(juce::roundToInt(v))+" Hz" : juce::String(v/1000,1)+" kHz";
            if(id.endsWith("Fine")) return (v>0 ? "+" : "")+juce::String(juce::roundToInt(v))+" ct";
            if(id.endsWith("Attack") || id.endsWith("Decay") || id.endsWith("Release")) return v<1 ? juce::String(juce::roundToInt(v*1000))+" ms" : juce::String(v,1)+" s";
            if(id=="resonance") return juce::String(v,2)+" Q";
            if(id=="filterAmount") return (v>0 ? "+" : "")+juce::String(v,1)+" oct";
            return juce::String(juce::roundToInt(v*100))+"%";
        };
        slider.valueFromTextFunction=[id](const juce::String& text) {
            auto number=text.retainCharacters("0123456789.-+").getDoubleValue();
            if(id.endsWith("Harmonics")) {
                const double ratios[]{0.5,1,1.5,2,3,4,5,6,8}; int best=0;
                for(int i=1;i<9;++i) if(std::abs(number-ratios[i])<std::abs(number-ratios[best])) best=i;
                return static_cast<double>(best);
            }
            if(id=="cutoff") return text.containsIgnoreCase("k") ? number*1000 : number;
            if(id.endsWith("Attack") || id.endsWith("Decay") || id.endsWith("Release")) return text.containsIgnoreCase("ms") ? number/1000 : number;
            if(id.endsWith("Fine") || id=="resonance" || id=="filterAmount") return number;
            return number/100;
        };
        slider.updateText();
    }
    void resized() override { auto r=getLocalBounds(); label.setBounds(r.removeFromTop(19)); slider.setBounds(r); }
private:
    juce::Label label;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};
