#include "PluginProcessor.h"
#include "EPianoVoice.h"
#include "PluginEditor.h"

namespace
{
    juce::StringArray noteNames()
    {
        juce::StringArray names;
        for (int i = 0; i < 12; ++i)
            names.add (sgk::noteName (i));
        return names;
    }

    template <typename Fn>
    juce::StringArray namesFrom (int count, Fn&& nameOf)
    {
        juce::StringArray names;
        for (int i = 0; i < count; ++i)
            names.add (nameOf (i));
        return names;
    }

    juce::String percent (float v, int) { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; }

    constexpr int stepsPerChordChoices[] { 8, 16, 32, 64 };
    constexpr int fillChoices[] { 0, 2, 4, 8 };
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SoulfulGrooveKeysProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto choice = [&layout] (const String& id, const String& name, const StringArray& items, int def)
    {
        layout.add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, items, def));
    };

    auto unitFloat = [&layout] (const String& id, const String& name, float lo, float def)
    {
        layout.add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, NormalisableRange<float> (lo, 1.0f),
                                                           def, AudioParameterFloatAttributes().withStringFromValueFunction (percent)));
    };

    // Harmony
    choice (ParamIDs::key, "Key", noteNames(), 9);
    choice (ParamIDs::scale, "Scale", namesFrom (sgk::numScales, sgk::scaleName), 1);
    choice (ParamIDs::extension, "Extensions", namesFrom (sgk::numExtensions, sgk::extensionName), 2);
    choice (ParamIDs::voicing, "Voicing", namesFrom (sgk::numVoicings, sgk::voicingName), 0);
    layout.add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::registerPos, 1 }, "Register", 48, 76, 64,
                    AudioParameterIntAttributes().withStringFromValueFunction ([] (int v, int)
                    {
                        return String (sgk::noteName (v)) + String (v / 12 - 1);
                    })));
    choice (ParamIDs::bass, "Bass", namesFrom (sgk::numBassModes, sgk::bassModeName), 1);

    // Progression
    const StringArray degrees { "I", "II", "III", "IV", "V", "VI", "VII" };
    const int defaults[sgk::maxProgressionLength] { 0, 3, 0, 4, 5, 3, 1, 4 };
    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        choice (ParamIDs::slot (i), "Chord " + String (i + 1), degrees, defaults[i]);

    layout.add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::progLength, 1 }, "Progression Length", 1,
                                                     sgk::maxProgressionLength, 2));
    choice (ParamIDs::chordLength, "Chord Length", { "1/2 bar", "1 bar", "2 bars", "4 bars" }, 1);

    // Groove
    choice (ParamIDs::pattern, "Groove", namesFrom ((int) sgk::groovePatterns().size(),
                                                    [] (int i) { return sgk::groovePatterns()[(size_t) i].name; }), 1);
    unitFloat (ParamIDs::swing, "Swing", 0.0f, 0.25f);
    unitFloat (ParamIDs::gate, "Gate", 0.1f, 0.55f);
    layout.add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::velocity, 1 }, "Velocity", 1, 127, 96));
    unitFloat (ParamIDs::humanize, "Humanize", 0.0f, 0.3f);
    choice (ParamIDs::fills, "Fills", { "Off", "Every 2 bars", "Every 4 bars", "Every 8 bars" }, 2);
    choice (ParamIDs::playMode, "Play Mode", { "Host transport", "Hold a key" }, 0);

    // Sound
    layout.add (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::sound, 1 }, "Internal Sound", true));
    unitFloat (ParamIDs::tone, "Tone", 0.0f, 0.5f);
    unitFloat (ParamIDs::reverb, "Reverb", 0.0f, 0.3f);
    layout.add (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::volume, 1 }, "Volume",
                                                       NormalisableRange<float> (-30.0f, 6.0f), -6.0f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                       {
                                                           return String (v, 1) + " dB";
                                                       })));
    return layout;
}

//==============================================================================
SoulfulGrooveKeysProcessor::SoulfulGrooveKeysProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "SoulfulGrooveKeys", createLayout())
{
    auto& toneParam = *apvts.getRawParameterValue (ParamIDs::tone);

    for (int i = 0; i < 24; ++i)
        synth.addVoice (new EPianoVoice (toneParam));

    synth.addSound (new EPianoSound());

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        slotParams[(size_t) i] = apvts.getRawParameterValue (ParamIDs::slot (i));
    events.reserve (1024);
}

float SoulfulGrooveKeysProcessor::raw (const char* id) const
{
    return apvts.getRawParameterValue (id)->load();
}

sgk::GrooveSettings SoulfulGrooveKeysProcessor::currentSettings() const
{
    sgk::GrooveSettings s;
    auto asInt = [this] (const char* id) { return juce::roundToInt (raw (id)); };

    s.keyRoot = asInt (ParamIDs::key);
    s.scale = asInt (ParamIDs::scale);
    s.extension = (sgk::Extension) asInt (ParamIDs::extension);
    s.voicing = (sgk::Voicing) asInt (ParamIDs::voicing);
    s.centre = asInt (ParamIDs::registerPos);
    s.bass = (sgk::BassMode) asInt (ParamIDs::bass);

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        s.progression[(size_t) i] = juce::roundToInt (slotParams[(size_t) i]->load());

    s.progressionLength = asInt (ParamIDs::progLength);
    s.stepsPerChord = stepsPerChordChoices[juce::jlimit (0, 3, asInt (ParamIDs::chordLength))];

    s.pattern = asInt (ParamIDs::pattern);
    s.swing = raw (ParamIDs::swing);
    s.gate = raw (ParamIDs::gate);
    s.velocity = asInt (ParamIDs::velocity);
    s.humanize = raw (ParamIDs::humanize);
    s.fillEveryBars = fillChoices[juce::jlimit (0, 3, asInt (ParamIDs::fills))];

    return s;
}

