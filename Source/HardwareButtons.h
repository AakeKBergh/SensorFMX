#pragma once
#include "CustomLookAndFeel.h"
#include <juce_audio_processors/juce_audio_processors.h>
class HardwareButtons : public juce::Component {
public:
    HardwareButtons(juce::AudioProcessorValueTreeState& state,const juce::String& id,
                    juce::StringArray titles,std::vector<int> choices,bool pressAgainOff)
        : values(std::move(choices)),toggleOff(pressAgainOff) {
        for(size_t i=0;i<values.size();++i) {
            auto button=std::make_unique<juce::TextButton>(titles[static_cast<int>(i)]);
            button->setName(id+" "+titles[static_cast<int>(i)]);
            button->setColour(juce::TextButton::buttonColourId,juce::Colour(0xff242725));
            button->setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff524635));
            button->setColour(juce::TextButton::textColourOffId,CustomLookAndFeel::muted);
            button->setColour(juce::TextButton::textColourOnId,CustomLookAndFeel::text);
            button->setTooltip(toggleOff ? "Press the active mode again to bypass chorus." : "Select low-pass filter slope.");
            button->onClick=[this,i] { attachment->setValueAsCompleteGesture(static_cast<float>(toggleOff && selected==values[i] ? 0 : values[i])); };
            addAndMakeVisible(*button); buttons.push_back(std::move(button));
        }
        attachment=std::make_unique<juce::ParameterAttachment>(*state.getParameter(id),[this](float v) {
            selected=juce::roundToInt(v);
            for(size_t i=0;i<buttons.size();++i) buttons[i]->setToggleState(values[i]==selected,juce::dontSendNotification);
        },nullptr);
        attachment->sendInitialUpdate();
    }
    void resized() override {
        auto r=getLocalBounds(); const auto width=r.getWidth()/static_cast<int>(buttons.size());
        for(auto& button : buttons) button->setBounds(r.removeFromLeft(width).reduced(3,0));
    }
private:
    std::vector<int> values;
    bool toggleOff;
    int selected=0;
    std::vector<std::unique_ptr<juce::TextButton>> buttons;
    std::unique_ptr<juce::ParameterAttachment> attachment;
};
