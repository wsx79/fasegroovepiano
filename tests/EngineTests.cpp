// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

// Unit tests for the music engine (no JUCE needed).

#include "../Source/engine/GrooveEngine.h"
#include "../Source/engine/HarmonyAnalyzer.h"
#include "../Source/engine/Theory.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <set>
#include <map>
#include <string>
#include <vector>

using namespace sgk;

static int failures = 0;

static bool isHitChar (char c)
{
    return c == 'X' || c == 'x' || c == 'g' || c == '>' || c == 'u' || c == 'U' || c == 'd' || c == 'D';
}

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
    for (int combo = 0; combo < 3; ++combo)
    for (int pattern = 0; pattern < (int) groovePatterns().size(); ++pattern)
    {
        GrooveSettings s;
        s.pattern = pattern;
        s.fillEveryBars = 2;
        s.bass = combo == 0 ? BassMode::octaves : BassMode::withChords;   // bass in every stab, too
        s.voicing = combo == 2 ? Voicing::octaveTriads : Voicing::wide;
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

    // Step 5 (the "e" of beat 2) is an odd 16th, so full swing delays it by a third of a step.
    bool sawSwungHit = false;
    for (const auto& e : events)
        if (e.isNoteOn() && std::abs (e.ppq - (1.25 + 0.25 / 3.0)) < 1.0e-6)
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
        if (! (c == 'X' || c == 'x' || c == 'g' || c == '>' || c == '=' || c == '.' || c == 'u' || c == 'U' || c == 'd' || c == 'D'))
            return false;
    return true;
}

