// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

// Unit tests for the music engine (no JUCE needed).

#include "../Source/engine/GrooveEngine.h"
#include "../Source/engine/Theory.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <set>
#include <string>
#include <vector>

using namespace sgk;

static int failures = 0;

#define CHECK(cond) \
    do { if (! (cond)) { std::printf ("FAIL %s:%d  %s\n", __FILE__, __LINE__, #cond); ++failures; } } while (0)

static void checkName (int key, int scale, int degree, Extension ext, const std::string& expected)
{
    const auto name = chordName (buildChord (key, scale, degree, ext));
    if (name != expected)
    {
        std::printf ("FAIL chord name: got '%s', expected '%s'\n", name.c_str(), expected.c_str());
        ++failures;
    }
}

static void testChordNames()
{
    const int A = 9, C = 0;

    checkName (A, 1, 0, Extension::ninth,    "Am9");      // A Dorian i
    checkName (A, 1, 3, Extension::ninth,    "D9");       // A Dorian IV
    checkName (A, 1, 3, Extension::eleventh, "D13");
    checkName (A, 1, 0, Extension::eleventh, "Am11");
    checkName (C, 0, 0, Extension::seventh,  "Cmaj7");
    checkName (C, 0, 3, Extension::ninth,    "Fmaj9");
    checkName (C, 0, 4, Extension::eleventh, "G13");
    checkName (C, 0, 1, Extension::ninth,    "Dm9");
    checkName (C, 0, 2, Extension::ninth,    "Em7");      // b9 dropped
    // v1.5 extensions
    checkName (C, 0, 0, Extension::sixNine,  "C6/9");
    checkName (A, 1, 0, Extension::sixNine,  "Am6/9");    // Dorian: natural 6
    checkName (A, 2, 0, Extension::sixNine,  "Am9");      // Aeolian b6 -> 9th chord instead
    checkName (C, 0, 0, Extension::add9,     "Cadd9");
    checkName (C, 0, 1, Extension::add9,     "Dmadd9");
    checkName (C, 0, 4, Extension::sus,      "G9sus4");
    checkName (C, 0, 1, Extension::sus,      "D9sus4");
    checkName (C, 0, 3, Extension::sus,      "Fmaj9");    // #4: no sus
    checkName (C, 0, 4, Extension::alteredDominant, "G13(b9)");
    checkName (A, 2, 4, Extension::alteredDominant, "E7(b9)(b13)");
    checkName (A, 1, 0, Extension::alteredDominant, "Am9");
    checkName (C, 10, 0, Extension::seventh, "C7");       // Lydian dominant
    checkName (C, 9, 3, Extension::seventh,  "Fm(maj7)"); // Harmonic major iv
    checkName (C, 11, 6, Extension::seventh, "Bb7");      // Mixolydian b6 bVII
    CHECK (romanNumeral (4, buildChord (C, 0, 4, Extension::sus)) == "Vsus");
    checkName (C, 0, 2, Extension::eleventh, "Em7(11)");
    checkName (C, 0, 6, Extension::seventh,  "Bm7b5");
    checkName (C, 0, 0, Extension::triad,    "C");
    checkName (C, 0, 1, Extension::triad,    "Dm");
    checkName (C, 0, 6, Extension::triad,    "Bdim");
    checkName (A, 6, 4, Extension::ninth,    "E7(b9)");   // A harmonic minor V
    checkName (A, 6, 6, Extension::seventh,  "Abdim7");   // (G#dim7, spelled with flats)

    CHECK (romanNumeral (0, buildChord (A, 1, 0, Extension::ninth)) == "i");
    CHECK (romanNumeral (3, buildChord (A, 1, 3, Extension::ninth)) == "IV");
}

static void testVoicings()
{
    for (int key = 0; key < 12; ++key)
        for (int scale = 0; scale < numScales; ++scale)
            for (int degree = 0; degree < 7; ++degree)
                for (int ext = 0; ext < numExtensions; ++ext)
                    for (int voicing = 0; voicing < numVoicings; ++voicing)
                    {
                        const auto chord = buildChord (key, scale, degree, (Extension) ext);
                        const auto v = voiceChord (chord, (Voicing) voicing, 64, nullptr);

                        CHECK (v.rightHand.size >= 2);
                        CHECK (v.rightHand.lowest() >= 64 - 20);
                        CHECK (v.rightHand.highest() <= 64 + 20);

                        // Every right-hand note must be a chord / scale tone.
                        std::set<int> allowed;
                        for (int i = 0; i < chord.tones.size; ++i)   allowed.insert ((chord.rootPc + chord.tones[i]) % 12);
                        for (int i = 0; i < chord.quartal.size; ++i) allowed.insert ((chord.rootPc + chord.quartal[i]) % 12);
                        for (int i = 0; i < v.rightHand.size; ++i)
                            CHECK (allowed.count (v.rightHand[i] % 12) == 1);
                    }

    // Voice leading: Am9 -> D9 should move by small steps.
    const auto am9 = voiceChord (buildChord (9, 1, 0, Extension::ninth), Voicing::close, 64, nullptr);
    const auto d9  = voiceChord (buildChord (9, 1, 3, Extension::ninth), Voicing::close, 64, &am9.rightHand);
    CHECK (std::abs (am9.rightHand.mean() - d9.rightHand.mean()) <= 4.0);
}

static void testEngineNoStuckNotes()
{
    for (int pattern = 0; pattern < (int) groovePatterns().size(); ++pattern)
    {
        GrooveSettings s;
        s.pattern = pattern;
        s.fillEveryBars = 2;
        s.bass = BassMode::octaves;
        s.voicing = Voicing::wide;
        s.progressionLength = 4;
        s.progression = { 3, 2, 1, 0, 0, 0, 0, 0 };

        GrooveEngine engine;
        std::vector<NoteEvent> events;

        // Odd-sized blocks, 16 bars.
        double t = 0.0;
        while (t < 64.0)
        {
            const double next = t + 0.0731;
            engine.render (t, next, s, events);
            t = next;
        }
        engine.allNotesOff (t, events);

        int sounding[128] {};
        double lastTime = -1.0;
        int ons = 0;
        bool ordered = true, valid = true;

        for (const auto& e : events)
        {
            if (e.ppq < lastTime - 1.0e-9) ordered = false;
            lastTime = e.ppq;

            if (e.isNoteOn()) { if (sounding[e.note] != 0) valid = false; sounding[e.note] = 1; ++ons; }
            else              { if (sounding[e.note] != 1) valid = false; sounding[e.note] = 0; }
        }

        CHECK (ordered);
        CHECK (valid);
        CHECK (ons > 16);
        CHECK (! engine.anyNoteSounding());
        for (int n = 0; n < 128; ++n)
            CHECK (sounding[n] == 0);
    }
}

static void testBlockSizeIndependence()
{
    GrooveSettings s;
    s.pattern = 3;
    s.humanize = 0.6f;
    s.swing = 0.7f;

    auto run = [&] (double block)
    {
        GrooveEngine engine;
        std::vector<NoteEvent> events;
        for (double t = 0.0; t < 16.0; t += block)
            engine.render (t, std::min (t + block, 16.0), s, events);
        engine.allNotesOff (16.0, events);
        return events;
    };

    const auto a = run (0.5), b = run (0.037);
    CHECK (a.size() == b.size());
    for (size_t i = 0; i < std::min (a.size(), b.size()); ++i)
        CHECK (a[i].note == b[i].note && a[i].velocity == b[i].velocity
               && std::abs (a[i].ppq - b[i].ppq) < 1.0e-9);
}

static void testSwing()
{
    GrooveSettings s;
    s.pattern = 3;      // Funky 16ths has hits on odd steps
    s.humanize = 0.0f;
    s.swing = 1.0f;
    s.bass = BassMode::off;

    GrooveEngine engine;
    std::vector<NoteEvent> events;
    engine.render (0.0, 4.0, s, events);

    // Step 3 (the "a" of beat 1) is an odd 16th, so full swing delays it by a third of a step.
    bool sawSwungHit = false;
    for (const auto& e : events)
        if (e.isNoteOn() && std::abs (e.ppq - (0.75 + 0.25 / 3.0)) < 1.0e-6)
            sawSwungHit = true;

    CHECK (sawSwungHit);
}

static void testPushIsTied()
{
    // "Soulful Push": the push on the "and" of beat 4 must be held into the
    // next bar, not struck again on the downbeat.
    GrooveSettings s;
    s.pattern = 1;
    s.humanize = 0.0f;
    s.swing = 0.0f;
    s.bass = BassMode::off;

    GrooveEngine engine;
    std::vector<NoteEvent> events;
    engine.render (0.0, 12.0, s, events);

    int onsAtBar2 = 0, onsAtPush = 0;
    for (const auto& e : events)
    {
        if (e.isNoteOn() && std::abs (e.ppq - 3.5) < 1.0e-9) ++onsAtPush;
        if (e.isNoteOn() && std::abs (e.ppq - 8.0) < 1.0e-9) ++onsAtBar2;
    }

    CHECK (onsAtPush > 0);
    CHECK (onsAtBar2 == 0);
}

static void testClipExport()
{
    GrooveSettings s;
    s.progressionLength = 3;          // 3 bars of chords vs a 2-bar groove -> 6-bar loop
    s.fillEveryBars = 4;              // ...and fills every 4 bars -> 12 bars
    CHECK (GrooveEngine::loopLengthInSteps (s) == 12 * 16);

    s.fillEveryBars = 0;
    s.bass = BassMode::octaves;
    const auto clip = GrooveEngine::renderClip (s, 6 * 4.0);

    int sounding[128] {}, bassOns = 0, chordOns = 0;
    for (const auto& e : clip)
    {
        CHECK (e.ppq >= 0.0 && e.ppq <= 24.0);
        if (e.isNoteOn()) { ++sounding[e.note]; (e.part == Part::bass ? bassOns : chordOns)++; }
        else              { --sounding[e.note]; }
    }

    for (int n = 0; n < 128; ++n)
        CHECK (sounding[n] == 0);

    CHECK (bassOns > 0 && chordOns > 0);
}

static bool wellFormed (const GroovePattern& p)
{
    if (p.length() != 32 && p.length() != 16) return false;
    if (p.leftHand.size() != p.rightHand.size()) return false;
    for (char c : p.rightHand + p.leftHand)
        if (! (c == 'X' || c == 'x' || c == 'g' || c == '>' || c == '=' || c == '.'))
            return false;
    return true;
}

static int countHits (const std::string& line)
{
    int n = 0;
    for (char c : line)
        if (c == 'X' || c == 'x' || c == 'g' || c == '>') ++n;
    return n;
}

static void testRandomRiffs()
{
    std::set<std::string> distinct;

    for (std::uint32_t seed = 1; seed < 400; ++seed)
    {
        const auto p = makeRandomPattern (seed, (seed % 5) / 4.0f);
        CHECK (wellFormed (p));
        CHECK (p.length() == 32);
        CHECK (countHits (p.rightHand) >= 6);
        CHECK (p.leftHand[0] == 'x' || p.leftHand[0] == '=');
        distinct.insert (p.rightHand);

        // Same seed, same riff.
        CHECK (makeRandomPattern (seed, (seed % 5) / 4.0f).rightHand == p.rightHand);
    }

    CHECK (distinct.size() > 350);

    // Denser riffs have more hits on average.
    int sparse = 0, busy = 0;
    for (std::uint32_t seed = 1; seed < 200; ++seed)
    {
        sparse += countHits (makeRandomPattern (seed, 0.0f).rightHand);
        busy   += countHits (makeRandomPattern (seed, 1.0f).rightHand);
    }
    CHECK (busy > sparse);

    // The engine plays them without stuck notes, in every section.
    for (int section = 0; section < numSections; ++section)
    {
        GrooveSettings s;
        s.pattern = randomPatternIndex();
        s.riffSeed = 1234u + (std::uint32_t) section;
        s.section = (Section) section;
        s.fillEveryBars = 2;

        const auto clip = GrooveEngine::renderClip (s, 32.0);
        int sounding[128] {}, ons = 0;
        for (const auto& e : clip)
        {
            if (e.isNoteOn()) { CHECK (sounding[e.note] == 0); sounding[e.note] = 1; ++ons; }
            else sounding[e.note] = 0;
        }
        CHECK (ons > 0);
        for (int n = 0; n < 128; ++n) CHECK (sounding[n] == 0);
    }
}

static void testRandomProgressions()
{
    for (std::uint32_t seed = 0; seed < 300; ++seed)
        for (int len = 1; len <= maxProgressionLength; ++len)
        {
            const auto prog = randomProgression (seed, len);
            CHECK ((int) prog.size() == len);
            for (size_t i = 0; i < prog.size(); ++i)
            {
                CHECK (prog[i] >= 0 && prog[i] < 7);
                if (i > 0) CHECK (prog[i] != prog[i - 1]);
            }
        }
}

static void testSections()
{
    for (const auto& base : groovePatterns())
        for (int section = 0; section < numSections; ++section)
        {
            const auto p = applySection (base, (Section) section);
            CHECK (wellFormed (p));
            CHECK (p.length() == base.length());
            CHECK (countHits (p.rightHand) > 0);

            if (section == (int) Section::intro || section == (int) Section::breakdown)
                CHECK (countHits (p.leftHand) == 0);
            if (section == (int) Section::intro)
                CHECK (countHits (p.rightHand) <= countHits (base.rightHand));
        }

    // A full arrangement: 60 bars, markers in order, nothing left hanging.
    GrooveSettings s;
    s.progressionLength = 4;
    s.progression = { 3, 2, 1, 0, 0, 0, 0, 0 };
    std::vector<GrooveEngine::Marker> markers;
    const auto song = GrooveEngine::renderArrangement (s, defaultArrangement(), &markers);

    CHECK (markers.size() == defaultArrangement().size());
    CHECK (markers.back().ppq == (60 - 8) * 4.0);

    int sounding[128] {};
    double last = 0.0;
    for (const auto& e : song)
    {
        CHECK (e.ppq >= last - 1.0e-9);
        last = e.ppq;
        sounding[e.note] += e.isNoteOn() ? 1 : -1;
    }
    CHECK (last <= 240.0);
    for (int n = 0; n < 128; ++n) CHECK (sounding[n] == 0);
}

static void testSegmentsAndUserPattern()
{
    const auto& base = groovePatterns()[1];   // Soulful Push

    // Identity order changes nothing; A A A A repeats the first half bar.
    CHECK (applySegmentOrder (base, { 0, 1, 2, 3 }).rightHand == base.rightHand);
    const auto aaaa = applySegmentOrder (base, { 0, 0, 0, 0 });
    CHECK (wellFormed (aaaa));
    for (int i = 0; i < 32; ++i)
        CHECK (aaaa.rightHand[(size_t) i] == base.rightHand[(size_t) (i % 8)]
               || (aaaa.rightHand[(size_t) i] == '.' && base.rightHand[(size_t) (i % 8)] == '='));

    // Reordering every preset keeps it well formed and playable.
    for (const auto& p : groovePatterns())
    {
        const auto r = applySegmentOrder (p, { 3, 1, 0, 2 });
        CHECK (wellFormed (r));
        CHECK (r.length() == p.length());
    }

    // copySegment only touches the chosen segment.
    auto target = groovePatterns()[0];
    const auto& other = groovePatterns()[3];
    copySegment (target, other, 2);
    for (int i = 0; i < 32; ++i)
    {
        if (i >= 16 && i < 24) CHECK (target.rightHand[(size_t) i] == other.rightHand[(size_t) i] || target.rightHand[(size_t) i] == '.');
        else                   CHECK (target.rightHand[(size_t) i] == groovePatterns()[0].rightHand[(size_t) i] || target.rightHand[(size_t) i] == '.');
    }

    // The user pattern plays exactly what is written in the grid.
    GrooveSettings s;
    s.pattern = userPatternIndex();
    s.bass = BassMode::root;
    s.humanize = 0.0f;
    s.swing = 0.0f;
    s.userRight.fill ('.');
    s.userLeft.fill ('.');
    s.userRight[0] = 'X';
    s.userRight[6] = 'x';
    s.userLeft[0] = 'x';

    CHECK (GrooveEngine::resolvePattern (s).rightHand == "X.....x.........................");
    const auto clip = GrooveEngine::renderClip (s, 8.0);
    std::set<double> chordHits, bassHits;
    for (const auto& e : clip)
        if (e.isNoteOn()) (e.part == Part::bass ? bassHits : chordHits).insert (e.ppq);

    CHECK ((chordHits == std::set<double> { 0.0, 1.5 }));
    CHECK ((bassHits == std::set<double> { 0.0 }));

    // Segment order also applies to the user pattern: D C B A moves the hit at 0 to bar 2, beat 3.
    s.segmentOrder = { 3, 2, 1, 0 };
    CHECK (GrooveEngine::resolvePattern (s).rightHand.substr (24, 8) == "X.....x.");
}

static void testFillStylesStrumOctave()
{
    for (int style = 0; style < numFillStyles; ++style)
    {
        GrooveSettings s;
        s.fillEveryBars = 2;
        s.fillStyle = (FillStyle) style;
        s.humanize = 0.0f;
        s.swing = 0.0f;
        s.bass = BassMode::off;
        s.pattern = 9;   // Long Chords: nothing else plays on the last beat of bar 2

        const auto clip = GrooveEngine::renderClip (s, 8.0);
        std::vector<int> fill;
        for (const auto& e : clip)
            if (e.isNoteOn() && e.ppq >= 7.0 && e.ppq < 8.0)
                fill.push_back (e.note);

        CHECK (fill.size() == 4);
        if (fill.size() == 4)
        {
            if (style == (int) FillStyle::pentatonicUp)   CHECK (fill[0] < fill[3]);
            if (style == (int) FillStyle::pentatonicDown) CHECK (fill[0] > fill[3]);
        }
    }

    // Strum spreads the chord notes in time; octaveTop adds one note above.
    GrooveSettings s;
    s.humanize = 0.0f;
    s.swing = 0.0f;
    s.bass = BassMode::off;
    s.pattern = 9;
    s.strum = 1.0f;
    s.octaveTop = true;

    const auto clip = GrooveEngine::renderClip (s, 4.0);
    std::vector<NoteEvent> firstChord;
    for (const auto& e : clip)
        if (e.isNoteOn() && e.ppq < 0.5)
            firstChord.push_back (e);

    std::array<Chord, maxProgressionLength> chords;
    std::array<VoicedChord, maxProgressionLength> voiced;
    GrooveEngine::voiceProgression (s, chords, voiced);

    CHECK ((int) firstChord.size() == voiced[0].rightHand.size + 1);
    CHECK (firstChord.back().ppq > firstChord.front().ppq);
    int top = 0;
    for (const auto& e : firstChord) top = std::max (top, e.note);
    CHECK (top == voiced[0].rightHand.highest() + 12);
}

static void testApproachChords()
{
    GrooveSettings s;
    s.humanize = 0.0f;
    s.swing = 0.0f;
    s.progression = { 0, 3, 0, 3, 0, 3, 0, 3 };
    s.progressionLength = 2;
    s.stepsPerChord = 16;
    s.pattern = userPatternIndex();
    s.userRight.fill ('.');
    s.userLeft.fill ('.');
    for (int i : { 0, 8, 16, 24 }) { s.userRight[(size_t) i] = 'x'; s.userLeft[(size_t) i] = 'x'; }

    std::array<Chord, maxProgressionLength> chords;
    std::array<VoicedChord, maxProgressionLength> voiced;
    GrooveEngine::voiceProgression (s, chords, voiced);

    for (int style = 0; style < numApproachStyles; ++style)
    {
        s.approach = (ApproachStyle) style;
        const int shift = style == 1 ? 1 : style == 2 ? -1 : 0;
        const auto clip = GrooveEngine::renderClip (s, 8.0);

        std::set<int> chordNotes, bassNotes;
        for (const auto& e : clip)
            if (e.isNoteOn() && std::abs (e.ppq - 3.75) < 0.05)
                (e.part == Part::bass ? bassNotes : chordNotes).insert (e.note);

        if (shift == 0)
        {
            CHECK (chordNotes.empty() && bassNotes.empty());
            continue;
        }

        std::set<int> expected;
        for (int i = 0; i < voiced[1].rightHand.size; ++i)
            expected.insert (voiced[1].rightHand[i] + shift);
        CHECK (chordNotes == expected);
        CHECK ((bassNotes == std::set<int> { 36 + chords[1].rootPc + shift }));

        // Every note is closed before the clip ends.
        int balance = 0;
        for (const auto& e : clip) balance += e.isNoteOn() ? 1 : -1;
        CHECK (balance == 0);
    }

    // A muted bass (break) stays muted.
    s.approach = ApproachStyle::fromBelow;
    s.userLeft.fill ('.');
    for (const auto& e : GrooveEngine::renderClip (s, 8.0))
        CHECK (e.part != Part::bass);
}

static void testFreeChords()
{
    GrooveSettings s;
    s.keyRoot = 0;
    s.scale = 0;
    s.extension = Extension::ninth;
    s.progression = { 0, 3, 4, 0, 0, 0, 0, 0 };
    s.progressionLength = 3;
    s.freeChords[1] = freeChordCode (8, false);   // Ab major in C: bVI (8 semitones above C)
    s.freeChords[2] = freeChordCode (6, true);    // F# minor: #iv

    CHECK (chordName (GrooveEngine::chordForSlot (s, 0)) == "Cmaj9");
    CHECK (chordName (GrooveEngine::chordForSlot (s, 1)) == "Abmaj9");
    CHECK (GrooveEngine::numeralForSlot (s, 1) == "bVI");
    CHECK (chordName (GrooveEngine::chordForSlot (s, 2)) == "F#m9");
    CHECK (GrooveEngine::numeralForSlot (s, 2) == "#iv");

    std::array<Chord, maxProgressionLength> chords;
    std::array<VoicedChord, maxProgressionLength> voiced;
    GrooveEngine::voiceProgression (s, chords, voiced);
    CHECK (chords[1].rootPc == 8);
    const std::set<int> abMaj9 { 8, 0, 3, 7, 10 };   // Ab C Eb G Bb

    for (int i = 0; i < voiced[1].rightHand.size; ++i)
        CHECK (abMaj9.count (voiced[1].rightHand[i] % 12) == 1);

    // Free chords follow the key: in D, the same slot is Bbmaj9 (bVI).
    s.keyRoot = 2;
    CHECK (chordName (GrooveEngine::chordForSlot (s, 1)) == "Bbmaj9");
    CHECK (GrooveEngine::numeralForSlot (s, 1) == "bVI");
}

static void testHarmonyScores()
{
    auto scoreOf = [] (GrooveSettings s)
    {
        std::array<Chord, maxProgressionLength> chords;
        std::array<VoicedChord, maxProgressionLength> voiced;
        GrooveEngine::voiceProgression (s, chords, voiced);
        return GrooveEngine::harmonyScores (s, chords, voiced);
    };

    GrooveSettings s;
    s.keyRoot = 0;
    s.scale = 0;
    s.progressionLength = 3;
    s.progression = { 1, 4, 0, 0, 0, 0, 0, 0 };   // ii - V - I
    const auto good = scoreOf (s);

    // Same, but the V becomes a free F#maj chord (tritone away, outside the key).
    s.freeChords[1] = freeChordCode (6, false);
    const auto bad = scoreOf (s);

    for (int i = 0; i < 3; ++i)
    {
        CHECK (good.slot[(size_t) i] >= 0.0f && good.slot[(size_t) i] <= 1.0f);
        CHECK (bad.slot[(size_t) i] >= 0.0f && bad.slot[(size_t) i] <= 1.0f);
    }
    CHECK (good.slot[1] > bad.slot[1] + 0.15f);
    CHECK (good.transition[0] > 0.7f);   // ii -> V: down a fifth, shared notes

    std::printf ("  harmony ii-V-I: %.2f %.2f %.2f  |  ii-F#-I: %.2f %.2f %.2f\n",
                 good.slot[0], good.slot[1], good.slot[2], bad.slot[0], bad.slot[1], bad.slot[2]);
}

static void testPatternsWellFormed()
{
    CHECK (groovePatterns().size() == 34);

    for (const auto& p : groovePatterns())
    {
        CHECK (p.length() == 16 || p.length() == 32);
        CHECK (p.leftHand.size() == p.rightHand.size());
        for (char c : p.rightHand + p.leftHand)
            CHECK (c == 'X' || c == 'x' || c == 'g' || c == '>' || c == '=' || c == '.');
    }

    for (const auto& p : progressionPresets())
    {
        CHECK (! p.degrees.empty() && (int) p.degrees.size() <= maxProgressionLength);
        CHECK (p.scale >= 0 && p.scale < numScales);
        const auto& cats = progressionCategories();
        CHECK (std::find_if (cats.begin(), cats.end(), [&p] (const char* c) { return std::string (c) == p.category; }) != cats.end());
    }
}

int main()
{
    testChordNames();
    testVoicings();
    testEngineNoStuckNotes();
    testBlockSizeIndependence();
    testSwing();
    testPushIsTied();
    testClipExport();
    testRandomRiffs();
    testRandomProgressions();
    testSections();
    testSegmentsAndUserPattern();
    testFillStylesStrumOctave();
    testApproachChords();
    testFreeChords();
    testHarmonyScores();
    testPatternsWellFormed();

    if (failures == 0)
        std::printf ("All engine tests passed.\n");

    return failures == 0 ? 0 : 1;
}
