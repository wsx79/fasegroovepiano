// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginProcessor.h"
#include "GuideVoice.h"
#include "PluginEditor.h"

#include <mutex>

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
    choice (ParamIDs::voicing, "Voicing", namesFrom (sgk::numVoicings, sgk::voicingName), 2);   // rootless: the house piano sound
    add (std::make_unique<AudioParameterInt> (ParameterID { ParamIDs::registerPos, 1 }, "Register", 48, 76, 61,
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
    choice (ParamIDs::variation, "Variation", { "Off", "Every 4 bars", "Every 8 bars" }, 0);
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
    group ("sound", "Guide sound");
    return layout;
}

//==============================================================================
FaseGroovePianoProcessor::FaseGroovePianoProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "FaseGroovePiano", createLayout())
{
    auto& toneParam = *apvts.getRawParameterValue (ParamIDs::tone);
    auto& soundParam = *apvts.getRawParameterValue (ParamIDs::soundType);

    for (int i = 0; i < 24; ++i)
        synth.addVoice (new GuideVoice (toneParam, soundParam));

    synth.addSound (new GuideSoundTag());

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
        { "House Piano Loop",     "Minor Flow  i - VI - III - VII",    "Dotted House Stabs",   2, 2, 6, 0, 2, 61, 0.25f, 0.65f, 0.15f, 0.00f, false, 0, 0 },
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
    setStateInformation (data.getData(), (int) data.getSize());
    editorScale = scale;
    shrinkWhileDragging = shrink;
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
            notes.push_back ({ start, juce::jmax (start + 0.05, end), m.getNoteNumber() });
            if (notes.size() > 20000)
                return {};   // not a loop
        }
    }
    return sgk::analyzeHarmony (notes, beatsPerBar);
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
    s.fillEveryBars = fillChoices[juce::jlimit (0, 4, asInt (ParamIDs::fills))];
    s.fillStyle = (sgk::FillStyle) juce::jlimit (0, sgk::numFillStyles - 1, asInt (ParamIDs::fillStyle));
    s.strum = raw (ParamIDs::strum);
    s.octaveTop = raw (ParamIDs::octaveTop) > 0.5f;
    s.approach = (sgk::ApproachStyle) juce::jlimit (0, sgk::numApproachStyles - 1, asInt (ParamIDs::approach));
    s.topLine = (sgk::TopLine) juce::jlimit (0, sgk::numTopLines - 1, asInt (ParamIDs::topLine));
    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        s.inversions[(size_t) i] = juce::roundToInt (inversionParams[(size_t) i]->load());
    {
        constexpr int every[] { 0, 4, 8 };
        constexpr int bars[] { 2, 1, 4 };
        s.variationEvery = every[juce::jlimit (0, 2, asInt (ParamIDs::variation))];
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
    }

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
            sgk::copySegment (current, fresh, seg);

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

    const auto seed = (std::uint32_t) juce::Random::getSystemRandom().nextInt ({ 1, 100000 });
    const auto fresh = sgk::makeRandomPattern (seed, s.riffDensity);
    const int segLength = sgk::userPatternLength / sgk::numSegments;
    for (int seg = 0; seg < sgk::numSegments; ++seg)
    {
        if (segmentLocked[(size_t) seg])
            continue;
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
    setUserPattern (current);
}

void FaseGroovePianoProcessor::setUserPattern (const sgk::GroovePattern& p)
{
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

    wasRunning = false;
    heldCount = 0;
}

bool FaseGroovePianoProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void FaseGroovePianoProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
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
    const bool running = holdMode ? (heldCount > 0 || preview) : (hostPlaying || preview);

    double startPpq = 0.0;
    if (followHost)
        startPpq = hostPpq;
    else if (running)
        startPpq = wasRunning ? internalPpq : 0.0;   // free-running clock from the start

    events.clear();

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
        if (muteChords && e.isNoteOn() && e.part != sgk::Part::bass)
            continue;   // SOLO BASS (note-offs still pass, so nothing hangs)

        const int pos = juce::jlimit (0, numSamples - 1, (int) std::floor ((e.ppq - startPpq) * samplesPerBeat));
        const int channel = (bassOnChannel2 && e.part == sgk::Part::bass) ? 2 : 1;
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

        // Pump: the classic house sidechain duck on every beat (only while the groove runs).
        if (const float pumpAmount = raw (ParamIDs::pump); pumpAmount > 0.001f && running)
        {
            for (int i = 0; i < numSamples; ++i)
            {
                const double beat = startPpq + i / samplesPerBeat;
                const double frac = beat - std::floor (beat);
                const double recover = juce::jmin (1.0, frac / 0.55);
                const float g = 1.0f - pumpAmount * 0.8f * (float) ((1.0 - recover) * (1.0 - recover));
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
            const int channel = (useBassChannel && e.part == sgk::Part::bass) ? 2 : 1;
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
        const bool anyFill = std::any_of (clipEvents.begin(), clipEvents.end(), [] (const sgk::NoteEvent& e) { return e.part == sgk::Part::fill; });
        struct Lane { const char* name; sgk::Part part; };
        for (const auto& lane : { Lane { "Piano", sgk::Part::chords }, Lane { "Bass", sgk::Part::bass }, Lane { "Fills", sgk::Part::fill } })
        {
            if (lane.part == sgk::Part::fill && ! anyFill)
                continue;
            juce::MidiMessageSequence track;
            track.addEvent (juce::MidiMessage::textMetaEvent (3, lane.name), 0.0);
            notesOf (track, [&lane] (sgk::Part p) { return p == lane.part; }, bassOnChannel2);   // same channel rule as the other exports
            file.addTrack (track);
        }
        return file;
    }

    // A single track: all parts, or chords (with their fills) only, or bass only.
    notesOf (header, [part] (sgk::Part p)
    {
        return part == ClipPart::all || (part == ClipPart::bass ? p == sgk::Part::bass : p != sgk::Part::bass);
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

    const juce::String partName = part == ClipPart::chords ? " chords" : part == ClipPart::bass ? " bass" : part == ClipPart::tracks ? " tracks" : "";
    const juce::String length = bars == songArrangement ? juce::String ("Song ") : juce::String();
    const auto name = chords.joinIntoString ("-") + " " + juce::String (pattern.name) + partName + " " + length
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

    juce::StringArray free;
    for (const auto& f : freeChords)
        free.add (juce::String (f.load()));
    state.setProperty ("freeChords", free.joinIntoString (","), nullptr);
    state.setProperty ("freeCircle", freeCircle, nullptr);

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

    const auto free = juce::StringArray::fromTokens (state.getProperty ("freeChords").toString(), ",", {});
    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        setFreeChord (i, i < free.size() ? free[i].getIntValue() : sgk::noFreeChord);
    freeCircle = (bool) state.getProperty ("freeCircle", false);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FaseGroovePianoProcessor();
}
