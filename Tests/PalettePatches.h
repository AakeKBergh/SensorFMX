#pragma once
#include <juce_audio_formats/juce_audio_formats.h>
// Small, reproducible acceptance recipes; not an additional preset UI.
struct PalettePatch {
    const char* name;
    int note;
    bool chord;
    std::vector<std::pair<const char*,float>> values;
};
inline std::vector<PalettePatch> palettePatches() {
    return {
        {"Airy Pluck",72,false,{{"aWave",1},{"bWave",2},{"aCharacter",0.12f},{"bCharacter",0.06f},{"aPunch",0.35f},{"width",0.3f},{"air",0.4f},{"chorus",1},{"filterSlope",0},{"cutoff",4200},{"filterAmount",2},{"filterDecay",0.16f},{"filterSustain",0},{"ampAttack",0.004f},{"ampDecay",0.25f},{"ampSustain",0.08f},{"ampRelease",0.2f}}},
        {"Soft Pad",60,true,{{"aLevel",0.5f},{"bLevel",0.35f},{"aCharacter",0.08f},{"bCharacter",0.04f},{"aPunch",0},{"width",0.85f},{"air",0.08f},{"chorus",2},{"filterSlope",0},{"cutoff",6500},{"filterAmount",0.7f},{"filterAttack",0.6f},{"ampAttack",0.7f},{"ampDecay",1},{"ampSustain",0.8f},{"ampRelease",2},{"velocity",0.35f}}},
        {"Wide Lead",72,false,{{"bWave",3},{"bShape",0.46f},{"bLevel",0.16f},{"aCharacter",0.38f},{"bCharacter",0.15f},{"aPunch",0.15f},{"width",0.55f},{"air",0.08f},{"chorus",1},{"filterSlope",1},{"cutoff",6000},{"filterAmount",1.4f},{"filterDecay",0.35f},{"filterSustain",0.5f},{"ampAttack",0.008f},{"ampDecay",0.2f},{"ampSustain",0.8f},{"ampRelease",0.25f}}},
        {"Bass",36,false,{{"bWave",3},{"aLevel",0.75f},{"bLevel",0.25f},{"aCharacter",0.35f},{"aPunch",0.4f},{"width",0},{"air",0},{"chorus",0},{"filterSlope",1},{"cutoff",700},{"filterAmount",3},{"resonance",1},{"filterAttack",0.003f},{"filterDecay",0.2f},{"filterSustain",0},{"ampAttack",0.004f},{"ampDecay",0.18f},{"ampSustain",0.4f},{"ampRelease",0.12f}}}
    };
}
inline int patchTests() {
    int checks=0;
    for(const auto& patch : palettePatches()) {
        SensorFMXProcessor p; for(const auto& setting:patch.values) set(p,setting.first,setting.second);
        p.prepareToPlay(48000,256);
        juce::AudioBuffer<float> captured(2,384000); captured.clear();
        juce::AudioBuffer<float> block(2,256); juce::MidiBuffer events;
        const int offsets[]{0,4,7,11};
        for(int i=0;i<(patch.chord ? 4 : 1);++i) events.addEvent(juce::MidiMessage::noteOn(1,patch.note+offsets[i],0.85f),0);
        double sum=0,side=0,tail=0,onset=0,sustain=0,peak=0;
        for(int pos=0;pos<captured.getNumSamples();pos+=256) {
            if(pos==144128) for(int i=0;i<(patch.chord ? 4 : 1);++i) events.addEvent(juce::MidiMessage::noteOff(1,patch.note+offsets[i]),0);
            p.processBlock(block,events);
            for(int c=0;c<2;++c) captured.copyFrom(c,pos,block,c,0,256);
            for(int i=0;i<256;++i) {
                const float l=block.getSample(0,i),r=block.getSample(1,i);
                require(std::isfinite(l) && std::isfinite(r),"Invalid musical patch render");
                const double e=l*l+r*r; sum+=e; side+=(l-r)*(l-r); peak=juce::jmax(peak,static_cast<double>(std::abs(l)),static_cast<double>(std::abs(r)));
                if(pos<4800) onset+=e; if(pos>=48000 && pos<52800) sustain+=e;
                if(pos>=336000) tail+=e;
            }
        }
        require(sum>1 && peak<1,"Musical patch silent or clipping");
        require(tail<1e-10,"Musical patch has a stuck tail");
        if(patch.chord) require(sustain>onset*3,"Pad does not swell");
        if(juce::String(patch.name)=="Airy Pluck") require(onset>sustain*2,"Pluck is not transient-shaped");
        if(juce::String(patch.name)=="Bass") require(side==0,"Bass recipe is not centred");
        else require(side/sum>0.001,"Wide patch recipe has no stereo image");
        if(const auto* folder=std::getenv("SENSOR_FMX_DEMOS")) {
            const auto directory=juce::File(juce::String::fromUTF8(folder));
            require(directory.createDirectory().wasOk(),"Demo directory failed");
            auto stream=directory.getChildFile(juce::String(patch.name)+".wav").createOutputStream();
            require(stream && stream->openedOk(),"Demo file failed"); stream->setPosition(0); stream->truncate();
            juce::WavAudioFormat format;
            std::unique_ptr<juce::AudioFormatWriter> writer(format.createWriterFor(stream.release(),48000,2,16,{},0));
            require(writer && writer->writeFromAudioSampleBuffer(captured,0,captured.getNumSamples()),"Demo rendering failed");
        }
        std::cout << "PASS: " << patch.name << " recipe; peak=" << peak << ", stereo ratio=" << side/sum << '\n'; ++checks;
    }
    return checks;
}
