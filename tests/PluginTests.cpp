// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Plug-in level tests (they need JUCE, unlike EngineTests): project state and presets,
// undo snapshots, exported MIDI files, DAW transport (play, stop, seek, loop, tempo changes,
// odd block sizes) and the portable update's all-or-nothing file swap.

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
    struct ClipCheck { int noteOns = 0, unmatched = 0, markers = 0; double lastTick = 0.0; bool ordered = true, channelsOk = true; };

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

        // Bass on channel 2.
        p.setParameterFromUI (ParamIDs::bassChannel, 1.0f);
        const auto split = readClip (p.writeMidiClip (3, FaseGroovePianoProcessor::ClipPart::all), true);
        CHECK (split.unmatched == 0);
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
}

int main()
{
    juce::ScopedJuceInitialiser_GUI juce;

    testStateRoundTrip();
    testOldAndBrokenStates();
    testMidiExport();
    testTransport();
    testMidiImport();
    testUpdateSwap();

    if (failures == 0)
        std::printf ("All plug-in tests passed.\n");
    return failures == 0 ? 0 : 1;
}
