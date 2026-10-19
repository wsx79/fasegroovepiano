// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Plug-in level tests (they need JUCE, unlike EngineTests): project state and presets,
// undo snapshots, exported MIDI files, DAW transport (play, stop, seek, loop, tempo changes,
// odd block sizes) and the portable update's all-or-nothing file swap.

#include <tuple>
#include "../Source/PluginProcessor.h"
#include "../Source/Updater.h"
#include "../Source/PadController.h"
#include "../Source/Localisation.h"

#include <cstdio>
#include <map>
#include <set>

namespace
{
    int failures = 0;

    #define CHECK(cond) \
        do { if (! (cond)) { ++failures; std::printf ("FAILED %s:%d  %s\n", __FILE__, __LINE__, #cond); } } while (false)

    struct PlayHead : juce::AudioPlayHead
    {
        double ppq = 0.0, bpm = 124.0;
        bool playing = true;
        juce::Optional<PositionInfo> getPosition() const override
        {
            PositionInfo p;
            p.setBpm (bpm);
            p.setPpqPosition (ppq);
            p.setIsPlaying (playing);
            return p;
        }
    };

    juce::String stateXml (FaseGroovePianoProcessor& p)
    {
        juce::MemoryBlock data;
        p.getStateInformation (data);
        auto xml = juce::AudioProcessor::getXmlFromBinary (data.getData(), (int) data.getSize());
        return xml != nullptr ? xml->toString() : juce::String();
    }

    float value (FaseGroovePianoProcessor& p, const char* id)
    {
        return p.apvts.getRawParameterValue (id)->load();
    }

    //==============================================================================
    void testStateRoundTrip()
    {
        FaseGroovePianoProcessor a;
        a.setParameterFromUI (ParamIDs::key, 4.0f);
        a.setParameterFromUI (ParamIDs::scale, 2.0f);
        a.setParameterFromUI (ParamIDs::pattern, 17.0f);
        a.setParameterFromUI (ParamIDs::swing, 0.4f);
        a.setParameterFromUI (ParamIDs::segment (1), 0.0f);
        a.setFreeChord (2, sgk::freeDominantCode (10));
        a.setSegmentLocked (3, true);
        a.chordsMuted = true;
        a.editStep (false, 5, 'X');   // makes it a user pattern

        juce::MemoryBlock data;
        a.getStateInformation (data);

        FaseGroovePianoProcessor b;
        b.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (stateXml (a) == stateXml (b));
        CHECK (b.freeChord (2) == sgk::freeDominantCode (10));
        CHECK (b.isSegmentLocked (3));
        CHECK (b.chordsMuted.load());
        CHECK (juce::roundToInt (value (b, ParamIDs::key)) == 4);
        CHECK (sgk::GrooveEngine::basePattern (a.currentSettings()).rightHand
               == sgk::GrooveEngine::basePattern (b.currentSettings()).rightHand);

        // Undo / redo snapshots restore exactly.
        const auto snap = a.snapshot();
        a.setParameterFromUI (ParamIDs::pattern, 3.0f);
        a.setFreeChord (2, sgk::noFreeChord);
        a.restoreSnapshot (snap);
        CHECK (a.snapshot() == snap);
    }

    void testPresetFiles()
    {
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::key, 5.0f);
        const auto before = stateXml (p);
        const auto folder = juce::File::createTempFile ("fgp-presets");
        folder.createDirectory();

        // Damaged, foreign, empty: loadPreset says no and nothing changes.
        const auto damaged = folder.getChildFile ("damaged.fgp");
        damaged.replaceWithText ("VC2!garbage that is not a state");
        const auto foreign = folder.getChildFile ("foreign.fgp");
        {
            juce::XmlElement other ("SomeOtherPlugin");
            juce::MemoryBlock data;
            juce::AudioProcessor::copyXmlToBinary (other, data);
            foreign.replaceWithData (data.getData(), data.getSize());
        }
        const auto empty = folder.getChildFile ("empty.fgp");
        empty.create();
        for (const auto& f : { damaged, foreign, empty, folder.getChildFile ("missing.fgp") })
        {
            CHECK (! p.loadPreset (f));
            CHECK (stateXml (p) == before);
        }

        // A good preset loads and reports it.
        FaseGroovePianoProcessor other;
        other.setParameterFromUI (ParamIDs::key, 2.0f);
        juce::MemoryBlock data;
        other.getStateInformation (data);
        const auto good = folder.getChildFile ("good.fgp");
        good.replaceWithData (data.getData(), data.getSize());
        CHECK (p.loadPreset (good));
        CHECK (juce::roundToInt (value (p, ParamIDs::key)) == 2);
        folder.deleteRecursively();
    }

    void testOldAndBrokenStates()
    {
        // A preset from an old version: only a few parameters, none of the newer properties.
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::movement, 2.0f);   // not in the old preset
        p.setParameterFromUI (ParamIDs::pattern, 40.0f);
        p.setFreeChord (1, 5);
        p.editStep (true, 3, 'x');

        juce::XmlElement old ("FaseGroovePiano");
        auto* param = old.createNewChildElement ("PARAM");
        param->setAttribute ("id", "key");
        param->setAttribute ("value", 7.0);
        juce::MemoryBlock data;
        juce::AudioProcessor::copyXmlToBinary (old, data);
        p.setStateInformation (data.getData(), (int) data.getSize());

        CHECK (juce::roundToInt (value (p, ParamIDs::key)) == 7);
        // What the old preset doesn't mention goes back to the defaults, not to what was there.
        FaseGroovePianoProcessor fresh;
        CHECK (value (p, ParamIDs::movement) == value (fresh, ParamIDs::movement));
        CHECK (value (p, ParamIDs::pattern) == value (fresh, ParamIDs::pattern));
        CHECK (p.freeChord (1) == sgk::noFreeChord);
        CHECK (sgk::GrooveEngine::basePattern (p.currentSettings()).leftHand
               == sgk::GrooveEngine::basePattern (fresh.currentSettings()).leftHand);

        // Garbage and wrong documents are ignored: the state stays as it was.
        const auto before = stateXml (p);
        const char junk[] = "not a preset at all";
        p.setStateInformation (junk, (int) sizeof (junk));
        juce::XmlElement other ("SomethingElse");
        juce::AudioProcessor::copyXmlToBinary (other, data);
        p.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (stateXml (p) == before);

        // Out-of-range values are clamped by the parameters / validated on load.
        juce::XmlElement bad ("FaseGroovePiano");
        bad.setAttribute ("userRight", "too short");
        bad.setAttribute ("freeChords", "999,-5,abc");
        bad.setAttribute ("editorScale", 50.0);
        auto* badParam = bad.createNewChildElement ("PARAM");
        badParam->setAttribute ("id", "pattern");
        badParam->setAttribute ("value", 99999.0);
        juce::AudioProcessor::copyXmlToBinary (bad, data);
        p.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (p.freeChord (0) == sgk::noFreeChord && p.freeChord (1) == sgk::noFreeChord);
        CHECK (juce::roundToInt (value (p, ParamIDs::pattern)) <= sgk::userPatternIndex());
        CHECK (p.editorScale <= 2.0f);
    }

    //==============================================================================
    struct ClipCheck { int noteOns = 0, unmatched = 0, markers = 0, onChannel1 = 0, onChannel2 = 0, onChannel3 = 0; double lastTick = 0.0; bool ordered = true, channelsOk = true; };

    ClipCheck readClip (const juce::File& file, bool bassOnChannel2)
    {
        ClipCheck c;
        juce::FileInputStream in (file);
        juce::MidiFile mf;
        if (! in.openedOk() || ! mf.readFrom (in) || mf.getNumTracks() == 0)
        {
            c.unmatched = 999;
            return c;
        }

        std::map<int, int> open;   // channel * 128 + note -> count
        double previous = -1.0;
        for (auto* ev : *mf.getTrack (0))
        {
            const auto& m = ev->message;
            if (m.getTimeStamp() < previous) c.ordered = false;
            previous = m.getTimeStamp();
            c.lastTick = juce::jmax (c.lastTick, m.getTimeStamp());
            if (m.isTextMetaEvent() && m.getMetaEventType() == 6) ++c.markers;
            if (m.isNoteOn())
            {
                ++c.noteOns;
                ++open[m.getChannel() * 128 + m.getNoteNumber()];
                if (m.getChannel() == 1) ++c.onChannel1;
                if (m.getChannel() == 2) ++c.onChannel2;
                if (m.getChannel() == 3) ++c.onChannel3;   // the strings: always their own channel
                if (m.getChannel() > 3) c.channelsOk = false;
                if (! bassOnChannel2 && m.getChannel() == 2) c.channelsOk = false;
            }
            else if (m.isNoteOff())
            {
                if (--open[m.getChannel() * 128 + m.getNoteNumber()] < 0) ++c.unmatched;
            }
        }
        for (const auto& [k, n] : open)
            c.unmatched += std::abs (n);
        return c;
    }

    void testMidiExport()
    {
        FaseGroovePianoProcessor p;
        p.applyStyle (0);
        const int ticksPerBar = 960 * 4;

        for (int bars : { 1, 2, 4 })   // clip-length menu: 1 bar, 2 bars, 4 bars
        {
            int total = 0;
            for (auto part : { FaseGroovePianoProcessor::ClipPart::all, FaseGroovePianoProcessor::ClipPart::chords,
                               FaseGroovePianoProcessor::ClipPart::bass })
            {
                const auto c = readClip (p.writeMidiClip (bars, part), false);
                CHECK (c.unmatched == 0);
                CHECK (c.ordered);
                CHECK (c.channelsOk);
                CHECK (c.noteOns > 0);
                // Nothing rings past the clip length (notes at the boundary end on it).
                CHECK (c.lastTick <= (double) (p.clipLengthInBars (bars) * ticksPerBar) + 1.0);
                total += part == FaseGroovePianoProcessor::ClipPart::all ? c.noteOns : -c.noteOns;
            }
            CHECK (total == 0);   // chords + bass = full clip
        }

        // The whole song: 60 bars and the six section markers.
        const auto song = readClip (p.writeMidiClip (FaseGroovePianoProcessor::songArrangement, FaseGroovePianoProcessor::ClipPart::all), false);
        CHECK (song.unmatched == 0 && song.ordered);
        CHECK (song.markers == 6);
        CHECK (song.lastTick <= 60.0 * ticksPerBar + 1.0);

        // Bass on channel 2: the chords are all on channel 1, the bass all on channel 2.
        const auto chordsOnly = readClip (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::chords), false);
        const auto bassOnly = readClip (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::bass), false);
        CHECK (chordsOnly.noteOns > 0 && bassOnly.noteOns > 0);
        CHECK (chordsOnly.onChannel1 == chordsOnly.noteOns && bassOnly.onChannel1 == bassOnly.noteOns);   // all on 1 by default
        p.setParameterFromUI (ParamIDs::bassChannel, 1.0f);
        const auto split = readClip (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::all), true);
        CHECK (split.unmatched == 0 && split.channelsOk);
        CHECK (split.onChannel1 == chordsOnly.noteOns);
        CHECK (split.onChannel2 == bassOnly.noteOns);
        const auto bassSplit = readClip (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::bass), true);
        CHECK (bassSplit.onChannel2 == bassSplit.noteOns && bassSplit.onChannel1 == 0);
        const auto chordSplit = readClip (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::chords), true);
        CHECK (chordSplit.onChannel1 == chordSplit.noteOns && chordSplit.onChannel2 == 0);
        p.setParameterFromUI (ParamIDs::bassChannel, 0.0f);
    }

    //==============================================================================
    void testTransport()
    {
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::sound, 0.0f);
        p.setParameterFromUI (ParamIDs::bass, (float) sgk::BassMode::withChords);
        p.setParameterFromUI (ParamIDs::fills, 4.0f);
        PlayHead head;
        p.setPlayHead (&head);
        const double sr = 44100.0;
        p.prepareToPlay (sr, 512);

        int sounding[17][128] {};
        bool doubleOn = false, badOff = false;
        int noteOns = 0;
        auto run = [&] (int blocks, int blockSize, double ppqJumpAt = -1.0, double jumpTo = 0.0)
        {
            for (int b = 0; b < blocks; ++b)
            {
                juce::AudioBuffer<float> buffer (2, blockSize);
                juce::MidiBuffer midi;
                p.processBlock (buffer, midi);
                for (const auto meta : midi)
                {
                    const auto m = meta.getMessage();
                    auto& s = sounding[m.getChannel()][m.getNoteNumber()];
                    if (m.isNoteOn()) { if (s) doubleOn = true; s = 1; ++noteOns; }
                    else if (m.isNoteOff()) { if (! s) badOff = true; s = 0; }
                }
                if (head.playing)
                    head.ppq += blockSize / sr * head.bpm / 60.0;
                if (ppqJumpAt >= 0.0 && head.ppq >= ppqJumpAt)
                {
                    head.ppq = jumpTo;
                    ppqJumpAt = -1.0;
                }
            }
        };
        auto anySounding = [&]
        {
            for (auto& ch : sounding)
                for (int n : ch)
                    if (n) return true;
            return false;
        };

        run (300, 512);                     // play (almost two bars)
        CHECK (noteOns > 20);               // it really plays
        run (200, 333, 7.3, 0.0);           // DAW loop: jump back to the start
        run (200, 97, 3.1, 40.0);           // seek forward
        head.bpm = 98.0;  run (150, 1024);  // tempo change
        head.bpm = 160.0; run (150, 64);
        head.playing = false; run (4, 512); // stop
        CHECK (! anySounding());
        CHECK (! doubleOn);
        CHECK (! badOff);

        // Stop exactly while notes are held, start again somewhere else.
        head.playing = true; head.ppq = 12.0; run (37, 480);
        head.playing = false; run (2, 480);
        CHECK (! anySounding());

        // SOLO BASS: no chord note-ons, nothing left hanging when it is switched off again.
        p.chordsMuted = true;
        head.playing = true; head.ppq = 0.0; run (200, 512);
        p.chordsMuted = false; run (50, 512);
        head.playing = false; run (2, 512);
        CHECK (! anySounding());
        p.setPlayHead (nullptr);
    }

    //==============================================================================
    void testMidiImport()
    {
        // A chord loop as a DAW would save it: Am7 - Fmaj7 - C - G, one bar each, played twice.
        juce::MidiMessageSequence seq;
        const int chords[4][5] { { 45, 57, 60, 64, 67 }, { 41, 57, 60, 64, 65 }, { 48, 55, 60, 64, 67 }, { 43, 55, 59, 62, 67 } };
        for (int bar = 0; bar < 8; ++bar)
            for (int n : chords[bar % 4])
            {
                const double t = bar * 4 * 960.0;
                seq.addEvent (juce::MidiMessage::noteOn (1, n, (juce::uint8) 100), t);
                seq.addEvent (juce::MidiMessage::noteOff (1, n), t + 3.5 * 960.0);
            }
        seq.addEvent (juce::MidiMessage::noteOn (10, 36, (juce::uint8) 100), 0.0);   // drums: ignored
        seq.addEvent (juce::MidiMessage::noteOff (10, 36), 100.0);
        seq.updateMatchedPairs();

        juce::MidiFile mf;
        mf.setTicksPerQuarterNote (960);
        mf.addTrack (seq);
        const auto file = juce::File::createTempFile (".mid");
        {
            juce::FileOutputStream out (file);
            CHECK (mf.writeTo (out));
        }

        const auto analysis = FaseGroovePianoProcessor::analyzeMidiFile (file);
        CHECK (analysis.ok);
        FaseGroovePianoProcessor p;
        p.applyAnalysis (analysis);
        const auto s = p.currentSettings();
        CHECK (s.progressionLength == 4);
        CHECK (s.stepsPerChord == 16);   // one bar per chord
        if (s.progressionLength == 4)
        {
            CHECK (sgk::GrooveEngine::chordForSlot (s, 0).rootPc == 9);
            CHECK (sgk::GrooveEngine::chordForSlot (s, 1).rootPc == 5);
            CHECK (sgk::GrooveEngine::chordForSlot (s, 2).rootPc == 0);
            CHECK (sgk::GrooveEngine::chordForSlot (s, 3).rootPc == 7);
        }

        // Not a MIDI file: nothing happens.
        const auto junk = juce::File::createTempFile (".mid");
        junk.replaceWithText ("hello");
        CHECK (! FaseGroovePianoProcessor::analyzeMidiFile (junk).ok);
        file.deleteFile();
        junk.deleteFile();
    }

    //==============================================================================
    void testBassLineControls()
    {
        FaseGroovePianoProcessor p;
        const float firstLine = (float) sgk::numBassPatternModes;
        p.setParameterFromUI (ParamIDs::bass, firstLine);

        // RND BASS with a library line picks another line.
        p.rerollHand (true);
        const int now = juce::roundToInt (value (p, ParamIDs::bass));
        CHECK (now >= sgk::numBassPatternModes && now < sgk::numBassModes && now != (int) firstLine);

        // Editing the bass row keeps the line's rhythm as your own, played on the roots.
        const auto shown = sgk::GrooveEngine::resolvePattern (p.currentSettings()).leftHand;
        p.editStep (true, 1, 'x');
        CHECK (juce::roundToInt (value (p, ParamIDs::bass)) == (int) sgk::BassMode::root);
        auto edited = sgk::GrooveEngine::basePattern (p.currentSettings()).leftHand;
        auto expected = shown.substr (0, (size_t) sgk::userPatternLength);
        expected[1] = 'x';
        CHECK (edited == expected);
    }

    //==============================================================================
    void testMorphAndSong()
    {
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::pattern, 3.0f);
        const auto source = sgk::GrooveEngine::basePattern (p.currentSettings());
        const auto target = sgk::groovePatterns()[60];

        p.setSegmentLocked (1, true);   // part B stays
        p.beginMorph();
        p.setMorph (60, 1.0f);
        auto now = sgk::GrooveEngine::basePattern (p.currentSettings());
        CHECK (juce::roundToInt (value (p, ParamIDs::pattern)) == sgk::userPatternIndex());
        CHECK (now.rightHand.substr (8, 8) == source.rightHand.substr (8, 8));    // locked part untouched
        CHECK (now.rightHand.substr (16, 16) == target.rightHand.substr (16, 16)); // the rest is the target
        p.setMorph (60, 0.0f);
        now = sgk::GrooveEngine::basePattern (p.currentSettings());
        CHECK (now.rightHand == source.rightHand && now.leftHand == source.leftHand);

        // Song: bars per section, a variation for one section, saved with the project.
        p.setSongBars (0, 0);
        p.setSongBars (4, 32);
        p.regenerateSongPart (4);
        CHECK (p.clipLengthInBars (FaseGroovePianoProcessor::songArrangement) == 16 + 8 + 4 + 32 + 8);
        juce::MemoryBlock data;
        p.getStateInformation (data);
        FaseGroovePianoProcessor q;
        q.setStateInformation (data.getData(), (int) data.getSize());
        const auto a = p.songArrangementParts(), b = q.songArrangementParts();
        for (size_t i = 0; i < a.size(); ++i)
            CHECK (a[i].bars == b[i].bars && a[i].variation == b[i].variation);
        CHECK (b[4].variation != 0);

        // The song clip: markers only for the sections in it, notes well paired, right length.
        const auto song = readClip (p.writeMidiClip (FaseGroovePianoProcessor::songArrangement, FaseGroovePianoProcessor::ClipPart::all), false);
        CHECK (song.markers == 5 && song.unmatched == 0);
        CHECK (song.lastTick <= 68.0 * 3840.0 + 1.0);

        // One file per section.
        const auto folder = juce::File::createTempFile ("fgp-sections");
        CHECK (p.saveSongSections (folder, FaseGroovePianoProcessor::ClipPart::all) == 5);
        CHECK (folder.findChildFiles (juce::File::findFiles, false, "*.mid").size() == 5);
        for (const auto& f : folder.findChildFiles (juce::File::findFiles, false, "*.mid"))
            CHECK (readClip (f, false).unmatched == 0);
        folder.deleteRecursively();
    }

    //==============================================================================
    void testMusicalDna()
    {
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::pattern, 3.0f);
        const auto before = p.currentSettings();
        const auto sourceRight = sgk::GrooveEngine::basePattern (before).rightHand;
        std::array<int, 4> chordRoots {};
        for (int i = 0; i < 4; ++i) chordRoots[(size_t) i] = sgk::GrooveEngine::chordForSlot (before, i).rootPc;

        // Funkier, harmony kept: swing +8%, the grid changes (not the locked part), chords and voicing stay.
        p.setSegmentLocked (0, true);
        const float swing = value (p, ParamIDs::swing);
        p.applyIntention ((int) sgk::Intention::funky, false, true);
        CHECK (std::abs (value (p, ParamIDs::swing) - (swing + 0.08f)) < 0.01f);
        const auto after = p.currentSettings();
        const auto right = sgk::GrooveEngine::basePattern (after).rightHand;
        CHECK (right.substr (0, 8) == sourceRight.substr (0, 8));
        CHECK (after.voicing == before.voicing && after.extension == before.extension);
        for (int i = 0; i < 4; ++i)
            CHECK (sgk::GrooveEngine::chordForSlot (after, i).rootPc == chordRoots[(size_t) i]);

        // More sophisticated, rhythm kept: only extensions / voicing move, the grid stays.
        const auto grid = sgk::GrooveEngine::basePattern (p.currentSettings()).rightHand;
        const float swingNow = value (p, ParamIDs::swing);
        p.applyIntention ((int) sgk::Intention::sophisticated, true, false);
        const auto smart = p.currentSettings();
        CHECK (sgk::GrooveEngine::basePattern (smart).rightHand == grid);
        CHECK (value (p, ParamIDs::swing) == swingNow);
        CHECK ((int) smart.extension == 3 || (int) smart.extension == 4);
        for (int i = 0; i < 4; ++i)
            CHECK (sgk::GrooveEngine::chordForSlot (smart, i).rootPc == chordRoots[(size_t) i]);
    }

    //==============================================================================
    void testV124()
    {
        // Chord Variation Lab: a variant replaces the chords, not the length or the key.
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::key, 0.0f);
        p.setParameterFromUI (ParamIDs::scale, 0.0f);
        p.setParameterFromUI (ParamIDs::progLength, 4.0f);
        for (int i = 0; i < 4; ++i)
            p.setParameterFromUI (ParamIDs::slot (i), (float) (std::array<int, 4> { 0, 5, 3, 4 }[(size_t) i]));
        const auto variants = sgk::GrooveEngine::progressionVariants (p.currentSettings());
        CHECK (! variants.empty());
        for (const auto& v : variants)
        {
            p.applyProgressionCodes (v.codes);
            const auto s = p.currentSettings();
            CHECK (s.progressionLength == 4 && s.keyRoot == 0 && s.scale == 0);
            CHECK (sgk::GrooveEngine::progressionCodes (s) == v.codes);   // exactly the chords of the variant
        }

        // Humanize style and NEW FEEL are parameters (saved, automatable).
        const float seed = value (p, ParamIDs::humanizeSeed);
        p.newFeel();
        CHECK (value (p, ParamIDs::humanizeSeed) != seed);
        p.setParameterFromUI (ParamIDs::humanizeStyle, 1.0f);
        CHECK (p.currentSettings().humanizeStyle == 1);

        // A section's own fill: saved, and heard while that section plays.
        p.setSongFill (4, (int) sgk::FillStyle::chordRoll);   // Drop
        p.setSongFill (1, -2);                                // Main: no fill
        juce::MemoryBlock data;
        p.getStateInformation (data);
        FaseGroovePianoProcessor q;
        q.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (q.songArrangementParts()[4].endFill == (int) sgk::FillStyle::chordRoll);
        CHECK (q.songArrangementParts()[1].endFill == -2);
        q.setParameterFromUI (ParamIDs::section, (float) sgk::Section::drop);
        CHECK (q.currentSettings().fillStyle == sgk::FillStyle::chordRoll && q.currentSettings().fillEveryBars > 0);
        q.setParameterFromUI (ParamIDs::section, (float) sgk::Section::main);
        q.setParameterFromUI (ParamIDs::fills, 2.0f);
        CHECK (q.currentSettings().fillEveryBars == 0);

        // Separate tracks (with "Bass on channel 2"): a conductor track, then Piano (ch 1), Bass (ch 2) and Fills.
        q.setParameterFromUI (ParamIDs::bassChannel, 1.0f);
        q.setParameterFromUI (ParamIDs::section, (float) sgk::Section::drop);
        q.setParameterFromUI (ParamIDs::fills, 1.0f);
        const auto file = q.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::tracks);
        juce::FileInputStream in (file);
        juce::MidiFile mf;
        CHECK (in.openedOk() && mf.readFrom (in));
        CHECK (mf.getNumTracks() == 4);
        auto channelsOf = [&mf] (int track, int& ch1, int& ch2, juce::String& name)
        {
            ch1 = ch2 = 0;
            for (auto* ev : *mf.getTrack (track))
            {
                if (ev->message.isTrackNameEvent()) name = ev->message.getTextFromTextMetaEvent();
                if (ev->message.isNoteOn()) (ev->message.getChannel() == 2 ? ch2 : ch1)++;
            }
        };
        int a1, a2, b1, b2, f1, f2, h1, h2;
        juce::String pianoName, bassName, fillName, headName;
        channelsOf (0, h1, h2, headName);
        channelsOf (1, a1, a2, pianoName);
        channelsOf (2, b1, b2, bassName);
        if (mf.getNumTracks() == 4) channelsOf (3, f1, f2, fillName);
        CHECK (h1 + h2 == 0);
        // Each track named after its instrument and the part of the song ("Piano Main groove"...).
        CHECK (pianoName.startsWith ("Piano ") && a1 > 0 && a2 == 0);
        CHECK (bassName.startsWith ("Bass ") && b2 > 0 && b1 == 0);
        CHECK (fillName.startsWith ("Fills ") && f1 > 0);
        // The parts add up to the single-track clip.
        const auto all = readClip (q.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::all), true);
        CHECK (all.noteOns == a1 + b2 + f1);
    }

    //==============================================================================
    double exportedBpm (const juce::File& file)
    {
        juce::FileInputStream in (file);
        juce::MidiFile mf;
        if (! in.openedOk() || ! mf.readFrom (in))
            return 0.0;
        for (int t = 0; t < mf.getNumTracks(); ++t)
            for (auto* ev : *mf.getTrack (t))
                if (ev->message.isTempoMetaEvent())
                    return 60.0 / ev->message.getTempoSecondsPerQuarterNote();
        return 0.0;
    }

    void testExportTempoAndChannels()
    {
        // The tempo in the file is the one set now, even with nothing played since.
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::tempoSync, 0.0f);
        p.setParameterFromUI (ParamIDs::tempo, 124.0f);
        CHECK (std::abs (exportedBpm (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::all)) - 124.0) < 0.05);
        p.setParameterFromUI (ParamIDs::tempo, 128.0f);
        CHECK (std::abs (exportedBpm (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::all)) - 128.0) < 0.05);
        CHECK (std::abs (exportedBpm (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::tracks)) - 128.0) < 0.05);

        // Synced to the DAW: its tempo, as last reported by the host.
        PlayHead head;
        head.bpm = 97.0;
        head.playing = false;
        p.setParameterFromUI (ParamIDs::tempoSync, 1.0f);
        p.setPlayHead (&head);
        p.prepareToPlay (44100.0, 256);
        juce::AudioBuffer<float> buffer (2, 256);
        juce::MidiBuffer midi;
        p.processBlock (buffer, midi);   // a stopped host still calls processBlock
        CHECK (std::abs (exportedBpm (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::all)) - 97.0) < 0.05);
        p.setPlayHead (nullptr);

        // Separate tracks follow the bass-channel setting like the other exports.
        p.setParameterFromUI (ParamIDs::bassChannel, 0.0f);
        {
            juce::FileInputStream in (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::tracks));
            juce::MidiFile mf;
            CHECK (mf.readFrom (in));
            for (int t = 0; t < mf.getNumTracks(); ++t)
                for (auto* ev : *mf.getTrack (t))
                    if (ev->message.isNoteOn()) CHECK (ev->message.getChannel() == 1);
        }
    }

    void testSectionFillsNoStuckNotes()
    {
        // A song where every section closes with a different fill: no note left hanging or
        // struck twice at the section changes.
        FaseGroovePianoProcessor p;
        p.applyStyle (0);
        for (int i = 0; i < FaseGroovePianoProcessor::numSongParts; ++i)
            p.setSongFill (i, i % sgk::numFillStyles);
        p.setParameterFromUI (ParamIDs::fills, 1.0f);
        const auto events = sgk::GrooveEngine::renderArrangement (p.currentSettings(), p.songArrangementParts());
        int sounding[128] {};
        bool valid = true;
        int fillNotes = 0;
        for (const auto& e : events)
        {
            if (e.isNoteOn()) { if (sounding[e.note]) valid = false; sounding[e.note] = 1; fillNotes += e.part == sgk::Part::fill ? 1 : 0; }
            else              { if (! sounding[e.note]) valid = false; sounding[e.note] = 0; }
        }
        CHECK (valid);
        CHECK (fillNotes > 0);
        for (int n : sounding) CHECK (n == 0);
    }

    //==============================================================================
    void testUpdateSwap()
    {
        CHECK (Updater::isNewer ("v1.18.1", "1.18.0"));
        CHECK (! Updater::isNewer ("1.18.0", "1.18.0"));
        CHECK (! Updater::isNewer ("v1.9.0", "1.18.0"));

        const auto root = juce::File::createTempFile ("fgp-update-test");
        const auto staged = root.getChildFile ("staged"), installed = root.getChildFile ("installed");

        auto writeBundle = [] (const juce::File& bundle, const juce::String& text, bool extra)
        {
            bundle.getChildFile ("Contents/x86_64-linux/Fase Groove Piano.so").create();
            bundle.getChildFile ("Contents/x86_64-linux/Fase Groove Piano.so").replaceWithText (text);
            bundle.getChildFile ("Contents/Resources/moduleinfo.json").create();
            bundle.getChildFile ("Contents/Resources/moduleinfo.json").replaceWithText (text);
            if (extra)
                bundle.getChildFile ("Contents/Resources/new.txt").replaceWithText (text);
        };
        auto contents = [] (const juce::File& bundle)
        {
            return bundle.getChildFile ("Contents/x86_64-linux/Fase Groove Piano.so").loadFileAsString()
                 + "|" + bundle.getChildFile ("Contents/Resources/moduleinfo.json").loadFileAsString()
                 + "|" + bundle.getChildFile ("Contents/Resources/new.txt").loadFileAsString();
        };

        writeBundle (staged.getChildFile ("VST3/Fase Groove Piano.vst3"), "new", true);
        const auto copyA = installed.getChildFile ("a/Fase Groove Piano.vst3");
        const auto copyB = installed.getChildFile ("b/Fase Groove Piano.vst3");
        writeBundle (copyA, "old", false);
        writeBundle (copyB, "old", false);
        const juce::Array<juce::File> targets { copyA, copyB };

        // A failure at any step leaves every copy exactly as it was (also the ones already done).
        for (int failAt = 0; failAt < 12; ++failAt)
        {
            const bool ok = Updater::testing::replaceAll (staged, targets, failAt);
            if (ok)
                break;
            CHECK (contents (copyA) == "old|old|");
            CHECK (contents (copyB) == "old|old|");
            CHECK (! copyA.getSiblingFile ("Fase Groove Piano.vst3.old").exists());
            CHECK (! copyA.getSiblingFile ("Fase Groove Piano.vst3.new").exists());
        }

        // Without failures: both copies are new and nothing is left behind.
        writeBundle (copyA, "old", false);
        writeBundle (copyB, "old", false);
        copyA.getChildFile ("Contents/Resources/new.txt").deleteFile();
        copyB.getChildFile ("Contents/Resources/new.txt").deleteFile();
        CHECK (Updater::testing::replaceAll (staged, targets, -1));
        CHECK (contents (copyA) == "new|new|new");
        CHECK (contents (copyB) == "new|new|new");
        CHECK (installed.findChildFiles (juce::File::findFilesAndDirectories, true, "*.old").isEmpty());
        CHECK (installed.findChildFiles (juce::File::findFilesAndDirectories, true, "*.new").isEmpty());

        // A target whose staged counterpart is missing: nothing is touched.
        const auto lonely = installed.getChildFile ("c/Fase Groove Piano.component");
        lonely.getChildFile ("Contents/x").create();
        CHECK (! Updater::testing::replaceAll (staged, { copyA, lonely }, -1));
        CHECK (contents (copyA) == "new|new|new");

        root.deleteRecursively();
    }
    //==============================================================================
    void testEditedNotes()
    {
        // Notes edited in the piano roll: saved with the project, undone, exported, cleared.
        FaseGroovePianoProcessor a;
        auto loop = a.generatedLoop (8.0);
        CHECK (! loop.empty());
        const auto generatedCount = loop.size();
        loop.erase (loop.begin());
        loop.push_back ({ 7.5, 0.5, 84, 77, sgk::Part::chords });
        a.setEditedLoop (loop, 8.0);
        CHECK (a.hasEditedLoop() && a.editedLoop()->size() == generatedCount);

        juce::MemoryBlock data;
        a.getStateInformation (data);
        FaseGroovePianoProcessor b;
        b.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (b.hasEditedLoop() && b.editedLoop()->size() == generatedCount);
        CHECK (std::abs (b.editedLoopLength() - 8.0) < 1.0e-9);
        CHECK (stateXml (a) == stateXml (b));

        // The full-loop export is the edited loop: two bars, exactly its notes.
        const auto clip = readClip (b.writeMidiClip (0, FaseGroovePianoProcessor::ClipPart::all), false);
        CHECK (clip.noteOns == (int) generatedCount && clip.unmatched == 0);

        // Undo to before the edit, and back to the generated notes.
        const auto edited = a.snapshot();
        a.clearEditedLoop();
        CHECK (! a.hasEditedLoop());
        const auto plain = a.snapshot();
        a.restoreSnapshot (edited);
        CHECK (a.hasEditedLoop());
        a.restoreSnapshot (plain);
        CHECK (! a.hasEditedLoop());

        // The edits follow a change of the settings (another key, another voicing): same notes
        // deleted, same velocities, the rest regenerated.
        {
            FaseGroovePianoProcessor d;
            d.setParameterFromUI (ParamIDs::key, 9.0f);
            const double length = d.naturalLoopBeats();
            auto edited = d.generatedLoop (length);
            const auto total = edited.size();
            const int firstNote = edited.front().note;
            edited.erase (edited.begin());          // delete the first note
            edited.front().velocity = 33;           // and play the next one softer
            const int softOrigin = edited.front().origin;
            d.setEditedLoop (edited, length);

            d.setParameterFromUI (ParamIDs::key, 11.0f);   // two semitones up
            d.refreshEditedLoop();
            const auto playing = d.editedLoop();
            CHECK (playing != nullptr && playing->size() == total - 1);
            bool soft = false, deletedBack = false;
            for (const auto& n : *playing)
            {
                if (n.velocity == 33) soft = true;
                if (std::abs (n.start - 0.0) < 0.05 && n.note == firstNote + 2) deletedBack = true;
            }
            CHECK (soft && softOrigin >= 0);
            CHECK (! deletedBack);

            d.setParameterFromUI (ParamIDs::voicing, 0.0f);
            d.refreshEditedLoop();
            CHECK (d.editedLoop()->size() + 1 >= total - 1);   // still the same loop, re-voiced
        }

        // A broken note list is ignored, not loaded half way.
        auto xml = juce::AudioProcessor::getXmlFromBinary (data.getData(), (int) data.getSize());
        xml->setAttribute ("editedNotes", "x y z;0 0.5 300 100 0");
        juce::MemoryBlock broken;
        juce::AudioProcessor::copyXmlToBinary (*xml, broken);
        FaseGroovePianoProcessor c;
        c.setStateInformation (broken.getData(), (int) broken.getSize());
        CHECK (! c.hasEditedLoop());
    }

    //==============================================================================
    //==============================================================================
    void testLibrary()
    {
        // Every style loads its own progression and groove, and plays.
        const auto names = FaseGroovePianoProcessor::styleNames();
        CHECK (names.size() >= 28);
        for (int i = 0; i < names.size(); ++i)
        {
            FaseGroovePianoProcessor p;
            p.applyStyle (i);
            CHECK (FaseGroovePianoProcessor::stylePreset (i) >= 0);
            const auto s = p.currentSettings();
            const auto events = sgk::GrooveEngine::renderClip (s, 16.0);
            int ons = 0, offs = 0;
            for (const auto& e : events)
                (e.isNoteOn() ? ons : offs)++;
            CHECK (ons > 8 && ons == offs);
        }
        CHECK (sgk::progressionPresets().size() >= 90);
        CHECK (sgk::groovePatterns().size() >= 170);
    }

    //==============================================================================
    void testRandomPianoKeepsBass()
    {
        // RND PIANO with the default piano-stab bass: the piano changes, the bass stays exactly.
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::bass, (float) sgk::BassMode::withChords);
        auto bassOf = [&p]
        {
            std::vector<std::tuple<long long, int, int>> out;
            for (const auto& e : sgk::GrooveEngine::renderClip (p.currentSettings(), 16.0))
                if (e.isNoteOn() && e.part == sgk::Part::bass)
                    out.push_back ({ std::llround (e.ppq * 960.0), e.note, e.velocity });
            return out;
        };
        auto pianoOf = [&p]
        {
            std::vector<long long> out;
            for (const auto& e : sgk::GrooveEngine::renderClip (p.currentSettings(), 16.0))
                if (e.isNoteOn() && e.part != sgk::Part::bass)
                    out.push_back (std::llround (e.ppq * 960.0));
            return out;
        };
        for (int round = 0; round < 6; ++round)
        {
            const auto bassBefore = bassOf();
            const auto pianoBefore = pianoOf();
            p.rerollHand (false);
            CHECK (bassOf() == bassBefore);
            if (round == 0)
                CHECK (pianoOf() != pianoBefore);
        }
        CHECK (juce::roundToInt (value (p, ParamIDs::bass)) == sgk::stabsOwnRhythmMode);
    }


    void testStepVelocities()
    {
        // VELOCITY page: a level on a library groove makes it the user groove (same rhythm),
        // is saved with the project, follows the A-D order and goes away with a new rhythm.
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::pattern, 3.0f);
        const auto before = sgk::GrooveEngine::resolvePattern (p.currentSettings());
        int hit = 0;
        while (hit < before.length() && ! (before.rightHand[(size_t) hit] == 'X' || before.rightHand[(size_t) hit] == 'x'))
            ++hit;
        CHECK (hit < before.length());
        CHECK (! p.hasStepVelocities());

        p.setStepVelocity (false, hit, 30);
        CHECK (juce::roundToInt (value (p, ParamIDs::pattern)) == sgk::userPatternIndex());
        CHECK (sgk::GrooveEngine::resolvePattern (p.currentSettings()).rightHand == before.rightHand);
        CHECK (p.stepVelocity (false, hit) == 30 && p.stepVelocity (true, hit) == 0);
        CHECK (p.hasStepVelocities());

        juce::MemoryBlock data;
        p.getStateInformation (data);
        FaseGroovePianoProcessor q;
        q.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (q.stepVelocity (false, hit) == 30);

        // Swapping parts A and B moves the level with its step.
        const int segLength = sgk::userPatternLength / sgk::numSegments;
        q.setParameterFromUI (ParamIDs::segment (0), 1.0f);
        q.setParameterFromUI (ParamIDs::segment (1), 0.0f);
        const int moved = hit < segLength ? hit + segLength : hit < 2 * segLength ? hit - segLength : hit;
        CHECK (q.stepVelocity (false, moved) == 30);

        // RND BASS keeps the piano levels, RND PIANO clears them; a library groove has none.
        p.setStepVelocity (true, hit, 50);
        p.rerollHand (true);
        CHECK (p.stepVelocity (false, hit) == 30 && p.stepVelocity (true, hit) == 0);
        p.rerollHand (false);
        CHECK (! p.hasStepVelocities());
        p.setStepVelocity (false, hit, 30);
        p.setParameterFromUI (ParamIDs::pattern, 3.0f);
        CHECK (! p.hasStepVelocities());
        p.editStep (false, hit, 'X');   // the library groove becomes yours again: no old levels
        CHECK (! p.hasStepVelocities());

        // On the velocity page the random buttons change the levels only, and keep locked parts.
        const auto rhythm = sgk::GrooveEngine::resolvePattern (p.currentSettings());
        p.setVelocityLocked (1, true);
        p.randomizeVelocities (true, false);
        const auto after = sgk::GrooveEngine::resolvePattern (p.currentSettings());
        CHECK (after.rightHand == rhythm.rightHand && after.leftHand == rhythm.leftHand);
        int set = 0, lockedSet = 0, bassSet = 0;
        for (int i = 0; i < sgk::userPatternLength; ++i)
        {
            const bool hitHere = rhythm.rightHand[(size_t) i] != '.' && rhythm.rightHand[(size_t) i] != '=';
            const int level = p.stepVelocity (false, i);
            if (hitHere && level > 0) ++set;
            if (! hitHere) CHECK (level == 0);
            if (i / 8 == 1 && level > 0) ++lockedSet;
            if (p.stepVelocity (true, i) > 0) ++bassSet;
        }
        CHECK (set > 0 && lockedSet == 0 && bassSet == 0);

        // Velocity order A A C D: the second half bar plays the first one's levels.
        p.setVelocityPartAt (1, 0);
        for (int i = 0; i < 8; ++i)
            CHECK (p.stepVelocity (false, 8 + i) == p.stepVelocity (false, i));
        juce::MemoryBlock saved;
        p.getStateInformation (saved);
        FaseGroovePianoProcessor r;
        r.setStateInformation (saved.getData(), (int) saved.getSize());
        CHECK (r.velocityPartAt (1) == 0 && r.isVelocityLocked (1));
    }

    void testSidechain()
    {
        // The pump ducks the guide sound with the tempo (internal) or with the sidechain input.
        auto loudness = [] (int source, bool feedSidechain)
        {
            FaseGroovePianoProcessor p;
            juce::AudioProcessor::BusesLayout layout;
            layout.inputBuses.add (juce::AudioChannelSet::stereo());
            layout.outputBuses.add (juce::AudioChannelSet::stereo());
            CHECK (p.setBusesLayout (layout));
            p.setParameterFromUI (ParamIDs::pump, source < 0 ? 0.0f : 1.0f);
            p.setParameterFromUI (ParamIDs::pumpSource, (float) juce::jmax (0, source));
            p.setParameterFromUI (ParamIDs::reverb, 0.0f);
            PlayHead head;
            p.setPlayHead (&head);
            p.prepareToPlay (44100.0, 512);
            double sum = 0.0;
            juce::MidiBuffer midi;
            for (int block = 0; block < 400; ++block)
            {
                juce::AudioBuffer<float> buffer (p.getTotalNumInputChannels() > p.getTotalNumOutputChannels() ? p.getTotalNumInputChannels() : p.getTotalNumOutputChannels(), 512);
                buffer.clear();
                if (feedSidechain)
                    for (int ch = 0; ch < 2; ++ch)
                        for (int i = 0; i < 512; ++i)
                            buffer.setSample (ch, i, 0.8f * std::sin ((float) (block * 512 + i) * 0.05f));
                midi.clear();
                p.processBlock (buffer, midi);
                head.ppq += 512.0 / 44100.0 * head.bpm / 60.0;
                for (int i = 0; i < 512; ++i)
                    sum += std::abs (buffer.getSample (0, i));
            }
            p.setPlayHead (nullptr);
            return sum;
        };

        const double dry = loudness (-1, false);
        CHECK (dry > 0.0);
        CHECK (loudness (0, false) < dry * 0.9);                  // tempo pump
        CHECK (loudness (1, true) < dry * 0.4);                   // a loud sidechain ducks hard
        CHECK (std::abs (loudness (1, false) - dry) < dry * 0.01);   // a silent one does nothing
    }

    void testSequencerOff()
    {
        // Sequencer off: with the DAW playing nothing is generated, incoming MIDI still passes.
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::sequencer, 0.0f);
        PlayHead head;
        p.setPlayHead (&head);
        p.prepareToPlay (44100.0, 512);
        int generated = 0, passed = 0;
        for (int block = 0; block < 200; ++block)
        {
            juce::AudioBuffer<float> buffer (2, 512);
            juce::MidiBuffer midi;
            if (block == 10) midi.addEvent (juce::MidiMessage::noteOn (1, 61, (juce::uint8) 100), 0);
            if (block == 20) midi.addEvent (juce::MidiMessage::noteOff (1, 61), 0);
            p.processBlock (buffer, midi);
            head.ppq += 512.0 / 44100.0 * head.bpm / 60.0;
            for (const auto m : midi)
                if (m.getMessage().isNoteOn())
                    (m.getMessage().getNoteNumber() == 61 && block == 10 ? passed : generated) += 1;
        }
        CHECK (generated == 0 && passed == 1);

        // On again: the groove plays.
        p.setParameterFromUI (ParamIDs::sequencer, 1.0f);
        for (int block = 0; block < 100; ++block)
        {
            juce::AudioBuffer<float> buffer (2, 512);
            juce::MidiBuffer midi;
            p.processBlock (buffer, midi);
            head.ppq += 512.0 / 44100.0 * head.bpm / 60.0;
            for (const auto m : midi)
                if (m.getMessage().isNoteOn()) ++generated;
        }
        CHECK (generated > 0);
        p.setPlayHead (nullptr);
    }

    void testLaunchpadLayout()
    {
        // Launchpad: pads <-> grid steps, lights for the hits and the playhead, presses edit the groove.
        CHECK (PadController::padNote (0, 0) == 81 && PadController::padNote (7, 7) == 18);
        CHECK (PadController::topButton (0) == 91 && PadController::sideButton (0) == 89 && PadController::sideButton (7) == 19);

        std::set<std::pair<bool, int>> seen;
        for (int row = 0; row < 8; ++row)
            for (int column = 0; column < 8; ++column)
            {
                bool bass = false;
                int step = 0;
                PadController::padToStep (row, column, bass, step);
                CHECK (bass == (row >= 4) && step >= 0 && step < 32);
                seen.insert ({ bass, step });
            }
        CHECK (seen.size() == 64);

        FaseGroovePianoProcessor p;
        auto& pads = *p.pads;
        const auto pattern = sgk::GrooveEngine::resolvePattern (p.currentSettings());
        const auto leds = pads.computeLeds();
        for (int column = 0; column < 8; ++column)
        {
            const char c = pattern.rightHand[(size_t) column];
            CHECK ((leds.pads[(size_t) column] > 1) == (c != '.'));   // top row = chords, steps 1-8
        }

        // Pressing the top-left pad cycles step 1 of the chords like a click on the grid.
        const char before = p.stepSymbol (false, 0);
        pads.handlePress (81, false);
        const char after = p.stepSymbol (false, 0);
        CHECK (after == (before == 'x' ? 'X' : before == 'X' ? 'g' : before == 'g' ? '.' : 'x'));

        // Piano off (bass only): its rows go dark, like the bass rows when the bass is off.
        p.hearPart (false);
        CHECK (p.chordsMuted.load());
        for (int i = 0; i < 32; ++i)
            CHECK (pads.computeLeds().pads[(size_t) i] == 0);
        p.hearPart (false);
        CHECK (! p.chordsMuted.load());

        // Round buttons: sequencer off / on, lock part A.
        pads.handlePress (96, true);
        CHECK (p.apvts.getRawParameterValue (ParamIDs::sequencer)->load() < 0.5f);
        pads.handlePress (96, true);
        CHECK (p.apvts.getRawParameterValue (ParamIDs::sequencer)->load() > 0.5f);
        // Right column, bottom four: NEW RIFF (a new random groove).
        const auto grooveBefore = sgk::GrooveEngine::resolvePattern (p.currentSettings());
        bool newRiff = false;
        for (int tries = 0; tries < 5 && ! newRiff; ++tries)
        {
            pads.handlePress (49, true);
            const auto now = sgk::GrooveEngine::resolvePattern (p.currentSettings());
            newRiff = now.rightHand != grooveBefore.rightHand || now.leftHand != grooveBefore.leftHand;
        }
        CHECK (newRiff);
        CHECK (! p.isSegmentLocked (p.segmentAtStep (0)));
        CHECK (PadController::enabled() == false || pads.status() != PadController::Status::taken);

        // Push (User mode): pads 36 (bottom left) .. 99 (top right); the same steps and buttons.
        CHECK (PadController::pushPadNote (0, 0) == 92 && PadController::pushPadNote (7, 7) == 43);
        const char beforePush = p.stepSymbol (false, 1);
        pads.handlePushPress (93, false);   // top row, second pad = chords step 2
        const char afterPush = p.stepSymbol (false, 1);
        CHECK (afterPush == (beforePush == 'x' ? 'X' : beforePush == 'X' ? 'g' : beforePush == 'g' ? '.' : 'x'));
        pads.handlePushPress (107, true);   // sixth button above the display = sequencer
        CHECK (p.apvts.getRawParameterValue (ParamIDs::sequencer)->load() < 0.5f);
        const auto beforeScene = sgk::GrooveEngine::resolvePattern (p.currentSettings());
        bool pushRiff = false;
        for (int tries = 0; tries < 5 && ! pushRiff; ++tries)
        {
            pads.handlePushPress (39, true);   // scene button 5 = NEW RIFF
            const auto now = sgk::GrooveEngine::resolvePattern (p.currentSettings());
            pushRiff = now.rightHand != beforeScene.rightHand || now.leftHand != beforeScene.leftHand;
        }
        CHECK (pushRiff);
    }

    void testTotalRandom()
    {
        // TOTAL RANDOM changes the music (key, chords or groove) and stays playable.
        FaseGroovePianoProcessor p;
        bool changed = false;
        for (int round = 0; round < 8; ++round)
        {
            const auto before = stateXml (p);
            p.totalRandom();
            changed = changed || stateXml (p) != before;
            const auto s = p.currentSettings();
            CHECK (s.progressionLength >= 1 && s.progressionLength <= sgk::maxProgressionLength);
            int notes = 0;
            for (const auto& e : sgk::GrooveEngine::renderClip (s, 16.0))
                if (e.isNoteOn()) ++notes;
            CHECK (notes > 8);
        }
        CHECK (changed);
    }

    void testViewModeRestore()
    {
        // The simple / advanced view is saved with the project and restored, also over an open state.
        FaseGroovePianoProcessor p;
        CHECK (! p.advancedView);
        p.advancedView = true;
        juce::MemoryBlock advanced;
        p.getStateInformation (advanced);
        p.advancedView = false;
        juce::MemoryBlock simple;
        p.getStateInformation (simple);

        p.setStateInformation (advanced.getData(), (int) advanced.getSize());
        CHECK (p.advancedView);
        p.setStateInformation (simple.getData(), (int) simple.getSize());
        CHECK (! p.advancedView);
    }

    void testGrooveFromMidiFile()
    {
        // A MIDI clip exported by the plug-in, imported back: its rhythm becomes the user pattern.
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::pattern, 5.0f);
        p.setParameterFromUI (ParamIDs::humanize, 0.0f);
        p.setParameterFromUI (ParamIDs::swing, 0.0f);
        const auto before = sgk::GrooveEngine::resolvePattern (p.currentSettings());
        const auto file = p.writeMidiClip (2, FaseGroovePianoProcessor::ClipPart::all);
        const auto analysis = FaseGroovePianoProcessor::analyzeMidiFile (file);
        CHECK (analysis.groove.length() == sgk::userPatternLength);

        p.setParameterFromUI (ParamIDs::pattern, 1.0f);
        p.applyGroove (analysis.groove);
        CHECK (juce::roundToInt (value (p, ParamIDs::pattern)) == sgk::userPatternIndex());
        const auto after = sgk::GrooveEngine::resolvePattern (p.currentSettings());
        int same = 0, total = 0;
        for (int i = 0; i < 32; ++i)
        {
            const bool a = before.rightHand[(size_t) i] != '.' && before.rightHand[(size_t) i] != '=';
            const bool b = after.rightHand[(size_t) i] != '.' && after.rightHand[(size_t) i] != '=';
            same += (a == b) ? 1 : 0;
            ++total;
        }
        CHECK (same >= total - 2);   // the same chord rhythm (a push may land a step apart)
    }

    void testLiveKeys()
    {
        // Live keys: the lowest octave picks the section (and doesn't sound); off, it is just notes.
        FaseGroovePianoProcessor p;
        const bool before = FaseGroovePianoProcessor::liveKeysEnabled();
        FaseGroovePianoProcessor::setLiveKeysEnabled (true);
        p.prepareToPlay (44100.0, 256);
        juce::AudioBuffer<float> buffer (2, 256);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, FaseGroovePianoProcessor::liveKeysLowest + 7, (juce::uint8) 100), 0);   // G = drop
        p.processBlock (buffer, midi);
        bool passed = false;
        for (const auto m : midi)
            passed = passed || (m.getMessage().isNoteOn() && m.getMessage().getNoteNumber() == FaseGroovePianoProcessor::liveKeysLowest + 7);
        CHECK (! passed);
        p.runLiveKey (7);
        CHECK (juce::roundToInt (value (p, ParamIDs::section)) == (int) sgk::Section::drop);
        p.runLiveKey (0);
        CHECK (juce::roundToInt (value (p, ParamIDs::section)) == (int) sgk::Section::intro);
        FaseGroovePianoProcessor::setLiveKeysEnabled (before);
    }

    void testMidiClock()
    {
        // No host position (the standalone app): a MIDI clock at 124 BPM sets the tempo and,
        // with Start, plays the groove.
        FaseGroovePianoProcessor p;
        p.setPlayHead (nullptr);
        p.prepareToPlay (44100.0, 256);
        const double samplesPerTick = 44100.0 * 60.0 / 124.0 / 24.0;
        double nextTick = 0.0;
        juce::int64 now = 0;
        int notes = 0;
        for (int block = 0; block < 400; ++block)
        {
            juce::AudioBuffer<float> buffer (2, 256);
            juce::MidiBuffer midi;
            if (block == 2) midi.addEvent (juce::MidiMessage::midiStart(), 0);
            while (nextTick < (double) (now + 256))
            {
                midi.addEvent (juce::MidiMessage::midiClock(), (int) (nextTick - (double) now));
                nextTick += samplesPerTick;
            }
            p.processBlock (buffer, midi);
            for (const auto m : midi)
            {
                CHECK (! m.getMessage().isMidiClock());
                if (m.getMessage().isNoteOn()) ++notes;
            }
            now += 256;
        }
        CHECK (p.clockSync.load());
        CHECK (std::abs (p.hostBpm.load() - 124.0) < 0.3);
        CHECK (notes > 0);
    }

    void testSongSectionClip()
    {
        // One section of the song as its own clip, named after it.
        FaseGroovePianoProcessor p;
        const auto parts = p.songArrangementParts();
        int intro = -1;
        for (int i = 0; i < (int) parts.size(); ++i)
            if (parts[(size_t) i].section == sgk::Section::intro && parts[(size_t) i].bars > 0) intro = i;
        CHECK (intro >= 0);
        const auto file = p.writeSongSectionClip (intro, FaseGroovePianoProcessor::ClipPart::all);
        CHECK (file.existsAsFile());
        CHECK (file.getFileName().startsWith ("Piano " + Localisation::tr ("Intro")));
        const auto clip = readClip (file, true);
        CHECK (clip.noteOns > 0);
        CHECK (! p.writeSongSectionClip (99, FaseGroovePianoProcessor::ClipPart::all).existsAsFile());
        CHECK (p.writeMidiClip (2, FaseGroovePianoProcessor::ClipPart::all).getFileName().startsWith ("Piano "));
    }
    void testStrings()
    {
        FaseGroovePianoProcessor p;
        CHECK (! p.isStringsOn());   // off by default: old projects sound the same
        CHECK (readClip (p.writeMidiClip (2, FaseGroovePianoProcessor::ClipPart::all), false).onChannel3 == 0);

        // On / off remembers the mode.
        p.toggleStrings();
        CHECK (juce::roundToInt (value (p, ParamIDs::strings)) == (int) sgk::StringsMode::pad);
        p.setParameterFromUI (ParamIDs::strings, (float) sgk::StringsMode::widePad);
        p.toggleStrings();
        CHECK (! p.isStringsOn());
        p.toggleStrings();
        CHECK (juce::roundToInt (value (p, ParamIDs::strings)) == (int) sgk::StringsMode::widePad);

        // Clips: everything (strings on channel 3), chords only without them, strings only.
        const auto all = readClip (p.writeMidiClip (2, FaseGroovePianoProcessor::ClipPart::all), false);
        CHECK (all.onChannel3 > 0 && all.unmatched == 0 && all.channelsOk);
        CHECK (readClip (p.writeMidiClip (2, FaseGroovePianoProcessor::ClipPart::chords), false).onChannel3 == 0);
        const auto stringsFile = p.writeMidiClip (2, FaseGroovePianoProcessor::ClipPart::strings);
        CHECK (stringsFile.getFileName().startsWith ("Strings "));
        const auto only = readClip (stringsFile, false);
        CHECK (only.noteOns > 0 && only.noteOns == only.onChannel3 && only.unmatched == 0);
        const auto song = readClip (p.writeMidiClip (FaseGroovePianoProcessor::songArrangement, FaseGroovePianoProcessor::ClipPart::strings), false);
        CHECK (song.noteOns > 0 && song.unmatched == 0);
        {
            juce::FileInputStream in (p.writeMidiClip (2, FaseGroovePianoProcessor::ClipPart::tracks));
            juce::MidiFile mf;
            CHECK (mf.readFrom (in));
            bool stringsTrack = false;
            for (int t = 0; t < mf.getNumTracks(); ++t)
                for (auto* ev : *mf.getTrack (t))
                    if (ev->message.isTextMetaEvent() && ev->message.getTextFromTextMetaEvent().startsWith ("Strings ")) stringsTrack = true;
            CHECK (stringsTrack);
        }

        // Playing: the strings come out on channel 3, and muting the piano leaves them on.
        p.chordsMuted = true;
        PlayHead head;
        p.setPlayHead (&head);
        p.prepareToPlay (44100.0, 512);
        int onThree = 0, onOne = 0;
        for (int block = 0; block < 200; ++block)
        {
            juce::AudioBuffer<float> buffer (2, 512);
            juce::MidiBuffer midi;
            p.processBlock (buffer, midi);
            head.ppq += 512.0 / 44100.0 * head.bpm / 60.0;
            for (const auto m : midi)
                if (m.getMessage().isNoteOn())
                    (m.getMessage().getChannel() == 3 ? onThree : onOne) += 1;
        }
        CHECK (onThree > 0);
        p.setPlayHead (nullptr);

        // The piano roll shows (and edits) the strings too; edited strings are saved and loaded.
        auto loop = p.generatedLoop (8.0);
        CHECK (std::any_of (loop.begin(), loop.end(), [] (const sgk::LoopNote& n) { return n.part == sgk::Part::strings; }));
        for (auto& n : loop)
            if (n.part == sgk::Part::strings) n.velocity = 33;
        p.setEditedLoop (loop, 8.0);
        juce::MemoryBlock state;
        p.getStateInformation (state);
        FaseGroovePianoProcessor q;
        q.setStateInformation (state.getData(), (int) state.getSize());
        const auto restored = q.editedLoop();
        CHECK (restored != nullptr && std::any_of (restored->begin(), restored->end(),
                                                   [] (const sgk::LoopNote& n) { return n.part == sgk::Part::strings && n.velocity == 33; }));
    }
    void testHearIsOnlyAMute()
    {
        // HEAR BASS off mutes the bass: the notes (the piano's voicing too), the roll and the
        // exports stay the same; it comes back as it was.
        FaseGroovePianoProcessor p;
        const auto before = p.generatedLoop (8.0);
        p.hearPart (true);
        CHECK (p.isBassOn() && ! p.isBassHeard());
        const auto muted = p.generatedLoop (8.0);
        CHECK (muted.size() == before.size());
        bool same = muted.size() == before.size();
        for (size_t i = 0; same && i < muted.size(); ++i)
            same = muted[i].note == before[i].note && muted[i].part == before[i].part;
        CHECK (same);

        PlayHead head;
        p.setPlayHead (&head);
        p.prepareToPlay (44100.0, 512);
        int bassOn = 0, others = 0;
        for (int block = 0; block < 200; ++block)
        {
            juce::AudioBuffer<float> buffer (2, 512);
            juce::MidiBuffer midi;
            p.processBlock (buffer, midi);
            head.ppq += 512.0 / 44100.0 * head.bpm / 60.0;
            for (const auto m : midi)
                if (m.getMessage().isNoteOn())
                    (m.getMessage().getNoteNumber() < 48 ? bassOn : others) += 1;
        }
        CHECK (others > 0);
        p.setPlayHead (nullptr);
        p.hearPart (true);
        CHECK (p.isBassHeard());
    }
    void testLastGrooveName()
    {
        // MY GROOVES offers the name of the groove saved or opened last (kept with the project).
        FaseGroovePianoProcessor p;
        const juce::String name = "zz test groove name";
        CHECK (p.saveGroove (name));
        CHECK (p.lastGrooveName == name);
        juce::MemoryBlock state;
        p.getStateInformation (state);
        FaseGroovePianoProcessor q;
        q.setStateInformation (state.getData(), (int) state.getSize());
        CHECK (q.lastGrooveName == name);
        FaseGroovePianoProcessor r;
        CHECK (r.loadGroove (FaseGroovePianoProcessor::grooveFolder().getChildFile (name + ".fgg")));
        CHECK (r.lastGrooveName == name);
        FaseGroovePianoProcessor::grooveFolder().getChildFile (name + ".fgg").deleteFile();
    }
    void testSegmentDiceKeepsOthers()
    {
        // The dice of one part (B) changes only that part of the grid.
        for (int start : { 0, 1, 2 })
        {
            FaseGroovePianoProcessor p;
            if (start == 1) p.rollNewRiff();
            if (start == 2) p.setParameterFromUI (ParamIDs::pattern, 30.0f);
            const auto before = sgk::GrooveEngine::resolvePattern (p.currentSettings());
            p.randomizeSegment (1);
            const auto after = sgk::GrooveEngine::resolvePattern (p.currentSettings());
            int changed = 0;
            for (int i = 0; i < sgk::userPatternLength; ++i)
                if (i / 8 != 1 && i != 16 && (before.rightHand[(size_t) i] != after.rightHand[(size_t) i] || before.leftHand[(size_t) i] != after.leftHand[(size_t) i]))
                    ++changed;
            if (changed != 0)
                std::printf ("start %d\n%s\n%s\n%s\n%s\n", start, before.rightHand.c_str(), after.rightHand.c_str(), before.leftHand.c_str(), after.leftHand.c_str());
            CHECK (changed == 0);
        }
    }
    void testSongTracksHaveFills()
    {
        // The whole song as separate tracks: the fills have their own track (when there are fills).
        for (int fills : { 0, 2 })
        {
            FaseGroovePianoProcessor p;
            p.setParameterFromUI (ParamIDs::fills, (float) fills);
            juce::FileInputStream in (p.writeMidiClip (FaseGroovePianoProcessor::songArrangement, FaseGroovePianoProcessor::ClipPart::tracks));
            juce::MidiFile mf;
            CHECK (mf.readFrom (in));
            std::map<juce::String, int> notes;
            for (int t = 1; t < mf.getNumTracks(); ++t)
            {
                juce::String name;
                int count = 0;
                for (auto* ev : *mf.getTrack (t))
                {
                    if (ev->message.isTextMetaEvent() && ev->message.getMetaEventType() == 3) name = ev->message.getTextFromTextMetaEvent().upToFirstOccurrenceOf (" ", false, false);
                    if (ev->message.isNoteOn()) ++count;
                }
                notes[name] = count;
            }
            const auto all = sgk::GrooveEngine::renderArrangement (p.currentSettings(), p.songArrangementParts());
            int fillOns = 0;
            for (const auto& e : all) if (e.isNoteOn() && e.part == sgk::Part::fill) ++fillOns;
            std::printf ("fills param %d: engine fill notes %d, tracks:", fills, fillOns);
            for (const auto& [n, c] : notes) std::printf (" %s=%d", n.toRawUTF8(), c);
            std::printf ("\n");
            CHECK (notes["Fills"] == fillOns);
            if (fills > 0) CHECK (fillOns > 0);
        }
    }
    void testClipsSameLength()
    {
        // Piano, bass and strings clips of the same export reach exactly the same end, even when
        // an instrument stops early (the song's outro has no bass): DAWs size the clip by it.
        FaseGroovePianoProcessor p;
        p.setParameterFromUI (ParamIDs::strings, (float) sgk::StringsMode::pad);
        using CP = FaseGroovePianoProcessor::ClipPart;
        auto lastEvent = [] (const juce::File& f)
        {
            juce::FileInputStream in (f);
            juce::MidiFile mf;
            mf.readFrom (in);
            double last = 0.0;
            for (int t = 0; t < mf.getNumTracks(); ++t)
                for (auto* ev : *mf.getTrack (t))
                    if (! ev->message.isMetaEvent())
                        last = juce::jmax (last, ev->message.getTimeStamp());
            return last;
        };
        for (int bars : { 2, FaseGroovePianoProcessor::songArrangement })
        {
            const double piano = lastEvent (p.writeMidiClip (bars, CP::chords));
            CHECK (piano > 0.0);
            CHECK (juce::approximatelyEqual (lastEvent (p.writeMidiClip (bars, CP::bass)), piano));
            CHECK (juce::approximatelyEqual (lastEvent (p.writeMidiClip (bars, CP::strings)), piano));
            CHECK (juce::approximatelyEqual (lastEvent (p.writeMidiClip (bars, CP::all)), piano));
        }
        // One song section too (the outro has no bass at all).
        const auto parts = p.songArrangementParts();
        for (int i = 0; i < (int) parts.size(); ++i)
            if (parts[(size_t) i].bars > 0)
                CHECK (juce::approximatelyEqual (lastEvent (p.writeSongSectionClip (i, CP::bass)), lastEvent (p.writeSongSectionClip (i, CP::chords))));
    }
    void testSectionTrackNames()
    {
        // A song section as separate tracks: its tracks are named "Piano Intro", "Bass Intro"...
        FaseGroovePianoProcessor p;
        const auto parts = p.songArrangementParts();
        for (int i = 0; i < (int) parts.size(); ++i)
        {
            if (parts[(size_t) i].bars <= 0) continue;
            const auto sectionText = Localisation::tr (sgk::sectionName ((int) parts[(size_t) i].section));
            juce::FileInputStream in (p.writeSongSectionClip (i, FaseGroovePianoProcessor::ClipPart::tracks));
            juce::MidiFile mf;
            CHECK (mf.readFrom (in));
            for (int t = 1; t < mf.getNumTracks(); ++t)
                for (auto* ev : *mf.getTrack (t))
                    if (ev->message.isTrackNameEvent())
                        CHECK (ev->message.getTextFromTextMetaEvent().endsWith (" " + sectionText));
        }
    }
}

