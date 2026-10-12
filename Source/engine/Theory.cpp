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
    }};

    const char* const scaleNames[numScales] {
        "Major (Ionian)", "Dorian", "Minor (Aeolian)", "Mixolydian",
        "Lydian", "Phrygian", "Harmonic Minor", "Melodic Minor"
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

    /** Chooses the inversion of `pcs` (absolute pitch classes) that best fits the
        centre and voice-leads from `previous`. */
    NoteSet bestInversion (const NoteSet& pcs, double centre, const NoteSet* previous) noexcept
    {
        NoteSet best;
        double bestCost = std::numeric_limits<double>::max();

        for (int first = 0; first < pcs.size; ++first)
        {
            const auto shape = stackFrom (pcs, first, pcs[first]);

            for (int octaveNudge = -12; octaveNudge <= 12; octaveNudge += 12)
            {
                auto candidate = placeNear (shape, centre);
                for (int i = 0; i < candidate.size; ++i)
                    candidate.notes[(size_t) i] += octaveNudge;

                double cost = 0.6 * std::abs (candidate.mean() - centre);
                if (previous != nullptr && previous->size > 0)
                    cost += movement (candidate, *previous) + 0.5 * movement (*previous, candidate);

                if (cost < bestCost - 1.0e-9)
                {
                    bestCost = cost;
                    best = candidate;
                }
            }
        }

        return best;
    }
}

//==============================================================================
const char* scaleName (int scale) noexcept          { return scaleNames[mod (scale, numScales)]; }
const std::array<int, 7>& scaleIntervals (int scale) noexcept { return scales[(size_t) mod (scale, numScales)]; }
const char* noteName (int pitchClass) noexcept      { return noteNames[mod (pitchClass, 12)]; }

const char* extensionName (int ext) noexcept
{
    static const char* const names[numExtensions] { "Triads", "7ths", "9ths", "11ths / 13ths" };
    return names[mod (ext, numExtensions)];
}

