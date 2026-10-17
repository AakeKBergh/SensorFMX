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

std::vector<float> oscillator(int wave,float shape,float character,float punch,int harmonic=3,int skip=0) {
    HybridOscillator osc; osc.prepare(48000);
    osc.set(wave,shape,harmonic,character,punch,0,1,true); osc.noteOn(375);
    for(int i=0;i<skip;++i) osc.next();
    std::vector<float> result(4096);
    for(auto& value : result) { value=osc.next(); require(std::isfinite(value),"Invalid oscillator sample"); }
    return result;
}
double harmonicEnergy(const std::vector<float>& samples,int harmonic) {
    double re=0,im=0;
    for(size_t i=0;i<samples.size();++i) {
        const double phase=juce::MathConstants<double>::twoPi*375*harmonic*static_cast<double>(i)/48000;
        re+=samples[i]*std::cos(phase); im+=samples[i]*std::sin(phase);
    }
    return (re*re+im*im)/(samples.size()*samples.size());
}
double upperEnergy(const std::vector<float>& samples) {
    double sum=0; for(int h=2;h<=20;++h) sum+=harmonicEnergy(samples,h); return sum;
}
int oscillatorTests() {
    int checks=0;
    const float expected[]{0.5f,1,1.5f,2,3,4,5,6,8};
    for(int i=0;i<9;++i) require(HybridOscillator::ratioFor(i)==expected[i],"Harmonic ratio mapping failed"); ++checks;
    std::array<std::vector<float>,4> waves;
    for(int w=0;w<4;++w) {
        waves[static_cast<size_t>(w)]=oscillator(w,0.5f,0,0);
        double error=0;
        for(size_t i=0;i<waves[static_cast<size_t>(w)].size();++i) {
            const double t=std::fmod(static_cast<double>(i)/128,1.0);
            double ideal=0;
            if(w==0) ideal=std::sin(juce::MathConstants<double>::twoPi*t);
            if(w==1) ideal=t<0.5 ? -1+4*t : 3-4*t;
            if(w==2) ideal=2*t-1;
            if(w==3) ideal=t<0.5 ? 1 : -1;
            const double delta=waves[static_cast<size_t>(w)][i]-ideal; error+=delta*delta;
        }
        // BLEP deliberately rounds the discontinuity over one sample at either edge.
        require(error/waves[static_cast<size_t>(w)].size()<0.02,"Zero Character not a conventional waveform"); ++checks;
    }
    for(int a=0;a<4;++a) for(int b=a+1;b<4;++b) {
        double difference=0; for(size_t i=0;i<waves[0].size();++i) difference+=std::abs(waves[static_cast<size_t>(a)][i]-waves[static_cast<size_t>(b)][i]);
        require(difference/waves[0].size()>0.15,"Waveforms not distinct");
    } ++checks;
    require(upperEnergy(waves[0])<1e-10,"Uncoloured sine has FM harmonics");
    require(harmonicEnergy(waves[2],2)>0.01 && harmonicEnergy(waves[3],3)>0.005,"Saw/square harmonic structure missing");
    require(upperEnergy(waves[1])<upperEnergy(waves[2])*0.2,"Triangle is not soft"); ++checks;
    const auto gentle=oscillator(0,0.5f,0.25f,0), rich=oscillator(0,0.5f,0.65f,0);
    require(upperEnergy(gentle)>1e-6 && upperEnergy(rich)>upperEnergy(gentle)*10,"Character does not progressively add harmonics"); ++checks;
    require(HybridOscillator::characterDepth(0.5f)==0.25f && HybridOscillator::characterDepth(1)==4,"Character mapping not restrained"); ++checks;
    require(HybridOscillator::characterDepth(0.2f)<0.01f && HybridOscillator::characterDepth(0.65f)<0.75f,"Character reaches strong FM too early"); ++checks;
    const auto attack=oscillator(0,0.5f,0,0.8f), tail=oscillator(0,0.5f,0,0.8f,3,96000);
    require(upperEnergy(attack)>0.001 && upperEnergy(tail)<upperEnergy(attack)*0.001,"Punch is not transient harmonic coloration"); ++checks;
    for(int w=0;w<4;++w) {
        const auto shaped=oscillator(w,0.9f,0,0); double delta=0;
        for(size_t i=0;i<shaped.size();++i) delta+=std::abs(shaped[i]-waves[static_cast<size_t>(w)][i]);
        require(delta/shaped.size()>0.025,"Shape did not affect waveform");
    } ++checks;
    // Exercise every wave/ratio at extreme tuning and changing macros, at all rates.
    HybridOscillator automated;
    for(double rate : {44100.0,48000.0,96000.0}) {
        automated.prepare(rate); automated.noteOn(8000);
        for(int k=0;k<256;++k) {
            automated.set(k%4,(k%11)/10.0f,k%9,(k%7)/6.0f,(k%5)/4.0f,k%2 ? 100 : -100,1);
            for(int n=0;n<128;++n) { auto sample=automated.next(); require(std::isfinite(sample) && std::abs(sample)<2,"Invalid automated oscillator output"); }
        }
    } ++checks;
    std::cout << "PASS: carrier references, harmonic spectra, Character, Punch, Shape, ratios and automation\n";
    return checks;
}

