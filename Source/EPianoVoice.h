#pragma once

// A small FM electric piano (Rhodes-like): a sine carrier, a 1:1 modulator whose
// index decays for the "bark", and a high tine partial for the attack click.

#include <juce_audio_basics/juce_audio_basics.h>

#include <atomic>
#include <cmath>

struct EPianoSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

class EPianoVoice : public juce::SynthesiserVoice
{
public:
    explicit EPianoVoice (const std::atomic<float>& toneParam) : tone (toneParam) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<EPianoSound*> (s) != nullptr; }

    void startNote (int note, float velocity, juce::SynthesiserSound*, int) override
    {
        const double sr = getSampleRate();
        const double freq = juce::MidiMessage::getMidiNoteInHertz (note);

        carrierInc = freq / sr;
        tineInc = freq * 14.0 / sr;
        carrierPhase = modPhase = tinePhase = 0.0;

        vel = velocity;
        time = 0.0;
        dt = 1.0 / sr;
        releasing = false;
        releaseLevel = 1.0;
        releaseCoeff = std::exp (-1.0 / (0.09 * sr));

        // Lower notes ring longer, like a real tine piano.
        decayTime = juce::jlimit (0.7, 6.0, 3.2 * std::pow (2.0, -(note - 60) / 22.0));

        const float bright = 0.35f + 1.3f * tone.load();
        modDepth = bright * (0.6 + 2.4 * vel * vel);
        tineDepth = bright * 0.9 * vel;
        gain = 0.16 * (0.2 + 0.8 * std::pow (vel, 1.4));

        const double pan = juce::jlimit (-1.0, 1.0, (note - 62) / 40.0);
        gainL = gain * std::cos ((pan + 1.0) * juce::MathConstants<double>::pi * 0.25) * juce::MathConstants<double>::sqrt2;
        gainR = gain * std::sin ((pan + 1.0) * juce::MathConstants<double>::pi * 0.25) * juce::MathConstants<double>::sqrt2;
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff)
            releasing = true;
        else
            clearCurrentNote();
    }

    using SynthesiserVoice::renderNextBlock;

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& buffer, int start, int num) override
    {
        if (! isVoiceActive())
            return;

        constexpr double twoPi = juce::MathConstants<double>::twoPi;
        auto* left  = buffer.getWritePointer (0);
        auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

        for (int i = start; i < start + num; ++i)
        {
            const double attack = juce::jmin (1.0, time / 0.0015);
            const double env = attack * std::exp (-time / decayTime);

            const double index = modDepth * std::exp (-time / 0.28) + 0.18 * modDepth;
            const double tine = tineDepth * std::exp (-time / 0.018);

            const double mod = index * std::sin (twoPi * modPhase) + tine * std::sin (twoPi * tinePhase);
            const double sample = std::sin (twoPi * carrierPhase + mod) * env * releaseLevel;

            left[i] += (float) (sample * gainL);
            if (right != nullptr)
                right[i] += (float) (sample * gainR);

            carrierPhase += carrierInc;  if (carrierPhase >= 1.0) carrierPhase -= 1.0;
            modPhase     += carrierInc;  if (modPhase >= 1.0)     modPhase -= 1.0;
            tinePhase    += tineInc;     if (tinePhase >= 1.0)    tinePhase -= 1.0;
            time += dt;

            if (releasing)
                releaseLevel *= releaseCoeff;

            if (releaseLevel < 0.0005 || (env < 0.0005 && time > 0.01))
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    const std::atomic<float>& tone;

    double carrierPhase = 0.0, modPhase = 0.0, tinePhase = 0.0;
    double carrierInc = 0.0, tineInc = 0.0;
    double time = 0.0, dt = 0.0, decayTime = 1.0;
    double modDepth = 0.0, tineDepth = 0.0, gain = 0.0, gainL = 0.0, gainR = 0.0;
    double releaseLevel = 1.0, releaseCoeff = 0.999;
    float vel = 0.0f;
    bool releasing = false;
};
