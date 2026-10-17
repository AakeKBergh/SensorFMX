#pragma once
#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "CustomLookAndFeel.h"
class WaveSelector : public juce::Component {
public:
    WaveSelector(juce::AudioProcessorValueTreeState& state,const juce::String& id) {
        const char* names[]{"SINE","TRIANGLE","SAW","SQUARE"};
        for(int i=0;i<4;++i) {
            auto& b=buttons[static_cast<size_t>(i)]; b.setButtonText(names[i]); b.setName(id+" "+names[i]);
            b.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff242725));
            b.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff524635));
            b.setColour(juce::TextButton::textColourOffId,CustomLookAndFeel::muted);
            b.setColour(juce::TextButton::textColourOnId,CustomLookAndFeel::text);
            b.setConnectedEdges((i>0 ? juce::Button::ConnectedOnLeft : 0) | (i<3 ? juce::Button::ConnectedOnRight : 0));
            b.onClick=[this,i] { attachment->setValueAsCompleteGesture(static_cast<float>(i)); };
            addAndMakeVisible(b);
        }
        attachment=std::make_unique<juce::ParameterAttachment>(*state.getParameter(id),[this](float v) {
            for(int i=0;i<4;++i) buttons[static_cast<size_t>(i)].setToggleState(i==juce::roundToInt(v),juce::dontSendNotification);
        },nullptr);
        attachment->sendInitialUpdate();
    }
    void resized() override {
        auto r=getLocalBounds(); const int width=r.getWidth()/4;
        for(auto& button : buttons) button.setBounds(r.removeFromLeft(width));
    }
private:
    std::array<juce::TextButton,4> buttons;
    std::unique_ptr<juce::ParameterAttachment> attachment;
};