void SoulfulGrooveKeysProcessor::setParameterFromUI (const juce::String& id, float plainValue)
{
    if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (id)))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
        p->endChangeGesture();
    }
}

//==============================================================================
void SoulfulGrooveKeysProcessor::prepareToPlay (double sampleRate, int)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    reverb.setSampleRate (sampleRate);
    reverb.reset();
    outputGain.reset (sampleRate, 0.05);
    outputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (raw (ParamIDs::volume)));

    wasRunning = false;
    heldCount = 0;
}

bool SoulfulGrooveKeysProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void SoulfulGrooveKeysProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const double sampleRate = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
    buffer.clear();

    auto settings = currentSettings();
    const bool holdMode = juce::roundToInt (raw (ParamIDs::playMode)) == 1;

    // ---- MIDI in: in hold mode keys pick the root, otherwise they pass through ----
    outMidi.clear();
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();

        if (holdMode && m.isNoteOnOrOff())
        {
            const int note = m.getNoteNumber();
            int found = -1;
            for (int i = 0; i < heldCount; ++i)
                if (heldStack[(size_t) i] == note) found = i;

            if (found >= 0)
            {
                for (int i = found; i < heldCount - 1; ++i)
                    heldStack[(size_t) i] = heldStack[(size_t) i + 1];
                --heldCount;
            }

            if (m.isNoteOn() && heldCount < (int) heldStack.size())
                heldStack[(size_t) heldCount++] = note;

            continue;
        }

        if (holdMode && m.isAllNotesOff())
            heldCount = 0;

        outMidi.addEvent (m, meta.samplePosition);
    }

    if (! holdMode)
        heldCount = 0;

    if (heldCount > 0)
        settings.keyRoot = heldStack[(size_t) heldCount - 1] % 12;

    uiHeldRoot = heldCount > 0 ? settings.keyRoot : -1;

    // ---- Timing ----
    double bpm = 120.0, hostPpq = 0.0;
    bool hostPlaying = false, haveHostPpq = false;

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())          bpm = *b;
            if (auto p = pos->getPpqPosition()) { hostPpq = *p; haveHostPpq = true; }
            hostPlaying = pos->getIsPlaying();
        }
    }

    bpm = juce::jlimit (20.0, 400.0, bpm);
    const double samplesPerBeat = sampleRate * 60.0 / bpm;
    const double blockBeats = numSamples / samplesPerBeat;

    bool running = false;
    double startPpq = 0.0;

    if (holdMode)
    {
        running = heldCount > 0;

        if (hostPlaying && haveHostPpq)
            startPpq = hostPpq;
        else
            startPpq = wasRunning ? internalPpq : 0.0;   // free-running clock from the key press
    }
    else
    {
        running = hostPlaying && haveHostPpq;
        startPpq = hostPpq;
    }

    events.clear();

    if (running)
    {
        if (! wasRunning || std::abs (startPpq - expectedNextPpq) > 0.05)
            engine.allNotesOff (startPpq, events);   // start, loop or jump

        engine.render (startPpq, startPpq + blockBeats, settings, events);
        expectedNextPpq = internalPpq = startPpq + blockBeats;

        const auto step = (long long) std::floor (startPpq / sgk::stepLength);
        const auto& pattern = sgk::groovePatterns()[(size_t) juce::jlimit (0, (int) sgk::groovePatterns().size() - 1, settings.pattern)];
        uiStep = (int) (((step % pattern.length()) + pattern.length()) % pattern.length());
        uiSlot = sgk::GrooveEngine::slotForStep (step, settings);
    }
    else
    {
        if (engine.anyNoteSounding())
            engine.allNotesOff (startPpq, events);

        uiStep = -1;
        uiSlot = -1;
    }

    wasRunning = running;

    for (const auto& e : events)
    {
        const int pos = juce::jlimit (0, numSamples - 1, (int) std::floor ((e.ppq - startPpq) * samplesPerBeat));
        outMidi.addEvent (e.isNoteOn() ? juce::MidiMessage::noteOn (1, e.note, (juce::uint8) e.velocity)
                                       : juce::MidiMessage::noteOff (1, e.note),
                          pos);
    }

    // ---- Internal e-piano ----
    const bool soundOn = raw (ParamIDs::sound) > 0.5f;
    if (soundWasOn && ! soundOn)
        synth.allNotesOff (0, false);
    soundWasOn = soundOn;

    if (soundOn)
    {
        synth.renderNextBlock (buffer, outMidi, 0, numSamples);

        const float amount = raw (ParamIDs::reverb);
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.62f;
        rp.damping = 0.45f;
        rp.width = 1.0f;
        rp.wetLevel = amount * 0.45f;
        rp.dryLevel = 1.0f - amount * 0.3f;
        reverb.setParameters (rp);

        if (buffer.getNumChannels() > 1)
            reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);
        else
            reverb.processMono (buffer.getWritePointer (0), numSamples);

        outputGain.setTargetValue (juce::Decibels::decibelsToGain (raw (ParamIDs::volume)));
        outputGain.applyGain (buffer, numSamples);
    }

    midi.swapWith (outMidi);
}

//==============================================================================
juce::AudioProcessorEditor* SoulfulGrooveKeysProcessor::createEditor()
{
    return new SoulfulGrooveKeysEditor (*this);
}

void SoulfulGrooveKeysProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void SoulfulGrooveKeysProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SoulfulGrooveKeysProcessor();
}
