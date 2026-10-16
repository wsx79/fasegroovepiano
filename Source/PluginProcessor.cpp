// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginProcessor.h"
#include "PadController.h"
#include "GuideVoice.h"
#include "PluginEditor.h"

#include <map>
#include <mutex>
#include <numeric>

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
    constexpr int fillChoices[] { 0, 2, 4, 8, 1 };   // "Every bar" was added last
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout FaseGroovePianoProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    // Parameters are grouped (Harmony, Progression, Groove, Tempo & MIDI, Sound) so the DAW's
    // automation list is easy to browse. Groups don't change the parameter IDs.
    std::vector<std::unique_ptr<RangedAudioParameter>> current;
    auto add = [&current] (auto param) { current.push_back (std::move (param)); };
    auto group = [&layout, &current] (const String& id, const String& name)
    {
        auto g = std::make_unique<AudioProcessorParameterGroup> (id, name, " | ");
        for (auto& p : current)
            g->addChild (std::move (p));
        current.clear();
        layout.add (std::move (g));
    };

    auto choice = [&add] (const String& id, const String& name, const StringArray& items, int def)
    {
        add (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, items, def));
    };

    auto unitFloat = [&add] (const String& id, const String& name, float lo, float def)
    {
        add (std::make_unique<AudioParameterFloat> (ParameterID { id, 1 }, name, NormalisableRange<float> (lo, 1.0f),
                                                           def, AudioParameterFloatAttributes().withStringFromValueFunction (percent)));
    };

    // Harmony
    choice (ParamIDs::key, "Key", noteNames(), 9);
    choice (ParamIDs::scale, "Scale", namesFrom (sgk::numScales, sgk::scaleName), 1);
    choice (ParamIDs::extension, "Extensions", namesFrom (sgk::numExtensions, sgk::extensionName), 2);
    // Upper triad: the compact, classic house stab (3 notes within a 9th, around F4).
    choice (ParamIDs::voicing, "Voicing", namesFrom (sgk::numVoicings, sgk::voicingName), (int) sgk::Voicing::upperTriad);
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::registerPos, 1 }, "Register", 48, 76, 60,
                    AudioParameterIntAttributes().withStringFromValueFunction ([] (int v, int)
                    {
                        return String (sgk::noteName (v)) + String (v / 12 - 1);
                    })));
    choice (ParamIDs::bass, "Bass", namesFrom (sgk::numBassModes, sgk::bassModeName), (int) sgk::BassMode::withChords);   // the piano's left hand: bass in every stab
    choice (ParamIDs::approach, "Approach Chord", namesFrom (sgk::numApproachStyles, sgk::approachStyleName), 0);
    choice (ParamIDs::topLine, "Top Line", namesFrom (sgk::numTopLines, sgk::topLineName), (int) sgk::TopLine::pedal);
    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::inversion (i), 1 }, "Inversion " + String (i + 1), -3, 3, 0));

    group ("harmony", "Harmony");

    // Progression
    const StringArray degrees { "I", "II", "III", "IV", "V", "VI", "VII" };
    const int defaults[sgk::maxProgressionLength] { 0, 3, 0, 4, 5, 3, 1, 4 };
    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        choice (ParamIDs::slot (i), "Chord " + String (i + 1), degrees, defaults[i]);

    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::progLength, 1 }, "Progression Length", 1,
                                                     sgk::maxProgressionLength, 2));
    choice (ParamIDs::chordLength, "Chord Length", { "1/2 bar", "1 bar", "2 bars", "4 bars" }, 1);

    group ("progression", "Progression");

    // Groove
    auto grooveNames = namesFrom ((int) sgk::groovePatterns().size(),
                                  [] (int i) { return sgk::groovePatterns()[(size_t) i].name.c_str(); });
    grooveNames.add ("* Random riff *");
    grooveNames.add ("* User pattern *");
    choice (ParamIDs::pattern, "Groove", grooveNames, 1);
    choice (ParamIDs::section, "Section", namesFrom (sgk::numSections, sgk::sectionName), 0);
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::riffSeed, 1 }, "Riff Seed", 1, 99999, 1));
    unitFloat (ParamIDs::riffDensity, "Riff Density", 0.0f, 0.5f);

    for (int i = 0; i < sgk::numSegments; ++i)
        choice (ParamIDs::segment (i), "Segment " + String (i + 1), { "A", "B", "C", "D" }, i);
    unitFloat (ParamIDs::swing, "Swing", 0.0f, 0.25f);
    unitFloat (ParamIDs::gate, "Gate", 0.1f, 0.65f);
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::velocity, 1 }, "Velocity", 1, 127, 118));
    unitFloat (ParamIDs::humanize, "Humanize", 0.0f, 0.3f);
    choice (ParamIDs::humanizeStyle, "Humanize Style", namesFrom (sgk::numHumanizeStyles, sgk::humanizeStyleName), 0);
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::humanizeSeed, 1 }, "Humanize Feel", 0, 99999, 0));
    choice (ParamIDs::fills, "Fills", { "Off", "Every 2 bars", "Every 4 bars", "Every 8 bars", "Every bar" }, 0);
    choice (ParamIDs::fillStyle, "Fill Style", namesFrom (sgk::numFillStyles, sgk::fillStyleName), 0);
    choice (ParamIDs::variation, "Variation", { "Off", "Every 4 bars", "Every 8 bars", "Evolve over 8 bars", "Evolve over 16 bars" }, 0);
    choice (ParamIDs::grooveBars, "Groove Length", { "2 bars", "1 bar", "4 bars" }, 0);
    choice (ParamIDs::movement, "Voicing Movement", { "Off", "Subtle", "Lively" }, 0);
    choice (ParamIDs::strumDir, "Strum Direction", { "Up", "Down", "Alternate" }, 0);
    unitFloat (ParamIDs::strum, "Strum", 0.0f, 0.0f);
    add (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::octaveTop, 1 }, "Top Note +8va", false));
    choice (ParamIDs::playMode, "Play Mode", { "Host transport", "Hold a key" }, 0);

    group ("groove", "Groove");

    // Tempo: follows the DAW by default, or runs at its own BPM.
    add (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::tempoSync, 1 }, "Sync Tempo to DAW", true));
    add (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::tempo, 1 }, "Tempo",
                                                       NormalisableRange<float> (60.0f, 180.0f, 0.1f), 124.0f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                       {
                                                           return String (v, 1) + " BPM";
                                                       })));
    choice (ParamIDs::bassChannel, "MIDI Out Channels", { "All on channel 1", "Bass on channel 2" }, 0);

    group ("tempo", "Tempo & MIDI");

    // Sound
    add (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::sound, 1 }, "Guide Sound", true));
    choice (ParamIDs::soundType, "Guide Sound Type", namesFrom (numGuideSounds, guideSoundName), 0);
    unitFloat (ParamIDs::tone, "Tone", 0.0f, 0.5f);
    unitFloat (ParamIDs::reverb, "Reverb", 0.0f, 0.3f);
    unitFloat (ParamIDs::chorus, "Chorus", 0.0f, 0.0f);
    unitFloat (ParamIDs::pump, "Pump", 0.0f, 0.0f);
    add (std::make_unique<AudioParameterFloat> (ParameterID { ParamIDs::volume, 1 }, "Volume",
                                                       NormalisableRange<float> (-30.0f, 6.0f), -6.0f,
                                                       AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                                       {
                                                           return String (v, 1) + " dB";
                                                       })));
    // Sidechain: the pump follows the tempo (internal) or a track sent to the sidechain input (external).
    choice (ParamIDs::pumpSource, "Sidechain Source", { "Tempo (internal)", "Sidechain input (external)" }, 0);
    choice (ParamIDs::pumpRate, "Sidechain Rate", { "1/4", "1/8", "1/2", "1/16" }, 0);
    unitFloat (ParamIDs::pumpRelease, "Sidechain Release", 0.0f, 0.5f);
    add (std::make_unique<AudioParameterBool> (ParameterID { ParamIDs::sequencer, 1 }, "Internal Sequencer", true));
    // The chords' range: no chord note below / above (the defaults leave everything as voiced).
    auto noteText = [] (int v, int) { return String (sgk::noteName (v)) + String (v / 12 - 1); };
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::chordLow, 1 }, "Chord Range Low", 24, 72, 24,
                                              AudioParameterIntAttributes().withStringFromValueFunction (noteText)));
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::chordHigh, 1 }, "Chord Range High", 60, 108, 108,
                                              AudioParameterIntAttributes().withStringFromValueFunction (noteText)));
    // The strings: a third part for body (appended, so saved projects keep their settings).
    choice (ParamIDs::strings, "Strings", namesFrom (sgk::numStringsModes, sgk::stringsModeName), 0);
    group ("sound", "Guide sound");
    return layout;
}

//==============================================================================
juce::AudioProcessor::BusesProperties FaseGroovePianoProcessor::makeBuses()
{
    auto buses = BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true);

    // In a DAW: one input, the sidechain (active, so hosts like Live offer it at once; in VST3 it
    // is an aux bus, not a main input). The standalone app has none: no microphone needed.
    if (juce::PluginHostType::getPluginLoadedAs() != wrapperType_Standalone)
        buses = buses.withInput ("Sidechain", juce::AudioChannelSet::stereo(), true);
    return buses;
}

FaseGroovePianoProcessor::FaseGroovePianoProcessor()
    : AudioProcessor (makeBuses()),
      apvts (*this, nullptr, "FaseGroovePiano", createLayout())
{
    auto& toneParam = *apvts.getRawParameterValue (ParamIDs::tone);
    auto& soundParam = *apvts.getRawParameterValue (ParamIDs::soundType);

    for (int i = 0; i < 32; ++i)
        synth.addVoice (new GuideVoice (toneParam, soundParam));

    synth.addSound (new GuideSoundTag (stringsChannel));
    synth.addSound (new GuideStringsTag (stringsChannel));

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        slotParams[(size_t) i] = apvts.getRawParameterValue (ParamIDs::slot (i));

    for (int i = 0; i < sgk::numSegments; ++i)
        segmentParams[(size_t) i] = apvts.getRawParameterValue (ParamIDs::segment (i));

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        inversionParams[(size_t) i] = apvts.getRawParameterValue (ParamIDs::inversion (i));

    clearFreeChords();
    resetUserPattern();
    resetSong();
    events.reserve (1024);

    // Edits made in the piano roll follow the settings: a change carries them over to the new notes.
    editRefresher.callback = [this] { refreshEditedLoop(); };
    editRefresher.startTimer (150);

    pads = std::make_unique<PadController> (*this);

    // Live keys pressed in the audio thread run here, on the message thread.
    liveKeyRunner.callback = [this]
    {
        int keys[32];
        int count = 0;
        {
            const auto scope = liveKeyFifo.read (liveKeyFifo.getNumReady());
            for (int i = 0; i < scope.blockSize1; ++i) keys[count++] = liveKeyBuffer[(size_t) (scope.startIndex1 + i)];
            for (int i = 0; i < scope.blockSize2; ++i) keys[count++] = liveKeyBuffer[(size_t) (scope.startIndex2 + i)];
        }
        for (int i = 0; i < count; ++i)
            runLiveKey (keys[i]);
    };
    liveKeyRunner.startTimer (25);
    liveKeysEnabled();   // reads the setting now, so the audio thread only reads an atomic
}

namespace
{
    std::atomic<int> liveKeysChoice { -1 };   // -1 = not read yet

    std::unique_ptr<juce::PropertiesFile> openLiveKeySettings()
    {
        juce::PropertiesFile::Options o;
        o.applicationName = "Fase Groove Piano";
        o.folderName = "Fase Groove Piano";
        o.filenameSuffix = ".settings";
        o.osxLibrarySubFolder = "Application Support";
        return std::make_unique<juce::PropertiesFile> (o);
    }
}

bool FaseGroovePianoProcessor::liveKeysEnabled()
{
    if (liveKeysChoice.load() < 0)
        liveKeysChoice = openLiveKeySettings()->getBoolValue ("liveKeys", false) ? 1 : 0;   // (read once, on the message thread)
    return liveKeysChoice.load() == 1;
}

void FaseGroovePianoProcessor::setLiveKeysEnabled (bool on)
{
    liveKeysChoice = on ? 1 : 0;
    auto settings = openLiveKeySettings();
    settings->setValue ("liveKeys", on);
    settings->saveIfNeeded();
}

void FaseGroovePianoProcessor::runLiveKey (int key)
{
    // White keys: the song sections (the parameter's order: main, intro, break, build-up, drop, outro).
    switch (key)
    {
        case 0:  setParameterFromUI (ParamIDs::section, (float) sgk::Section::intro); break;
        case 2:  setParameterFromUI (ParamIDs::section, (float) sgk::Section::main); break;
        case 4:  setParameterFromUI (ParamIDs::section, (float) sgk::Section::breakdown); break;
        case 5:  setParameterFromUI (ParamIDs::section, (float) sgk::Section::buildup); break;
        case 7:  setParameterFromUI (ParamIDs::section, (float) sgk::Section::drop); break;
        case 9:  setParameterFromUI (ParamIDs::section, (float) sgk::Section::outro); break;
        case 3:  rollNewRiff(); break;
        case 6:  hearPart (false); break;
        case 8:  hearPart (true); break;
        case 10: setParameterFromUI (ParamIDs::sequencer, raw (ParamIDs::sequencer) > 0.5f ? 0.0f : 1.0f); break;
        case 11: previewToggle(); break;
        default: break;   // C# (the fill) is played by the audio thread itself
    }
}