static int countHits (const std::string& line)
{
    int n = 0;
    for (char c : line)
        if (c == 'X' || c == 'x' || c == 'g' || c == '>' || c == 'u' || c == 'U' || c == 'd' || c == 'D') ++n;
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
        {
            // As busy as the density asks, never a long burst.
            const float density = (seed % 5) / 4.0f;
            int strong = 0;
            for (char c : p.rightHand)
                strong += (c == 'X' || c == 'x' || c == '>' || c == 'u' || c == 'U' || c == 'd' || c == 'D') ? 1 : 0;
            CHECK (strong <= (int) std::lround (12.0 + 8.0 * density));
        }
        CHECK (p.leftHand[0] == 'x' || p.leftHand[0] == '=');
        distinct.insert (p.rightHand);

        // Same seed, same riff.
        CHECK (makeRandomPattern (seed, (seed % 5) / 4.0f).rightHand == p.rightHand);
    }

    std::printf ("  random riffs: %d distinct of 399\n", (int) distinct.size());
    CHECK (distinct.size() > 250);   // fewer than before on purpose: classic loop figures, mostly repeated bars

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
    s.movement = 0;   // compare the raw rhythm
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
        const double from = 4.0 + fillStartStep ((FillStyle) style) * 0.25;   // bar 2
        std::vector<int> fill;
        int sounding[128] {};
        for (const auto& e : clip)
        {
            if (e.isNoteOn() && e.ppq >= from && e.ppq < 8.0)
                fill.push_back (e.note);
            sounding[e.note] += e.isNoteOn() ? 1 : -1;
        }
        for (int n : sounding)
            CHECK (n == 0);   // nothing left hanging

        const auto st = (FillStyle) style;
        if (st == FillStyle::pentatonicUp || st == FillStyle::pentatonicDown
            || st == FillStyle::chromaticWalk || st == FillStyle::bluesLick)
            CHECK (fill.size() == 4);
        if (st == FillStyle::gospelRun || st == FillStyle::gospelThirds || st == FillStyle::glissando || st == FillStyle::chordRoll)
            CHECK (fill.size() == 8);
        if (st == FillStyle::chordChops)
            CHECK (fill.size() == 12);
        if (st == FillStyle::pushStabs || st == FillStyle::mix)
            CHECK (fill.size() >= 4);

        if (fill.size() >= 4)
        {
            if (st == FillStyle::pentatonicUp)   CHECK (fill[0] < fill[3]);
            if (st == FillStyle::pentatonicDown) CHECK (fill[0] > fill[3]);
            if (st == FillStyle::chromaticWalk)  CHECK (fill[3] - fill[0] == 3);
            if (st == FillStyle::bluesLick)      CHECK (fill[0] > fill[3]);
            if (st == FillStyle::gospelRun)      CHECK (fill.back() > fill.front());
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

static void testV19Features()
{
    auto voicedOf = [] (const GrooveSettings& s)
    {
        std::array<Chord, maxProgressionLength> chords;
        std::array<VoicedChord, maxProgressionLength> voiced;
        GrooveEngine::voiceProgression (s, chords, voiced);
        return voiced;
    };

    // Top line: rising ends higher than it starts, falling the other way round.
    GrooveSettings s;
    s.keyRoot = 0; s.scale = 0; s.progressionLength = 4;
    s.progression = { 0, 5, 1, 4, 0, 0, 0, 0 };
    s.topLine = TopLine::rising;
    auto up = voicedOf (s);
    CHECK (up[3].rightHand.highest() > up[0].rightHand.highest());
    s.topLine = TopLine::falling;
    auto down = voicedOf (s);
    CHECK (down[3].rightHand.highest() < down[0].rightHand.highest());

    // Manual inversion: +1 lifts the chord.
    s.topLine = TopLine::automatic;
    const auto plain = voicedOf (s);
    s.inversions[1] = 1;
    const auto lifted = voicedOf (s);
    CHECK (lifted[1].rightHand.mean() > plain[1].rightHand.mean());
    s.inversions[1] = 0;

    // Groove length: 1 bar = 16 steps, 4 bars = 64 steps.
    s.pattern = 1;
    s.grooveBars = 1;
    CHECK (GrooveEngine::resolvePattern (s).length() == 16 && GrooveEngine::patternLength (s) == 16);
    s.grooveBars = 4;
    CHECK (GrooveEngine::resolvePattern (s).length() == 64 && GrooveEngine::patternLength (s) == 64);
    s.grooveBars = 2;

    // Variations: with "every 4 bars" bar 4 differs from bar 2, and nothing is left hanging.
    s.humanize = 0.0f; s.swing = 0.0f;
    auto hitsInBar = [] (const std::vector<NoteEvent>& clip, int bar)
    {
        std::vector<std::pair<double, int>> out;
        for (const auto& e : clip)
            if (e.isNoteOn() && e.ppq >= bar * 4.0 && e.ppq < bar * 4.0 + 4.0)
                out.push_back ({ e.ppq - bar * 4.0, e.note });
        return out;
    };
    s.progressionLength = 1;
    s.variationEvery = 4;
    bool anyDifferent = false;
    for (int pattern = 0; pattern < 12 && ! anyDifferent; ++pattern)
    {
        s.pattern = pattern;
        const auto clip = GrooveEngine::renderClip (s, 16.0);
        anyDifferent = hitsInBar (clip, 1) != hitsInBar (clip, 3);
        int balance = 0;
        for (const auto& e : clip) balance += e.isNoteOn() ? 1 : -1;
        CHECK (balance == 0);
    }
    CHECK (anyDifferent);
    s.variationEvery = 0;

    // Walking bass: plays more than one pitch class over one chord.
    s.pattern = 8;   // Disco Octaves: bass on every 8th
    s.bass = BassMode::walking;
    s.progressionLength = 2;
    s.progression = { 0, 3, 0, 0, 0, 0, 0, 0 };
    std::set<int> bassNotes;
    for (const auto& e : GrooveEngine::renderClip (s, 4.0))
        if (e.isNoteOn() && e.part == Part::bass)
            bassNotes.insert (e.note);
    CHECK (bassNotes.size() >= 3);

    // Build-up: louder at the end than at the start.
    s.bass = BassMode::off;
    s.section = Section::buildup;
    s.pattern = 1;
    const auto clip = GrooveEngine::renderClip (s, 8.0);
    int first = 0, last = 0;
    for (const auto& e : clip)
        if (e.isNoteOn()) { if (first == 0) first = e.velocity; last = e.velocity; }
    CHECK (last > first);
}

static void testVoicingMovement()
{
    // 'u' / 'd' move only the top voice to a neighbouring scale note; the lower voices stay.
    GrooveSettings s;
    s.humanize = 0.0f; s.swing = 0.0f; s.bass = BassMode::off; s.progressionLength = 1;
    s.pattern = userPatternIndex();
    s.userRight.fill ('.'); s.userLeft.fill ('.');
    s.userRight[0] = 'x'; s.userRight[8] = 'u'; s.userRight[16] = 'd';
    std::vector<int> a, b, c;
    for (const auto& e : GrooveEngine::renderClip (s, 8.0))
        if (e.isNoteOn())
            (e.ppq < 1.0 ? a : e.ppq < 3.0 ? b : c).push_back (e.note);

    std::sort (a.begin(), a.end()); std::sort (b.begin(), b.end()); std::sort (c.begin(), c.end());
    CHECK (! b.empty() && ! c.empty());
    const int top = a.back();
    CHECK (b.back() > top && b.back() - top <= 3);
    CHECK (c.back() < top && top - c.back() <= 3);
    // The voices under the top are the right hand's own, unchanged.
    CHECK (std::vector<int> (b.begin(), b.end() - 1) == std::vector<int> (c.begin(), c.end() - 1));
}

static void testHousePedalAndStabBass()
{
    // House pedal: one top note through each chord, whatever the moves; piano-stab bass: a root
    // under every chord hit and never a 16th away from one (no flams).
    for (int pattern : { 0, 1, 5 })
    {
        GrooveSettings s;
        s.humanize = 0.0f; s.swing = 0.0f; s.movement = 0;
        s.voicing = Voicing::rootless; s.centre = 61;
        s.progression = { 0, 3, 0, 4, 5, 3, 1, 4 }; s.progressionLength = 2;
        s.bass = BassMode::withChords; s.topLine = TopLine::pedal; s.pattern = pattern;

        std::map<long long, std::vector<int>> chordHits, bassHits;
        for (const auto& e : GrooveEngine::renderClip (s, 16.0))
            if (e.isNoteOn())
                (e.part == Part::bass ? bassHits : chordHits)[std::llround (e.ppq * 4.0)].push_back (e.note);

        std::map<long long, std::set<int>> topsPerBar;
        for (const auto& [step, notes] : chordHits)
        {
            topsPerBar[step / 16].insert (*std::max_element (notes.begin(), notes.end()));
            CHECK (bassHits.count (step) == 1);
        }
        bool hooked = false;
        for (const auto& [bar, tops] : topsPerBar)
        {
            CHECK (tops.size() <= 3);   // the pedal, the hook step, a push bringing the next chord's
            if (bar % 2 == 1 && tops.size() >= 2) hooked = true;
        }
        CHECK (hooked);
        if (pattern == 1) CHECK (bassHits.count (0) == 1 && bassHits[0].size() == 2 && bassHits[0][1] - bassHits[0][0] == 12);   // octave on the accented stab
        for (const auto& [step, notes] : bassHits)
            if (chordHits.count (step) == 0)
                CHECK (chordHits.count (step - 1) == 0 && chordHits.count (step + 1) == 0);
    }
}

static void testStepVelocities()
{
    // Levels set by hand on the user pattern: chords and bass follow them, step by step,
    // also when the A-D order moves the steps; 0 keeps the automatic level.
    GrooveSettings s;
    s.humanize = 0.0f; s.swing = 0.0f; s.velocity = 118; s.movement = 0;
    s.pattern = userPatternIndex();
    s.bass = BassMode::root;
    for (int i = 0; i < userPatternLength; ++i)
    {
        s.userRight[(size_t) i] = i % 4 == 0 ? 'X' : '.';
        s.userLeft[(size_t) i]  = i % 8 == 2 ? 'x' : '.';
    }

    auto levels = [] (const GrooveSettings& set)
    {
        std::map<std::pair<int, long long>, int> top;
        for (const auto& e : GrooveEngine::renderClip (set, 8.0))
            if (e.isNoteOn())
            {
                auto& v = top[{ (int) e.part, std::llround (e.ppq * 4.0) }];
                v = std::max (v, (int) e.velocity);
            }
        return top;
    };

    auto at = [] (const std::map<std::pair<int, long long>, int>& m, Part part, long long step)
    {
        const auto it = m.find ({ (int) part, step });
        return it == m.end() ? -1 : it->second;
    };

    const auto automatic = levels (s);
    s.userVelRight[4] = 40;
    s.userVelLeft[10] = 30;
    auto set = levels (s);
    CHECK (at (set, Part::chords, 4) < at (automatic, Part::chords, 4) - 40);
    CHECK (at (set, Part::bass, 10) < at (automatic, Part::bass, 10) - 40);
    CHECK (at (set, Part::chords, 0) == at (automatic, Part::chords, 0));
    CHECK (at (set, Part::chords, 12) == at (automatic, Part::chords, 12));

    // Order B A C D: step 4 of the written groove (segment A) now plays at step 12.
    s.segmentOrder = { 1, 0, 2, 3 };
    set = levels (s);
    CHECK (at (set, Part::chords, 12) < at (automatic, Part::chords, 12) - 40);
    CHECK (at (set, Part::chords, 4) == at (automatic, Part::chords, 4));

    // The velocity page's own order: A A C D plays part A's levels (now at step 12) in both halves.
    s.velocityOrder = { 1, 1, 2, 3 };
    set = levels (s);
    CHECK (at (set, Part::chords, 4) < at (automatic, Part::chords, 4) - 40);
    CHECK (at (set, Part::chords, 12) < at (automatic, Part::chords, 12) - 40);
}

static void testClearLowRegister()
{
    // Bass and chords: below E3 only roots and 5ths, never a chord tone under the bass, whatever
    // the voicing and the register.
    for (int v = 0; v < numVoicings; ++v)
        for (int centre = 50; centre <= 72; centre += 2)
            for (int key = 0; key < 12; key += 5)
            {
                GrooveSettings s;
                s.keyRoot = key; s.voicing = (Voicing) v; s.centre = centre;
                s.extension = Extension::ninth; s.bass = BassMode::withChords;
                s.progression = { 0, 3, 4, 5, 1, 0, 0, 0 }; s.progressionLength = 5;
                std::array<Chord, maxProgressionLength> chords {};
                std::array<VoicedChord, maxProgressionLength> voiced {};
                GrooveEngine::voiceProgression (s, chords, voiced);
                for (int i = 0; i < s.progressionLength; ++i)
                    for (const auto* hand : { &voiced[(size_t) i].rightHand, &voiced[(size_t) i].leftHand })
                        for (int k = 0; k < hand->size; ++k)
                        {
                            const int n = (*hand)[k];
                            const int interval = ((n - chords[(size_t) i].rootPc) % 12 + 12) % 12;
                            CHECK (n >= 52 || interval == 0 || interval == 7);
                            CHECK (n >= 36 + chords[(size_t) i].rootPc);
                        }
                CHECK (voiced[0].rightHand.size >= 2);
            }
}

static void testEvolution()
{
    // Evolve over 8 bars: bars 1-2 as written, 3-4 with ghost notes added, 5-8 varied, a fill in
    // bar 8; the loop covers the whole 8 bars; nothing changes when it is off.
    GrooveSettings s;
    s.humanize = 0.0f; s.swing = 0.0f; s.pattern = 1; s.fillEveryBars = 0;
    auto notesPerBar = [] (const GrooveSettings& set)
    {
        std::vector<int> bars (8, 0);
        std::vector<int> fills (8, 0);
        for (const auto& e : GrooveEngine::renderClip (set, 32.0))
            if (e.isNoteOn() && e.ppq < 32.0)
            {
                ++bars[(size_t) (e.ppq / 4.0)];
                if (e.part == Part::fill) ++fills[(size_t) (e.ppq / 4.0)];
            }
        return std::make_pair (bars, fills);
    };

    const auto [plain, plainFills] = notesPerBar (s);
    s.evolveBars = 8;
    CHECK (GrooveEngine::loopLengthInSteps (s) % (16 * 8) == 0);
    const auto [grown, grownFills] = notesPerBar (s);
    CHECK (grown[0] == plain[0] && grown[1] == plain[1]);                  // stage 1: as written
    CHECK (grown[2] + grown[3] > plain[2] + plain[3]);                    // stage 2: + ghost notes
    CHECK (grownFills[7] > 0 && plainFills[7] == 0);                       // the last bar fills
    for (int b = 0; b < 7; ++b)
        CHECK (grownFills[(size_t) b] == 0);
}

static void testChordRange()
{
    // The chord range keeps every chord note between its limits (and the default changes nothing).
    GrooveSettings s;
    s.voicing = Voicing::wide; s.extension = Extension::ninth; s.centre = 64;
    s.progression = { 0, 3, 4, 5, 0, 0, 0, 0 }; s.progressionLength = 4;
    std::array<Chord, maxProgressionLength> chords {};
    std::array<VoicedChord, maxProgressionLength> plain {}, ranged {};
    GrooveEngine::voiceProgression (s, chords, plain);
    s.rangeLow = 55; s.rangeHigh = 74;
    GrooveEngine::voiceProgression (s, chords, ranged);
    bool changed = false;
    for (int i = 0; i < 4; ++i)
    {
        for (const auto* hand : { &ranged[(size_t) i].rightHand, &ranged[(size_t) i].leftHand })
            for (int k = 0; k < hand->size; ++k)
                CHECK ((*hand)[k] >= 55 && (*hand)[k] <= 74);
        CHECK (ranged[(size_t) i].rightHand.size >= 2);
        changed = changed || ranged[(size_t) i].rightHand.notes != plain[(size_t) i].rightHand.notes
                          || ranged[(size_t) i].leftHand.notes != plain[(size_t) i].leftHand.notes;
    }
    CHECK (changed);
}

static void testContextualFills()
{
    // Fills "Mix": a busy groove gets short fills (no runs), a sparse one runs; the chosen style
    // is followed when it isn't Mix.
    auto fillNotes = [] (int pattern, FillStyle style)
    {
        GrooveSettings s;
        s.humanize = 0.0f; s.swing = 0.0f; s.pattern = pattern; s.fillEveryBars = 1; s.fillStyle = style;
        int n = 0;
        for (const auto& e : GrooveEngine::renderClip (s, 16.0))
            if (e.isNoteOn() && e.part == Part::fill) ++n;
        return n;
    };
    for (int p = 0; p < 20; ++p)
        CHECK (fillNotes (p, FillStyle::mix) > 0);
    CHECK (fillNotes (1, FillStyle::pentatonicUp) > 0);
}

static void testGrooveFromMidi()
{
    // A groove played by the engine, read back from its notes: the same chord and bass rhythm.
    for (int pattern : { 1, 5, 12 })
    {
        GrooveSettings s;
        s.humanize = 0.0f; s.swing = 0.0f; s.pattern = pattern; s.bass = BassMode::withChords;
        std::vector<AnalyzedNote> notes;
        std::set<int> chordSteps, bassSteps;
        for (const auto& e : GrooveEngine::renderClip (s, 32.0))
            if (e.isNoteOn())
            {
                notes.push_back ({ e.ppq, e.ppq + 0.2, e.note, e.velocity });
                const int step = (int) std::lround (e.ppq * 4.0) % 32;
                (e.part == Part::bass ? bassSteps : chordSteps).insert (step);
            }
        const auto g = grooveFromNotes (notes, 4);
        CHECK (g.length() == 32);
        std::set<int> readChords, readBass;
        for (int i = 0; i < g.length(); ++i)
        {
            if (g.rightHand[(size_t) i] != '.' && g.rightHand[(size_t) i] != '=') readChords.insert (i);
            if (g.leftHand[(size_t) i] != '.' && g.leftHand[(size_t) i] != '=') readBass.insert (i);
        }
        CHECK (readChords == chordSteps);
        CHECK (readBass == bassSteps);
    }
    CHECK (grooveFromNotes ({}, 4).length() == 0);
    CHECK (grooveFromNotes ({ { 0.0, 1.0, 60, 100 } }, 3).length() == 0);
}

static void testSectionHarmony()
{
    // Sections colour the harmony: lighter chords in the intro / outro, fuller in the drop, the
    // main loop as chosen.
    GrooveSettings s;
    s.extension = Extension::ninth;
    s.section = Section::main;     CHECK (GrooveEngine::sectionExtension (s) == Extension::ninth);
    s.section = Section::intro;    CHECK (GrooveEngine::sectionExtension (s) == Extension::seventh);
    s.section = Section::outro;    CHECK (GrooveEngine::sectionExtension (s) == Extension::seventh);
    s.extension = Extension::seventh;
    s.section = Section::drop;     CHECK (GrooveEngine::sectionExtension (s) == Extension::ninth);
    s.section = Section::breakdown; CHECK (GrooveEngine::sectionExtension (s) == Extension::seventh);
    s.section = Section::intro;
    CHECK (GrooveEngine::chordForSlot (s, 0).tones.size <= 4);
}

static void testStraighter()
{
    // DNA "Straighter": everything on 8ths, no ghosts, no pushes; the downbeat plays.
    for (int pattern : { 1, 5, 12, 40 })
    {
        GrooveSettings s;
        s.pattern = pattern;
        const auto p = applyIntention (GrooveEngine::resolvePattern (s), Intention::straighter, 7u);
        for (int i = 0; i < p.length(); ++i)
        {
            const char r = p.rightHand[(size_t) i];
            CHECK (r != 'g' && r != '>');
            if (i % 2 == 1) CHECK (r == '.' || r == '=');
        }
        CHECK (p.rightHand[0] != '.');
    }
}

static void testEditedLoop()
{
    // Notes edited in the piano roll replace the groove in the main section and loop at their length.
    GrooveSettings s;
    s.humanize = 0.0f; s.swing = 0.0f; s.velocity = 118;
    auto loop = std::make_shared<std::vector<LoopNote>>();
    loop->push_back ({ 0.0, 0.5, 60, 100, Part::chords });
    loop->push_back ({ 1.5, 0.25, 40, 90, Part::bass });
    s.editedLoop = loop;
    s.editedLoopLength = 4.0;

    CHECK (GrooveEngine::loopLengthInSteps (s) == 16);
    std::vector<double> ons;
    int bass = 0;
    for (const auto& e : GrooveEngine::renderClip (s, 8.0))
        if (e.isNoteOn())
        {
            ons.push_back (e.ppq);
            if (e.part == Part::bass) { ++bass; CHECK (e.note == 40 && e.velocity == 90); }
            else                      CHECK (e.note == 60 && e.velocity == 100);
        }
    CHECK (ons.size() == 4 && bass == 2);
    CHECK (std::abs (ons[2] - 4.0) < 1.0e-9);

    // The other song sections are still generated.
    s.section = Section::breakdown;
    bool other = false;
    for (const auto& e : GrooveEngine::renderClip (s, 8.0))
        if (e.isNoteOn() && e.note != 60 && e.note != 40) other = true;
    CHECK (other);
}

static void testCarryEdits()
{
    // Hand edits survive a change of the generated notes (here: everything a tone up).
    std::vector<LoopNote> base {
        { 0.0, 0.5, 60, 100, Part::chords, 0 }, { 0.0, 0.5, 64, 100, Part::chords, 1 }, { 0.0, 0.5, 67, 100, Part::chords, 2 },
        { 0.0, 0.5, 36, 100, Part::bass, 3 },   { 2.0, 0.5, 60, 100, Part::chords, 4 },
    };
    std::vector<LoopNote> edited {
        { 0.0, 0.5, 60, 100, Part::chords, 0 },     // untouched
        { 0.1, 1.0, 65, 80, Part::chords, 1 },      // moved later, a semitone up, longer, softer
        // 2 (G) deleted
        { 0.0, 0.5, 36, 100, Part::bass, 3 },
        { 2.0, 0.5, 60, 100, Part::chords, 4 },
        { 3.0, 0.25, 72, 90, Part::chords, -1 },    // added by hand
    };
    auto now = base;
    for (auto& n : now) n.note += 2;

    const auto carried = carryEdits (base, edited, 4.0, now, 4.0);
    auto find = [&] (double start, int note) { for (const auto& n : carried) if (std::abs (n.start - start) < 1.0e-9 && n.note == note) return &n; return (const LoopNote*) nullptr; };
    CHECK (carried.size() == 5);
    CHECK (find (0.0, 62) != nullptr);                      // the untouched one, a tone up
    CHECK (find (0.0, 69) == nullptr);                      // the deleted one stays deleted
    const auto* moved = find (0.1, 67);
    CHECK (moved != nullptr && std::abs (moved->length - 1.0) < 1.0e-9 && moved->velocity == 80);
    CHECK (find (3.0, 74) != nullptr);                      // the added one follows the harmony
    CHECK (find (0.0, 38) != nullptr && find (2.0, 62) != nullptr);

    // Nothing changed: the edit is returned as it is.
    CHECK (carryEdits (base, edited, 4.0, base, 4.0).size() == edited.size());

    // A longer loop now: the edits repeat in each copy of the old loop.
    auto longer = base;
    for (auto n : base) { n.start += 4.0; longer.push_back (n); }
    const auto twice = carryEdits (base, edited, 4.0, longer, 8.0);
    CHECK (twice.size() == 10);
}

static void testAutoMovement()
{
    // Movement turns some plain hits into inversion moves; off leaves the groove untouched.
    GrooveSettings s;
    s.pattern = 9;   // Long Chords
    s.movement = 0;
    const auto plain = GrooveEngine::resolvePattern (s);
    for (int level = 1; level <= 2; ++level)
    {
        s.pattern = 2;   // Deep Chords
        s.movement = 0;
        const auto off = GrooveEngine::resolvePattern (s).rightHand;
        s.movement = level;
        const auto on = GrooveEngine::resolvePattern (s).rightHand;
        CHECK (off.size() == on.size());
        int moved = 0;
        for (size_t i = 0; i < on.size(); ++i)
        {
            if (on[i] != off[i]) ++moved;
            CHECK (isHitChar (on[i]) == isHitChar (off[i]));   // same rhythm
        }
        CHECK (moved > 0);
    }
    CHECK (plain.length() == 32);
}

static void testStrum()
{
    // The roll is a real time: at half the tempo it covers half as many beats.
    auto spread = [] (double bpm, int direction)
    {
        GrooveSettings s;
        s.humanize = 0.0f; s.swing = 0.0f; s.bass = BassMode::off; s.pattern = 9;   // Long Chords
        s.strum = 1.0f; s.bpm = bpm; s.strumDirection = direction;
        std::vector<NoteEvent> first;
        for (const auto& e : GrooveEngine::renderClip (s, 4.0))
            if (e.isNoteOn() && e.ppq < 0.5) first.push_back (e);
        std::sort (first.begin(), first.end(), [] (const NoteEvent& a, const NoteEvent& b) { return a.ppq < b.ppq; });
        return first;
    };

    const auto fast = spread (120.0, 0), slow = spread (60.0, 0);
    const double fastSpan = fast.back().ppq - fast.front().ppq, slowSpan = slow.back().ppq - slow.front().ppq;
    CHECK (fastSpan > 0.1 && fastSpan < 0.2);            // ~90 ms at 120 BPM = 0.18 beats
    CHECK (std::abs (slowSpan - fastSpan / 2.0) < 0.01);
    CHECK (fast.front().note < fast.back().note);         // up: low note first

    const auto down = spread (120.0, 1);
    CHECK (down.front().note > down.back().note);         // down: top note first
}

static void testPatternsWellFormed()
{
    CHECK (groovePatterns().size() == 179);
    for (const auto& p : groovePatterns())
        CHECK (std::find_if (grooveCategories().begin(), grooveCategories().end(), [&p] (const char* c) { return p.category == c; }) != grooveCategories().end());

    for (const auto& p : groovePatterns())
    {
        CHECK (p.length() == 16 || p.length() == 32);
        CHECK (p.leftHand.size() == p.rightHand.size());
        for (char c : p.rightHand + p.leftHand)
            CHECK (c == 'X' || c == 'x' || c == 'g' || c == '>' || c == '=' || c == '.' || c == 'u' || c == 'U' || c == 'd' || c == 'D');
    }

    for (const auto& p : progressionPresets())
    {
        CHECK (! p.degrees.empty() && (int) p.degrees.size() <= maxProgressionLength);
        CHECK (p.scale >= 0 && p.scale < numScales);
        const auto& cats = progressionCategories();
        CHECK (std::find_if (cats.begin(), cats.end(), [&p] (const char* c) { return std::string (c) == p.category; }) != cats.end());
    }
}

static void testDancePack()
{
    // Dominant free chords: Bb7 in C is a bVII7.
    GrooveSettings s;
    s.keyRoot = 0;
    s.scale = 0;
    s.extension = Extension::seventh;
    s.progressionLength = 2;
    s.freeChords[1] = freeDominantCode (10);
    CHECK (chordName (GrooveEngine::chordForSlot (s, 1)) == "Bb7");
    CHECK (GrooveEngine::numeralForSlot (s, 1) == "bVII");

    // Every preset maps to valid slots; diatonic chords stay degrees, borrowed ones become free chords.
    for (const auto& p : progressionPresets())
    {
        const auto slots = presetSlots (p);
        CHECK (! slots.empty() && (int) slots.size() <= maxProgressionLength);
        for (const auto& slot : slots)
            CHECK (slot.degree >= 0 && slot.degree < 7 && slot.freeCode < numFreeChordCodes);
    }
    for (const auto& p : progressionPresets())
        if (std::string (p.name).rfind ("Backdoor", 0) == 0)
        {
            const auto slots = presetSlots (p);
            CHECK (slots[0].degree == 1 && slots[0].freeCode == noFreeChord);   // ii is diatonic
            CHECK (slots[1].freeCode == freeDominantCode (10));                 // bVII7 is borrowed
            CHECK (slots[2].degree == 0 && slots[2].freeCode == noFreeChord);
        }

    // Octave triads: R 3 5 doubled an octave up.
    const auto c = buildChord (0, 0, 0, Extension::ninth);
    const auto v = voiceChord (c, Voicing::octaveTriads, 64, nullptr);
    CHECK (v.rightHand.size == 6);
    for (int i = 0; i < 3; ++i)
        CHECK (v.rightHand[i + 3] - v.rightHand[i] == 12);
    for (int i = 0; i < v.rightHand.size; ++i)
        CHECK (v.rightHand[i] % 12 == 0 || v.rightHand[i] % 12 == 4 || v.rightHand[i] % 12 == 7);

    int stabs = 0;
    for (const auto& p : groovePatterns())
        stabs += p.category == "Dance Stabs" ? 1 : 0;
    CHECK (stabs == 8);
}

static void testHarmonyAnalyzer()
{
    // Chords as a pianist plays them: bass note + voicing, one per bar (or half bar), looped.
    auto addChord = [] (std::vector<AnalyzedNote>& notes, double start, double length, int bass, std::vector<int> upper)
    {
        notes.push_back ({ start, start + length * 0.9, bass });
        for (int n : upper) notes.push_back ({ start + 0.5, start + 0.5 + length * 0.35, n });   // offbeat stabs
        for (int n : upper) notes.push_back ({ start + 2.5, start + 2.5 + std::min (1.0, length * 0.3), n });
    };
    auto rootOf = [] (const HarmonyAnalysis& a, int i) { return (a.keyRoot + a.chordCodes[(size_t) i] % 12) % 12; };
    auto kindOf = [] (const HarmonyAnalysis& a, int i) { return a.chordCodes[(size_t) i] / 12; };

    {
        // Am9 - Fmaj9 - Cmaj7 - G, one bar each, played twice: rootless-ish voicings over the bass.
        std::vector<AnalyzedNote> notes;
        for (int rep = 0; rep < 2; ++rep)
        {
            const double t = rep * 16.0;
            addChord (notes, t + 0.0,  4.0, 45, { 60, 64, 67, 71 });   // Am9: C E G B over A
            addChord (notes, t + 4.0,  4.0, 41, { 57, 60, 64, 67 });   // Fmaj9: A C E G over F
            addChord (notes, t + 8.0,  4.0, 48, { 55, 59, 64, 67 });   // Cmaj7: G B E G over C
            addChord (notes, t + 12.0, 4.0, 43, { 55, 59, 62, 67 });   // G: G B D G over G
        }
        const auto a = analyzeHarmony (notes);
        CHECK (a.ok);
        CHECK (a.chordCodes.size() == 4);
        CHECK (a.chordLengthChoice == 1);
        if (a.chordCodes.size() == 4)
        {
            CHECK (rootOf (a, 0) == 9 && kindOf (a, 0) == 1);   // A minor
            CHECK (rootOf (a, 1) == 5 && kindOf (a, 1) == 0);   // F major
            CHECK (rootOf (a, 2) == 0 && kindOf (a, 2) == 0);   // C major
            CHECK (rootOf (a, 3) == 7 && kindOf (a, 3) != 1);   // G (major)
        }
        CHECK (a.outOfScale < 0.05);
        CHECK ((a.keyRoot == 0 && a.scale == 0) || (a.keyRoot == 9 && a.scale == 2));
        CHECK (a.slots.size() == 4);
        for (const auto& slot : a.slots) CHECK (slot.freeCode == noFreeChord);   // all in the scale
    }
    {
        // ii-V: Dm7 - G7 every half bar, four bars.
        std::vector<AnalyzedNote> notes;
        for (int bar = 0; bar < 4; ++bar)
        {
            const double t = bar * 4.0;
            notes.push_back ({ t, t + 1.8, 38 });
            for (int n : { 60, 65, 69, 72 }) notes.push_back ({ t + 0.5, t + 1.5, n });   // Dm7: C F A C
            notes.push_back ({ t + 2.0, t + 3.8, 43 });
            for (int n : { 59, 65, 67, 71 }) notes.push_back ({ t + 2.5, t + 3.5, n });   // G7: B F G B
        }
        const auto a = analyzeHarmony (notes);
        CHECK (a.ok && a.chordCodes.size() == 2 && a.chordLengthChoice == 0);
        if (a.chordCodes.size() == 2)
        {
            CHECK (rootOf (a, 0) == 2 && kindOf (a, 0) == 1);   // D minor
            CHECK (rootOf (a, 1) == 7 && kindOf (a, 1) == 2);   // G dominant
        }
    }
    {
        // Dorian vamp: Am7 - D (with F#), two bars each.
        std::vector<AnalyzedNote> notes;
        for (int rep = 0; rep < 2; ++rep)
        {
            const double t = rep * 16.0;
            addChord (notes, t + 0.0, 8.0, 45, { 60, 64, 67, 69 });   // Am7
            addChord (notes, t + 4.0, 4.0, 45, { 60, 64, 67, 69 });
            addChord (notes, t + 8.0, 8.0, 38, { 57, 62, 66, 69 });   // D: A D F# A
            addChord (notes, t + 12.0, 4.0, 38, { 57, 62, 66, 69 });
        }
        const auto a = analyzeHarmony (notes);
        CHECK (a.ok && a.chordCodes.size() == 2 && a.chordLengthChoice == 2);
        CHECK (a.keyRoot == 9 && a.scale == 1);   // A Dorian
    }
    {
        // Out-of-scale notes are reported; an empty part is not analysed.
        CHECK (! analyzeHarmony ({}).ok);
        std::vector<AnalyzedNote> notes;
        addChord (notes, 0.0, 4.0, 48, { 60, 64, 67 });
        addChord (notes, 4.0, 4.0, 44, { 60, 63, 68 });   // Ab major: borrowed bVI
        const auto a = analyzeHarmony (notes);
        CHECK (a.ok && a.chordCodes.size() == 2);
        if (a.chordCodes.size() == 2 && a.keyRoot == 0 && a.scale == 0)
            CHECK (a.slots[1].freeCode != noFreeChord);
    }
}

static void testBassLines()
{
    CHECK ((int) bassLines().size() == numBassLines);
    for (const auto& l : bassLines())
    {
        CHECK (l.line.size() == 32);
        CHECK (l.line.find_first_not_of ("ROL5376bGAS=.") == std::string::npos);
        CHECK (l.line[0] != '.' || l.line.find ('R') != std::string::npos);
    }

    for (int i = 0; i < numBassLines; ++i)
    {
        GrooveSettings s;
        s.pattern = 1;
        s.bass = (BassMode) (numBassPatternModes + i);
        s.humanize = 0.0f;
        s.swing = 0.0f;
        s.fillEveryBars = 2;
        s.progressionLength = 4;
        s.progression = { 5, 3, 0, 4, 0, 0, 0, 0 };   // vi IV I V in C

        GrooveEngine engine;
        std::vector<NoteEvent> events;
        for (double t = 0.0; t < 32.0; t += 0.37)
            engine.render (t, t + 0.37, s, events);
        engine.allNotesOff (32.0, events);

        int sounding[128] {}, bassOns = 0;
        bool valid = true, inRange = true;
        for (const auto& e : events)
        {
            if (e.isNoteOn()) { if (sounding[e.note]) valid = false; sounding[e.note] = 1; }
            else              { if (! sounding[e.note]) valid = false; sounding[e.note] = 0; }
            if (e.isNoteOn() && e.part == Part::bass)
            {
                ++bassOns;
                inRange = inRange && e.note >= 24 && e.note <= 64;
            }
        }
        CHECK (valid);
        CHECK (inRange);
        CHECK (bassOns >= 8);

        // Approach notes lead into the next chord's root by a semitone.
        const auto& line = bassLines()[(size_t) i].line;
        for (int step = 0; step < 32; ++step)
        {
            if (line[(size_t) step] != 'A' || step % 16 < 12)
                continue;   // only the ones right before a chord change
            const int nextRoot = GrooveEngine::chordForSlot (s, GrooveEngine::slotForStep (step + 16 - step % 16, s)).rootPc;
            for (const auto& e : events)
                if (e.isNoteOn() && e.part == Part::bass && std::abs (e.ppq - step * 0.25) < 0.01)
                    CHECK ((e.note + 1) % 12 == nextRoot);
        }
    }
}

static void testVoicingCoach()
{
    GrooveSettings s;
    s.progressionLength = 4;
    s.progression = { 5, 3, 0, 4, 0, 0, 0, 0 };
    s.extension = Extension::ninth;

    s.voicing = Voicing::close;
    const auto close = GrooveEngine::voicingReport (s);
    s.voicing = Voicing::wide;
    const auto wide = GrooveEngine::voicingReport (s);
    s.voicing = Voicing::rootless;
    const auto rootless = GrooveEngine::voicingReport (s);

    CHECK (close.chordNotes.size() == 4 && close.names.size() == 4);
    CHECK (close.highest - close.lowest < wide.highest - wide.lowest);   // two hands span more
    CHECK (rootless.muddy == 0);
    CHECK (close.averageMove >= 0.0 && close.averageMove < 6.0);
    CHECK (rootless.biggestTopLeap <= 7);   // the top line stays smooth
    for (const auto& notes : wide.chordNotes)
        CHECK (notes.size >= 4);

    // Only the voicing changes, never the chords.
    CHECK (close.names == wide.names);
}

static void testMorphAndSections()
{
    const auto& a = groovePatterns()[3];
    const auto& b = groovePatterns()[60];
    CHECK (morphPatterns (a, b, 0.0, 7).rightHand == a.rightHand);
    CHECK (morphPatterns (a, b, 1.0, 7).rightHand == b.rightHand);
    CHECK (morphPatterns (a, b, 1.0, 7).leftHand == b.leftHand);

    // The blend only grows: a step that follows b at 40% still follows b at 70%.
    auto fromB = [&] (double amount)
    {
        const auto m = morphPatterns (a, b, amount, 7);
        std::vector<bool> taken;
        for (int i = 0; i < m.length(); ++i)
            taken.push_back (m.rightHand[(size_t) i] == b.rightHand[(size_t) i] && a.rightHand[(size_t) i] != b.rightHand[(size_t) i]);
        return taken;
    };
    const auto t40 = fromB (0.4), t70 = fromB (0.7);
    int grew = 0;
    for (size_t i = 0; i < t40.size(); ++i)
    {
        if (t40[i] && ! t70[i]) CHECK (false);
        grew += (t70[i] ? 1 : 0) - (t40[i] ? 1 : 0);
    }
    CHECK (grew >= 0);

    // Every blend is a valid pattern: no hold without a note before it.
    for (double amount : { 0.1, 0.3, 0.5, 0.8 })
    {
        const auto m = morphPatterns (a, b, amount, 99);
        for (const auto* line : { &m.rightHand, &m.leftHand })
            for (size_t i = 1; i < line->size(); ++i)
                if ((*line)[i] == '=') CHECK ((*line)[i - 1] != '.');
    }

    // Song sections: a regenerated section plays a variation; a section of 0 bars is left out.
    GrooveSettings s;
    s.pattern = 3;
    s.humanize = 0.0f;
    std::vector<ArrangementPart> parts { { Section::main, 4 }, { Section::drop, 4 } };
    const auto plain = GrooveEngine::renderArrangement (s, parts);
    parts[1].variation = 12345;
    const auto varied = GrooveEngine::renderArrangement (s, parts);
    int firstHalfSame = 0, firstHalfTotal = 0, secondDiffers = 0;
    for (size_t i = 0; i < std::min (plain.size(), varied.size()); ++i)
    {
        if (plain[i].ppq < 16.0) { ++firstHalfTotal; firstHalfSame += plain[i].ppq == varied[i].ppq && plain[i].note == varied[i].note ? 1 : 0; }
        else if (plain[i].ppq != varied[i].ppq || plain[i].note != varied[i].note) ++secondDiffers;
    }
    CHECK (firstHalfSame == firstHalfTotal);   // the main groove is untouched
    CHECK (secondDiffers > 0 || plain.size() != varied.size());

    parts[0].bars = 0;
    double last = 0.0;
    for (const auto& e : GrooveEngine::renderArrangement (s, parts)) last = std::max (last, e.ppq);
    CHECK (last <= 16.0 + 1.0e-9);
}

static void testMusicalDna()
{
    auto hits = [] (const std::string& line, const char* set) { int n = 0; for (char c : line) n += std::strchr (set, c) != nullptr && c != 0 ? 1 : 0; return n; };
    for (int pattern : { 1, 3, 26, 60, 100 })
    {
        const auto& src = groovePatterns()[(size_t) pattern];
        const auto minimal = applyIntention (src, Intention::minimal, 5);
        const auto energy = applyIntention (src, Intention::energetic, 5);
        const auto funky = applyIntention (src, Intention::funky, 5);
        const auto soulful = applyIntention (src, Intention::soulful, 5);
        const auto smart = applyIntention (src, Intention::sophisticated, 5);

        CHECK (hits (minimal.rightHand, "XxgUuDd>") <= hits (src.rightHand, "XxgUuDd>"));
        CHECK (hits (minimal.rightHand, "g") == 0);
        CHECK (hits (energy.rightHand, "XxgUuDd>") >= hits (src.rightHand, "XxgUuDd>"));
        CHECK (hits (funky.rightHand, "=") == 0);                                   // short notes
        CHECK (hits (soulful.rightHand, "=") >= hits (src.rightHand, "="));         // held chords
        CHECK (hits (smart.rightHand, "XxgUuDd>") == hits (src.rightHand, "XxgUuDd>"));   // same rhythm

        for (const auto* p : { &minimal, &energy, &funky, &soulful, &smart })
        {
            CHECK (p->length() == src.length() && p->leftHand.size() == src.leftHand.size());
            CHECK (hits (p->rightHand, "XxgUuDd>") > 0);
            for (const auto* line : { &p->rightHand, &p->leftHand })
                for (size_t i = 1; i < line->size(); ++i)
                    if ((*line)[i] == '=') CHECK ((*line)[i - 1] != '.');
        }
        // The same seed gives the same result.
        CHECK (applyIntention (src, Intention::funky, 5).rightHand == funky.rightHand);
    }
}

static void testHarmonyAnalyzerEdgeCases()
{
    auto add = [] (std::vector<AnalyzedNote>& v, double t, double len, std::vector<int> notes)
    {
        for (int n : notes) v.push_back ({ t, t + len, n });
    };
    // Am - F - C - G (one bar each), read back as root + kind relative to the key found.
    auto names = [] (const HarmonyAnalysis& a)
    {
        static const char* const pcs[12] { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
        std::string out;
        for (int c : a.chordCodes)
            out += std::string (pcs[(a.keyRoot + c % 12) % 12]) + (c / 12 == 1 ? "m" : c / 12 == 2 ? "7" : "") + " ";
        return out;
    };
    auto loop = [&] (std::vector<AnalyzedNote>& v, double offset, int reps)
    {
        for (int r = 0; r < reps; ++r)
        {
            const double t = offset + r * 16.0;
            add (v, t, 3.5, { 45, 57, 60, 64 });  add (v, t + 4, 3.5, { 41, 57, 60, 65 });
            add (v, t + 8, 3.5, { 48, 55, 60, 64 }); add (v, t + 12, 3.5, { 43, 55, 59, 62 });
        }
    };

    { std::vector<AnalyzedNote> v; loop (v, 8.0, 2); CHECK (names (analyzeHarmony (v)) == "Am F C G "); }        // two bars of silence first
    { std::vector<AnalyzedNote> v; v.push_back ({ 3.5, 4.0, 67 }); loop (v, 4.0, 2);
      CHECK (names (analyzeHarmony (v)) == "Am F C G "); }                                                       // a pick-up note
    { std::vector<AnalyzedNote> v; loop (v, 0.0, 3); for (double t = 44; t < 48; t += 0.25) v.push_back ({ t, t + 0.2, 67 + ((int) (t * 4)) % 5 });
      CHECK (names (analyzeHarmony (v)) == "Am F C G "); }                                                       // a fill at the end
    { std::vector<AnalyzedNote> v;                                                                              // inversions: Am/E F/A C/E G/D
      for (int r = 0; r < 2; ++r) { const double t = r * 16.0; add (v, t, 3.5, { 52, 57, 60, 64 }); add (v, t + 4, 3.5, { 45, 53, 57, 60 });
                                    add (v, t + 8, 3.5, { 52, 55, 60, 64 }); add (v, t + 12, 3.5, { 50, 55, 59, 62 }); }
      CHECK (names (analyzeHarmony (v)) == "Am F C G "); }
    { std::vector<AnalyzedNote> v;                                                                              // rootless stabs over a held bass
      for (int r = 0; r < 2; ++r) { const double t = r * 16.0;
          add (v, t, 3.5, { 45 });      add (v, t + 0.5, 1, { 60, 64, 67, 71 });
          add (v, t + 4, 3.5, { 38 });  add (v, t + 4.5, 1, { 60, 64, 65, 69 });
          add (v, t + 8, 3.5, { 43 });  add (v, t + 8.5, 1, { 59, 62, 65, 69 });
          add (v, t + 12, 3.5, { 36 }); add (v, t + 12.5, 1, { 59, 62, 64, 67 }); }
      CHECK (names (analyzeHarmony (v)) == "Am Dm G7 C "); }
    { std::vector<AnalyzedNote> v;                                                                              // the bass moves an 8th early
      for (int r = 0; r < 2; ++r) { const double t = r * 16.0;
          add (v, t, 3, { 57, 60, 64 }); v.push_back ({ t, t + 3.5, 45 });
          add (v, t + 4, 3.5, { 57, 60, 65 }); v.push_back ({ t + 3.5, t + 7.5, 41 });
          add (v, t + 8, 3.5, { 55, 60, 64 }); v.push_back ({ t + 7.5, t + 11.5, 48 });
          add (v, t + 12, 3.5, { 55, 59, 62 }); v.push_back ({ t + 11.5, t + 15.5, 43 }); }
      CHECK (names (analyzeHarmony (v)) == "Am F C G "); }
    { std::vector<AnalyzedNote> v;                                                                              // ii - V7(b9 #5) - I
      for (int r = 0; r < 2; ++r) { const double t = r * 16.0;
          add (v, t, 3.5, { 50, 60, 65, 69 }); add (v, t + 4, 3.5, { 43, 59, 63, 65, 68 }); add (v, t + 8, 7.5, { 48, 60, 64, 67, 71 }); }
      const auto a = analyzeHarmony (v);
      CHECK (names (a).rfind ("Dm G7 C", 0) == 0); }
    { std::vector<AnalyzedNote> v;                                                                              // Gsus4 resolving: read as G, not Gm
      for (int r = 0; r < 2; ++r) { const double t = r * 16.0;
          add (v, t, 3.5, { 43, 55, 60, 62 }); add (v, t + 4, 3.5, { 43, 55, 59, 62 }); add (v, t + 8, 3.5, { 41, 57, 60, 65 }); add (v, t + 12, 3.5, { 48, 55, 60, 64 }); }
      CHECK (names (analyzeHarmony (v)) == "G G F C "); }
    { std::vector<AnalyzedNote> v;                                                                              // root + 3rd stabs (house dyads)
      for (int r = 0; r < 4; ++r) add (v, r * 4.0, 1.0, { 38, 50, 62, 65 });
      CHECK (names (analyzeHarmony (v)) == "Dm "); }
    { std::vector<AnalyzedNote> v;                                                                              // chromatic: I bIII bVI bVII
      for (int r = 0; r < 2; ++r) { const double t = r * 16.0;
          add (v, t, 3.5, { 48, 60, 64, 67 }); add (v, t + 4, 3.5, { 51, 63, 67, 70 }); add (v, t + 8, 3.5, { 44, 60, 63, 68 }); add (v, t + 12, 3.5, { 46, 62, 65, 70 }); }
      const auto a = analyzeHarmony (v);
      CHECK (names (a) == "C Eb Ab Bb ");
      CHECK (a.confidence > 0.8); }
}

static void testVariationsHumanizeFills()
{
    // Chord Variation Lab on I - vi - IV - V in C.
    GrooveSettings s;
    s.keyRoot = 0; s.scale = 0; s.extension = Extension::triad;
    s.progressionLength = 4;
    s.progression = { 0, 5, 3, 4, 0, 0, 0, 0 };
    CHECK ((GrooveEngine::progressionCodes (s) == std::vector<int> { 0, 21, 5, 7 }));
    const auto variants = GrooveEngine::progressionVariants (s);
    CHECK (variants.size() == 4);
    auto find = [&] (const std::string& name)
    {
        for (const auto& v : variants) if (name == v.name) return v.codes;
        return std::vector<int> {};
    };
    CHECK ((find ("More soulful") == std::vector<int> { 0, 21, 14, 7 }));   // I vi ii V
    CHECK ((find ("Gospel") == std::vector<int> { 0, 21, 17, 31 }));        // I vi iv V7
    CHECK ((find ("Deep") == std::vector<int> { 0, 21, 5, 19 }));           // I vi IV v
    CHECK ((find ("Chromatic") == std::vector<int> { 0, 21, 8, 25 }));      // I vi bVI bII7
    for (const auto& v : variants) CHECK (v.codes.size() == 4);             // same length: the rhythm stays

    // Minor key and a one-chord vamp still give alternatives.
    s.scale = 2; s.progression = { 0, 5, 2, 6, 0, 0, 0, 0 };
    CHECK (GrooveEngine::progressionVariants (s).size() >= 3);
    s.progressionLength = 1;
    for (const auto& v : GrooveEngine::progressionVariants (s)) CHECK (v.codes.size() == 1);

    // Humanize per voice: Even keeps the old timing; Pianist plays the top note early;
    // a new seed gives another (repeatable) feel.
    auto render = [] (const GrooveSettings& g)
    {
        GrooveEngine e;
        std::vector<NoteEvent> ev;
        e.render (0.0, 16.0, g, ev);
        e.allNotesOff (16.0, ev);
        return ev;
    };
    GrooveSettings h;
    h.pattern = 3; h.humanize = 0.6f; h.bass = BassMode::root;
    const auto even = render (h), evenAgain = render (h);
    CHECK (even.size() == evenAgain.size());
    for (size_t i = 0; i < std::min (even.size(), evenAgain.size()); ++i) CHECK (even[i].ppq == evenAgain[i].ppq);
    h.humanizeSeed = 77;
    const auto other = render (h);
    bool differs = false;
    for (size_t i = 0; i < std::min (even.size(), other.size()); ++i) differs = differs || even[i].ppq != other[i].ppq;
    CHECK (differs);
    h.humanizeSeed = 0; h.humanizeStyle = 3;   // tight bass
    double bassJitterTight = 0.0, bassJitterEven = 0.0;
    for (const auto& e : render (h)) if (e.isNoteOn() && e.part == Part::bass) bassJitterTight += std::abs (e.ppq * 4.0 - std::round (e.ppq * 4.0));
    for (const auto& e : even)       if (e.isNoteOn() && e.part == Part::bass) bassJitterEven  += std::abs (e.ppq * 4.0 - std::round (e.ppq * 4.0));
    CHECK (bassJitterTight < bassJitterEven);

    // Fills are marked, and a song section can have its own fill on its last bar only.
    GrooveSettings f;
    f.pattern = 3; f.fillEveryBars = 2; f.humanize = 0.0f;
    int fillNotes = 0;
    for (const auto& e : render (f)) if (e.isNoteOn() && e.part == Part::fill) ++fillNotes;
    CHECK (fillNotes > 0);
    f.fillEveryBars = 0;
    std::vector<ArrangementPart> parts { { Section::main, 4, 0, (int) FillStyle::chordRoll }, { Section::drop, 4, 0, -2 } };
    double firstFill = 1.0e9, lastFill = -1.0;
    for (const auto& e : GrooveEngine::renderArrangement (f, parts))
        if (e.isNoteOn() && e.part == Part::fill) { firstFill = std::min (firstFill, e.ppq); lastFill = std::max (lastFill, e.ppq); }
    CHECK (firstFill >= 12.0 && lastFill < 16.0);   // only in bar 4 of the first section, none in the drop
}

static void testStrings()
{
    GrooveSettings s;
    s.keyRoot = 9; s.scale = 1; s.progressionLength = 4;
    s.progression = { 0, 3, 4, 6, 0, 0, 0, 0 };
    s.pattern = 1;

    auto strings = [] (const std::vector<NoteEvent>& events, bool on)
    {
        std::vector<NoteEvent> out;
        for (const auto& e : events) if (e.part == Part::strings && e.isNoteOn() == on) out.push_back (e);
        return out;
    };

    // Off: no strings, and the piano and bass don't change.
    const auto plain = GrooveEngine::renderClip (s, 16.0);
    CHECK (strings (plain, true).empty());

    // Pad: every chord held (4 voices, mid register), all notes closed, piano and bass unchanged.
    s.strings = StringsMode::pad;
    const auto pad = GrooveEngine::renderClip (s, 16.0);
    const auto ons = strings (pad, true), offs = strings (pad, false);
    CHECK (ons.size() == 16 && offs.size() == 16);
    for (const auto& e : ons) CHECK (e.note >= 55 && e.note <= 79 && std::fmod (e.ppq, 4.0) == 0.0);
    for (const auto& e : offs) CHECK (std::fmod (e.ppq + 0.02, 4.0) < 1.0e-6);
    size_t piano = 0;
    for (const auto& e : pad) if (e.part != Part::strings) ++piano;
    CHECK (piano == plain.size());
    // Voice-led: the pad's notes move a little from chord to chord.
    std::array<NoteSet, maxProgressionLength> chordsNotes;
    std::array<Chord, maxProgressionLength> chords;
    std::array<VoicedChord, maxProgressionLength> voiced;
    GrooveEngine::voiceProgression (s, chords, voiced);
    GrooveEngine::voiceStrings (s, chords, chordsNotes);
    for (int i = 1; i < 4; ++i)
        CHECK (std::abs (chordsNotes[(size_t) i].mean() - chordsNotes[(size_t) i - 1].mean()) <= 4.0);

    // Wide pad: root and fifth under it, above the bass; high pad: three voices an octave up.
    s.strings = StringsMode::widePad;
    for (const auto& e : strings (GrooveEngine::renderClip (s, 16.0), true)) CHECK (e.note >= 41 && e.note <= 80);
    s.strings = StringsMode::highPad;
    const auto high = strings (GrooveEngine::renderClip (s, 16.0), true);
    CHECK (high.size() == 12);
    for (const auto& e : high) CHECK (e.note >= 64);

    // Stabs: on the piano's chord hits only.
    s.strings = StringsMode::stabs;
    const auto stabs = GrooveEngine::renderClip (s, 16.0);
    std::set<double> chordTimes;
    for (const auto& e : stabs) if (e.isNoteOn() && e.part == Part::chords) chordTimes.insert (e.ppq);
    const auto stabOns = strings (stabs, true);
    CHECK (! stabOns.empty());
    for (const auto& e : stabOns) CHECK (chordTimes.count (e.ppq) > 0 || std::any_of (chordTimes.begin(), chordTimes.end(),
                                                                                       [&] (double t) { return std::abs (t - e.ppq) < 0.01; }));

    // Starting in the middle of a chord, the pad comes in at once for the rest of it.
    s.strings = StringsMode::pad;
    GrooveEngine engine;
    std::vector<NoteEvent> live;
    engine.render (1.0, 1.5, s, live);
    const auto late = strings (live, true);
    CHECK (late.size() == 4 && late.front().ppq == 1.0);
    live.clear();
    engine.render (1.5, 4.5, s, live);
    CHECK (strings (live, true).size() == 4);   // the next chord, once
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
    testV19Features();
    testVoicingMovement();
    testHousePedalAndStabBass();
    testStepVelocities();
    testClearLowRegister();
    testEvolution();
    testChordRange();
    testContextualFills();
    testGrooveFromMidi();
    testSectionHarmony();
    testStraighter();
    testEditedLoop();
    testCarryEdits();
    testAutoMovement();
    testStrum();
    testPatternsWellFormed();
    testDancePack();
    testHarmonyAnalyzer();
    testHarmonyAnalyzerEdgeCases();
    testBassLines();
    testVoicingCoach();
    testMorphAndSections();
    testMusicalDna();
    testVariationsHumanizeFills();
    testStrings();

    if (failures == 0)
        std::printf ("All engine tests passed.\n");

    return failures == 0 ? 0 : 1;
}
