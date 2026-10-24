// Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once
#include "GuideVoice.h"
#include "FamilySoundProfiles.h"
#include <juce_dsp/juce_dsp.h>

// Explicit Arranger presets only. The legacy synth remains unchanged for old
// sessions. MIDI channels are the existing visible routing: keys 1, bass 2,
// strings 3. No MIDI velocities are rewritten to obtain the audio balance.
class FamilyGuideRenderer {
    struct Part {
        std::atomic<float> tone{.5f},sound{0};
        juce::Synthesiser synth;
        juce::AudioBuffer<float> audio;
        juce::MidiBuffer midi;
        juce::Reverb reverb;
        juce::dsp::Chorus<float> chorus;
        juce::dsp::StateVariableTPTFilter<float> filter;
        float reverbWet=-1.f;    // the wet level last given to the reverb (-1: none yet)
        int chorusRampLeft=0;    // samples until the chorus's mix ramp after prepare() has ended
        explicit Part(int kind){
            for(int i=0;i<48;++i)synth.addVoice(new GuideVoice(tone,sound,kind==1));
            if(kind==2)synth.addSound(new GuideStringsTag(3));else synth.addSound(new GuideSoundTag);
            midi.ensureSize(262144);
        }
        void prepare(double rate,int capacity){
            synth.setCurrentPlaybackSampleRate(rate);audio.setSize(2,capacity);
            reverb.setSampleRate(rate);chorus.prepare({rate,(juce::uint32)capacity,2});
            chorus.setRate(.35f);chorus.setDepth(.25f);
            chorusRampLeft=(int)std::ceil(rate*.05)+1;reverbWet=-1.f;   // the mixer's 50 ms smoothing
            filter.prepare({rate,(juce::uint32)capacity,2});filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            reset();
        }
        void reset(){synth.allNotesOff(0,false);reverb.reset();chorus.reset();filter.reset();midi.clear();}
    };
    Part keys{0},bass{1},strings{2};
    int capacity=0;
public:
    void prepare(double rate,int block){capacity=juce::jmax(8192,2*block);for(auto* p:{&keys,&bass,&strings})p->prepare(rate,capacity);}
    void reset(){for(auto* p:{&keys,&bass,&strings})p->reset();}
    void render(juce::AudioBuffer<float>& output,const juce::MidiBuffer& input,int genre,
                float keySound,float keyTone,float keyReverb,float keyChorus){
        const auto& profile=fgfamily::sound(genre);
        keys.sound=keySound;keys.tone=keyTone;bass.sound=(float)profile.bass;bass.tone=(float)profile.bassTone;
        strings.sound=0;strings.tone=genre==8?.56f:.25f;
        int offset=0;
        // A host may deliver a larger block than announced. Process it in
        // preallocated chunks; there is no allocation or dropped audio here.
        while(offset<output.getNumSamples()){
            const int count=juce::jmin(capacity,output.getNumSamples()-offset);
            for(auto* p:{&keys,&bass,&strings}){p->midi.clear();p->audio.clear();}
            for(const auto meta:input){
                if(meta.samplePosition<offset||meta.samplePosition>=offset+count)continue;
                const auto m=meta.getMessage();
                if(m.isAllNotesOff()||m.isAllSoundOff())for(auto* p:{&keys,&bass,&strings})p->midi.addEvent(m,meta.samplePosition-offset);
                else {auto& p=m.getChannel()==2?bass:m.getChannel()==3?strings:keys;p.midi.addEvent(m,meta.samplePosition-offset);}
            }
            int kind=0;
            for(auto* p:{&keys,&bass,&strings}){
                p->synth.renderNextBlock(p->audio,p->midi,0,count);
                if(kind==1){
                    p->filter.setCutoffFrequency((float)(100+1400*profile.bassTone));
                    juce::dsp::AudioBlock<float> b(p->audio);auto block=b.getSubBlock(0,(size_t)count);
                    p->filter.process(juce::dsp::ProcessContextReplacing<float>(block));
                }else{
                    const float chorus=kind==0?keyChorus:0.f;
                    // The strings' chorus is always at mix 0: once the ramp from its default mix has
                    // ended it returns its input unchanged, so it is skipped.
                    if(kind==0||p->chorusRampLeft>0){
                        if(kind!=0)p->chorusRampLeft-=count;
                        p->chorus.setMix(chorus*.5f);juce::dsp::AudioBlock<float> b(p->audio);auto block=b.getSubBlock(0,(size_t)count);p->chorus.process(juce::dsp::ProcessContextReplacing<float>(block));
                    }
                    const float wet=(kind==0?keyReverb:.18f)*.4f;
                    if(! juce::exactlyEqual(wet,p->reverbWet)){   // the other parameters never change
                        juce::Reverb::Parameters rp;rp.roomSize=.4f;rp.wetLevel=wet;rp.dryLevel=.5f;rp.width=1;
                        p->reverb.setParameters(rp);p->reverbWet=wet;
                    }
                    p->reverb.processStereo(p->audio.getWritePointer(0),p->audio.getWritePointer(1),count);
                }
                const float gain=(float)(kind==0?profile.pianoGain:kind==1?profile.bassGain*.5:profile.stringsGain);
                for(int ch=0;ch<output.getNumChannels();++ch){
                    if(output.getNumChannels()==1){output.addFrom(0,offset,p->audio,0,0,count,gain*.5f);output.addFrom(0,offset,p->audio,1,0,count,gain*.5f);}
                    else output.addFrom(ch,offset,p->audio,juce::jmin(1,ch),0,count,gain);
                }
                ++kind;
            }
            offset+=count;
        }
    }
};
