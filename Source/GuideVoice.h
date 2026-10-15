// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// The guide sound: four small synthesized keyboards (no samples), chosen per note.
//  - Rhodes:        FM electric piano (decaying 1:1 modulator "bark" + tine click)
//  - Soulful Grand: additive acoustic piano (inharmonic partials, two detuned strings,
//                   hammer noise, damper on release)
//  - House Piano:   bright, hollow "M1-style" piano (additive body + FM bite + shimmer)
//  - House Organ:   drawbar organ with percussion, key click and vibrato
//  - Wurlitzer:     reedy FM electric piano with a little drive ("bark")
//  - Gospel Organ:  full drawbars through a rotating speaker (tremolo + vibrato)
//  - Warm Pad:      three detuned saws through a soft low-pass, slow attack

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>
#include <atomic>
#include <cmath>

enum class GuideSound { rhodes = 0, grand, housePiano, organ, wurlitzer, gospelOrgan, pad };
constexpr int numGuideSounds = 7;

inline const char* guideSoundName (int sound) noexcept
{
    static const char* const names[numGuideSounds] { "Rhodes (FM)", "Soulful Grand", "House Piano (M1 style)", "House Organ",
                                                     "Wurlitzer", "Gospel Organ (Leslie)", "Warm Pad" };
    return names[juce::jlimit (0, numGuideSounds - 1, sound)];
}

struct GuideSoundTag : public juce::SynthesiserSound
{
    bool appliesToNote (int) override    { return true; }
    bool appliesToChannel (int) override { return true; }
};

class GuideVoice : public juce::SynthesiserVoice
{
public:
    GuideVoice (const std::atomic<float>& toneParam, const std::atomic<float>& soundParam)
        : tone (toneParam), soundChoice (soundParam) {}

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<GuideSoundTag*> (s) != nullptr; }

    using SynthesiserVoice::renderNextBlock;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void startNote (int note, float velocity, juce::SynthesiserSound*, int) override
    {
        sr = getSampleRate();
        sound = (GuideSound) juce::jlimit (0, numGuideSounds - 1, juce::roundToInt (soundChoice.load()));
        freq = juce::MidiMessage::getMidiNoteInHertz (note);
        vel = velocity;
        bright = 0.35f + 1.3f * tone.load();
        time = 0.0;
        releasing = false;
        releaseLevel = 1.0;
        noiseState = 0.0;
        sampleCount = 0;

        const double pan = juce::jlimit (-1.0, 1.0, (note - 62) / 40.0);
        panL = std::cos ((pan + 1.0) * juce::MathConstants<double>::pi * 0.25) * juce::MathConstants<double>::sqrt2;
        panR = std::sin ((pan + 1.0) * juce::MathConstants<double>::pi * 0.25) * juce::MathConstants<double>::sqrt2;

        switch (sound)
        {
            case GuideSound::grand:      startGrand (note, false); break;
            case GuideSound::housePiano: startGrand (note, true);  break;
            case GuideSound::organ:      startOrgan();             break;
            case GuideSound::gospelOrgan: startOrgan(); startLeslie(); break;
            case GuideSound::wurlitzer:  startRhodes (note); startWurli(); break;
            case GuideSound::pad:        startPad();               break;
            case GuideSound::rhodes:
            default:                     startRhodes (note);       break;
        }
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (! allowTailOff)
        {
            clearCurrentNote();
            return;
        }

        releasing = true;
        const double releaseSeconds = sound == GuideSound::organ || sound == GuideSound::gospelOrgan ? 0.06
                                    : sound == GuideSound::pad ? 0.6
                                    : sound == GuideSound::rhodes || sound == GuideSound::wurlitzer ? 0.09 : 0.14;
        releaseCoeff = std::exp (-1.0 / (releaseSeconds * sr));
    }

    void renderNextBlock (juce::AudioBuffer<float>& buffer, int start, int num) override
    {
        if (! isVoiceActive())
            return;

        auto* left  = buffer.getWritePointer (0);
        auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

        for (int i = start; i < start + num; ++i)
        {
            double env = 1.0;
            double sample = 0.0;

            switch (sound)
            {
                case GuideSound::grand:
                case GuideSound::housePiano: sample = renderGrand (env); break;
                case GuideSound::organ:      sample = renderOrgan (env); break;
                case GuideSound::gospelOrgan: sample = renderOrgan (env) * leslieGain(); break;
                case GuideSound::wurlitzer:  sample = renderWurli (env); break;
                case GuideSound::pad:        sample = renderPad (env); break;
                case GuideSound::rhodes:
                default:                     sample = renderRhodes (env); break;
            }

            sample *= releaseLevel * gain;
            left[i] += (float) (sample * panL);
            if (right != nullptr)
                right[i] += (float) (sample * panR);

            time += 1.0 / sr;
            if (releasing)
                releaseLevel *= releaseCoeff;

            if (releaseLevel < 0.0005 || (env < 0.0003 && time > 0.02))
            {
                clearCurrentNote();
                break;
            }
        }
    }