FaseGroovePianoProcessor::~FaseGroovePianoProcessor()
{
    pads.reset();   // lights off, the Launchpad back to its own mode
}

void FaseGroovePianoProcessor::hearPart (bool bass)
{
    const bool bassOn = isBassOn();
    const bool pianoOn = ! chordsMuted.load() || ! bassOn;
    if (bass)
    {
        if (bassOn) { toggleBass(); chordsMuted = false; }   // off (the piano plays)
        else        toggleBass();
    }
    else
    {
        if (pianoOn && bassOn)  chordsMuted = true;                    // bass only
        else if (pianoOn)      { chordsMuted = true; toggleBass(); }
        else                   chordsMuted = false;
    }
}

void FaseGroovePianoProcessor::resetUserPattern()
{
    // The user pattern starts as a copy of "Soulful Push".
    const auto& start = sgk::groovePatterns()[1];
    for (int i = 0; i < sgk::userPatternLength; ++i)
    {
        userRight[(size_t) i] = start.rightHand[(size_t) i];
        userLeft[(size_t) i]  = start.leftHand[(size_t) i];
    }
    resetStepVelocities();
}

int FaseGroovePianoProcessor::baseStepIndex (int displayedStep) const
{
    displayedStep %= sgk::userPatternLength;   // 4-bar grooves: bars 3-4 vary bars 1-2
    const int segLength = sgk::userPatternLength / sgk::numSegments;
    return segmentAtStep (displayedStep) * segLength + displayedStep % segLength;
}

int FaseGroovePianoProcessor::levelIndex (int displayedStep) const
{
    // Through the velocity page's A-D order, then the groove's.
    displayedStep %= sgk::userPatternLength;
    const int segLength = sgk::userPatternLength / sgk::numSegments;
    return baseStepIndex (velocityPartAt (displayedStep / segLength) * segLength + displayedStep % segLength);
}

bool FaseGroovePianoProcessor::makeUserPattern()
{
    // A groove from the library becomes yours (same rhythm) before it gets levels.
    if (juce::roundToInt (raw (ParamIDs::pattern)) == sgk::userPatternIndex())
        return true;
    const auto current = sgk::GrooveEngine::basePattern (currentSettings());
    if (current.length() != sgk::userPatternLength)
        return false;
    setUserPattern (current);
    return true;
}

int FaseGroovePianoProcessor::stepVelocity (bool bassRow, int displayedStep) const
{
    if (displayedStep < 0 || juce::roundToInt (raw (ParamIDs::pattern)) != sgk::userPatternIndex())
        return 0;
    return (bassRow ? userVelLeft : userVelRight)[(size_t) levelIndex (displayedStep)].load();
}

void FaseGroovePianoProcessor::setStepVelocity (bool bassRow, int displayedStep, int level)
{
    if (displayedStep < 0 || ! makeUserPattern())
        return;
    (bassRow ? userVelLeft : userVelRight)[(size_t) levelIndex (displayedStep)] = juce::jlimit (0, 127, level);
}

void FaseGroovePianoProcessor::setVelocityPartAt (int position, int part)
{
    if (! makeUserPattern())
        return;
    velocityOrder[(size_t) juce::jlimit (0, sgk::numSegments - 1, position)] = juce::jlimit (0, sgk::numSegments - 1, part);
}

void FaseGroovePianoProcessor::randomizeVelocities (bool chords, bool bass, int part)
{
    if (! makeUserPattern())
        return;

    const auto s = currentSettings();
    const auto p = sgk::GrooveEngine::resolvePattern (s);
    const bool bassLevels = bass && isBassOn() && ! sgk::isBassLine (s.bass);
    const auto bassLine = s.bass == sgk::BassMode::withChords ? sgk::stabBassLine (p) : p.leftHand;
    const int segLength = sgk::userPatternLength / sgk::numSegments;
    auto& random = juce::Random::getSystemRandom();

    auto automatic = [] (char c) { return c == 'g' ? 65 : c == 'x' || c == 'u' || c == 'd' ? 106 : c == '>' ? 114 : 118; };
    auto isHit = [] (char c) { return c != '.' && c != '='; };

    for (int k = 0; k < sgk::numSegments; ++k)
    {
        if (part >= 0 ? k != part : velocityLocked[(size_t) k])
            continue;

        // The levels of part k, made for the hits of the half bar where it plays.
        int shown = k;
        for (int pos = sgk::numSegments - 1; pos >= 0; --pos)
            if (velocityPartAt (pos) == k)
                shown = pos;

        // A shape for the half bar: loose, on the beat, rising or falling, then a little life.
        const int shape = random.nextInt (4);
        for (int row = 0; row < 2; ++row)
        {
            if ((row == 0 && ! chords) || (row == 1 && ! bassLevels))
                continue;
            const auto& line = row == 0 ? p.rightHand : bassLine;
            auto& levels = row == 0 ? userVelRight : userVelLeft;
            for (int i = 0; i < segLength; ++i)
            {
                const int d = shown * segLength + i;
                const char c = d < (int) line.size() ? line[(size_t) d] : '.';
                int level = 0;
                if (isHit (c))
                {
                    level = automatic (c) + random.nextInt (25) - 16;
                    if (shape == 1) level += i % 4 == 0 ? 8 : -6;
                    if (shape == 2) level += (i - segLength / 2) * 3;
                    if (shape == 3) level -= (i - segLength / 2) * 3;
                    level = c == 'g' ? juce::jlimit (40, 85, level) : juce::jlimit (72, 127, level);
                }
                levels[(size_t) baseStepIndex (k * segLength + i)] = level;
            }
        }
    }
}

void FaseGroovePianoProcessor::resetStepVelocities (bool chords, bool bass)
{
    if (chords && bass)
        for (int i = 0; i < sgk::numSegments; ++i)
            velocityOrder[(size_t) i] = i;

    for (int i = 0; i < sgk::userPatternLength; ++i)
    {
        if (chords) userVelRight[(size_t) i] = 0;
        if (bass)   userVelLeft[(size_t) i]  = 0;
    }
}

void FaseGroovePianoProcessor::resetSegmentVelocities (int segment, bool chords, bool bass)
{
    const int segLength = sgk::userPatternLength / sgk::numSegments;
    for (int i = segment * segLength; i < (segment + 1) * segLength; ++i)
    {
        if (chords) userVelRight[(size_t) i] = 0;
        if (bass)   userVelLeft[(size_t) i]  = 0;
    }
}

bool FaseGroovePianoProcessor::hasStepVelocities() const
{
    if (juce::roundToInt (raw (ParamIDs::pattern)) != sgk::userPatternIndex())
        return false;
    for (int i = 0; i < sgk::userPatternLength; ++i)
        if (userVelRight[(size_t) i].load() > 0 || userVelLeft[(size_t) i].load() > 0)
            return true;
    return false;
}

void FaseGroovePianoProcessor::resetToDefaults()
{
    for (auto* p : getParameters())
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            ranged->beginChangeGesture();
            ranged->setValueNotifyingHost (ranged->getDefaultValue());
            ranged->endChangeGesture();
        }

    clearFreeChords();
    resetUserPattern();
    segmentLocked.fill (false);
    lastBassMode = 1;
    lastStringsMode = 1;
    chordsMuted = false;
    resetSong();
    freeCircle = false;
    previewOn = false;
    editorScale = 1.0f;
    shrinkWhileDragging = true;
}

//==============================================================================
namespace
{
    struct Style
    {
        const char* name;
        const char* progression;    // name of a progression preset
        const char* groove;         // name of a groove
        int extension, voicing, bass, approach, sound, registerNote;
        float swing, gate, humanize, strum;
        bool octaveTop;
        int fills = 0, fillStyle = 0;   // fills: 0 off 1 every 2 bars 2 every 4 3 every 8 4 every bar
        int topLine = (int) sgk::TopLine::pedal;   // house loops hold one top note; 0 = smooth (auto)
    };

    // extension: 0 triads 1 7ths 2 9ths 3 11ths 4 6/9 5 add9 6 sus 7 9ths+altered V
    // voicing: 0 close 1 drop2 2 rootless 3 wide 4 quartal 5 shell 6 drop2+4 7 octave triads
    // bass: 0 off 1 root 2 octaves 3 root+5th 4 walking 5 syncopated 6 piano stabs (bass in every chord)
    //       7.. bass lines: 7 Disco Octave Gallop 8 Disco Octave Push 11 Funk Sixteenths 18 Gospel Walk (see sgk::bassLines)   approach: 0 off 1 above 2 below
    // sound: 0 Rhodes 1 Soulful Grand 2 House Piano 3 House Organ
    const Style styles[] {
        { "House Piano Loop",     "Minor Flow  i - VI - III - VII",    "Dotted House Stabs",   2, 8, 6, 0, 2, 60, 0.25f, 0.65f, 0.15f, 0.00f, false, 0, 0 },
        { "Classic Deep House",   "Dorian Vamp  i - IV",               "Deep Chords",          2, 2, 1, 0, 0, 62, 0.20f, 0.60f, 0.25f, 0.10f, false },
        { "Soulful House",        "Turnaround  I - vi - ii - V",       "Soulful Sunday",       7, 1, 1, 1, 1, 64, 0.25f, 0.55f, 0.30f, 0.20f, false, 3, 7 },
        { "Gospel House",         "Gospel Lift  IV - V - vi - I",      "Gospel Shout",         7, 0, 18, 1, 1, 64, 0.30f, 0.50f, 0.30f, 0.15f, true, 2, 3 },
        { "Jackin' House",        "Jackin' Minor  i - iv",             "Jackin' Stabs",        1, 0, 2, 0, 2, 64, 0.45f, 0.35f, 0.20f, 0.00f, true, 2, 5 },
        { "'90s Piano House",     "Deep House Classic  vi - IV - I - V", "Italo Piano Anthem", 1, 0, 6, 0, 2, 66, 0.10f, 0.45f, 0.15f, 0.00f, true, 3, 8 },
        { "Organ Jack",           "Deep Dorian  i - ii - IV - i",      "M1 Organ Stabs",       1, 0, 1, 0, 3, 62, 0.35f, 0.40f, 0.20f, 0.00f, false, 2, 6 },
        { "Nu Disco",             "Disco Jack  ii - V - I - vi",       "Nu Disco Keys",        1, 1, 8, 0, 0, 64, 0.15f, 0.40f, 0.20f, 0.00f, false, 3, 4 },
        { "Funky Disco",          "Funky Dominant  I7 - IV7",          "Funky Clavinet 16ths", 2, 5, 11, 0, 2, 64, 0.20f, 0.30f, 0.20f, 0.00f, false, 2, 9 },
        { "UK Garage",            "Minor Flow  i - VI - III - VII",    "UK Garage 2-Step",     2, 2, 1, 0, 0, 64, 0.50f, 0.45f, 0.25f, 0.10f, false },
        { "Afro House",           "Deep Minor  i - v",                 "Afro House Keys",      2, 4, 1, 0, 0, 62, 0.15f, 0.50f, 0.30f, 0.00f, false },
        { "Neo Soul",             "Neo Soul  ii - iii - IV - V",       "Neo Soul Ghosts",      3, 6, 1, 2, 0, 62, 0.45f, 0.60f, 0.40f, 0.30f, false, 3, 7, 0 },
        { "Jackin' Piano",        "Jack Swing  i - v - VII - IV",      "Chicago Piano Bounce", 1, 0, 1, 0, 2, 66, 0.55f, 0.30f, 0.20f, 0.00f, true, 2, 5, 0 },
        { "Chill Lounge",         "Lydian Float  I - II",              "Lounge Keys",          2, 3, 1, 0, 0, 64, 0.20f, 0.80f, 0.30f, 0.35f, false, 0, 0, 0 },
        { "Future House",         "Maj9 Drift  I - bIII - bVI - bVII", "Future Stab 3-3-2",    2, 2, 6, 0, 2, 62, 0.05f, 0.30f, 0.10f, 0.00f, false, 2, 6 },
        { "Disco Piano",          "Minor Glide  i - bVII - bVI - V7",  "Disco Octave Pump",    0, 7, 7, 0, 1, 64, 0.10f, 0.40f, 0.15f, 0.00f, false, 3, 8 },
        // v2.1
        { "Sunday Soulful",       "Sunday Gospel  IV - iii - vi - ii - V", "Sunday Keys Push", 9, 9, 1, 1, 1, 62, 0.25f, 0.60f, 0.30f, 0.15f, false, 3, 3 },
        { "Deep Night Pads",      "Late Night Minor  i - v - VI - iv", "Deep Offbeat Pads",    8, 10, 14, 0, 0, 60, 0.20f, 0.80f, 0.25f, 0.10f, false },
        { "Chicago Jack",         "Jack Vamp  I7 - IV7 - I7 - V7",     "Chicago Stab Jack",    10, 0, 2, 0, 3, 64, 0.45f, 0.35f, 0.20f, 0.00f, true, 2, 5, 0 },
        { "Piano House Anthem",   "Sunrise  I - V - vi - IV",          "Anthem Offbeats Push", 2, 8, 6, 0, 2, 62, 0.10f, 0.45f, 0.15f, 0.00f, true, 3, 8 },
        { "Vocal House",          "Soul Cycle  I - iii - vi - IV",     "Vocal House Offbeats", 2, 8, 6, 0, 1, 64, 0.20f, 0.55f, 0.25f, 0.10f, false, 3, 7 },
        { "Deep Soul Keys",       "Deep Soul  ii - V - I - IV",        "Midnight Rhodes",      8, 2, 14, 0, 0, 62, 0.25f, 0.65f, 0.30f, 0.15f, false },
        { "Gospel Chops",         "Church Turn  I - I7 - IV - iv",     "Gospel Chop Comp",     7, 1, 18, 1, 1, 64, 0.30f, 0.45f, 0.30f, 0.10f, false, 2, 3 },
        { "Jackin' Funk",         "Funk Turn  I7 - bVII7 - IV7 - I7",  "Jack Bounce 16ths",    10, 5, 11, 0, 4, 64, 0.35f, 0.30f, 0.20f, 0.00f, false, 2, 6, 0 },
        { "Classic Piano House",  "Classic Soul  I - IV - ii - V",     "Classic Dotted Loop",  1, 0, 6, 0, 2, 60, 0.15f, 0.60f, 0.15f, 0.00f, false, 3, 0 },
        { "Deep Minimal",         "Minor Two  i - ii",                 "Deep Minimal Stab",    8, 4, 15, 0, 0, 62, 0.20f, 0.50f, 0.20f, 0.00f, false },
        { "Soulful Lift",         "Two-Five Lift  ii - V - iii - vi",  "Soulful Sway Comp",    9, 2, 17, 0, 0, 63, 0.30f, 0.60f, 0.30f, 0.15f, false, 3, 3 },
        { "Jackin' Piano Chicago", "Chicago Shuffle  I - vi - IV - V", "Jackin' Piano Hook",   1, 0, 6, 0, 2, 66, 0.50f, 0.35f, 0.20f, 0.00f, true, 2, 5, 0 },
    };

