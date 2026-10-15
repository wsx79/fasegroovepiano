// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Plug-in level tests (they need JUCE, unlike EngineTests): project state and presets,
// undo snapshots, exported MIDI files, DAW transport (play, stop, seek, loop, tempo changes,
// odd block sizes) and the portable update's all-or-nothing file swap.

#include <tuple>
#include "../Source/PluginProcessor.h"
#include "../Source/Updater.h"

#include <cstdio>
#include <map>

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
    struct ClipCheck { int noteOns = 0, unmatched = 0, markers = 0, onChannel1 = 0, onChannel2 = 0; double lastTick = 0.0; bool ordered = true, channelsOk = true; };

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
                if (m.getChannel() > 2) c.channelsOk = false;
                if (! bassOnChannel2 && m.getChannel() != 1) c.channelsOk = false;
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
        CHECK (pianoName == "Piano" && a1 > 0 && a2 == 0);
        CHECK (bassName == "Bass" && b2 > 0 && b1 == 0);
        CHECK (fillName == "Fills" && f1 > 0);
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

    if (failures == 0)
        std::printf ("All plug-in tests passed.\n");
    return failures == 0 ? 0 : 1;
}
