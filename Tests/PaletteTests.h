#pragma once
#include <juce_dsp/juce_dsp.h>

inline double bandEnergy(const std::vector<float>& samples,double low,double high) {
    juce::dsp::FFT fft(12); std::array<float,8192> data{};
    for(size_t i=0;i<4096;++i) data[i]=samples[i]*static_cast<float>(0.5-0.5*std::cos(juce::MathConstants<double>::twoPi*i/4095));
    fft.performFrequencyOnlyForwardTransform(data.data()); double sum=0;
    for(int i=1;i<2048;++i) if(i*48000.0/4096>=low && i*48000.0/4096<high) sum+=data[static_cast<size_t>(i)]*data[static_cast<size_t>(i)];
    return sum;
}
inline double energy(const std::vector<float>& samples) { double sum=0; for(auto x:samples) sum+=x*x; return sum/samples.size(); }
inline int paletteTests() {
    int checks=0;
    HybridOscillator single; single.prepare(48000); single.set(2,0.5f,3,0.2f,0.1f,0,1,true); single.noteOn(375);
    UnisonLayer mono; mono.prepare(48000); mono.set(2,0.5f,3,0.2f,0.1f,0,1,0,true); mono.noteOn(375);
    for(int i=0;i<8192;++i) { const auto expected=single.next(); const auto frame=mono.next(); require(frame[0]==frame[1] && std::abs(frame[0]-expected)<1e-7f,"Width zero does not match single oscillator"); } ++checks;
    auto spread=[](float width) {
        UnisonLayer osc; osc.prepare(48000); osc.set(2,0.5f,3,0.15f,0,0,1,width,true); osc.noteOn(220);
        double side=0,total=0,folded=0;
        for(int i=0;i<96000;++i) {
            const auto s=osc.next(); if(i<24000) continue;
            side+=(s[0]-s[1])*(s[0]-s[1]); total+=s[0]*s[0]+s[1]*s[1]; folded+=0.25*(s[0]+s[1])*(s[0]+s[1]);
        }
        return std::pair<double,double>{side/total,folded/(0.5*total)};
    };
    const auto narrow=spread(0.2f),wide=spread(0.85f);
    require(narrow.first>0.0001 && wide.first>narrow.first*3,"Width does not progressively widen output");
    require(wide.second>0.4,"Unison collapses when folded to mono"); ++checks;
    AirGenerator air; air.prepare(48000); air.set(1,true); air.noteOn(60);
    std::vector<float> breath(4096),tail(4096);
    for(auto& x:breath) x=air.next();
    for(int i=0;i<96000;++i) air.next(); for(auto& x:tail) x=air.next();
    require(bandEnergy(breath,6000,12000)/6000>bandEnergy(breath,100,1000)/900*4,"Air not concentrated in upper frequencies");
    require(energy(tail)<energy(breath)*0.0001,"Air does not decay after note onset"); ++checks;
    air.set(0,true); air.noteOn(60); for(int i=0;i<256;++i) require(air.next()==0,"Air zero is not silent"); ++checks;
    auto chorusMeasure=[](int mode) {
        SimpleChorus fx; fx.prepare(48000); fx.setMode(mode); double side=0,difference=0;
        for(int i=0;i<432000;++i) {
            const float input=static_cast<float>(0.6*std::sin(juce::MathConstants<double>::twoPi*375*i/48000));
            const auto s=fx.process({input,input});
            require(std::isfinite(s[0]) && std::isfinite(s[1]) && std::abs(s[0])<1,"Unstable chorus");
            if(mode==0) require(s[0]==input && s[1]==input,"Chorus bypass not transparent");
            if(i<48000) continue;
            side+=(s[0]-s[1])*(s[0]-s[1]); difference+=(s[0]-input)*(s[0]-input)+(s[1]-input)*(s[1]-input);
        }
        return std::pair<double,double>{side,difference};
    };
    chorusMeasure(0); const auto modeI=chorusMeasure(1),modeII=chorusMeasure(2);
    require(modeI.first>10 && modeI.second>10,"Chorus I does not affect stereo output");
    require(modeII.first>modeI.first*1.2 && modeII.second>modeI.second*1.2,"Chorus II is not stronger than I"); ++checks;
    auto response=[](bool steep,double hz) {
        VoiceFilter filter; filter.prepare(48000,256); filter.setSlope(steep,true); filter.set(500,0.70710678f);
        double sum=0;
        for(int i=0;i<16384;++i) {
            const float x=static_cast<float>(std::sin(juce::MathConstants<double>::twoPi*hz*i/48000));
            auto out=filter.process({x,x}); if(i>=8192) sum+=out[0]*out[0];
        }
        return sum/8192;
    };
    const double lp12a=response(false,2000),lp12b=response(false,4000),lp24a=response(true,2000),lp24b=response(true,4000);
    const double drop12=10*std::log10(lp12a/lp12b),drop24=10*std::log10(lp24a/lp24b);
    require(drop12>10 && drop12<15 && drop24>21 && drop24<30 && lp24b<lp12b*0.03,"LP12/LP24 slopes are incorrect"); ++checks;
    auto dynamics=[](float velocity,float sensitivity) {
        SensorFMXProcessor p; set(p,"width",0); set(p,"air",0); set(p,"velocity",sensitivity); set(p,"filterAmount",0);
        p.prepareToPlay(48000,256); juce::MidiBuffer midi; midi.addEvent(juce::MidiMessage::noteOn(1,60,velocity),0);
        render(p,midi,60); return render(p,midi,16);
    };
    const auto flatSoft=dynamics(0.25f,0),flatHard=dynamics(1,0),sensitiveSoft=dynamics(0.25f,1),sensitiveHard=dynamics(1,1);
    require(std::abs(flatSoft-flatHard)<flatHard*1e-5,"Velocity zero still responds to note velocity");
    require(sensitiveHard>sensitiveSoft*12,"Velocity does not change level"); ++checks;
    // Sustained automation must stay bounded across all new modes and sample rates.
    for(double rate : {44100.0,48000.0,96000.0}) {
        SensorFMXProcessor p; p.prepareToPlay(rate,256); juce::MidiBuffer midi;
        for(int n=48;n<56;++n) midi.addEvent(juce::MidiMessage::noteOn(1,n,1.0f),0);
        for(int i=0;i<80;++i) {
            set(p,"width",i%2); set(p,"air",(i%5)/4.0f); set(p,"chorus",i%3); set(p,"filterSlope",i%2);
            set(p,"velocity",i%2); set(p,"resonance",8); set(p,"aCharacter",i%2); set(p,"bCharacter",i%2);
            render(p,midi,1);
        }
    } ++checks;
    // Mono layout must remain finite with the stereo effects active.
    SensorFMXProcessor monoHost;
    auto layout=monoHost.getBusesLayout(); layout.outputBuses.set(0,juce::AudioChannelSet::mono());
    require(monoHost.setBusesLayout(layout),"Mono layout rejected");
    set(monoHost,"width",1); set(monoHost,"chorus",2); monoHost.prepareToPlay(48000,256);
    juce::AudioBuffer<float> oneChannel(1,256); juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1,60,1.0f),0);
    double total=0; for(int block=0;block<100;++block) {
        monoHost.processBlock(oneChannel,midi);
        for(int i=0;i<256;++i) { const auto x=oneChannel.getSample(0,i); require(std::isfinite(x),"Invalid mono fold-down"); total+=x*x; }
    }
    require(total>0.1,"Mono fold-down lost the sound"); ++checks;
    std::cout << "PASS: Width stereo/mono, Air spectrum/decay, chorus modes, LP12/24, velocity and extremes\n";
    return checks;
}