    int indexOfGroove (const char* name)
    {
        const auto& patterns = sgk::groovePatterns();
        for (int i = 0; i < (int) patterns.size(); ++i)
            if (patterns[(size_t) i].name == name)
                return i;
        return 1;
    }

    int indexOfPreset (const char* name)
    {
        const auto& presets = sgk::progressionPresets();
        for (int i = 0; i < (int) presets.size(); ++i)
            if (juce::String (presets[(size_t) i].name) == name)
                return i;
        return -1;
    }
}

//==============================================================================
bool FaseGroovePianoProcessor::recogniseChord (int mask, int lowestPc, int& rootPc, bool& minor)
{
    auto has = [mask] (int pc) { return (mask & (1 << (((pc % 12) + 12) % 12))) != 0; };

    int bestScore = -1;
    for (int r = 0; r < 12; ++r)
    {
        if (! has (r))
            continue;

        int score = 0;
        const bool major3 = has (r + 4), minor3 = has (r + 3);
        if (major3 || minor3)               score += 4;
        if (has (r + 7))                    score += 3;
        else if (has (r + 6) || has (r + 8)) score += 1;
        if (has (r + 10) || has (r + 11))   score += 2;
        if (r == lowestPc)                  score += 2;   // the bass usually plays the root

        if (score > bestScore)
        {
            bestScore = score;
            rootPc = r;
            minor = minor3 && ! major3;
        }
    }

    return bestScore >= 5;
}

int FaseGroovePianoProcessor::learnChord (int mask, int lowestPc)
{
    int root = 0;
    bool minor = false;
    if (! recogniseChord (mask, lowestPc, root, minor))
        return -1;

    const auto s = currentSettings();
    const int slot = juce::jlimit (0, sgk::maxProgressionLength - 1, learnSlot);

    int degree = -1;
    for (int d = 0; d < 7 && degree < 0; ++d)
    {
        const auto c = sgk::buildChord (s.keyRoot, s.scale, d, s.extension);
        if (c.rootPc == root && (c.third == 3) == minor)
            degree = d;
    }

    if (degree >= 0)
    {
        setFreeChord (slot, sgk::noFreeChord);
        setParameterFromUI (ParamIDs::slot (slot), (float) degree);
    }
    else
    {
        setFreeChord (slot, sgk::freeChordCode (root - s.keyRoot, minor));
    }

    setParameterFromUI (ParamIDs::progLength, (float) juce::jmax (slot + 1, slot == 0 ? 1 : s.progressionLength));
    if (slot == 0)
        setParameterFromUI (ParamIDs::progLength, 1.0f);

    learnSlot = (slot + 1) % sgk::maxProgressionLength;
    return slot;
}

//==============================================================================
juce::File FaseGroovePianoProcessor::userFolder()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Fase Groove Piano");
}

juce::File FaseGroovePianoProcessor::presetFolder()
{
    const auto folder = userFolder().getChildFile ("Presets");

    // Up to v1.16 presets lived in the hidden application-data folder: bring them over once.
    static std::once_flag moved;
    std::call_once (moved, [folder]
    {
        const auto old = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                             .getChildFile ("Fase Groove Piano").getChildFile ("Presets");
        for (const auto& f : old.findChildFiles (juce::File::findFiles, false, "*.fgp"))
            if (folder.createDirectory() && ! folder.getChildFile (f.getFileName()).exists())
                f.copyFileTo (folder.getChildFile (f.getFileName()));
    });
    return folder;
}

juce::File FaseGroovePianoProcessor::grooveFolder()
{
    return userFolder().getChildFile ("Grooves");
}

juce::File FaseGroovePianoProcessor::clipFolder()
{
    return userFolder().getChildFile ("MIDI clips");
}

juce::Array<juce::File> FaseGroovePianoProcessor::grooveFiles()
{
    auto files = grooveFolder().findChildFiles (juce::File::findFiles, false, "*.fgg");
    files.sort();
    return files;
}

bool FaseGroovePianoProcessor::saveGroove (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty() || ! grooveFolder().createDirectory())
        return false;

    const auto s = currentSettings();
    const auto p = sgk::applySegmentOrder (sgk::GrooveEngine::basePattern (s), s.segmentOrder);

    juce::XmlElement xml ("FaseGroovePianoGroove");
    xml.setAttribute ("version", 1);
    xml.setAttribute ("name", name.trim());
    xml.setAttribute ("chords", juce::String (p.rightHand));
    xml.setAttribute ("bass", juce::String (p.leftHand));
    if (hasStepVelocities())
    {
        // Levels as heard, in the saved (A B C D) order.
        juce::StringArray velRight, velLeft;
        const int segLength = sgk::userPatternLength / sgk::numSegments;
        for (int i = 0; i < sgk::userPatternLength; ++i)
        {
            const int from = juce::jlimit (0, sgk::numSegments - 1, s.velocityOrder[(size_t) (i / segLength)]) * segLength + i % segLength;
            const int index = juce::jlimit (0, sgk::numSegments - 1, s.segmentOrder[(size_t) (from / segLength)]) * segLength + from % segLength;
            velRight.add (juce::String (userVelRight[(size_t) index].load()));
            velLeft.add (juce::String (userVelLeft[(size_t) index].load()));
        }
        xml.setAttribute ("chordLevels", velRight.joinIntoString (","));
        xml.setAttribute ("bassLevels", velLeft.joinIntoString (","));
    }
    return xml.writeTo (grooveFolder().getChildFile (clean + ".fgg"));
}

bool FaseGroovePianoProcessor::loadGroove (const juce::File& file)
{
    const auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName ("FaseGroovePianoGroove"))
        return false;

    const auto right = xml->getStringAttribute ("chords"), left = xml->getStringAttribute ("bass");
    auto valid = [] (const juce::String& line)
    {
        return line.length() == sgk::userPatternLength && line.containsOnly ("XxgUuDd>=.");
    };
    if (! valid (right) || ! valid (left))
        return false;

    sgk::GroovePattern p;
    p.rightHand = right.toStdString();
    p.leftHand = left.toStdString();
    for (int i = 0; i < sgk::numSegments; ++i)   // saved as heard: play the parts in order
        setParameterFromUI (ParamIDs::segment (i), (float) i);
    setUserPattern (p);
    auto readLevels = [&xml] (const char* id, std::array<std::atomic<int>, sgk::userPatternLength>& levels)
    {
        const auto list = juce::StringArray::fromTokens (xml->getStringAttribute (id), ",", "");
        for (int i = 0; i < sgk::userPatternLength; ++i)
            levels[(size_t) i] = list.size() == sgk::userPatternLength ? juce::jlimit (0, 127, list[i].getIntValue()) : 0;
    };
    readLevels ("chordLevels", userVelRight);
    readLevels ("bassLevels", userVelLeft);
    for (int i = 0; i < sgk::numSegments; ++i)   // saved as heard
        velocityOrder[(size_t) i] = i;
    if (left.containsAnyOf ("XxgUuDd>") && ! isBassOn())
        toggleBass();
    return true;
}

juce::Array<juce::File> FaseGroovePianoProcessor::presetFiles()
{
    auto files = presetFolder().findChildFiles (juce::File::findFiles, false, "*.fgp");
    files.sort();
    return files;
}

bool FaseGroovePianoProcessor::savePreset (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty())
        return false;

    presetFolder().createDirectory();
    juce::MemoryBlock data;
    getStateInformation (data);
    return presetFolder().getChildFile (clean + ".fgp").replaceWithData (data.getData(), data.getSize());
}

bool FaseGroovePianoProcessor::loadPreset (const juce::File& file)
{
    juce::MemoryBlock data;
    if (! file.loadFileAsData (data) || data.getSize() == 0)
        return false;

    // Only a real Fase Groove Piano state counts: a damaged or foreign file changes nothing.
    const auto xml = getXmlFromBinary (data.getData(), (int) data.getSize());
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return false;

    // A preset is music, not window layout: keep the current size.
    const auto scale = editorScale;
    const auto shrink = shrinkWhileDragging;
    const auto advanced = advancedView;
    setStateInformation (data.getData(), (int) data.getSize());
    editorScale = scale;
    shrinkWhileDragging = shrink;
    advancedView = advanced;
    return true;
}

juce::String FaseGroovePianoProcessor::snapshot()
{
    juce::MemoryBlock data;
    getStateInformation (data);
    auto xml = getXmlFromBinary (data.getData(), (int) data.getSize());
    if (xml == nullptr)
        return {};

    xml->removeAttribute ("editorScale");
    xml->removeAttribute ("shrinkWhileDragging");
    return xml->toString (juce::XmlElement::TextFormat().singleLine());
}

void FaseGroovePianoProcessor::restoreSnapshot (const juce::String& text)
{
    auto xml = juce::parseXML (text);
    if (xml == nullptr)
        return;

    xml->setAttribute ("editorScale", editorScale);
    xml->setAttribute ("shrinkWhileDragging", shrinkWhileDragging);

    juce::MemoryBlock data;
    copyXmlToBinary (*xml, data);
    setStateInformation (data.getData(), (int) data.getSize());
}

juce::StringArray FaseGroovePianoProcessor::styleNames()
{
    juce::StringArray names;
    for (const auto& s : styles)
        names.add (s.name);
    return names;
}

int FaseGroovePianoProcessor::stylePreset (int index)
{
    return juce::isPositiveAndBelow (index, (int) std::size (styles)) ? indexOfPreset (styles[index].progression) : -1;
}

void FaseGroovePianoProcessor::applyProgressionPreset (int index)
{
    const auto& presets = sgk::progressionPresets();
    if (! juce::isPositiveAndBelow (index, (int) presets.size()))
        return;

    const auto& p = presets[(size_t) index];
    const auto slots = sgk::presetSlots (p);
    clearFreeChords();
    setParameterFromUI (ParamIDs::scale, (float) p.scale);
    setParameterFromUI (ParamIDs::progLength, (float) slots.size());
    for (int i = 0; i < (int) slots.size() && i < sgk::maxProgressionLength; ++i)
    {
        setParameterFromUI (ParamIDs::slot (i), (float) slots[(size_t) i].degree);
        setFreeChord (i, slots[(size_t) i].freeCode);
    }
}

sgk::HarmonyAnalysis FaseGroovePianoProcessor::analyzeMidiFile (const juce::File& file)
{
    juce::MidiFile midi;
    juce::FileInputStream in (file);
    if (! in.openedOk() || ! midi.readFrom (in) || midi.getTimeFormat() <= 0)
        return {};

    const double ticksPerBeat = midi.getTimeFormat();
    int beatsPerBar = 4;
    std::vector<sgk::AnalyzedNote> notes;

    for (int t = 0; t < midi.getNumTracks(); ++t)
    {
        juce::MidiMessageSequence track (*midi.getTrack (t));
        track.updateMatchedPairs();
        for (auto* ev : track)
        {
            const auto& m = ev->message;
            if (m.isTimeSignatureMetaEvent() && m.getTimeStamp() <= 0.0)
            {
                int numerator = 4, denominator = 4;
                m.getTimeSignatureInfo (numerator, denominator);
                if (denominator > 0)
                    beatsPerBar = juce::jlimit (2, 12, juce::roundToInt (numerator * 4.0 / denominator));
            }
            if (! m.isNoteOn() || m.getChannel() == 10)   // channel 10: drums
                continue;
            const double start = m.getTimeStamp() / ticksPerBeat;
            const double end = ev->noteOffObject != nullptr ? ev->noteOffObject->message.getTimeStamp() / ticksPerBeat : start + 0.25;
            notes.push_back ({ start, juce::jmax (start + 0.05, end), m.getNoteNumber(), (int) m.getVelocity() });
            if (notes.size() > 20000)
                return {};   // not a loop
        }
    }
    auto analysis = sgk::analyzeHarmony (notes, beatsPerBar);
    analysis.groove = sgk::grooveFromNotes (notes, beatsPerBar);
    return analysis;
}