private:
    //==============================================================================
    // Cheap maths for the per-sample loops (the guide sound runs up to 24 voices):

    /** sin (2 pi x), x in cycles (any value), from a table with linear interpolation. */
    static double fastSin (double x) noexcept
    {
        static constexpr int size = 4096;
        static const auto table = []
        {
            std::array<double, size + 1> t {};
            for (int i = 0; i <= size; ++i)
                t[(size_t) i] = std::sin (juce::MathConstants<double>::twoPi * i / size);
            return t;
        }();
        const double p = (x - std::floor (x)) * size;
        const int i = (int) p;
        const double f = p - i;
        return table[(size_t) i] + f * (table[(size_t) i + 1] - table[(size_t) i]);
    }

    /** exp (-time / seconds), as a value multiplied down each sample instead of an exp per sample. */
    struct Decay
    {
        double value = 1.0, coeff = 1.0;
        void set (double seconds, double sampleRate) noexcept { value = 1.0; coeff = std::exp (-1.0 / (seconds * sampleRate)); }
        double next() noexcept { const double v = value; value *= coeff; return v; }
    };

    /** tanh for the Wurlitzer's soft clipping (a rational approximation, exact enough for |x| < 4). */
    static double fastTanh (double x) noexcept
    {
        if (x > 3.0)  return 1.0;
        if (x < -3.0) return -1.0;
        const double x2 = x * x;
        return x * (27.0 + x2) / (27.0 + 9.0 * x2);
    }

    //==============================================================================
    // Cheap sine oscillator: a rotating phasor (no std::sin per sample).
    struct Partial
    {
        double re = 1.0, im = 0.0, c = 1.0, s = 0.0, amp = 0.0, decay = 1.0;

        void set (double frequency, double sampleRate, double amplitude, double t60Seconds)
        {
            const double w = juce::MathConstants<double>::twoPi * frequency / sampleRate;
            re = 1.0; im = 0.0; c = std::cos (w); s = std::sin (w);
            amp = amplitude;
            decay = std::exp (-6.9 / (juce::jmax (0.01, t60Seconds) * sampleRate));
        }

        double tick() noexcept
        {
            const double r = re * c - im * s;
            im = re * s + im * c;
            re = r;
            const double out = im * amp;
            amp *= decay;
            return out;
        }

        void normalise() noexcept
        {
            const double m = std::sqrt (re * re + im * im);
            if (m > 0.0) { re /= m; im /= m; }
        }
    };

    static constexpr int maxPartials = 20;

    //==============================================================================
    void startRhodes (int note)
    {
        carrierInc = freq / sr;
        tineInc = freq * 14.0 / sr;
        carrierPhase = modPhase = tinePhase = 0.0;
        decayTime = juce::jlimit (0.7, 6.0, 3.2 * std::pow (2.0, -(note - 60) / 22.0));
        modDepth = bright * (0.6 + 2.4 * vel * vel);
        tineDepth = bright * 0.9 * vel;
        gain = 0.16 * (0.2 + 0.8 * std::pow (vel, 1.4));
        mainDecay.set (decayTime, sr);
        indexDecay.set (0.28, sr);
        clickDecay.set (0.018, sr);
    }

    double renderRhodes (double& env) noexcept
    {
        constexpr double invTwoPi = 1.0 / juce::MathConstants<double>::twoPi;
        env = juce::jmin (1.0, time / 0.0015) * mainDecay.next();

        const double index = modDepth * indexDecay.next() + 0.18 * modDepth;
        const double tine = tineDepth * clickDecay.next();
        const double mod = index * fastSin (modPhase) + tine * fastSin (tinePhase);
        const double out = fastSin (carrierPhase + mod * invTwoPi) * env;

        carrierPhase += carrierInc; if (carrierPhase >= 1.0) carrierPhase -= 1.0;
        modPhase     += carrierInc; if (modPhase >= 1.0)     modPhase -= 1.0;
        tinePhase    += tineInc;    if (tinePhase >= 1.0)    tinePhase -= 1.0;
        return out;
    }

    //==============================================================================
    void startGrand (int note, bool house)
    {
        // String stiffness: partials go slightly sharp, more so in the treble.
        const double B = 0.00012 * std::pow (2.0, (note - 48) / 24.0);
        const double base = juce::jlimit (0.6, 9.0, (house ? 2.6 : 5.0) * std::pow (2.0, -(note - 60) / 20.0));

        // Harder playing = brighter tone.
        const double tilt = juce::jlimit (0.35, 2.2, (house ? 1.05 : 1.65) - 0.9 * vel * (bright - 0.35));

        numPartials = 0;
        for (int n = 1; n <= 16 && numPartials < maxPartials - 4; ++n)
        {
            const double f = freq * n * std::sqrt (1.0 + B * n * n);
            if (f > sr * 0.45)
                break;

            // Hammer strikes ~1/8 of the string: it mutes partials near multiples of 8.
            const double hammer = house ? 1.0 : 0.25 + std::abs (std::sin (juce::MathConstants<double>::pi * n / 8.3));
            const double amp = std::pow ((double) n, -tilt) * hammer;
            const double t60 = base / (1.0 + 0.28 * std::pow (n - 1.0, 1.15));
            partials[(size_t) numPartials++].set (f, sr, amp, t60);
        }

        // Second string, slightly detuned, rings longer: beating + "aftersound".
        for (int n = 1; n <= 3 && numPartials < maxPartials; ++n)
        {
            const double f = freq * n * std::sqrt (1.0 + B * n * n) * (1.0 + (house ? 0.0035 : 0.0009));
            if (f > sr * 0.45)
                break;
            partials[(size_t) numPartials++].set (f, sr, 0.45 * std::pow ((double) n, -tilt), base * 2.2 / n);
        }

        // M1-style bite: a short FM attack layer on top.
        biteInc = freq / sr;
        bitePhase = 0.0;
        biteDepth = house ? (1.2 + 2.0 * vel) * bright : 0.0;

        hammerLevel = (house ? 0.10 : 0.22) * vel;
        hammerCoeff = 0.15 + 0.6 * vel;   // brighter thump when played harder
        gain = (house ? 0.072 : 0.125) * (0.2 + 0.8 * std::pow (vel, 1.3));
        clickDecay.set (0.004, sr);
        indexDecay.set (0.12, sr);
        mainDecay.set (0.9, sr);
    }

    double renderGrand (double& env) noexcept
    {
        double out = 0.0;
        for (int p = 0; p < numPartials; ++p)
            out += partials[(size_t) p].tick();

        if ((++sampleCount & 63) == 0)
            for (int p = 0; p < numPartials; ++p)
                partials[(size_t) p].normalise();

        // Hammer thump: a few ms of low-passed noise.
        if (const double hammerEnv = clickDecay.next(); hammerEnv > 1.0e-4)
        {
            noiseState += hammerCoeff * (random.nextDouble() * 2.0 - 1.0 - noiseState);
            out += noiseState * hammerLevel * hammerEnv;
        }

        if (biteDepth > 0.0)
        {
            constexpr double invTwoPi = 1.0 / juce::MathConstants<double>::twoPi;
            const double index = biteDepth * indexDecay.next();
            out += 0.35 * fastSin (bitePhase + index * fastSin (2.0 * bitePhase) * invTwoPi) * mainDecay.next();
            bitePhase += biteInc; if (bitePhase >= 1.0) bitePhase -= 1.0;
        }

        const double attack = juce::jmin (1.0, time / 0.001);
        env = std::abs (partials[0].amp) + (numPartials > 1 ? std::abs (partials[1].amp) : 0.0);
        return out * attack;
    }

    //==============================================================================
    void startOrgan()
    {
        // Drawbars 16', 8', 4', 2 2/3', 2' (tone pulls the upper ones out).
        static constexpr double ratios[] { 0.5, 1.0, 2.0, 3.0, 4.0 };
        const double t = juce::jlimit (0.0, 1.0, (bright - 0.35) / 1.3);
        const double levels[] { 0.55, 1.0, 0.35 + 0.4 * t, 0.15 + 0.35 * t, 0.1 + 0.3 * t };

        for (int i = 0; i < 5; ++i)
        {
            organInc[(size_t) i] = freq * ratios[i] / sr;
            organPhase[(size_t) i] = 0.0;
            organLevel[(size_t) i] = freq * ratios[i] < sr * 0.45 ? levels[i] : 0.0;
        }

        percussion = 0.55 * vel;
        lfoPhase = random.nextDouble();
        hammerLevel = 0.06 * vel;          // key click
        hammerCoeff = 0.7;
        gain = 0.085 * (0.55 + 0.45 * vel);
        indexDecay.set (0.22, sr);         // percussion
        clickDecay.set (0.002, sr);
    }

    double renderOrgan (double& env) noexcept
    {
        const double vibrato = 1.0 + 0.0022 * fastSin (lfoPhase);
        lfoPhase += 6.2 / sr; if (lfoPhase >= 1.0) lfoPhase -= 1.0;

        double out = 0.0;
        for (int i = 0; i < 5; ++i)
        {
            if (organLevel[(size_t) i] > 0.0)
                out += organLevel[(size_t) i] * fastSin (organPhase[(size_t) i]);
            organPhase[(size_t) i] += organInc[(size_t) i] * vibrato;
            if (organPhase[(size_t) i] >= 1.0) organPhase[(size_t) i] -= 1.0;
        }

        // Percussion (2 2/3' harmonic, fast decay) and key click.
        if (const double perc = indexDecay.next(); perc > 1.0e-4)
            out += percussion * perc * fastSin (organPhase[3]);
        if (const double click = clickDecay.next(); click > 1.0e-4)
        {
            noiseState += hammerCoeff * (random.nextDouble() * 2.0 - 1.0 - noiseState);
            out += noiseState * hammerLevel * click;
        }

        const double attack = juce::jmin (1.0, time / 0.005);
        env = attack;
        return out * attack;
    }

    //==============================================================================
    void startWurli()
    {
        decayTime *= 0.65;                 // shorter, reedier than a Rhodes
        modDepth = bright * (0.9 + 2.0 * vel * vel);
        tineDepth = 0.0;
        drive = 1.2 + 2.2 * vel;
        driveNorm = 1.0 / std::tanh (drive);
        gain = 0.095 * (0.2 + 0.8 * std::pow (vel, 1.4));
        mainDecay.set (decayTime, sr);
        indexDecay.set (0.2, sr);
    }

    double renderWurli (double& env) noexcept
    {
        constexpr double invTwoPi = 1.0 / juce::MathConstants<double>::twoPi;
        env = juce::jmin (1.0, time / 0.001) * mainDecay.next();

        // Odd-harmonic reed tone: the modulator at 1x plus a 3x component, then soft clipping.
        const double index = modDepth * indexDecay.next() + 0.25 * modDepth;
        const double mod = index * (fastSin (modPhase) + 0.3 * fastSin (3.0 * modPhase));
        const double raw = fastSin (carrierPhase + mod * invTwoPi) * env;
        const double out = fastTanh (raw * drive) * driveNorm;

        carrierPhase += carrierInc; if (carrierPhase >= 1.0) carrierPhase -= 1.0;
        modPhase     += carrierInc; if (modPhase >= 1.0)     modPhase -= 1.0;
        return out;
    }

    void startLeslie()
    {
        // Full gospel drawbars and a fast rotating speaker.
        for (auto& l : organLevel)
            l = l > 0.0 ? juce::jmax (l, 0.7) : 0.0;
        percussion = 0.35 * vel;
        lesliePhase = random.nextDouble();
        gain = 0.06 * (0.55 + 0.45 * vel);
    }

    double leslieGain() noexcept
    {
        // Amplitude tremolo of the horn (~6.8 Hz); the organ's vibrato does the pitch part.
        lesliePhase += 6.8 / sr; if (lesliePhase >= 1.0) lesliePhase -= 1.0;
        return 0.82 + 0.18 * fastSin (lesliePhase);
    }

    void startPad()
    {
        static constexpr double detune[3] { -0.006, 0.0, 0.0065 };
        for (int i = 0; i < 3; ++i)
        {
            padInc[(size_t) i] = freq * (1.0 + detune[i]) / sr;
            padPhase[(size_t) i] = random.nextDouble();
        }
        padLow = 0.0;
        padCutoff = juce::jlimit (0.02, 0.45, (freq * (2.0 + 4.0 * bright)) / sr * juce::MathConstants<double>::twoPi);
        gain = 0.15 * (0.5 + 0.5 * vel);
    }

    double renderPad (double& env) noexcept
    {
        // Band-limited saws (polyBLEP), mixed, then a gentle low-pass.
        auto blep = [] (double t, double dt)
        {
            if (t < dt)       { t /= dt; return t + t - t * t - 1.0; }
            if (t > 1.0 - dt) { t = (t - 1.0) / dt; return t * t + t + t + 1.0; }
            return 0.0;
        };

        double saw = 0.0;
        for (int i = 0; i < 3; ++i)
        {
            const double t = padPhase[(size_t) i], dt = padInc[(size_t) i];
            saw += (2.0 * t - 1.0) - blep (t, dt);
            padPhase[(size_t) i] += dt; if (padPhase[(size_t) i] >= 1.0) padPhase[(size_t) i] -= 1.0;
        }

        padLow += padCutoff * (saw / 3.0 - padLow);
        env = juce::jmin (1.0, time / 0.25);
        return padLow * env;
    }

    //==============================================================================
    const std::atomic<float>& tone;
    const std::atomic<float>& soundChoice;
    juce::Random random;

    GuideSound sound = GuideSound::rhodes;
    double sr = 44100.0, freq = 440.0, time = 0.0, gain = 0.0, panL = 1.0, panR = 1.0;
    float vel = 0.0f, bright = 1.0f;
    bool releasing = false;
    double releaseLevel = 1.0, releaseCoeff = 0.999;
    double noiseState = 0.0, hammerLevel = 0.0, hammerCoeff = 0.5;
    juce::uint32 sampleCount = 0;

    // Rhodes
    double carrierPhase = 0.0, modPhase = 0.0, tinePhase = 0.0, carrierInc = 0.0, tineInc = 0.0;
    double decayTime = 1.0, modDepth = 0.0, tineDepth = 0.0;

    // Grand / House piano
    std::array<Partial, maxPartials> partials;
    int numPartials = 0;
    double bitePhase = 0.0, biteInc = 0.0, biteDepth = 0.0;

    // Organ
    std::array<double, 5> organPhase {}, organInc {}, organLevel {};
    double percussion = 0.0, lfoPhase = 0.0;

    // Wurlitzer / Leslie / Pad
    double drive = 1.0, driveNorm = 1.0, lesliePhase = 0.0;
    Decay mainDecay, indexDecay, clickDecay;   // the per-sound envelopes (see the start functions)
    std::array<double, 3> padPhase {}, padInc {};
    double padLow = 0.0, padCutoff = 0.1;
};
