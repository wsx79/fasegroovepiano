// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Theory.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace sgk
{

//==============================================================================
bool NoteSet::contains (int n) const noexcept
{
    for (int i = 0; i < size; ++i)
        if (notes[(size_t) i] == n)
            return true;

    return false;
}

void NoteSet::removeAt (int index) noexcept
{
    if (index < 0 || index >= size)
        return;

    for (int i = index; i < size - 1; ++i)
        notes[(size_t) i] = notes[(size_t) i + 1];

    --size;
}

void NoteSet::sort() noexcept
{
    std::sort (notes.begin(), notes.begin() + size);
}

double NoteSet::mean() const noexcept
{
    if (size == 0)
        return 0.0;

    double sum = 0.0;
    for (int i = 0; i < size; ++i)
        sum += notes[(size_t) i];

    return sum / size;
}

int NoteSet::lowest() const noexcept
{
    return size > 0 ? *std::min_element (notes.begin(), notes.begin() + size) : 0;
}

int NoteSet::highest() const noexcept
{
    return size > 0 ? *std::max_element (notes.begin(), notes.begin() + size) : 0;
}

//==============================================================================
namespace
{
    const std::array<std::array<int, 7>, numScales> scales {{
        { 0, 2, 4, 5, 7, 9, 11 },  // Major (Ionian)
        { 0, 2, 3, 5, 7, 9, 10 },  // Dorian
        { 0, 2, 3, 5, 7, 8, 10 },  // Natural minor (Aeolian)
        { 0, 2, 4, 5, 7, 9, 10 },  // Mixolydian
        { 0, 2, 4, 6, 7, 9, 11 },  // Lydian
        { 0, 1, 3, 5, 7, 8, 10 },  // Phrygian
        { 0, 2, 3, 5, 7, 8, 11 },  // Harmonic minor
        { 0, 2, 3, 5, 7, 9, 11 },  // Melodic minor
        { 0, 1, 3, 5, 6, 8, 10 },  // Locrian
        { 0, 2, 4, 5, 7, 8, 11 },  // Harmonic major (gospel minor iv)
        { 0, 2, 4, 6, 7, 9, 10 },  // Lydian dominant (funky I7#11)
        { 0, 2, 4, 5, 7, 8, 10 },  // Mixolydian b6 (soulful iv, bVI, bVII)
        { 0, 1, 4, 5, 7, 8, 10 },  // Phrygian dominant (dark deep house)
    }};

    const char* const scaleNames[numScales] {
        "Major (Ionian)", "Dorian", "Minor (Aeolian)", "Mixolydian",
        "Lydian", "Phrygian", "Harmonic Minor", "Melodic Minor",
        "Locrian", "Harmonic Major", "Lydian Dominant", "Mixolydian b6", "Phrygian Dominant"
    };

    const char* const noteNames[12] { "C", "Db", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };

    int mod (int a, int m) noexcept { return ((a % m) + m) % m; }

    /** Semitones above the tonic of the scale tone with (unbounded) index i. */
    int scaleTone (const std::array<int, 7>& s, int i) noexcept
    {
        const int octave = (i >= 0 ? i / 7 : (i - 6) / 7);
        return s[(size_t) mod (i, 7)] + 12 * octave;
    }

    /** Close-position stack of the given pitch classes starting from element `first`. */
    NoteSet stackFrom (const NoteSet& pcs, int first, int startNote) noexcept
    {
        NoteSet out;
        int prev = startNote;
        out.add (prev);

        for (int k = 1; k < pcs.size; ++k)
        {
            const int pc = pcs[(first + k) % pcs.size];
            int n = prev + mod (pc - prev, 12);
            if (n == prev) n += 12;
            out.add (n);
            prev = n;
        }

        return out;
    }

    /** Average distance from each note to the nearest note of the other set. */
    double movement (const NoteSet& a, const NoteSet& b) noexcept
    {
        if (a.size == 0 || b.size == 0)
            return 0.0;

        double total = 0.0;
        for (int i = 0; i < a.size; ++i)
        {
            int best = std::numeric_limits<int>::max();
            for (int j = 0; j < b.size; ++j)
                best = std::min (best, std::abs (a[i] - b[j]));
            total += best;
        }

        return total / a.size;
    }

    /** Transposes a shape by octaves so that its mean lands nearest to `centre`. */
    NoteSet placeNear (NoteSet shape, double centre) noexcept
    {
        const int shift = (int) std::lround ((centre - shape.mean()) / 12.0) * 12;
        for (int i = 0; i < shape.size; ++i)
            shape.notes[(size_t) i] += shift;
        return shape;
    }

}

//==============================================================================
const char* scaleName (int scale) noexcept          { return scaleNames[mod (scale, numScales)]; }
const std::array<int, 7>& scaleIntervals (int scale) noexcept { return scales[(size_t) mod (scale, numScales)]; }
const char* noteName (int pitchClass) noexcept      { return noteNames[mod (pitchClass, 12)]; }

bool keySpellsFlats (int keyRoot, int scale) noexcept
{
    // Spell from the parent major of the mode, so A major uses C# and F major uses Bb.
    int parent = mod (keyRoot, 12);
    switch (mod (scale, numScales))
    {
        case 1: parent = mod (keyRoot - 2, 12); break;                 // Dorian
        case 2: case 6: case 7: parent = mod (keyRoot + 3, 12); break; // minor family
        case 3: case 11: parent = mod (keyRoot + 5, 12); break;        // Mixolydian
        case 4: case 10: parent = mod (keyRoot - 5, 12); break;        // Lydian
        case 5: case 12: parent = mod (keyRoot - 4, 12); break;        // Phrygian
        case 8: parent = mod (keyRoot + 1, 12); break;                  // Locrian
        default: break;
    }
    switch (parent)
    {
        case 1: case 3: case 5: case 6: case 8: case 10: return true;   // Db F Gb Ab Bb Eb
        default: return false;
    }
}

const char* spelledNote (int pitchClass, int keyRoot, int scale) noexcept
{
    static const char* const sharp[12] { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    static const char* const flat[12]  { "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B" };
    return (keySpellsFlats (keyRoot, scale) ? flat : sharp)[mod (pitchClass, 12)];
}

const char* extensionName (int ext) noexcept
{
    static const char* const names[numExtensions] { "Triads", "7ths", "9ths", "11ths / 13ths",
                                                       "6/9", "Add 9", "Sus (9sus4)", "9ths + altered V",
                                                       "Lush 11ths (m11, maj9#11)", "Soulful 13ths", "Funky 7#9" };
    return names[mod (ext, numExtensions)];
}

const char* voicingName (int voicing) noexcept
{
    static const char* const names[numVoicings] { "Close", "Drop 2", "Rootless", "Wide (2 hands)", "Quartal (4ths)",
                                                     "Shell (3-7-top)", "Drop 2+4", "Octave triads (disco)",
                                                     "Upper triad (house stab)", "Spread 10ths (2 hands)", "Open 5ths (deep pad)" };
    return names[mod (voicing, numVoicings)];
}

bool scaleIsMinor (int scale) noexcept
{
    return scaleIntervals (scale)[2] == 3;
}

//==============================================================================
Chord buildChord (int keyRoot, int scale, int degree, Extension ext) noexcept
{
    const auto& s = scaleIntervals (scale);
    degree = mod (degree, 7);

    const int base = scaleTone (s, degree);
    auto above = [&] (int stackIndex) { return scaleTone (s, degree + stackIndex) - base; };

    Chord c;
    c.rootPc = mod (keyRoot + base, 12);
    c.third  = above (2);
    c.fifth  = above (4);

    const bool lush = ext == Extension::lushEleventh, soul = ext == Extension::soulThirteenth, funky = ext == Extension::funkySharpNine;
    bool wantsSeventh = ext == Extension::seventh || ext == Extension::ninth || ext == Extension::eleventh
                     || ext == Extension::sus || ext == Extension::alteredDominant || lush || soul || funky;
    bool wantsNinth   = ext == Extension::ninth || ext == Extension::eleventh || ext == Extension::sixNine
                     || ext == Extension::add9 || ext == Extension::sus || ext == Extension::alteredDominant || lush || soul;

    if (ext == Extension::sixNine)
    {
        const int sixth = above (5) % 12;
        if (sixth == 9 && c.fifth == 7)
            c.sixth = sixth;
        else
            wantsSeventh = true;   // a b6 (or a diminished chord) sounds better as a 9th chord
    }

    if (wantsSeventh)
        c.seventh = above (6) % 12;

    // Gospel / soulful dominant on degree V, whatever the scale says.
    const bool alteredV = ext == Extension::alteredDominant && degree == 4 && s[4] == 7;
    if (alteredV)
    {
        c.third = 4;
        c.fifth = 7;
        c.seventh = 10;
    }

    const bool dominant = (c.third == 4 && c.seventh == 10);

    if (wantsNinth)
    {
        const int ninth = above (8) % 12;
        if (alteredV)
            c.ninth = 1;                    // b9
        else if (ninth != 1 || dominant)    // b9 only sounds right on dominants
            c.ninth = ninth;
    }

    if (alteredV)
        c.thirteenth = scaleIsMinor (scale) ? 8 : 9;   // b13 in minor keys, 13 in major keys

    // Lush 11ths: m11 on minor chords, maj9#11 where the #11 belongs to the scale, 13 on dominants.
    if (lush)
    {
        const int eleventh = above (10) % 12;
        if (c.third == 3 && eleventh == 5)
            c.eleventh = 5;
        else if (c.third == 4 && c.seventh == 11 && eleventh == 6)
            c.eleventh = 6;
        else if (dominant && c.ninth == 2)
            c.thirteenth = 9;
    }

    // Soulful 13ths: every dominant becomes a 13 (9 + 13); the others stay 9ths.
    if (soul && dominant && c.ninth == 2)
        c.thirteenth = 9;

    // Funky 7#9: plain 7ths, the dominants with the #9 (the "Hendrix" chord of funk and jackin').
    if (funky && dominant)
        c.ninth = 3;

    if (ext == Extension::eleventh)
    {
        if (c.third == 4)
            c.thirteenth = above (12) % 12;  // the 11th clashes with a major third
        else
            c.eleventh = above (10) % 12;
    }

    // sus4: the 4th replaces the 3rd, only where it is a perfect 4th over a perfect 5th.
    if (ext == Extension::sus && above (3) % 12 == 5 && c.fifth == 7)
    {
        c.third = 5;
        c.suspended = true;
    }

    // Stack the tones upwards in their natural (compound) order.
    c.tones.add (0);
    c.tones.add (c.third);
    c.tones.add (c.fifth);
    if (c.sixth      >= 0) c.tones.add (c.sixth);
    if (c.seventh    >= 0) c.tones.add (c.seventh);
    if (c.ninth      >= 0) c.tones.add (c.ninth + 12);
    if (c.eleventh   >= 0) c.tones.add (c.eleventh + 12);
    if (c.thirteenth >= 0) c.tones.add (c.thirteenth + 12);

    // Diatonic fourths on the same degree (as many voices as the chord has, 3..5).
    const int quartalVoices = std::clamp (c.tones.size, 3, 5);
    for (int i = 0; i < quartalVoices; ++i)
        c.quartal.add (above (3 * i));

    return c;
}

std::string chordName (const Chord& c, int keyRoot, int scale)
{
    const std::string root = keyRoot < 0 ? noteName (c.rootPc) : spelledNote (c.rootPc, keyRoot, scale);

    if (c.suspended)
    {
        if (c.seventh == 10) return root + (c.ninth == 2 ? "9sus4" : "7sus4");
        if (c.seventh == 11) return root + (c.ninth == 2 ? "maj9sus4" : "maj7sus4");
        return root + (c.ninth == 2 ? "sus4(9)" : "sus4");
    }

    if (c.sixth >= 0 && c.seventh < 0 && c.fifth == 7 && (c.third == 3 || c.third == 4))
        return root + (c.third == 3 ? "m" : "") + (c.ninth == 2 ? "6/9" : "6");

    std::string quality;
    bool canExtend = false;   // base ends in a "7" that a natural 9/11/13 may replace

    if (c.third == 4 && c.fifth == 7)
    {
        if      (c.seventh == 11) { quality = "maj7"; canExtend = true; }
        else if (c.seventh == 10) { quality = "7";    canExtend = true; }
        else                        quality = "";
    }
    else if (c.third == 3 && c.fifth == 7)
    {
        if      (c.seventh == 10) { quality = "m7"; canExtend = true; }
        else if (c.seventh == 11) { quality = "m(maj7)"; }
        else                        quality = "m";
    }
    else if (c.third == 3 && c.fifth == 6)
    {
        if      (c.seventh == 10) { quality = "m7b5"; canExtend = true; }
        else if (c.seventh == 9)    quality = "dim7";
        else                        quality = "dim";
    }
    else if (c.third == 4 && c.fifth == 8)
    {
        if      (c.seventh == 11) quality = "maj7#5";
        else if (c.seventh == 10) quality = "7#5";
        else                      quality = "aug";
    }
    else
    {
        quality = "?";
    }

    // Highest natural extension in an unbroken chain from the 9th.
    int number = 7;
    if (c.ninth == 2)
    {
        number = 9;
        if (c.eleventh == 5)    number = 11;
        if (c.thirteenth == 9)  number = 13;
    }

    std::string suffix;
    auto addParen = [&suffix] (const char* t) { suffix += "("; suffix += t; suffix += ")"; };

    // Dominant with an altered 9th and a natural 13th: "G13(b9)".
    const bool alteredThirteen = canExtend && c.seventh == 10 && c.ninth >= 0 && c.ninth != 2 && c.thirteenth == 9;
    if (alteredThirteen)
        number = 13;

    if (canExtend && number > 7)
    {
        const auto pos = quality.find ('7');
        quality.replace (pos, 1, std::to_string (number));
    }
    else if (number == 9 && c.seventh < 0 && (quality.empty() || quality == "m"))
    {
        quality += "add9";
    }
    else if (number > 7)
    {
        addParen (number == 9 ? "9" : number == 11 ? "9,11" : "9,13");
    }

    if (c.ninth == 1)       addParen ("b9");
    if (c.ninth == 3)       addParen ("#9");
    if (c.ninth < 0 && c.eleventh == 5)    addParen ("11");
    if (c.eleventh == 6)    addParen ("#11");
    if (c.ninth < 0 && c.thirteenth == 9 && ! alteredThirteen)  addParen ("13");
    if (c.thirteenth == 8)  addParen ("b13");

    return root + quality + suffix;
}

std::string romanNumeral (int degree, const Chord& c)
{
    static const char* const upper[7] { "I", "II", "III", "IV", "V", "VI", "VII" };
    static const char* const lower[7] { "i", "ii", "iii", "iv", "v", "vi", "vii" };

    const int d = mod (degree, 7);
    if (c.suspended)
        return std::string (upper[d]) + "sus";

    std::string r = (c.third == 4) ? upper[d] : lower[d];

    if (c.third == 3 && c.fifth == 6)  r += (c.seventh == 10 ? "\xc3\xb8" : "\xc2\xb0");  // ø / °
    if (c.third == 4 && c.fifth == 8)  r += "+";

    return r;
}

std::string chromaticNumeral (int keyRoot, const Chord& c)
{
    static const char* const upper[12] { "I", "bII", "II", "bIII", "III", "IV", "#IV", "V", "bVI", "VI", "bVII", "VII" };
    static const char* const lower[12] { "i", "bii", "ii", "biii", "iii", "iv", "#iv", "v", "bvi", "vi", "bvii", "vii" };

    const int semis = mod (c.rootPc - keyRoot, 12);
    if (c.suspended)
        return std::string (upper[semis]) + "sus";

    std::string r = c.third == 3 ? lower[semis] : upper[semis];
    if (c.third == 3 && c.fifth == 6)  r += (c.seventh == 10 ? "\xc3\xb8" : "\xc2\xb0");
    if (c.third == 4 && c.fifth == 8)  r += "+";
    return r;
}

//==============================================================================
namespace
{
    /** Fixed-capacity list of candidate voicings (no allocation on the audio thread). */
    struct Candidates
    {
        std::array<NoteSet, 128> items {};
        int size = 0;
        void add (const NoteSet& n) noexcept { if (size < (int) items.size() && n.size > 0) items[(size_t) size++] = n; }
    };

    /** How a voicing sounds on its own, like a pianist would judge it (0 = clean).
        Punishes semitone rubs between neighbouring voices, muddy small intervals in the
        low register, minor 9ths, holes in the middle and an over-wide hand. */
    double voicingPenalty (const NoteSet& v, bool spread, bool flatNine) noexcept
    {
        double p = 0.0;
        for (int i = 1; i < v.size; ++i)
        {
            const int d = v[i] - v[i - 1];
            const int low = v[i - 1];

            if (d == 0)                 p += 50.0;
            // A semitone between two inner voices in the middle register is the house piano
            // sound (the 9th against the minor 3rd in "b7 9 b3 5"); anywhere else it rubs.
            const bool innerPair = i >= 2 && i <= v.size - 2;
            if (d == 1)                 p += (innerPair && low >= 55) ? 1.0 : 9.0;
            if (d == 2 && low < 52)     p += 3.0;
            if (d <= 4 && low < 48)     p += 4.0;
            if (d < 3 && low < 45)      p += 6.0;

            // Two seconds in a row make a cluster (C D E): pianists spread those notes.
            if (i >= 2 && d <= 2 && v[i - 1] - v[i - 2] <= 2)
                p += 4.0;

            const int maxGap = spread ? 12 : 9;
            if (d > maxGap)             p += (d - maxGap) * 0.8;
        }

        for (int i = 0; i < v.size; ++i)
            for (int j = i + 1; j < v.size; ++j)
                if (v[j] - v[i] == 13 && ! flatNine)
                    p += 5.0;

        const int span = v.highest() - v.lowest();
        const int maxSpan = spread ? 24 : 16;
        if (span > maxSpan)
            p += (span - maxSpan) * 0.8;

        return p;
    }

    /** Close-position stacks of `pcs` in the given order, one per rotation, each placed at
        three octaves around the centre. */
    void addStacks (Candidates& out, const NoteSet& pcs, int centre) noexcept
    {
        for (int first = 0; first < pcs.size; ++first)
        {
            const auto shape = placeNear (stackFrom (pcs, first, pcs[first]), centre);
            for (int shift = -12; shift <= 12; shift += 12)
            {
                auto c = shape;
                for (int i = 0; i < c.size; ++i)
                    c.notes[(size_t) i] += shift;
                out.add (c);
            }
        }
    }

    /** Close stacks with one inner voice lifted an octave (e.g. C D E F# A -> C E F# A D):
        the way pianists open a cluster without leaving the hand position. */
    void addLiftedStacks (Candidates& out, const NoteSet& pcs, int centre) noexcept
    {
        if (pcs.size < 4)
            return;

        for (int first = 0; first < pcs.size; ++first)
        {
            const auto shape = stackFrom (pcs, first, pcs[first]);
            for (int lift = 0; lift < shape.size - 1; ++lift)
            {
                auto lifted = shape;
                lifted.notes[(size_t) lift] += 12;
                lifted.sort();
                lifted = placeNear (lifted, centre);

                for (int shift = -12; shift <= 12; shift += 12)
                {
                    auto c = lifted;
                    for (int i = 0; i < c.size; ++i)
                        c.notes[(size_t) i] += shift;
                    out.add (c);
                }
            }
        }
    }

    NoteSet sortedFromRoot (NoteSet pcs, int rootPc) noexcept
    {
        std::sort (pcs.notes.begin(), pcs.notes.begin() + pcs.size,
                   [rootPc] (int a, int b) { return mod (a - rootPc, 12) < mod (b - rootPc, 12); });
        return pcs;
    }

    void removePc (NoteSet& pcs, int pc) noexcept
    {
        for (int i = 0; i < pcs.size; ++i)
            if (pcs[i] == pc) { pcs.removeAt (i); return; }
    }

    /** Lowers voices (counted from the top: 1 = second from the top) by an octave. */
    NoteSet dropVoices (NoteSet v, bool dropSecond, bool dropFourth) noexcept
    {
        v.sort();
        if (dropSecond && v.size >= 2) v.notes[(size_t) v.size - 2] -= 12;
        if (dropFourth && v.size >= 4) v.notes[(size_t) v.size - 4] -= 12;
        v.sort();
        return v;
    }
}

void invert (NoteSet& v, int steps) noexcept
{
    if (v.size < 2)
        return;

    for (int i = 0; i < std::abs (steps); ++i)
    {
        v.sort();
        if (steps > 0) v.notes[0] += 12;
        else           v.notes[(size_t) v.size - 1] -= 12;
    }
    v.sort();
}

VoicedChord voiceChord (const Chord& chord, Voicing voicing, int centre,
                        const NoteSet* previousRightHand, int topTarget) noexcept
{
    VoicedChord v;

    auto absolutePc = [&chord] (int interval) { return mod (chord.rootPc + interval, 12); };

    // A fixed shape (fourths, the open pad) placed where it moves the least from the chord before
    // and lets the top note follow the line asked for: by octaves, and for the fourths also by
    // inversions (the lowest note up an octave), as a pianist moves a So What shape.
    auto placeShape = [&] (const NoteSet& shape, bool inversions)
    {
        NoteSet best = placeNear (shape, centre);
        if ((previousRightHand == nullptr || previousRightHand->size == 0) && topTarget < 0)
            return best;
        double bestCost = std::numeric_limits<double>::max();
        for (int inv = 0; inv < (inversions ? shape.size : 1); ++inv)
        {
            NoteSet turned = shape;
            invert (turned, inv);
            turned = placeNear (turned, centre);
            for (int shift = -12; shift <= 12; shift += 12)
            {
                NoteSet c = turned;
                for (int i = 0; i < c.size; ++i)
                    c.notes[(size_t) i] += shift;
                double cost = 0.6 * std::abs (c.mean() - centre) + 1.5 * inv;
                if (previousRightHand != nullptr && previousRightHand->size > 0)
                    cost += movement (c, *previousRightHand) + 0.35 * std::abs (c.highest() - previousRightHand->highest());
                if (topTarget >= 0)
                    cost += 3.0 * std::abs (c.highest() - topTarget);
                if (cost < bestCost - 1.0e-9)
                {
                    bestCost = cost;
                    best = c;
                }
            }
        }
        return best;
    };

    if (voicing == Voicing::quartal)
    {
        // Keep the fourths shape; move it by octaves or inversions.
        NoteSet shape;
        const int bottom = centre - 7 + mod (chord.rootPc - (centre - 7), 12);
        for (int i = 0; i < chord.quartal.size; ++i)
            shape.add (bottom + chord.quartal[i]);

        v.rightHand = placeShape (shape, true);
        return v;
    }

    if (voicing == Voicing::spreadTenths)
    {
        // The pianist's spread: left hand root and 10th (the 3rd an octave up), right hand the
        // 7th, 9th and the colour note, each just above the one before: never crossing.
        const int lhRoot = (centre - 24) + mod (chord.rootPc - (centre - 24), 12);
        v.leftHand.add (lhRoot);
        v.leftHand.add (lhRoot + 12 + chord.third);

        NoteSet upper;
        if (chord.seventh >= 0)    upper.add (chord.seventh);
        else if (chord.sixth >= 0) upper.add (chord.sixth);
        if (chord.ninth >= 0)      upper.add (chord.ninth);
        if (chord.thirteenth >= 0) upper.add (chord.thirteenth);
        else if (chord.eleventh >= 0) upper.add (chord.eleventh);
        else                       upper.add (chord.fifth);
        if (upper.size < 2)
            upper.add (0);   // triads: the root on top

        int note = v.leftHand[1];
        for (int i = 0; i < upper.size; ++i)
        {
            const int pc = absolutePc (upper[i]);
            int next = note + 1 + mod (pc - (note + 1), 12);
            if (next - note < 2 && i == 0)
                next += 12;   // no semitone straight above the 10th
            v.rightHand.add (next);
            note = next;
        }
        return v;
    }

    if (voicing == Voicing::openFifths)
    {
        // The deep house pad: root, 5th, 9th and the 3rd on top (R 5 9 10), open, moving in
        // parallel like a held pad shape.
        NoteSet shape;
        const int bottom = centre - 9 + mod (chord.rootPc - (centre - 9), 12);
        shape.add (bottom);
        shape.add (bottom + (chord.fifth >= 0 ? chord.fifth : 7));
        shape.add (bottom + 12 + (chord.ninth >= 0 ? chord.ninth : 0));   // the 9th, or the octave
        shape.add (bottom + 12 + chord.third);
        shape.sort();
        v.rightHand = placeShape (shape, false);
        return v;
    }

    if (voicing == Voicing::octaveTriads)
    {
        // Disco piano: a plain triad doubled an octave up (R 3 5 R 3 5), both hands locked.
        const int pcs[3] { chord.rootPc, absolutePc (chord.third), absolutePc (chord.fifth) };
        double bestCost = std::numeric_limits<double>::max();
        NoteSet best;
        for (int first = 0; first < 3; ++first)
        {
            NoteSet triad;
            int note = (centre - 9) + mod (pcs[first] - (centre - 9), 12);
            triad.add (note);
            for (int k = 1; k < 3; ++k)
            {
                const int pc = pcs[(first + k) % 3];
                note += mod (pc - note, 12) == 0 ? 12 : mod (pc - note, 12);
                triad.add (note);
            }
            for (int shift : { -12, 0 })
            {
                NoteSet c;
                for (int i = 0; i < 3; ++i) c.add (triad[i] + shift);
                for (int i = 0; i < 3; ++i) c.add (triad[i] + shift + 12);
                c.sort();
                double cost = 0.6 * std::abs (c.mean() - centre);
                if (previousRightHand != nullptr && previousRightHand->size > 0)
                    cost += movement (c, *previousRightHand) + 0.35 * std::abs (c.highest() - previousRightHand->highest());
                if (topTarget >= 0)
                    cost += 3.0 * std::abs (c.highest() - topTarget);
                if (cost < bestCost - 1.0e-9)
                {
                    bestCost = cost;
                    best = c;
                }
            }
        }
        v.rightHand = best;
        return v;
    }

    // All chord tones (as pitch classes) in stacked-thirds order: 1 3 5 (6) 7 9 11 13.
    NoteSet all;
    for (int i = 0; i < chord.tones.size; ++i)
        all.add (absolutePc (chord.tones[i]));

    const int third = absolutePc (chord.third), fifth = absolutePc (chord.fifth);
    const bool flatNine = chord.ninth == 1;

    // The notes each voicing uses.
    NoteSet pcs;
    bool spread = false;

    switch (voicing)
    {
        case Voicing::rootless:
        {
            // 3 5 7 9 (or 3 7 9 13 / 3 7 9 11): the bass plays the root.
            pcs = all;
            removePc (pcs, chord.rootPc);
            if (pcs.size > 4) removePc (pcs, fifth);
            if (pcs.size < 3) pcs.add (chord.rootPc);   // triads: first inversion
            break;
        }

        case Voicing::shell:
        {
            // 3rd, 7th (or 6th / 5th) and the top colour note.
            pcs.add (third);
            const int guide = chord.seventh >= 0 ? chord.seventh : chord.sixth >= 0 ? chord.sixth : chord.fifth;
            pcs.add (absolutePc (guide));
            const int top = chord.tones[chord.tones.size - 1];
            const int topPc = absolutePc (top);
            pcs.add (chord.tones.size > 3 && ! pcs.contains (topPc) ? topPc : chord.rootPc);
            break;
        }

        case Voicing::upperTriad:
        {
            // The upper three tones of the chord as a close triad (over Am9: C E G... B), the
            // modern house stab; the bass plays the root.
            for (int i = std::max (0, all.size - 3); i < all.size; ++i)
                pcs.add (all[i]);
            break;
        }

        case Voicing::wide:
        {
            // Left hand: root + 7th (or 5th) about an octave and a half below the centre.
            const int lhRoot = (centre - 19) + mod (chord.rootPc - (centre - 19), 12);
            v.leftHand.add (lhRoot);
            v.leftHand.add (lhRoot + (chord.seventh >= 0 ? chord.seventh : chord.fifth));

            pcs = all;
            removePc (pcs, chord.rootPc);
            if (chord.seventh >= 0) removePc (pcs, absolutePc (chord.seventh));
            if (pcs.size > 3) removePc (pcs, fifth);
            if (pcs.size < 2) pcs.add (chord.rootPc);
            centre += 3;
            break;
        }

        case Voicing::drop2:
        case Voicing::drop24:
        {
            // Four (or five) voices, then open them up.
            pcs = all;
            if (pcs.size > 4 && voicing == Voicing::drop2) removePc (pcs, chord.rootPc);
            if (pcs.size > 4) removePc (pcs, fifth);
            if (pcs.size > 5) removePc (pcs, chord.rootPc);
            spread = true;
            break;
        }

        case Voicing::close:
        case Voicing::quartal:
        default:
        {
            pcs = all;
            if (pcs.size > 5) removePc (pcs, fifth);
            if (pcs.size > 5) removePc (pcs, chord.rootPc);
            break;
        }
    }

    // Candidates: every rotation of the stacked-thirds order and of the close (sorted) order.
    Candidates candidates;
    Candidates closeShapes;
    addStacks (closeShapes, pcs, centre);
    addStacks (closeShapes, sortedFromRoot (pcs, chord.rootPc), centre);
    if (voicing != Voicing::drop2 && voicing != Voicing::drop24)
        addLiftedStacks (closeShapes, sortedFromRoot (pcs, chord.rootPc), centre);

    for (int i = 0; i < closeShapes.size; ++i)
    {
        const auto& c = closeShapes.items[(size_t) i];
        if (voicing == Voicing::drop2)
            candidates.add (dropVoices (c, true, false));
        else if (voicing == Voicing::drop24)
            candidates.add (dropVoices (c, true, c.size >= 4));
        else
            candidates.add (c);
    }

    // Pick the best: clean on its own, near the register, smooth from the previous chord,
    // with a top line that moves little (it is the melody the ear follows).
    double bestCost = std::numeric_limits<double>::max();
    NoteSet best;

    for (int i = 0; i < candidates.size; ++i)
    {
        auto c = candidates.items[(size_t) i];
        c.sort();

        double cost = 2.0 * voicingPenalty (c, spread, flatNine) + 0.6 * std::abs (c.mean() - centre);

        // Stay inside the playing range.
        cost += 10.0 * (std::max (0, (centre - 18) - c.lowest()) + std::max (0, c.highest() - (centre + 18)));

        if (previousRightHand != nullptr && previousRightHand->size > 0)
        {
            cost += movement (c, *previousRightHand) + 0.5 * movement (*previousRightHand, c);
            cost += 0.35 * std::abs (c.highest() - previousRightHand->highest());
        }

        // Top line: steer the highest note towards the melody note asked for.
        if (topTarget >= 0)
            cost += 3.0 * std::abs (c.highest() - topTarget);

        if (cost < bestCost - 1.0e-9)
        {
            bestCost = cost;
            best = c;
        }
    }

    v.rightHand = best;
    auto clampMidi = [] (NoteSet& notes) { for (int i = 0; i < notes.size; ++i) notes.notes[(size_t) i] = std::clamp (notes.notes[(size_t) i], 0, 127); };
    clampMidi (v.rightHand);
    clampMidi (v.leftHand);
    return v;
}

float scaleFit (const Chord& chord, int keyRoot, int scale) noexcept
{
    if (chord.tones.size == 0)
        return 1.0f;

    const auto& s = scaleIntervals (scale);
    int inside = 0;
    for (int i = 0; i < chord.tones.size; ++i)
    {
        const int pc = mod (chord.rootPc + chord.tones[i] - keyRoot, 12);
        inside += std::find (s.begin(), s.end(), pc) != s.end() ? 1 : 0;
    }

    return (float) inside / (float) chord.tones.size;
}

float transitionScore (const Chord& a, const VoicedChord& va, const Chord& b, const VoicedChord& vb,
                       int keyRoot, int scale) noexcept
{
    // Shared pitch classes.
    bool inA[12] {}, inB[12] {};
    for (int i = 0; i < a.tones.size; ++i) inA[mod (a.rootPc + a.tones[i], 12)] = true;
    for (int i = 0; i < b.tones.size; ++i) inB[mod (b.rootPc + b.tones[i], 12)] = true;

    int shared = 0, sizeA = 0, sizeB = 0;
    for (int pc = 0; pc < 12; ++pc)
    {
        shared += (inA[pc] && inB[pc]) ? 1 : 0;
        sizeA += inA[pc] ? 1 : 0;
        sizeB += inB[pc] ? 1 : 0;
    }
    const float common = std::min (1.0f, (float) shared / (float) std::max (1, std::min (sizeA, sizeB) - 1));

    // Root motion, by interval upwards: up a 4th (= down a 5th) is the strongest.
    static constexpr float rootMove[12] { 0.6f, 0.4f, 0.75f, 0.8f, 0.7f, 1.0f, 0.3f, 0.85f, 0.75f, 0.8f, 0.75f, 0.45f };
    const float root = rootMove[mod (b.rootPc - a.rootPc, 12)];

    // Voice leading: average distance each voice has to move.
    const double move = movement (vb.rightHand, va.rightHand);
    const float voiceLeading = std::clamp (1.0f - (float) (move - 1.0) / 4.0f, 0.0f, 1.0f);

    const float fit = 0.5f * (scaleFit (a, keyRoot, scale) + scaleFit (b, keyRoot, scale));

    return std::clamp (0.3f * common + 0.3f * root + 0.2f * voiceLeading + 0.2f * fit, 0.0f, 1.0f);
}

NoteSet pentatonicNotes (int keyRoot, int scale, int low, int high) noexcept
{
    // Five scale tones, not a walk of diatonic degrees (that walks off the pentatonic
    // on Locrian, harmonic major, Mixolydian b6 and Phrygian dominant).
    // Major and natural minor match the usual 1 2 3 5 6 and 1 b3 4 5 b7 sets.
    static constexpr int tones[numScales][5] {
        { 0, 2, 4, 7, 9 }, { 0, 3, 5, 7, 10 }, { 0, 3, 5, 7, 10 }, { 0, 2, 4, 7, 9 },
        { 0, 2, 4, 7, 9 }, { 0, 3, 5, 7, 10 }, { 0, 3, 5, 7, 8 }, { 0, 3, 5, 7, 11 },
        { 0, 1, 3, 6, 10 }, { 0, 2, 4, 7, 11 }, { 0, 2, 4, 7, 9 }, { 0, 2, 4, 7, 10 },
        { 0, 1, 4, 7, 10 }
    };
    const auto& row = tones[mod (scale, numScales)];
    bool isMember[12] {};
    for (int i = 0; i < 5; ++i)
        isMember[mod (keyRoot + row[i], 12)] = true;

    NoteSet out;
    for (int n = low; n <= high; ++n)
        if (isMember[mod (n, 12)])
            out.add (n);

    return out;
}

} // namespace sgk