void FaseGroovePianoProcessor::applyGroove (const sgk::GroovePattern& groove)
{
    if (groove.length() != sgk::userPatternLength)
        return;

    // The rhythm becomes the user pattern, played in order; with a bass rhythm in it the bass plays
    // that rhythm (piano-stab bass on the bass row).
    for (int i = 0; i < sgk::numSegments; ++i)
        setParameterFromUI (ParamIDs::segment (i), (float) i);
    setUserPattern (groove);
    resetStepVelocities();
    if (groove.leftHand.find_first_of ("XxgUuDd>") != std::string::npos)
        setParameterFromUI (ParamIDs::bass, (float) sgk::stabsOwnRhythmMode);
}

void FaseGroovePianoProcessor::applyAnalysis (const sgk::HarmonyAnalysis& a)
{
    if (! a.ok || a.slots.empty())
        return;

    clearFreeChords();
    setParameterFromUI (ParamIDs::key, (float) a.keyRoot);
    setParameterFromUI (ParamIDs::scale, (float) a.scale);
    setParameterFromUI (ParamIDs::chordLength, (float) juce::jlimit (0, 3, a.chordLengthChoice));
    setParameterFromUI (ParamIDs::progLength, (float) juce::jmin ((int) a.slots.size(), sgk::maxProgressionLength));
    for (int i = 0; i < (int) a.slots.size() && i < sgk::maxProgressionLength; ++i)
    {
        setParameterFromUI (ParamIDs::slot (i), (float) a.slots[(size_t) i].degree);
        setFreeChord (i, a.slots[(size_t) i].freeCode);
    }
}

void FaseGroovePianoProcessor::applyStyle (int index)
{
    if (! juce::isPositiveAndBelow (index, (int) std::size (styles)))
        return;

    const auto& st = styles[index];
    clearFreeChords();

    applyProgressionPreset (indexOfPreset (st.progression));

    setParameterFromUI (ParamIDs::pattern, (float) indexOfGroove (st.groove));
    setParameterFromUI (ParamIDs::extension, (float) st.extension);
    setParameterFromUI (ParamIDs::voicing, (float) st.voicing);
    setParameterFromUI (ParamIDs::bass, (float) st.bass);
    setParameterFromUI (ParamIDs::approach, (float) st.approach);
    setParameterFromUI (ParamIDs::topLine, (float) st.topLine);
    setParameterFromUI (ParamIDs::soundType, (float) st.sound);
    setParameterFromUI (ParamIDs::registerPos, (float) st.registerNote);
    setParameterFromUI (ParamIDs::swing, st.swing);
    setParameterFromUI (ParamIDs::gate, st.gate);
    setParameterFromUI (ParamIDs::humanize, st.humanize);
    setParameterFromUI (ParamIDs::strum, st.strum);
    setParameterFromUI (ParamIDs::octaveTop, st.octaveTop ? 1.0f : 0.0f);
    setParameterFromUI (ParamIDs::section, 0.0f);
    {
        const juce::String name (st.name);
        // House piano keeps one voicing per chord; only the jackin' piano style dances.
        setParameterFromUI (ParamIDs::movement, name == "Jackin' Piano" ? 2.0f : 0.0f);
    }
    setParameterFromUI (ParamIDs::fills, (float) st.fills);
    setParameterFromUI (ParamIDs::fillStyle, (float) st.fillStyle);
    for (int i = 0; i < sgk::numSegments; ++i)
        setParameterFromUI (ParamIDs::segment (i), (float) i);
}

void FaseGroovePianoProcessor::totalRandom()
{
    auto& r = juce::Random::getSystemRandom();
    auto chance = [&r] (int percent) { return r.nextInt (100) < percent; };

    // A whole style first (harmony, groove, sound and feel that belong together)...
    applyStyle (r.nextInt (styleNames().size()));

    // ...then shake it up.
    setParameterFromUI (ParamIDs::key, (float) r.nextInt (12));
    if (chance (50))
        applyProgressionPreset (r.nextInt ((int) sgk::progressionPresets().size()));
    else
        rollNewProgression();

    if (chance (60))
        setParameterFromUI (ParamIDs::pattern, (float) r.nextInt ((int) sgk::groovePatterns().size()));
    else
        rollNewRiff();

    if (chance (40)) setParameterFromUI (ParamIDs::extension, (float) r.nextInt (sgk::numExtensions));
    if (chance (40)) setParameterFromUI (ParamIDs::voicing, (float) r.nextInt (sgk::numVoicings));
    if (chance (35)) setParameterFromUI (ParamIDs::bass, (float) (1 + r.nextInt (sgk::numBassModes - 1)));
    if (chance (30)) setParameterFromUI (ParamIDs::soundType, (float) r.nextInt (numGuideSounds));
    if (chance (35)) setParameterFromUI (ParamIDs::strings, (float) r.nextInt (sgk::numStringsModes));
    if (chance (30)) setParameterFromUI (ParamIDs::fills, (float) r.nextInt (4));
    setParameterFromUI (ParamIDs::swing, 0.05f + 0.35f * r.nextFloat());
    setParameterFromUI (ParamIDs::humanize, 0.1f + 0.35f * r.nextFloat());
}

float FaseGroovePianoProcessor::raw (const char* id) const
{
    return apvts.getRawParameterValue (id)->load();
}

sgk::GrooveSettings FaseGroovePianoProcessor::currentSettings() const
{
    sgk::GrooveSettings s;
    auto asInt = [this] (const char* id) { return juce::roundToInt (raw (id)); };

    s.keyRoot = asInt (ParamIDs::key);
    s.scale = asInt (ParamIDs::scale);
    s.extension = (sgk::Extension) asInt (ParamIDs::extension);
    s.voicing = (sgk::Voicing) asInt (ParamIDs::voicing);
    s.centre = asInt (ParamIDs::registerPos);
    s.rangeLow = asInt (ParamIDs::chordLow);
    s.rangeHigh = juce::jmax (asInt (ParamIDs::chordHigh), s.rangeLow + 12);   // at least an octave
    s.bass = (sgk::BassMode) asInt (ParamIDs::bass);
    s.strings = (sgk::StringsMode) juce::jlimit (0, sgk::numStringsModes - 1, asInt (ParamIDs::strings));

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        s.progression[(size_t) i] = juce::roundToInt (slotParams[(size_t) i]->load());

    s.progressionLength = asInt (ParamIDs::progLength);
    s.stepsPerChord = stepsPerChordChoices[juce::jlimit (0, 3, asInt (ParamIDs::chordLength))];

    s.pattern = asInt (ParamIDs::pattern);
    s.swing = raw (ParamIDs::swing);
    s.gate = raw (ParamIDs::gate);
    s.velocity = asInt (ParamIDs::velocity);
    s.humanize = raw (ParamIDs::humanize);
    s.fillEveryBars = fillChoices[juce::jlimit (0, 4, asInt (ParamIDs::fills))];
    s.fillStyle = (sgk::FillStyle) juce::jlimit (0, sgk::numFillStyles - 1, asInt (ParamIDs::fillStyle));
    s.strum = raw (ParamIDs::strum);
    s.octaveTop = raw (ParamIDs::octaveTop) > 0.5f;
    s.approach = (sgk::ApproachStyle) juce::jlimit (0, sgk::numApproachStyles - 1, asInt (ParamIDs::approach));
    s.topLine = (sgk::TopLine) juce::jlimit (0, sgk::numTopLines - 1, asInt (ParamIDs::topLine));
    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        s.inversions[(size_t) i] = juce::roundToInt (inversionParams[(size_t) i]->load());
    {
        constexpr int every[] { 0, 4, 8, 0, 0 };
        constexpr int evolve[] { 0, 0, 0, 8, 16 };
        constexpr int bars[] { 2, 1, 4 };
        const int variationChoice = juce::jlimit (0, 4, asInt (ParamIDs::variation));
        s.variationEvery = every[variationChoice];
        s.evolveBars = evolve[variationChoice];
        s.grooveBars = bars[juce::jlimit (0, 2, asInt (ParamIDs::grooveBars))];
        s.movement = juce::jlimit (0, 2, asInt (ParamIDs::movement));
        s.strumDirection = juce::jlimit (0, 2, asInt (ParamIDs::strumDir));
        s.humanizeStyle = juce::jlimit (0, sgk::numHumanizeStyles - 1, asInt (ParamIDs::humanizeStyle));
        s.humanizeSeed = (std::uint32_t) juce::jmax (0, asInt (ParamIDs::humanizeSeed));
        s.bpm = lastBpm.load();
    }
    s.section = (sgk::Section) juce::jlimit (0, sgk::numSections - 1, asInt (ParamIDs::section));
    s.riffSeed = (std::uint32_t) asInt (ParamIDs::riffSeed);
    s.riffDensity = raw (ParamIDs::riffDensity);
    s.editedLoop = editedLoop();
    s.editedLoopLength = editedLength.load();

    for (int i = 0; i < sgk::numSegments; ++i)
        s.segmentOrder[(size_t) i] = juce::jlimit (0, sgk::numSegments - 1, juce::roundToInt (segmentParams[(size_t) i]->load()));

    // The section playing now uses its own variation, if it was regenerated in the song.
    for (int i = 0; i < numSongParts; ++i)
        if (sgk::defaultArrangement()[(size_t) i].section == s.section)
        {
            s.sectionVariation = songVariation[(size_t) i].load();

            // Its own fill, so you hear it while that section loops.
            const int fill = songFill[(size_t) i].load();
            if (fill == -2)
                s.fillEveryBars = 0;
            else if (fill >= 0)
            {
                s.fillStyle = (sgk::FillStyle) juce::jlimit (0, sgk::numFillStyles - 1, fill);
                if (s.fillEveryBars == 0)
                    s.fillEveryBars = 2;
            }
            break;
        }

    for (int i = 0; i < sgk::userPatternLength; ++i)
    {
        s.userRight[(size_t) i] = userRight[(size_t) i].load();
        s.userLeft[(size_t) i]  = userLeft[(size_t) i].load();
        s.userVelRight[(size_t) i] = (unsigned char) userVelRight[(size_t) i].load();
        s.userVelLeft[(size_t) i]  = (unsigned char) userVelLeft[(size_t) i].load();
    }
    for (int i = 0; i < sgk::numSegments; ++i)
        s.velocityOrder[(size_t) i] = velocityOrder[(size_t) i].load();

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        s.freeChords[(size_t) i] = freeChords[(size_t) i].load();

    return s;
}

void FaseGroovePianoProcessor::setParameterFromUI (const juce::String& id, float plainValue)
{
    if (auto* p = dynamic_cast<juce::RangedAudioParameter*> (apvts.getParameter (id)))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
        p->endChangeGesture();
    }
}

void FaseGroovePianoProcessor::rollNewRiff()
{
    const auto seed = juce::Random::getSystemRandom().nextInt ({ 1, 100000 });
    const auto s = currentSettings();
    auto current = sgk::GrooveEngine::basePattern (s);

    const bool anyLocked = std::any_of (segmentLocked.begin(), segmentLocked.end(), [] (bool b) { return b; });

    if (! anyLocked || current.length() != sgk::userPatternLength)
    {
        setParameterFromUI (ParamIDs::riffSeed, (float) seed);
        setParameterFromUI (ParamIDs::pattern, (float) sgk::randomPatternIndex());
        return;
    }

    const auto fresh = sgk::makeRandomPattern ((std::uint32_t) seed, s.riffDensity);
    for (int seg = 0; seg < sgk::numSegments; ++seg)
        if (! segmentLocked[(size_t) seg])
        {
            sgk::copySegment (current, fresh, seg);
            resetSegmentVelocities (seg, true, true);
        }

    setUserPattern (current);
}

void FaseGroovePianoProcessor::rerollHand (bool bass)
{
    const auto s = currentSettings();

    // With a bass line from the library, RND BASS picks another line.
    if (bass && sgk::isBassLine (s.bass))
    {
        const int current = (int) s.bass - sgk::numBassPatternModes;
        const int next = (current + 1 + juce::Random::getSystemRandom().nextInt (sgk::numBassLines - 1)) % sgk::numBassLines;
        setParameterFromUI (ParamIDs::bass, (float) (sgk::numBassPatternModes + next));
        return;
    }

    auto current = sgk::GrooveEngine::basePattern (s);
    if (current.length() != sgk::userPatternLength)
        return;

    // With the "Piano stabs" bass the bass follows the piano's stabs: RND PIANO would change it
    // too. Keep it: write the rhythm it plays now on the bass row and let the bass follow that row.
    if (! bass && s.bass == sgk::BassMode::withChords)
    {
        current.leftHand = sgk::stabBassLine (current);
        setParameterFromUI (ParamIDs::bass, (float) sgk::stabsOwnRhythmMode);
    }

    const auto seed = (std::uint32_t) juce::Random::getSystemRandom().nextInt ({ 1, 100000 });
    const auto fresh = sgk::makeRandomPattern (seed, s.riffDensity);
    const int segLength = sgk::userPatternLength / sgk::numSegments;
    for (int seg = 0; seg < sgk::numSegments; ++seg)
    {
        if (segmentLocked[(size_t) seg])
            continue;
        resetSegmentVelocities (seg, ! bass, bass);
        for (int i = seg * segLength; i < (seg + 1) * segLength; ++i)
        {
            if (bass) current.leftHand[(size_t) i]  = fresh.leftHand[(size_t) i];
            else      current.rightHand[(size_t) i] = fresh.rightHand[(size_t) i];
        }
    }
    sgk::repairHolds (bass ? current.leftHand : current.rightHand);

    if (bass && ! isBassOn())
        toggleBass();
    setUserPattern (current);
}