const char* voicingName (int voicing) noexcept
{
    static const char* const names[numVoicings] { "Close", "Drop 2", "Rootless", "Wide (2 hands)", "Quartal (4ths)" };
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

    if (ext >= Extension::seventh)
        c.seventh = above (6) % 12;

    const bool dominant = (c.third == 4 && c.seventh == 10);

    if (ext >= Extension::ninth)
    {
        const int ninth = above (8) % 12;
        if (ninth != 1 || dominant)   // b9 only sounds right on dominants
            c.ninth = ninth;
    }

    if (ext >= Extension::eleventh)
    {
        if (c.third == 4)
            c.thirteenth = above (12) % 12;  // the 11th clashes with a major third
        else
            c.eleventh = above (10) % 12;
    }

    // Stack the tones upwards in their natural (compound) order.
    c.tones.add (0);
    c.tones.add (c.third);
    c.tones.add (c.fifth);
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

std::string chordName (const Chord& c)
{
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

    if (canExtend && number > 7)
    {
        const auto pos = quality.find ('7');
        quality.replace (pos, 1, std::to_string (number));
    }
    else if (number > 7)
    {
        addParen (number == 9 ? "9" : number == 11 ? "9,11" : "9,13");
    }

    if (c.ninth == 1)       addParen ("b9");
    if (c.ninth == 3)       addParen ("#9");
    if (c.ninth < 0 && c.eleventh == 5)    addParen ("11");
    if (c.eleventh == 6)    addParen ("#11");
    if (c.ninth < 0 && c.thirteenth == 9)  addParen ("13");
    if (c.thirteenth == 8)  addParen ("b13");

    return std::string (noteName (c.rootPc)) + quality + suffix;
}

std::string romanNumeral (int degree, const Chord& c)
{
    static const char* const upper[7] { "I", "II", "III", "IV", "V", "VI", "VII" };
    static const char* const lower[7] { "i", "ii", "iii", "iv", "v", "vi", "vii" };

    const int d = mod (degree, 7);
    std::string r = (c.third == 4) ? upper[d] : lower[d];

    if (c.third == 3 && c.fifth == 6)  r += (c.seventh == 10 ? "\xc3\xb8" : "\xc2\xb0");  // ø / °
    if (c.third == 4 && c.fifth == 8)  r += "+";

    return r;
}

//==============================================================================
VoicedChord voiceChord (const Chord& chord, Voicing voicing, int centre,
                        const NoteSet* previousRightHand) noexcept
{
    VoicedChord v;

    auto absolutePc = [&chord] (int interval) { return mod (chord.rootPc + interval, 12); };

    // Pitch classes for the right hand, in stacking order.
    NoteSet pcs;

    switch (voicing)
    {
        case Voicing::quartal:
        {
            // Keep the fourths shape intact; just move it by octaves.
            NoteSet shape;
            const int bottom = centre - 7 + mod (chord.rootPc - (centre - 7), 12);
            for (int i = 0; i < chord.quartal.size; ++i)
                shape.add (bottom + chord.quartal[i]);

            v.rightHand = placeNear (shape, centre);
            return v;
        }

        case Voicing::rootless:
        {
            for (int i = 1; i < chord.tones.size; ++i)
                if (! (chord.tones.size > 4 && chord.tones[i] == chord.fifth))
                    pcs.add (absolutePc (chord.tones[i]));
            break;
        }

        case Voicing::wide:
        {
            // Left hand: root + 7th (or 5th) about an octave and a half below the centre.
            const int lhRoot = (centre - 19) + mod (chord.rootPc - (centre - 19), 12);
            v.leftHand.add (lhRoot);
            v.leftHand.add (lhRoot + (chord.seventh >= 0 ? chord.seventh : chord.fifth));

            for (int i = 1; i < chord.tones.size; ++i)
            {
                const int t = chord.tones[i];
                if (t == chord.seventh && chord.seventh >= 0) continue;
                pcs.add (absolutePc (t));
            }

            if (pcs.size > 3)                              // keep 3 notes: drop the 5th
                for (int i = 0; i < pcs.size; ++i)
                    if (pcs[i] == absolutePc (chord.fifth)) { pcs.removeAt (i); break; }

            if (pcs.size < 2)                              // triad: borrow the root
                pcs.add (chord.rootPc);

            v.rightHand = bestInversion (pcs, centre + 3, previousRightHand);
            v.rightHand.sort();
            return v;
        }

        case Voicing::close:
        case Voicing::drop2:
        default:
        {
            for (int i = 0; i < chord.tones.size; ++i)
                pcs.add (absolutePc (chord.tones[i]));
            break;
        }
    }

    v.rightHand = bestInversion (pcs, centre, previousRightHand);
    v.rightHand.sort();

    if (voicing == Voicing::drop2 && v.rightHand.size >= 4)
    {
        v.rightHand.notes[(size_t) v.rightHand.size - 2] -= 12;
        v.rightHand.sort();
    }

    return v;
}

NoteSet pentatonicNotes (int keyRoot, int scale, int low, int high) noexcept
{
    // Minor-third scales use 1 b3 4 5 b7, major-third scales 1 2 3 5 6.
    static constexpr int minorDegrees[5] { 0, 2, 3, 4, 6 };
    static constexpr int majorDegrees[5] { 0, 1, 2, 4, 5 };

    const auto& s = scaleIntervals (scale);
    const int* degrees = scaleIsMinor (scale) ? minorDegrees : majorDegrees;

    bool isMember[12] {};
    for (int i = 0; i < 5; ++i)
        isMember[mod (keyRoot + s[(size_t) degrees[i]], 12)] = true;

    NoteSet out;
    for (int n = low; n <= high; ++n)
        if (isMember[mod (n, 12)])
            out.add (n);

    return out;
}

} // namespace sgk
