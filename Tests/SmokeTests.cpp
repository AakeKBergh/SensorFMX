#include "../Source/PluginProcessor.h"
#include <iostream>
#include <stdexcept>
void require(bool condition,const char* message) { if (!condition) throw std::runtime_error(message); }
void set(SensorFMXProcessor& p,const char* id,float value) {
    auto* param=p.state.getParameter(id);
    param->setValueNotifyingHost(param->convertTo0to1(value));
}
float render(SensorFMXProcessor& p,juce::MidiBuffer& midi,int blocks=1) {
    juce::AudioBuffer<float> audio(2,256);
    float energy=0;
    for(int i=0;i<blocks;++i) {
        p.processBlock(audio,midi);
        for(int c=0;c<2;++c) for(int n=0;n<256;++n) {
            const float s=audio.getSample(c,n);
            require(std::isfinite(s),"Non-finite output"); require(std::abs(s)<4,"Unstable output");
            energy+=s*s;
        }
    }
    return energy;
}
int main() {
    juce::ScopedJuceInitialiser_GUI init;
    try {
        int checks=0;
        for(double rate : {44100.0,48000.0,96000.0}) {
            SensorFMXProcessor p; p.prepareToPlay(rate,256);
            juce::MidiBuffer midi;
            require(render(p,midi,8)==0,"Idle synth is not silent"); ++checks;
            midi.addEvent(juce::MidiMessage::noteOn(1,60,0.8f),0);
            require(render(p,midi,20)>0.01f,"Note produced no audio"); ++checks;
            midi.addEvent(juce::MidiMessage::noteOff(1,60),0);
            render(p,midi,static_cast<int>(rate/256));
            require(render(p,midi,8)<1e-12f,"Release did not finish"); ++checks;
            // Eight notes including note-offs/sustain, then release all voices.
            for(int n=48;n<56;++n) midi.addEvent(juce::MidiMessage::noteOn(1,n,0.8f),0);
            require(render(p,midi,24)>0.01f,"Chord produced no audio"); ++checks;
            midi.addEvent(juce::MidiMessage::controllerEvent(1,64,127),0);
            for(int n=48;n<56;++n) midi.addEvent(juce::MidiMessage::noteOff(1,n),0);
            require(render(p,midi,24)>0.01f,"Sustain pedal failed"); ++checks;
            midi.addEvent(juce::MidiMessage::controllerEvent(1,64,0),0);
            render(p,midi,static_cast<int>(rate/256));
            require(render(p,midi,8)<1e-12f,"Pedal release stuck"); ++checks;
            set(p,"cutoff",19000); set(p,"resonance",8); set(p,"aFM",12); set(p,"filterAmount",8);
            for(int n=72;n<80;++n) midi.addEvent(juce::MidiMessage::noteOn(1,n,1.0f),n-72);
            require(render(p,midi,100)>0.01f,"Extreme settings silent"); ++checks;
            set(p,"master",0); render(p,midi,static_cast<int>(rate/256));
            require(render(p,midi,8)==0,"Master mute failed"); ++checks;
        }
        // The chord must equal eight independent voices mixed together.
        juce::AudioBuffer<float> sum(2,512), chord(2,512); sum.clear();
        for (int note=48;note<56;++note) {
            SensorFMXProcessor one; one.prepareToPlay(48000,512);
            juce::AudioBuffer<float> single(2,512); juce::MidiBuffer m;
            m.addEvent(juce::MidiMessage::noteOn(1,note,0.8f),0); one.processBlock(single,m);
            for(int c=0;c<2;++c) sum.addFrom(c,0,single,c,0,512);
        }
        SensorFMXProcessor eight; eight.prepareToPlay(48000,512); juce::MidiBuffer notes;
        for(int note=48;note<56;++note) notes.addEvent(juce::MidiMessage::noteOn(1,note,0.8f),0);
        eight.processBlock(chord,notes);
        for(int c=0;c<2;++c) for(int n=0;n<512;++n)
            require(std::abs(chord.getSample(c,n)-sum.getSample(c,n))<1e-5f,"Eight-voice mix mismatch");
        ++checks;
        SensorFMXProcessor pitch; set(pitch,"bLevel",0); set(pitch,"aFM",0); set(pitch,"filterAmount",0); set(pitch,"cutoff",18000);
        pitch.prepareToPlay(48000,256); juce::MidiBuffer pm;
        pm.addEvent(juce::MidiMessage::noteOn(1,69,1.0f),0); render(pitch,pm,50);
        juce::AudioBuffer<float> wave(2,48000); pitch.processBlock(wave,pm);
        int crossings=0; for(int n=1;n<48000;++n) if(wave.getSample(0,n-1)<=0 && wave.getSample(0,n)>0) ++crossings;
        require(std::abs(crossings-440)<=1,"MIDI pitch is not A440"); ++checks;
        SensorFMXProcessor p;
        set(p,"aRatio",3.7f); set(p,"filterRelease",1.8f); set(p,"master",0.42f);
        juce::MemoryBlock saved; p.getStateInformation(saved);
        SensorFMXProcessor restored; restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
        for(auto id : {"aRatio","filterRelease","master"})
            require(std::abs(p.state.getRawParameterValue(id)->load()-restored.state.getRawParameterValue(id)->load())<1e-5f,"State round trip failed");
        ++checks;
        require(p.getParameters().size()==22,"Unexpected parameter count"); ++checks;
        // Exercise all APVTS attachments and layout under an X display.
        std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditor());
        require(editor->getWidth()==1050,"Editor failed to initialise");
        juce::Image image(juce::Image::ARGB,1050,660,true);
        juce::Graphics graphics(image); editor->paintEntireComponent(graphics,true); ++checks;
        if (const auto* path=std::getenv("SENSOR_FMX_SCREENSHOT")) {
            juce::FileOutputStream stream{juce::File(juce::String::fromUTF8(path))};
            require(stream.openedOk(),"Screenshot open failed");
            stream.setPosition(0); stream.truncate();
            require(juce::PNGImageFormat().writeImageToStream(image,stream),"Screenshot write failed");
        }
        std::cout << "PASS: " << checks << " checks; 3 sample rates, audio, sustain, extremes, state and GUI\n";
    } catch(const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