#include "PaletteTests.h"
#include "PalettePatches.h"

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
            set(p,"cutoff",19000); set(p,"resonance",8); set(p,"aCharacter",1); set(p,"aPunch",1); set(p,"filterAmount",8);
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
        SensorFMXProcessor pitch; set(pitch,"width",0); set(pitch,"air",0); set(pitch,"bLevel",0); set(pitch,"aWave",0); set(pitch,"aShape",0.5f); set(pitch,"aCharacter",0); set(pitch,"aPunch",0); set(pitch,"filterAmount",0); set(pitch,"cutoff",18000);
        pitch.prepareToPlay(48000,256); juce::MidiBuffer pm;
        pm.addEvent(juce::MidiMessage::noteOn(1,69,1.0f),0); render(pitch,pm,50);
        juce::AudioBuffer<float> wave(2,48000); pitch.processBlock(wave,pm);
        int crossings=0; for(int n=1;n<48000;++n) if(wave.getSample(0,n-1)<=0 && wave.getSample(0,n)>0) ++crossings;
        require(std::abs(crossings-440)<=1,"MIDI pitch is not A440"); ++checks;
        pm.addEvent(juce::MidiMessage::pitchWheel(1,16383),0); render(pitch,pm,4);
        pitch.processBlock(wave,pm); crossings=0;
        for(int n=1;n<48000;++n) if(wave.getSample(0,n-1)<=0 && wave.getSample(0,n)>0) ++crossings;
        require(std::abs(crossings-494)<=1,"Pitch bend no longer works"); ++checks;
        // Filter removes upper content; its own ADSR can open a closed cutoff.
        auto filteredEnergy=[](float cutoff,float amount,float attack) {
            SensorFMXProcessor instrument; set(instrument,"width",0); set(instrument,"air",0); set(instrument,"velocity",0);
            set(instrument,"aCharacter",0); set(instrument,"aPunch",0); set(instrument,"bLevel",0);
            set(instrument,"cutoff",cutoff); set(instrument,"filterAmount",amount);
            set(instrument,"filterSustain",1); set(instrument,"ampAttack",attack);
            instrument.prepareToPlay(48000,256); juce::MidiBuffer events;
            events.addEvent(juce::MidiMessage::noteOn(1,72,1.0f),0);
            const float onset=render(instrument,events,1);
            render(instrument,events,40);
            return std::pair<float,float>{onset,render(instrument,events,8)};
        };
        const auto dark=filteredEnergy(120,0,0.01f), bright=filteredEnergy(12000,0,0.01f), opened=filteredEnergy(120,7,0.01f);
        require(bright.second>dark.second*20,"Lowpass cutoff not filtering");
        require(opened.second>dark.second*20,"Filter envelope not opening cutoff"); ++checks;
        const auto slow=filteredEnergy(12000,0,0.3f);
        require(slow.first<bright.first*0.1f && slow.second>slow.first*5,"Amp attack not shaping onset"); ++checks;
        SensorFMXProcessor automated; automated.prepareToPlay(48000,256); juce::MidiBuffer automationMidi;
        automationMidi.addEvent(juce::MidiMessage::noteOn(1,60,1.0f),0);
        render(automated,automationMidi,4);
        for(int n=0;n<80;++n) {
            for(int i=0;i<automated.getParameters().size();++i)
                automated.getParameters()[i]->setValueNotifyingHost(static_cast<float>((n*17+i*7)%101)/100);
            render(automated,automationMidi,1);
        } ++checks;
        checks += oscillatorTests();
        checks += paletteTests();
        checks += patchTests();
        SensorFMXProcessor p;
        set(p,"aCharacter",0.37f); set(p,"filterRelease",1.8f); set(p,"master",0.42f);
        juce::MemoryBlock saved; p.getStateInformation(saved);
        SensorFMXProcessor restored; restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
        for(auto id : {"aCharacter","filterRelease","master"})
            require(std::abs(p.state.getRawParameterValue(id)->load()-restored.state.getRawParameterValue(id)->load())<1e-5f,"State round trip failed");
        ++checks;
        require(p.getParameters().size()==31,"Unexpected parameter count"); ++checks;
        // Every parameter, including discrete selectors, must round-trip.
        for(auto* param : p.getParameters()) {
            auto* ranged=dynamic_cast<juce::RangedAudioParameter*>(param);
            require(ranged!=nullptr,"Non-ranged parameter");
            ranged->setValueNotifyingHost(0.63f);
        }
        p.getStateInformation(saved); restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
        for(int i=0;i<p.getParameters().size();++i)
            require(std::abs(p.getParameters()[i]->getValue()-restored.getParameters()[i]->getValue())<1e-5f,"Parameter round trip failed");
        ++checks;
        // Legacy state: preserve common IDs, snap old ratios and provide missing defaults.
        juce::ValueTree old("SensorFMX");
        auto legacy=[&](const char* id,float value) { juce::ValueTree node("PARAM"); node.setProperty("id",id,nullptr); node.setProperty("value",value,nullptr); old.appendChild(node,nullptr); };
        legacy("aRatio",2.7f); legacy("aFM",0.5f); legacy("aFine",12); legacy("cutoff",1234);
        auto xml=old.createXml(); juce::AudioProcessor::copyXmlToBinary(*xml,saved);
        restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
        require(restored.state.getRawParameterValue("aHarmonics")->load()==4,"Legacy ratio not mapped to 3");
        require(restored.state.getRawParameterValue("aWave")->load()==0,"Legacy sine not restored");
        require(std::abs(restored.state.getRawParameterValue("aCharacter")->load()-std::pow(0.125f,0.25f))<1e-5f,"Legacy depth mapping failed");
        require(restored.state.getRawParameterValue("aFine")->load()==12 && restored.state.getRawParameterValue("cutoff")->load()==1234,"Legacy common parameters lost");
        require(restored.state.getRawParameterValue("bWave")->load()==1,"Missing parameters did not default"); ++checks;
        juce::ValueTree v2("SensorFMX"); v2.setProperty("schemaVersion",2,nullptr);
        juce::ValueTree character("PARAM"); character.setProperty("id","aCharacter",nullptr); character.setProperty("value",0.5f,nullptr); v2.appendChild(character,nullptr);
        xml=v2.createXml(); juce::AudioProcessor::copyXmlToBinary(*xml,saved); restored.setStateInformation(saved.getData(),static_cast<int>(saved.getSize()));
        require(std::abs(HybridOscillator::characterDepth(restored.state.getRawParameterValue("aCharacter")->load())-0.5f)<1e-5f,"V2 FM depth not preserved");
        require(restored.state.getRawParameterValue("width")->load()==0 && restored.state.getRawParameterValue("air")->load()==0 && restored.state.getRawParameterValue("velocity")->load()==1,"Legacy macro defaults incorrect"); ++checks;
        // Exercise all APVTS attachments and layout under an X display.
        SensorFMXProcessor ui;
        std::unique_ptr<juce::AudioProcessorEditor> editor(ui.createEditor());
        require(editor->getWidth()==1180,"Editor failed to initialise");
        int sliders=0,buttons=0;
        std::function<void(juce::Component&)> visit=[&](juce::Component& component) {
            if(auto* slider=dynamic_cast<juce::Slider*>(&component)) {
                ++sliders;
                const auto value=slider->getValue();
                const auto text=slider->getTextFromValue(value);
                require(std::abs(slider->getValueFromText(text)-value)<0.05,"Value units do not parse back");
            }
            if(auto* button=dynamic_cast<juce::TextButton*>(&component)) {
                ++buttons;
                if(button->getName()=="aWave SQUARE") { button->onClick(); require(ui.state.getRawParameterValue("aWave")->load()==3,"Wave button not attached"); }
            }
            for(auto* child : component.getChildren()) visit(*child);
        };
        visit(*editor); require(sliders==27 && buttons==12,"Missing visible controls");
        set(ui,"aWave",2);
        juce::TextButton *chorusI=nullptr,*chorusII=nullptr,*lp24=nullptr;
        std::function<void(juce::Component&)> findButtons=[&](juce::Component& c) {
            if(auto* b=dynamic_cast<juce::TextButton*>(&c)) {
                if(b->getName()=="chorus CHORUS I") chorusI=b;
                if(b->getName()=="chorus CHORUS II") chorusII=b;
                if(b->getName()=="filterSlope LP24") lp24=b;
            }
            for(auto* child : c.getChildren()) findButtons(*child);
        };
        findButtons(*editor); require(chorusI && chorusII && lp24,"Missing macro buttons");
        chorusI->onClick(); require(ui.state.getRawParameterValue("chorus")->load()==1 && chorusI->getToggleState(),"Chorus I attachment failed");
        chorusII->onClick(); require(ui.state.getRawParameterValue("chorus")->load()==2 && !chorusI->getToggleState(),"Chorus exclusivity failed");
        chorusII->onClick(); require(ui.state.getRawParameterValue("chorus")->load()==0 && !chorusII->getToggleState(),"Chorus repeated-press bypass failed");
        lp24->onClick(); require(ui.state.getRawParameterValue("filterSlope")->load()==1,"Slope button attachment failed");
        set(ui,"filterSlope",0); set(ui,"chorus",1); require(chorusI->getToggleState(),"Host chorus update not reflected in GUI");
        set(ui,"chorus",0); ++checks;
        juce::Image image(juce::Image::ARGB,1180,760,true);
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