void FaseGroovePianoProcessor::randomizeSegment (int segment)
{
    const auto s = currentSettings();
    auto current = sgk::GrooveEngine::basePattern (s);
    if (current.length() != sgk::userPatternLength)
        return;

    const auto seed = (std::uint32_t) juce::Random::getSystemRandom().nextInt ({ 1, 100000 });
    sgk::copySegment (current, sgk::makeRandomPattern (seed, s.riffDensity), segment);
    resetSegmentVelocities (segment, true, true);
    setUserPattern (current);
}

void FaseGroovePianoProcessor::setUserPattern (const sgk::GroovePattern& p)
{
    // Levels set on an earlier user groove don't belong to a groove taken from the library.
    if (juce::roundToInt (raw (ParamIDs::pattern)) != sgk::userPatternIndex())
        resetStepVelocities();

    for (int i = 0; i < sgk::userPatternLength; ++i)
    {
        userRight[(size_t) i] = i < p.length() ? p.rightHand[(size_t) i] : '.';
        userLeft[(size_t) i]  = i < p.length() ? p.leftHand[(size_t) i]  : '.';
    }

    if (juce::roundToInt (raw (ParamIDs::pattern)) != sgk::userPatternIndex())
        setParameterFromUI (ParamIDs::pattern, (float) sgk::userPatternIndex());
}

int FaseGroovePianoProcessor::segmentAtStep (int displayedStep) const
{
    const int pos = juce::jlimit (0, sgk::numSegments - 1, displayedStep / (sgk::userPatternLength / sgk::numSegments));
    return juce::jlimit (0, sgk::numSegments - 1, juce::roundToInt (segmentParams[(size_t) pos]->load()));
}

char FaseGroovePianoProcessor::stepSymbol (bool bassRow, int displayedStep) const
{
    const auto p = sgk::GrooveEngine::resolvePattern (currentSettings());
    const auto& line = bassRow ? p.leftHand : p.rightHand;
    return displayedStep >= 0 && displayedStep < (int) line.size() ? line[(size_t) displayedStep] : '.';
}

void FaseGroovePianoProcessor::editStep (bool bassRow, int displayedStep, char symbol)
{
    auto current = sgk::GrooveEngine::basePattern (currentSettings());
    if (displayedStep >= sgk::userPatternLength)
        displayedStep %= sgk::userPatternLength;   // 4-bar grooves: bars 3-4 vary bars 1-2
    if (current.length() != sgk::userPatternLength || displayedStep < 0)
        return;

    // Map the displayed step back through the segment order.
    const int segLength = sgk::userPatternLength / sgk::numSegments;
    const int index = segmentAtStep (displayedStep) * segLength + displayedStep % segLength;

    // Editing the bass of a library line: the line's rhythm becomes yours, played on the roots.
    if (bassRow && sgk::isBassLine (currentSettings().bass))
    {
        const auto s = currentSettings();
        current.leftHand = sgk::bassLineRhythm ((int) s.bass - sgk::numBassPatternModes, sgk::userPatternLength);
        setParameterFromUI (ParamIDs::bass, (float) sgk::BassMode::root);
    }

    auto& line = bassRow ? current.leftHand : current.rightHand;
    line[(size_t) index] = symbol;
    sgk::repairHolds (line);

    if (bassRow && symbol != '.' && ! isBassOn())
        toggleBass();

    setUserPattern (current);
}

bool FaseGroovePianoProcessor::isBassOn() const
{
    return juce::roundToInt (raw (ParamIDs::bass)) != (int) sgk::BassMode::off;
}

void FaseGroovePianoProcessor::toggleBass()
{
    const int mode = juce::roundToInt (raw (ParamIDs::bass));
    if (mode != (int) sgk::BassMode::off)
    {
        lastBassMode = mode;
        setParameterFromUI (ParamIDs::bass, 0.0f);
    }
    else
    {
        setParameterFromUI (ParamIDs::bass, (float) juce::jmax (1, lastBassMode));
    }
}

bool FaseGroovePianoProcessor::isStringsOn() const
{
    return juce::roundToInt (raw (ParamIDs::strings)) != (int) sgk::StringsMode::off;
}

void FaseGroovePianoProcessor::toggleStrings()
{
    const int mode = juce::roundToInt (raw (ParamIDs::strings));
    if (mode != (int) sgk::StringsMode::off)
    {
        lastStringsMode = mode;
        setParameterFromUI (ParamIDs::strings, 0.0f);
    }
    else
    {
        setParameterFromUI (ParamIDs::strings, (float) juce::jmax (1, lastStringsMode));
    }
}

juce::String FaseGroovePianoProcessor::instrumentName (ClipPart part)
{
    return part == ClipPart::bass ? "Bass" : part == ClipPart::strings ? "Strings" : "Piano";
}

void FaseGroovePianoProcessor::setFreeChord (int slot, int code)
{
    if (slot >= 0 && slot < sgk::maxProgressionLength)
        freeChords[(size_t) slot] = (code >= 0 && code < sgk::numFreeChordCodes) ? code : sgk::noFreeChord;
}

void FaseGroovePianoProcessor::clearFreeChords()
{
    for (auto& f : freeChords)
        f = sgk::noFreeChord;
}

void FaseGroovePianoProcessor::rollNewProgression()
{
    clearFreeChords();
    const int length = juce::roundToInt (raw (ParamIDs::progLength));
    const auto degrees = sgk::randomProgression ((std::uint32_t) juce::Random::getSystemRandom().nextInt(), length);

    for (int i = 0; i < (int) degrees.size(); ++i)
        setParameterFromUI (ParamIDs::slot (i), (float) degrees[(size_t) i]);
}

//==============================================================================
void FaseGroovePianoProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    reverb.setSampleRate (sampleRate);
    reverb.reset();
    chorusFx.prepare ({ sampleRate, (juce::uint32) juce::jmax (1, samplesPerBlock), 2 });
    chorusFx.reset();
    outputGain.reset (sampleRate, 0.05);
    outputGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (raw (ParamIDs::volume)));
    sidechainLevel.assign ((size_t) juce::jmax (1, samplesPerBlock), 0.0f);
    lastOutput.assign ((size_t) juce::jmax (1, samplesPerBlock), 0.0f);
    sidechainEnvelope = 0.0f;

    wasRunning = false;
    heldCount = 0;
}

bool FaseGroovePianoProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    auto monoOrStereo = [] (const juce::AudioChannelSet& set)
    {
        return set == juce::AudioChannelSet::mono() || set == juce::AudioChannelSet::stereo();
    };
    if (! monoOrStereo (layouts.getMainOutputChannelSet()))
        return false;

    // The inputs are optional: nothing plays through the main one, the sidechain only drives the pump.
    for (const auto& in : layouts.inputBuses)
        if (! in.isDisabled() && ! monoOrStereo (in))
            return false;
    return true;
}

void FaseGroovePianoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const double sampleRate = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;

    // Sidechain input: follow its level before the buffer is used for the output.
    const bool externalPump = juce::roundToInt (raw (ParamIDs::pumpSource)) == 1;
    if (externalPump)   // (only then: the meter is shown only then too)
    {
        if ((int) sidechainLevel.size() < numSamples)
            sidechainLevel.resize ((size_t) numSamples);

        auto side = juce::AudioBuffer<float>();
        if (auto* bus = getBus (true, 0); bus != nullptr && bus->isEnabled())
            side = getBusBuffer (buffer, true, 0);

        // With nothing routed some hosts hand back the plug-in's own last output: not a sidechain.
        // (Compared within a tiny tolerance: a host may round or dither what it hands back.)
        if (side.getNumChannels() > 0 && numSamples <= (int) lastOutput.size()
            && std::any_of (lastOutput.begin(), lastOutput.begin() + numSamples, [] (float v) { return v != 0.0f; }))
        {
            const float* in = side.getReadPointer (0);
            bool same = true;
            for (int i = 0; i < numSamples && same; ++i)
                same = std::abs (in[i] - lastOutput[(size_t) i]) <= 1.0e-4f + 1.0e-3f * std::abs (lastOutput[(size_t) i]);
            if (same)
                side = juce::AudioBuffer<float>();
        }

        float blockPeak = 0.0f;
        const float attack = 1.0f - std::exp (-1.0f / (0.002f * (float) sampleRate));
        const float releaseSeconds = 0.04f * std::pow (15.0f, raw (ParamIDs::pumpRelease));   // 40 ms .. 600 ms
        const float release = 1.0f - std::exp (-1.0f / (releaseSeconds * (float) sampleRate));
        for (int i = 0; i < numSamples; ++i)
        {
            float peak = 0.0f;
            for (int ch = 0; ch < side.getNumChannels(); ++ch)
                peak = juce::jmax (peak, std::abs (side.getSample (ch, i)));
            blockPeak = juce::jmax (blockPeak, peak);
            sidechainEnvelope += (peak - sidechainEnvelope) * (peak > sidechainEnvelope ? attack : release);
            sidechainLevel[(size_t) i] = sidechainEnvelope;
        }
        sidechainMeter = juce::jmax (blockPeak, sidechainMeter.load() * 0.8f);   // for the meter in the UI
    }
    buffer.clear();

    auto settings = currentSettings();

    // Sequencer off: the groove doesn't play with the DAW (e.g. the dragged clip plays on this
    // track); incoming MIDI still passes through and plays the guide sound. ▶ still previews.
    const bool sequencerOn = raw (ParamIDs::sequencer) > 0.5f;
    const bool holdMode = sequencerOn && juce::roundToInt (raw (ParamIDs::playMode)) == 1;

    // ---- MIDI in: in hold mode keys pick the root, otherwise they pass through ----
    outMidi.clear();
    const double clockPpqAtStart = clockPpq;
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();

        // MIDI clock: 24 ticks per beat give the tempo; start / continue / stop and the song
        // position drive the transport (they are not passed on).
        if (m.isMidiClock())
        {
            const auto at = samplesProcessed + meta.samplePosition;
            if (lastClockSample >= 0)
                if (const double gap = (double) (at - lastClockSample); gap > 0.0 && gap < sampleRate * 0.25)
                    clockInterval = clockInterval <= 0.0 ? gap : clockInterval * 0.92 + gap * 0.08;
            lastClockSample = at;
            if (clockRunning)
                clockPpq += 1.0 / 24.0;
            continue;
        }
        if (m.isMidiStart())             { clockRunning = true; clockPpq = 0.0; continue; }
        if (m.isMidiContinue())          { clockRunning = true; continue; }
        if (m.isMidiStop())              { clockRunning = false; continue; }
        if (m.isSongPositionPointer())   { clockPpq = m.getSongPositionPointerMidiBeat() / 4.0; continue; }

        // Live keys: the lowest octave plays commands, not notes.
        if (liveKeysChoice.load() == 1 && m.isNoteOnOrOff()
            && m.getNoteNumber() >= liveKeysLowest && m.getNoteNumber() < liveKeysLowest + 12)
        {
            if (m.isNoteOn())
            {
                const int key = m.getNoteNumber() - liveKeysLowest;
                if (key == 1)
                    liveFillRequested = true;
                else if (liveKeyFifo.getFreeSpace() > 0)
                {
                    const auto scope = liveKeyFifo.write (1);
                    if (scope.blockSize1 > 0) liveKeyBuffer[(size_t) scope.startIndex1] = key;
                    else if (scope.blockSize2 > 0) liveKeyBuffer[(size_t) scope.startIndex2] = key;
                }
            }
            continue;
        }

        if (learnMode.load() && m.isNoteOnOrOff())
        {
            learnDown[(size_t) m.getNoteNumber()] = m.isNoteOn();

            int mask = 0, count = 0, lowest = -1, down = 0;
            for (int n = 0; n < 128; ++n)
                if (learnDown[(size_t) n])
                {
                    ++down;
                    if (lowest < 0) lowest = n;
                    if ((mask & (1 << (n % 12))) == 0) ++count;
                    mask |= 1 << (n % 12);
                }

            // Remember the fullest chord of this press; it counts once all keys are up.
            if (count > learnPeakCount)
            {
                learnPeakCount = count;
                learnPeakMask = mask;
                learnPeakLowest = lowest % 12;
            }

            if (down == 0)
            {
                if (learnPeakCount >= 3)
                {
                    learnedMask = learnPeakMask;
                    learnedBass = learnPeakLowest;
                    ++learnedCount;
                }
                learnPeakCount = 0;
                learnPeakMask = 0;
            }
            continue;   // learning: the keys only teach chords
        }

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
    double dawBpm = 120.0, hostPpq = 0.0;
    bool hostPlaying = false, haveHostPpq = false;

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm())          dawBpm = *b;
            if (auto p = pos->getPpqPosition()) { hostPpq = *p; haveHostPpq = true; }
            hostPlaying = pos->getIsPlaying();
        }
    }

    // No position from a host (the standalone app): a MIDI clock, when one arrives, is the DAW.
    const bool clockAlive = lastClockSample >= 0 && clockInterval > 0.0
                         && (double) (samplesProcessed - lastClockSample) < sampleRate * 0.5;
    const bool useClock = ! haveHostPpq && clockAlive;
    if (useClock)
    {
        dawBpm = juce::roundToInt (600.0 * sampleRate / (24.0 * clockInterval)) / 10.0;
        hostPlaying = clockRunning;
        hostPpq = clockPpqAtStart;
        haveHostPpq = clockRunning;
    }
    clockSync = useClock;

    // Synced: the DAW's tempo and position. Unsynced: our own BPM and clock,
    // still started/stopped by the DAW's play button.
    const bool tempoSynced = raw (ParamIDs::tempoSync) > 0.5f;
    hostBpm = juce::jlimit (20.0, 400.0, dawBpm);

    const double bpm = tempoSynced ? hostBpm.load() : juce::jlimit (20.0, 400.0, (double) raw (ParamIDs::tempo));
    lastBpm = bpm;
    const double samplesPerBeat = sampleRate * 60.0 / bpm;
    settings.bpm = bpm;
    const double blockBeats = numSamples / samplesPerBeat;

    const bool followHost = tempoSynced && hostPlaying && haveHostPpq;
    const bool preview = previewOn.load();

    // Hold mode plays while a key is down; otherwise it follows the host, and
    // Preview lets the groove run on its own clock while the host is stopped.
    const bool running = holdMode ? (heldCount > 0 || preview) : ((sequencerOn && hostPlaying) || preview);

    double startPpq = 0.0;
    if (followHost)
        startPpq = hostPpq;
    else if (running)
    {
        // Free-running clock: from the start, or where PAUSE left it; REWIND jumps back.
        startPpq = wasRunning || ! previewFromStart.load() ? internalPpq : 0.0;
        if (previewRewind.exchange (false))
            startPpq = 0.0;
        if (! wasRunning)
            previewFromStart = true;   // a resume happens once; the next start is from the top again
    }
    else
        previewRewind = false;

    events.clear();

    // Live key C#: a fill in the next bar (in the fill style chosen, or one that suits the groove).
    if (liveFillRequested.exchange (false) && running)
        liveFillBar = (long long) std::floor (startPpq / 4.0) + 1;
    if (liveFillBar >= 0)
    {
        if (std::floor (startPpq / 4.0) > (double) liveFillBar)
            liveFillBar = -1;
        else
        {
            settings.forcedFillBar = liveFillBar;
            settings.forcedFillStyle = juce::roundToInt (raw (ParamIDs::fills)) == 0 ? (int) sgk::FillStyle::mix
                                                                                        : (int) settings.fillStyle;
        }
    }

    if (running)
    {
        if (! wasRunning || std::abs (startPpq - expectedNextPpq) > 0.05)
            engine.allNotesOff (startPpq, events);   // start, loop or jump

        engine.render (startPpq, startPpq + blockBeats, settings, events);
        expectedNextPpq = internalPpq = startPpq + blockBeats;

        const auto step = (long long) std::floor (startPpq / sgk::stepLength);
        const int patternLength = sgk::GrooveEngine::patternLength (settings);
        uiStep = (int) (((step % patternLength) + patternLength) % patternLength);
        uiSlot = sgk::GrooveEngine::slotForStep (step, settings);
        uiPpq = startPpq;
    }
    else
    {
        if (engine.anyNoteSounding())
            engine.allNotesOff (startPpq, events);

        uiStep = -1;
        uiSlot = -1;
        uiPpq = -1.0;
    }

    wasRunning = running;

    const bool bassOnChannel2 = juce::roundToInt (raw (ParamIDs::bassChannel)) == 1;

    const bool muteChords = chordsMuted.load();
    for (const auto& e : events)
    {
        if (muteChords && e.isNoteOn() && e.part != sgk::Part::bass && e.part != sgk::Part::strings)
            continue;   // the piano muted (note-offs still pass, so nothing hangs)

        const int pos = juce::jlimit (0, numSamples - 1, (int) std::floor ((e.ppq - startPpq) * samplesPerBeat));
        const int channel = e.part == sgk::Part::strings ? stringsChannel : (bassOnChannel2 && e.part == sgk::Part::bass) ? 2 : 1;
        outMidi.addEvent (e.isNoteOn() ? juce::MidiMessage::noteOn (channel, e.note, (juce::uint8) e.velocity)
                                       : juce::MidiMessage::noteOff (channel, e.note),
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

        // Chorus: a gentle stereo widening for keys.
        if (const float chorusAmount = raw (ParamIDs::chorus); chorusAmount > 0.001f && buffer.getNumChannels() > 1)
        {
            chorusFx.setRate (0.6f);
            chorusFx.setDepth (0.25f + 0.35f * chorusAmount);
            chorusFx.setCentreDelay (8.0f);
            chorusFx.setFeedback (0.0f);
            chorusFx.setMix (0.5f * chorusAmount);
            juce::dsp::AudioBlock<float> block (buffer);
            chorusFx.process (juce::dsp::ProcessContextReplacing<float> (block));
        }

        // Pump: the classic house sidechain duck. Internal: on the tempo (every beat, 8th, ...)
        // while the groove runs. External: following the track sent to the sidechain input.
        // (With the sequencer off it still pumps on the DAW's beat, for the clip playing on this track.)
        const double pumpPpq = running ? startPpq : hostPpq;
        if (const float pumpAmount = raw (ParamIDs::pump); pumpAmount > 0.001f && (externalPump || running || hostPlaying))
        {
            static constexpr double cycleBeats[] = { 1.0, 0.5, 2.0, 0.25 };
            const double cycle = cycleBeats[juce::jlimit (0, 3, juce::roundToInt (raw (ParamIDs::pumpRate)))];
            const double recoverTime = 0.15 + 0.8 * raw (ParamIDs::pumpRelease);   // part of the cycle
            for (int i = 0; i < numSamples; ++i)
            {
                float duck = 0.0f;
                if (externalPump)
                {
                    // -42 dB (nothing) .. -6 dB (full duck); no log below -42 dB (most of the time)
                    if (const float level = sidechainLevel[(size_t) i]; level > 0.00794f)
                        duck = 0.85f * juce::jmin (1.0f, (20.0f * std::log10 (level) + 42.0f) / 36.0f);
                }
                else
                {
                    const double beat = (pumpPpq + i / samplesPerBeat) / cycle;
                    const double recover = juce::jmin (1.0, (beat - std::floor (beat)) / recoverTime);
                    duck = 0.8f * (float) ((1.0 - recover) * (1.0 - recover));
                }
                const float g = 1.0f - pumpAmount * duck;
                for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                    buffer.getWritePointer (ch)[i] *= g;
            }
        }

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

    // Remember what went out (see the sidechain check above).
    if (numSamples <= (int) lastOutput.size())
    {
        if (buffer.getNumChannels() > 0)
            std::copy (buffer.getReadPointer (0), buffer.getReadPointer (0) + numSamples, lastOutput.begin());
        else
            std::fill (lastOutput.begin(), lastOutput.begin() + numSamples, 0.0f);
    }

    samplesProcessed += numSamples;
    midi.swapWith (outMidi);
}

//==============================================================================
std::vector<sgk::ArrangementPart> FaseGroovePianoProcessor::songArrangementParts() const
{
    std::vector<sgk::ArrangementPart> parts;
    for (int i = 0; i < numSongParts; ++i)
        parts.push_back ({ sgk::defaultArrangement()[(size_t) i].section, songBars[(size_t) i].load(), songVariation[(size_t) i].load(),
                           songFill[(size_t) i].load() });
    return parts;
}

void FaseGroovePianoProcessor::setSongBars (int part, int bars)
{
    if (juce::isPositiveAndBelow (part, numSongParts))
        songBars[(size_t) part] = juce::jlimit (0, 64, bars);
}

void FaseGroovePianoProcessor::setSongFill (int part, int fill)
{
    if (juce::isPositiveAndBelow (part, numSongParts))
        songFill[(size_t) part] = juce::jlimit (-2, sgk::numFillStyles - 1, fill);
}

void FaseGroovePianoProcessor::regenerateSongPart (int part)
{
    if (juce::isPositiveAndBelow (part, numSongParts))
        songVariation[(size_t) part] = (std::uint32_t) juce::Random::getSystemRandom().nextInt ({ 1, 1 << 30 });
}

void FaseGroovePianoProcessor::resetSongPart (int part)
{
    if (juce::isPositiveAndBelow (part, numSongParts))
        songVariation[(size_t) part] = 0;
}

void FaseGroovePianoProcessor::resetSong()
{
    for (int i = 0; i < numSongParts; ++i)
    {
        songBars[(size_t) i] = sgk::defaultArrangement()[(size_t) i].bars;
        songVariation[(size_t) i] = 0;
        songFill[(size_t) i] = -1;
    }
}

int FaseGroovePianoProcessor::saveSongSections (const juce::File& folder, ClipPart part) const
{
    if (! folder.createDirectory())
        return 0;

    const auto settings = currentSettings();
    const auto parts = songArrangementParts();
    int written = 0;
    for (int i = 0; i < (int) parts.size(); ++i)
    {
        if (parts[(size_t) i].bars <= 0)
            continue;
        std::vector<sgk::GrooveEngine::Marker> markers;
        const auto events = sgk::GrooveEngine::renderArrangement (settings, { parts[(size_t) i] }, &markers);
        const auto name = juce::String (i + 1) + " " + sgk::sectionName ((int) parts[(size_t) i].section);
        const auto midi = midiFromEvents (events, parts[(size_t) i].bars * 4.0, &markers, part, name);
        auto file = folder.getChildFile (juce::File::createLegalFileName (name + " - " + clipFileName (parts[(size_t) i].bars, part)));
        file.deleteFile();
        juce::FileOutputStream out (file);
        if (out.openedOk() && midi.writeTo (out))
            ++written;
    }
    return written;
}

void FaseGroovePianoProcessor::applyIntention (int intention, bool keepRhythm, bool keepHarmony)
{
    const auto which = (sgk::Intention) juce::jlimit (0, sgk::numIntentions - 1, intention);
    const auto s = currentSettings();
    auto nudge = [this] (const char* id, float delta, float lo, float hi)
    {
        setParameterFromUI (id, juce::jlimit (lo, hi, raw (id) + delta));
    };

    if (! keepRhythm)
    {
        const auto source = sgk::applySegmentOrder (sgk::GrooveEngine::basePattern (s), s.segmentOrder);
        if (source.length() == sgk::userPatternLength)
        {
            auto result = sgk::applyIntention (source, which, (std::uint32_t) juce::Random::getSystemRandom().nextInt ({ 1, 1 << 30 }));
            for (int seg = 0; seg < sgk::numSegments; ++seg)
                if (segmentLocked[(size_t) seg])
                    sgk::copySegment (result, source, seg);   // locked parts stay
            sgk::repairHolds (result.rightHand);
            sgk::repairHolds (result.leftHand);
            for (int i = 0; i < sgk::numSegments; ++i)
                setParameterFromUI (ParamIDs::segment (i), (float) i);
            setUserPattern (result);
        }

        // The feel goes with the rhythm.
        switch (which)
        {
            case sgk::Intention::soulful:   nudge (ParamIDs::swing, 0.05f, 0.0f, 0.5f); nudge (ParamIDs::gate, 0.08f, 0.1f, 1.0f); nudge (ParamIDs::humanize, 0.05f, 0.0f, 0.6f); break;
            case sgk::Intention::funky:     nudge (ParamIDs::swing, 0.08f, 0.0f, 0.55f); nudge (ParamIDs::gate, -0.12f, 0.2f, 1.0f); nudge (ParamIDs::velocity, 4.0f, 1.0f, 127.0f); break;
            case sgk::Intention::minimal:   nudge (ParamIDs::gate, 0.08f, 0.1f, 1.0f); setParameterFromUI (ParamIDs::fills, 0.0f); break;
            case sgk::Intention::energetic: nudge (ParamIDs::velocity, 6.0f, 1.0f, 127.0f); nudge (ParamIDs::gate, -0.05f, 0.2f, 1.0f);
                                            if (juce::roundToInt (raw (ParamIDs::fills)) == 0) setParameterFromUI (ParamIDs::fills, 2.0f);
                                            setParameterFromUI (ParamIDs::octaveTop, 1.0f); break;
            case sgk::Intention::sophisticated: nudge (ParamIDs::strum, 0.08f, 0.0f, 1.0f); break;
            case sgk::Intention::straighter: nudge (ParamIDs::swing, -0.1f, 0.0f, 0.55f); nudge (ParamIDs::humanize, -0.05f, 0.0f, 0.6f); break;
        }
    }

    if (! keepHarmony)
    {
        // Only colours and voicings: the chords of the progression never change.
        const int ext = juce::roundToInt (raw (ParamIDs::extension));
        const int voicing = juce::roundToInt (raw (ParamIDs::voicing));
        switch (which)
        {
            case sgk::Intention::soulful:
                if (ext < 2) setParameterFromUI (ParamIDs::extension, 2.0f);                     // 9ths
                if (voicing == (int) sgk::Voicing::close) setParameterFromUI (ParamIDs::voicing, (float) sgk::Voicing::rootless);
                if (juce::roundToInt (raw (ParamIDs::approach)) == 0) setParameterFromUI (ParamIDs::approach, 1.0f);
                break;
            case sgk::Intention::funky:
                if (ext == 0) setParameterFromUI (ParamIDs::extension, 1.0f);                    // 7ths
                if (voicing == (int) sgk::Voicing::wide) setParameterFromUI (ParamIDs::voicing, (float) sgk::Voicing::close);
                break;
            case sgk::Intention::minimal:
                if (ext > 2) setParameterFromUI (ParamIDs::extension, 1.0f);
                setParameterFromUI (ParamIDs::voicing, (float) sgk::Voicing::shell);
                break;
            case sgk::Intention::energetic:
                if (voicing == (int) sgk::Voicing::shell) setParameterFromUI (ParamIDs::voicing, (float) sgk::Voicing::close);
                break;
            case sgk::Intention::sophisticated:
                setParameterFromUI (ParamIDs::extension, ext == 3 ? 4.0f : 3.0f);                // 11ths / 13ths, or 6/9
                setParameterFromUI (ParamIDs::voicing, (float) (voicing == (int) sgk::Voicing::drop2 ? sgk::Voicing::drop24 : sgk::Voicing::drop2));
                if (juce::roundToInt (raw (ParamIDs::approach)) == 0) setParameterFromUI (ParamIDs::approach, 2.0f);
                break;
            case sgk::Intention::straighter:
                break;   // a rhythm move only
        }
    }
}

void FaseGroovePianoProcessor::applyProgressionCodes (const std::vector<int>& codes)
{
    if (codes.empty())
        return;
    const int scale = juce::roundToInt (raw (ParamIDs::scale));
    const sgk::ProgressionPreset asPreset { "Variation", scale, {}, "", codes };
    const auto slots = sgk::presetSlots (asPreset);
    clearFreeChords();
    setParameterFromUI (ParamIDs::progLength, (float) juce::jmin ((int) slots.size(), sgk::maxProgressionLength));
    for (int i = 0; i < (int) slots.size() && i < sgk::maxProgressionLength; ++i)
    {
        setParameterFromUI (ParamIDs::slot (i), (float) slots[(size_t) i].degree);
        setFreeChord (i, slots[(size_t) i].freeCode);
    }
}

void FaseGroovePianoProcessor::newFeel()
{
    setParameterFromUI (ParamIDs::humanizeSeed, (float) juce::Random::getSystemRandom().nextInt ({ 1, 100000 }));
}

void FaseGroovePianoProcessor::beginMorph()
{
    const auto s = currentSettings();
    morphSource = sgk::applySegmentOrder (sgk::GrooveEngine::basePattern (s), s.segmentOrder);
    morphSeed = (std::uint32_t) juce::Random::getSystemRandom().nextInt ({ 1, 1 << 30 });
}

void FaseGroovePianoProcessor::setMorph (int targetPattern, float amount)
{
    if (morphSource.length() != sgk::userPatternLength)
        return;

    auto s = currentSettings();
    s.pattern = juce::jlimit (0, (int) sgk::groovePatterns().size() - 1, targetPattern);
    auto target = sgk::GrooveEngine::basePattern (s);
    auto blend = sgk::morphPatterns (morphSource, target, amount, morphSeed);

    // Locked parts stay as they were (the parts are in playing order, A B C D).
    for (int seg = 0; seg < sgk::numSegments; ++seg)
        if (segmentLocked[(size_t) seg])
            sgk::copySegment (blend, morphSource, seg);
    sgk::repairHolds (blend.rightHand);
    sgk::repairHolds (blend.leftHand);

    for (int i = 0; i < sgk::numSegments; ++i)
        if (juce::roundToInt (segmentParams[(size_t) i]->load()) != i)
            setParameterFromUI (ParamIDs::segment (i), (float) i);
    setUserPattern (blend);
}

int FaseGroovePianoProcessor::clipLengthInBars (int bars) const
{
    if (bars > 0)
        return bars;

    if (bars == songArrangement)
    {
        int total = 0;
        for (const auto& part : songArrangementParts())
            total += juce::jmax (0, part.bars);
        return juce::jmax (1, total);
    }

    return juce::jmax (1, sgk::GrooveEngine::loopLengthInSteps (currentSettings()) / 16);
}

namespace
{
    void roundAndSort (std::vector<sgk::LoopNote>& notes)
    {
        // Times kept to 1/10000 of a beat (as saved), in a fixed order: a saved project loads back identical.
        for (auto& n : notes)
        {
            n.start = std::round (n.start * 10000.0) / 10000.0;
            n.length = juce::jmax (0.001, std::round (n.length * 10000.0) / 10000.0);
        }
        std::stable_sort (notes.begin(), notes.end(), [] (const sgk::LoopNote& a, const sgk::LoopNote& b)
        {
            return std::tie (a.start, a.note, a.part) < std::tie (b.start, b.note, b.part);
        });
    }

    FaseGroovePianoProcessor::EditedLoop share (std::vector<sgk::LoopNote> notes)
    {
        return std::make_shared<const std::vector<sgk::LoopNote>> (std::move (notes));
    }
}

double FaseGroovePianoProcessor::naturalLoopBeats() const
{
    auto s = currentSettings();
    s.editedLoop = nullptr;
    const int patternBars = juce::jmax (1, sgk::GrooveEngine::patternLength (s) / 16);
    const int progressionBars = juce::jmax (1, (s.progressionLength * s.stepsPerChord + 15) / 16);
    int bars = std::lcm (patternBars, progressionBars);
    if (bars > 8)
        bars = juce::jmin (8, juce::jmax (patternBars, progressionBars));
    return bars * 4.0;
}

void FaseGroovePianoProcessor::setEditedLoop (std::vector<sgk::LoopNote> notes, double lengthInBeats)
{
    const double length = juce::jlimit (4.0, 64.0 * 4.0, lengthInBeats);
    roundAndSort (notes);
    auto base = generatedLoop (length);

    lastGenerated = base;
    lastGeneratedLength = length;
    editedLength = length;
    playingLength = length;
    std::atomic_store (&editedBase, share (std::move (base)));
    std::atomic_store (&editedNotes, share (notes));
    std::atomic_store (&playingNotes, share (std::move (notes)));
}

void FaseGroovePianoProcessor::clearEditedLoop()
{
    std::atomic_store (&playingNotes, EditedLoop());
    std::atomic_store (&editedNotes, EditedLoop());
    std::atomic_store (&editedBase, EditedLoop());
    editedLength = 0.0;
    playingLength = 0.0;
    lastGenerated.clear();
}

void FaseGroovePianoProcessor::refreshEditedLoop()
{
    const auto edits = std::atomic_load (&editedNotes);
    const auto base = std::atomic_load (&editedBase);
    if (edits == nullptr || base == nullptr)
        return;

    const double length = naturalLoopBeats();
    auto now = generatedLoop (length);
    auto same = [] (const sgk::LoopNote& a, const sgk::LoopNote& b)
    {
        return juce::approximatelyEqual (a.start, b.start) && juce::approximatelyEqual (a.length, b.length)
               && a.note == b.note && a.velocity == b.velocity && a.part == b.part;
    };
    if (juce::approximatelyEqual (length, lastGeneratedLength) && now.size() == lastGenerated.size()
        && std::equal (now.begin(), now.end(), lastGenerated.begin(), same))
        return;   // nothing that makes the notes changed

    auto carried = sgk::carryEdits (*base, *edits, editedLength.load(), now, length);
    roundAndSort (carried);
    lastGenerated = std::move (now);
    lastGeneratedLength = length;
    playingLength = length;
    std::atomic_store (&playingNotes, share (std::move (carried)));
}

std::vector<sgk::LoopNote> FaseGroovePianoProcessor::generatedLoop (double lengthInBeats) const
{
    auto s = currentSettings();
    s.editedLoop = nullptr;
    s.section = sgk::Section::main;
    s.strings = sgk::StringsMode::off;   // the piano roll edits piano and bass; the strings follow the chords by themselves
    const double toRelative = 118.0 / juce::jmax (1, s.velocity);

    std::vector<sgk::LoopNote> notes;
    std::map<int, std::pair<double, int>> open;   // note + part -> start, velocity
    for (const auto& e : sgk::GrooveEngine::renderClip (s, lengthInBeats))
    {
        const int id = e.note + 128 * (int) e.part;
        if (e.isNoteOn())
            open[id] = { e.ppq, e.velocity };
        else if (auto it = open.find (id); it != open.end())
        {
            notes.push_back ({ it->second.first, juce::jmax (0.02, e.ppq - it->second.first), e.note,
                               juce::jlimit (1, 127, juce::roundToInt (it->second.second * toRelative)), e.part });
            open.erase (it);
        }
    }
    roundAndSort (notes);
    for (int i = 0; i < (int) notes.size(); ++i)
        notes[(size_t) i].origin = i;   // what the edits refer to
    return notes;
}

double FaseGroovePianoProcessor::exportBpm() const
{
    // Not lastBpm: that is only updated while audio runs, so a tempo just typed in would be missed.
    const bool synced = raw (ParamIDs::tempoSync) > 0.5f;
    return juce::jlimit (20.0, 400.0, synced ? hostBpm.load() : (double) raw (ParamIDs::tempo));
}

juce::MidiFile FaseGroovePianoProcessor::createMidiClip (int bars, ClipPart part) const
{
    const auto settings = currentSettings();
    const double beats = clipLengthInBars (bars) * 4.0;
    const auto name = clipFileName (bars, part).upToLastOccurrenceOf (".", false, false);

    if (bars == songArrangement)
    {
        std::vector<sgk::GrooveEngine::Marker> markers;
        const auto events = sgk::GrooveEngine::renderArrangement (settings, songArrangementParts(), &markers);
        return midiFromEvents (events, beats, &markers, part, name);
    }
    return midiFromEvents (sgk::GrooveEngine::renderClip (settings, beats), beats, nullptr, part, name);
}

juce::MidiFile FaseGroovePianoProcessor::midiFromEvents (const std::vector<sgk::NoteEvent>& clipEvents, double beats,
                                                         const std::vector<sgk::GrooveEngine::Marker>* markers,
                                                         ClipPart part, const juce::String& name) const
{
    constexpr int ticksPerQuarter = 960;
    const bool bassOnChannel2 = juce::roundToInt (raw (ParamIDs::bassChannel)) == 1;

    // Track 1 (or the conductor track of a multitrack file): name, tempo, metre and the markers.
    juce::MidiMessageSequence header;
    header.addEvent (juce::MidiMessage::textMetaEvent (3, name), 0.0);
    header.addEvent (juce::MidiMessage::tempoMetaEvent (juce::roundToInt (60000000.0 / exportBpm())), 0.0);
    header.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0.0);
    if (markers != nullptr)
        for (const auto& m : *markers)
            header.addEvent (juce::MidiMessage::textMetaEvent (6, m.name), m.ppq * ticksPerQuarter);   // DAW markers

    auto notesOf = [&] (juce::MidiMessageSequence& track, auto&& wanted, bool useBassChannel)
    {
        for (const auto& e : clipEvents)
        {
            if (! wanted (e.part))
                continue;
            const int channel = e.part == sgk::Part::strings ? stringsChannel : (useBassChannel && e.part == sgk::Part::bass) ? 2 : 1;
            const double tick = std::round (juce::jlimit (0.0, beats, e.ppq) * ticksPerQuarter);
            track.addEvent (e.isNoteOn() ? juce::MidiMessage::noteOn (channel, e.note, (juce::uint8) e.velocity)
                                         : juce::MidiMessage::noteOff (channel, e.note),
                            tick);
        }
        track.addEvent (juce::MidiMessage::endOfTrack(), beats * ticksPerQuarter);
        track.updateMatchedPairs();
    };

    juce::MidiFile file;
    file.setTicksPerQuarterNote (ticksPerQuarter);

    if (part == ClipPart::tracks)
    {
        // One file, one track per part: the DAW puts piano, bass and fills on separate tracks.
        header.addEvent (juce::MidiMessage::endOfTrack(), beats * ticksPerQuarter);
        file.addTrack (header);
        auto any = [&clipEvents] (sgk::Part p)
        {
            return std::any_of (clipEvents.begin(), clipEvents.end(), [p] (const sgk::NoteEvent& e) { return e.part == p; });
        };
        struct Lane { const char* name; sgk::Part part; };
        for (const auto& lane : { Lane { "Piano", sgk::Part::chords }, Lane { "Bass", sgk::Part::bass }, Lane { "Fills", sgk::Part::fill },
                                  Lane { "Strings", sgk::Part::strings } })
        {
            if ((lane.part == sgk::Part::fill || lane.part == sgk::Part::strings) && ! any (lane.part))
                continue;
            juce::MidiMessageSequence track;
            track.addEvent (juce::MidiMessage::textMetaEvent (3, lane.name), 0.0);
            notesOf (track, [&lane] (sgk::Part p) { return p == lane.part; }, bassOnChannel2);   // same channel rule as the other exports
            file.addTrack (track);
        }
        return file;
    }

    // A single track: all parts, or chords (with their fills) only, or bass only, or strings only.
    notesOf (header, [part] (sgk::Part p)
    {
        if (part == ClipPart::bass)    return p == sgk::Part::bass;
        if (part == ClipPart::strings) return p == sgk::Part::strings;
        if (part == ClipPart::chords)  return p == sgk::Part::chords || p == sgk::Part::fill;
        return true;
    }, bassOnChannel2);
    file.addTrack (header);
    return file;
}