namespace
{
    void testLinkWithDrums()
    {
        // LINK: the genre and swing written by Fase Groove Drums, taken here; ours written back.
        const auto link = FaseGroovePianoProcessor::linkFile();
        const bool existed = link.existsAsFile();
        const auto backup = link.loadFileAsString();
        auto value = [] (FaseGroovePianoProcessor& p, const char* id) { return p.apvts.getRawParameterValue (id)->load(); };
        auto categoryOf = [&] (FaseGroovePianoProcessor& p)
        {
            const int i = juce::roundToInt (value (p, ParamIDs::pattern));
            return juce::isPositiveAndBelow (i, (int) sgk::groovePatterns().size()) ? juce::String (sgk::groovePatterns()[(size_t) i].category) : juce::String();
        };

        FaseGroovePianoProcessor p;
        p.setLinked (true);
        p.updateLink();   // shares its own first
        juce::XmlElement drums ("FaseGrooveLink");
        drums.setAttribute ("from", "Fase Groove Drums");
        drums.setAttribute ("instance", "1234");
        drums.setAttribute ("genre", "Deep");
        drums.setAttribute ("swing", 0.5);
        link.getParentDirectory().createDirectory();
        CHECK (drums.writeTo (link));
        link.setLastModificationTime (juce::Time::getCurrentTime() + juce::RelativeTime::seconds (5.0));
        p.updateLink();
        CHECK (categoryOf (p) == "Deep");
        CHECK (std::abs (value (p, ParamIDs::swing) - 0.5f) < 0.01f);

        // A heavier swing (up to 1.5, as Drums and Guitar) is the same in the piano.
        drums.setAttribute ("swing", 1.2);
        drums.writeTo (link);
        link.setLastModificationTime (juce::Time::getCurrentTime() + juce::RelativeTime::seconds (10.0));
        p.updateLink();
        CHECK (std::abs (value (p, ParamIDs::swing) - 1.2f) < 0.01f);

        // Our change goes to the file.
        for (size_t i = 0; i < sgk::groovePatterns().size(); ++i)
            if (sgk::groovePatterns()[i].category == "Jackin'") { p.setParameterFromUI (ParamIDs::pattern, (float) i); break; }
        p.updateLink();
        if (auto xml = juce::parseXML (link))
            CHECK (xml->getStringAttribute ("genre") == "Jackin'");
        else
            CHECK (false);

        // A random or edited groove still shares the genre of its last preset, and keeps itself
        // when another genre comes in (only the genre follows).
        const float own = (float) sgk::groovePatterns().size();
        p.setParameterFromUI (ParamIDs::pattern, own);
        p.updateLink();
        if (auto xml = juce::parseXML (link))
            CHECK (xml->getStringAttribute ("genre") == "Jackin'");
        drums.setAttribute ("genre", "Soulful");
        drums.writeTo (link);
        link.setLastModificationTime (juce::Time::getCurrentTime() + juce::RelativeTime::seconds (15.0));
        p.updateLink();
        CHECK (std::abs (value (p, ParamIDs::pattern) - own) < 0.5f);
        CHECK (p.currentGenre() == 1);   // Soulful
        for (size_t i = 0; i < sgk::groovePatterns().size(); ++i)
            if (sgk::groovePatterns()[i].category == "Jackin'") { p.setParameterFromUI (ParamIDs::pattern, (float) i); break; }
        p.updateLink();

        // The tempo and the play button: the drums started playing a second ago at 120 BPM, so
        // the piano plays from beat 2 too.
        drums.setAttribute ("bpm", 120.0);
        drums.setAttribute ("sync", false);
        drums.setAttribute ("playing", true);
        drums.setAttribute ("anchor", juce::String (juce::Time::getMillisecondCounterHiRes() - 1000.0, 3));
        drums.writeTo (link);
        p.updateLink();
        CHECK (p.previewOn.load());
        CHECK (value (p, ParamIDs::tempoSync) < 0.5f);
        CHECK (std::abs (value (p, ParamIDs::tempo) - 120.0f) < 0.01f);
        p.setRateAndBufferSizeDetails (44100.0, 512);
        p.prepareToPlay (44100.0, 512);
        {
            juce::AudioBuffer<float> out (2, 512);
            juce::MidiBuffer none;
            p.processBlock (out, none);
        }
        CHECK (std::abs (p.uiPpq.load() - 2.0) < 0.05);
        drums.setAttribute ("playing", false);
        drums.setAttribute ("anchor", "-1");
        drums.writeTo (link);
        p.updateLink();
        CHECK (! p.previewOn.load());

        // Saved with the project.
        juce::MemoryBlock data;
        p.getStateInformation (data);
        FaseGroovePianoProcessor q;
        q.setStateInformation (data.getData(), (int) data.getSize());
        CHECK (q.isLinked());

        if (existed) link.replaceWithText (backup); else link.deleteFile();
    }
    void testLinkRoles()
    {
        using P = FaseGroovePianoProcessor;
        // Comping under a leading guitar: a chord on the first beat of every other bar and a
        // push into each four-bar phrase, nothing else.
        CHECK (P::compingKeeps (0.0) && P::compingKeeps (0.5));
        CHECK (! P::compingKeeps (1.5) && ! P::compingKeeps (2.0));
        CHECK (! P::compingKeeps (4.0));       // bar 2: nothing
        CHECK (P::compingKeeps (8.0));         // bar 3
        CHECK (P::compingKeeps (15.5));        // the push at the end of bar 4
        CHECK (! P::compingKeeps (7.5));
    }

}

int main()
{
    juce::ScopedJuceInitialiser_GUI juce;

    testStateRoundTrip();
    testOldAndBrokenStates();
    testPresetFiles();
    testMidiExport();
    testTransport();
    testMidiImport();
    testBassLineControls();
    testMorphAndSong();
    testMusicalDna();
    testV124();
    testExportTempoAndChannels();
    testSectionFillsNoStuckNotes();
    testUpdateSwap();
    testEditedNotes();
    testLibrary();
    testRandomPianoKeepsBass();
    testStepVelocities();
    testSidechain();
    testSequencerOff();
    testLaunchpadLayout();
    testTotalRandom();
    testViewModeRestore();
    testGrooveFromMidiFile();
    testLiveKeys();
    testMidiClock();
    testSongSectionClip();
    testStrings();
    testHearIsOnlyAMute();
    testLastGrooveName();
    testSegmentDiceKeepsOthers();
    testSongTracksHaveFills();
    testClipsSameLength();
    testSectionTrackNames();
    testLinkWithDrums();
    testLinkRoles();

    if (failures == 0)
        std::printf ("All plug-in tests passed.\n");
    return failures == 0 ? 0 : 1;
}
