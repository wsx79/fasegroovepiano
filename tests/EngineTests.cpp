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

static void testPatternsWellFormed()
{
    CHECK (groovePatterns().size() == 20);

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
    testPatternsWellFormed();

    if (failures == 0)
        std::printf ("All engine tests passed.\n");

    return failures == 0 ? 0 : 1;
}