juce::String FaseGroovePianoProcessor::clipFileName (int bars, ClipPart part) const
{
    const auto s = currentSettings();
    auto patternSettings = s;
    if (bars == songArrangement)
        patternSettings.section = sgk::Section::main;
    const auto pattern = sgk::GrooveEngine::resolvePattern (patternSettings);

    juce::StringArray chords;
    for (int i = 0; i < juce::jmin (s.progressionLength, 4); ++i)
        chords.add (sgk::chordName (sgk::GrooveEngine::chordForSlot (s, i)));
    if (s.progressionLength > 4)
        chords.add ("etc");

    const juce::String partName = part == ClipPart::chords ? " chords" : part == ClipPart::bass ? " bass" : part == ClipPart::tracks ? " tracks"
                                : part == ClipPart::strings ? " strings" : "";
    const juce::String length = bars == songArrangement ? juce::String ("Song ") : juce::String();
    // Named after the instrument and the part of the song first: "Piano Intro - ...", "Piano Song - ...".
    const juce::String instrument = instrumentName (part);
    const juce::String songPart = bars == songArrangement ? tr ("Song") : tr (sgk::sectionName ((int) s.section));
    const auto name = instrument + " " + songPart + " - " + chords.joinIntoString ("-") + " " + juce::String (pattern.name) + partName + " " + length
                    + juce::String (clipLengthInBars (bars)) + "bars " + juce::String (juce::roundToInt (exportBpm())) + "bpm";

    return juce::File::createLegalFileName (name) + ".mid";
}

juce::File FaseGroovePianoProcessor::writeMidiClip (int bars, ClipPart part) const
{
    auto folder = clipFolder();
    if (! folder.createDirectory())
        folder = juce::File::getSpecialLocation (juce::File::tempDirectory);

    auto file = folder.getChildFile (clipFileName (bars, part));
    file.deleteFile();

    juce::FileOutputStream stream (file);
    if (! stream.openedOk())
        return {};

    createMidiClip (bars, part).writeTo (stream);
    return file;
}

juce::File FaseGroovePianoProcessor::writeSongSectionClip (int partIndex, ClipPart part) const
{
    const auto parts = songArrangementParts();
    if (! juce::isPositiveAndBelow (partIndex, (int) parts.size()) || parts[(size_t) partIndex].bars <= 0)
        return {};

    auto folder = clipFolder();
    if (! folder.createDirectory())
        folder = juce::File::getSpecialLocation (juce::File::tempDirectory);

    const auto& section = parts[(size_t) partIndex];
    std::vector<sgk::GrooveEngine::Marker> markers;
    const auto events = sgk::GrooveEngine::renderArrangement (currentSettings(), { section }, &markers);
    const juce::String instrument = instrumentName (part);
    const auto title = instrument + " " + tr (sgk::sectionName ((int) section.section));
    const auto midi = midiFromEvents (events, section.bars * 4.0, &markers, part, title);

    auto file = folder.getChildFile (juce::File::createLegalFileName (title + " - " + juce::String (section.bars) + "bars "
                                                                      + juce::String (juce::roundToInt (exportBpm())) + "bpm") + ".mid");
    file.deleteFile();
    juce::FileOutputStream stream (file);
    if (! stream.openedOk() || ! midi.writeTo (stream))
        return {};
    return file;
}

//==============================================================================
juce::AudioProcessorEditor* FaseGroovePianoProcessor::createEditor()
{
    return new FaseGroovePianoEditor (*this);
}

void FaseGroovePianoProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    juce::String right, left, locks;
    for (int i = 0; i < sgk::userPatternLength; ++i)
    {
        right += juce::String::charToString (userRight[(size_t) i].load());
        left  += juce::String::charToString (userLeft[(size_t) i].load());
    }
    for (bool locked : segmentLocked)
        locks += locked ? "1" : "0";

    state.setProperty ("userRight", right, nullptr);
    state.setProperty ("userLeft", left, nullptr);
    state.setProperty ("segmentLocks", locks, nullptr);
    {
        juce::StringArray velRight, velLeft;
        for (int i = 0; i < sgk::userPatternLength; ++i)
        {
            velRight.add (juce::String (userVelRight[(size_t) i].load()));
            velLeft.add (juce::String (userVelLeft[(size_t) i].load()));
        }
        state.setProperty ("userVelRight", velRight.joinIntoString (","), nullptr);
        state.setProperty ("userVelLeft", velLeft.joinIntoString (","), nullptr);
        juce::String order, velLocks;
        for (int i = 0; i < sgk::numSegments; ++i)
        {
            order += juce::String (velocityOrder[(size_t) i].load());
            velLocks += velocityLocked[(size_t) i] ? "1" : "0";
        }
        state.setProperty ("velocityOrder", order, nullptr);
        state.setProperty ("velocityLocks", velLocks, nullptr);
    }
    state.setProperty ("lastBassMode", lastBassMode, nullptr);
    state.setProperty ("soloBass", chordsMuted.load(), nullptr);
    {
        juce::StringArray bars, variations, fills;
        for (int i = 0; i < numSongParts; ++i)
        {
            bars.add (juce::String (songBars[(size_t) i].load()));
            variations.add (juce::String ((juce::int64) songVariation[(size_t) i].load()));
            fills.add (juce::String (songFill[(size_t) i].load()));
        }
        state.setProperty ("songBars", bars.joinIntoString (","), nullptr);
        state.setProperty ("songFills", fills.joinIntoString (","), nullptr);
        state.setProperty ("songVariations", variations.joinIntoString (","), nullptr);
    }
    state.setProperty ("editorScale", editorScale, nullptr);
    state.setProperty ("shrinkWhileDragging", shrinkWhileDragging, nullptr);
    state.setProperty ("advancedView", advancedView, nullptr);

    juce::StringArray free;
    for (const auto& f : freeChords)
        free.add (juce::String (f.load()));
    state.setProperty ("freeChords", free.joinIntoString (","), nullptr);
    state.setProperty ("freeCircle", freeCircle, nullptr);

    // The notes edited in the piano roll and the generated notes they were made on:
    // "start length note velocity part origin" per note.
    const auto edits = std::atomic_load (&editedNotes);
    const auto base = std::atomic_load (&editedBase);
    if (edits != nullptr && base != nullptr && ! edits->empty())
    {
        auto write = [] (const std::vector<sgk::LoopNote>& list)
        {
            juce::StringArray notes;
            for (const auto& n : list)
                notes.add (juce::String (n.start, 4) + " " + juce::String (n.length, 4) + " " + juce::String (n.note) + " "
                           + juce::String (n.velocity) + " " + juce::String ((int) n.part) + " " + juce::String (n.origin));
            return notes.joinIntoString (";");
        };
        state.setProperty ("editedNotes", write (*edits), nullptr);
        state.setProperty ("editedBase", write (*base), nullptr);
        state.setProperty ("editedLength", editedLength.load(), nullptr);
    }
    else
    {
        state.removeProperty ("editedNotes", nullptr);
        state.removeProperty ("editedBase", nullptr);
        state.removeProperty ("editedLength", nullptr);
    }

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void FaseGroovePianoProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    apvts.replaceState (juce::ValueTree::fromXml (*xml));
    const auto& state = apvts.state;

    const auto right = state.getProperty ("userRight").toString();
    const auto left  = state.getProperty ("userLeft").toString();
    auto valid = [] (const juce::String& line)
    {
        return line.length() == sgk::userPatternLength && line.containsOnly ("XxgUuDd>=.");
    };

    if (valid (right) && valid (left))
    {
        for (int i = 0; i < sgk::userPatternLength; ++i)
        {
            userRight[(size_t) i] = (char) right[i];
            userLeft[(size_t) i]  = (char) left[i];
        }
    }

    {
        // Levels set by hand on the steps (older projects have none: all automatic).
        auto readLevels = [&state] (const char* id, std::array<std::atomic<int>, sgk::userPatternLength>& levels)
        {
            const auto list = juce::StringArray::fromTokens (state.getProperty (id).toString(), ",", "");
            for (int i = 0; i < sgk::userPatternLength; ++i)
                levels[(size_t) i] = list.size() == sgk::userPatternLength ? juce::jlimit (0, 127, list[i].getIntValue()) : 0;
        };
        readLevels ("userVelRight", userVelRight);
        readLevels ("userVelLeft", userVelLeft);
        const auto order = state.getProperty ("velocityOrder").toString();
        const auto velLocks = state.getProperty ("velocityLocks").toString();
        for (int i = 0; i < sgk::numSegments; ++i)
        {
            velocityOrder[(size_t) i] = order.length() == sgk::numSegments ? juce::jlimit (0, sgk::numSegments - 1, (int) (order[i] - '0')) : i;
            velocityLocked[(size_t) i] = velLocks[i] == '1';
        }
    }

    const auto locks = state.getProperty ("segmentLocks").toString();
    for (int i = 0; i < sgk::numSegments; ++i)
        segmentLocked[(size_t) i] = locks[i] == '1';

    lastBassMode = juce::jlimit (1, sgk::numBassModes - 1, (int) state.getProperty ("lastBassMode", 1));
    chordsMuted = (bool) state.getProperty ("soloBass", false);
    {
        resetSong();
        const auto bars = juce::StringArray::fromTokens (state.getProperty ("songBars").toString(), ",", {});
        const auto variations = juce::StringArray::fromTokens (state.getProperty ("songVariations").toString(), ",", {});
        const auto fills = juce::StringArray::fromTokens (state.getProperty ("songFills").toString(), ",", {});
        for (int i = 0; i < numSongParts; ++i)
        {
            if (i < bars.size() && bars[i].containsOnly ("0123456789"))
                setSongBars (i, bars[i].getIntValue());
            if (i < variations.size() && variations[i].containsOnly ("0123456789"))
                songVariation[(size_t) i] = (std::uint32_t) variations[i].getLargeIntValue();
            if (i < fills.size() && fills[i].containsOnly ("-0123456789") && fills[i].isNotEmpty())
                setSongFill (i, fills[i].getIntValue());
        }
    }
    editorScale = juce::jlimit (0.6f, 2.0f, (float) state.getProperty ("editorScale", 1.0f));
    shrinkWhileDragging = (bool) state.getProperty ("shrinkWhileDragging", true);
    advancedView = (bool) state.getProperty ("advancedView", false);

    const auto free = juce::StringArray::fromTokens (state.getProperty ("freeChords").toString(), ",", {});
    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        setFreeChord (i, i < free.size() ? free[i].getIntValue() : sgk::noFreeChord);
    freeCircle = (bool) state.getProperty ("freeCircle", false);

    {
        const double length = (double) state.getProperty ("editedLength", 0.0);
        auto read = [length] (const juce::String& text, bool& ok)
        {
            std::vector<sgk::LoopNote> notes;
            for (const auto& item : juce::StringArray::fromTokens (text, ";", {}))
            {
                const auto f = juce::StringArray::fromTokens (item, " ", {});
                if (f.size() != 5 && f.size() != 6)
                {
                    ok = false;
                    continue;
                }
                sgk::LoopNote n;
                n.start = f[0].getDoubleValue();
                n.length = f[1].getDoubleValue();
                n.note = f[2].getIntValue();
                n.velocity = f[3].getIntValue();
                n.part = (sgk::Part) juce::jlimit (0, 2, f[4].getIntValue());
                n.origin = f.size() == 6 ? f[5].getIntValue() : -1;
                if (n.start >= 0.0 && n.start < length && n.length > 0.0 && juce::isPositiveAndBelow (n.note, 128)
                    && juce::isPositiveAndBelow (n.velocity, 128) && n.velocity > 0)
                    notes.push_back (n);
                else
                    ok = false;
            }
            return notes;
        };
        bool ok = true;
        auto notes = read (state.getProperty ("editedNotes").toString(), ok);
        auto base = read (state.getProperty ("editedBase").toString(), ok);
        clearEditedLoop();
        if (ok && ! notes.empty() && length > 0.0)
        {
            if (base.empty())
            {
                // Saved before the edits followed the settings: they are their own base.
                base = notes;
                for (int i = 0; i < (int) notes.size(); ++i)
                    base[(size_t) i].origin = notes[(size_t) i].origin = i;
            }
            editedLength = length;
            playingLength = length;
            lastGenerated = base;
            lastGeneratedLength = length;
            std::atomic_store (&editedBase, share (std::move (base)));
            std::atomic_store (&editedNotes, share (notes));
            std::atomic_store (&playingNotes, share (std::move (notes)));
        }
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FaseGroovePianoProcessor();
}
