// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// Music theory core: scales, diatonic chords, chord names and voicings.
// Pure C++ (no JUCE), allocation-free so it can run on the audio thread.

#include <array>
#include <string>

namespace sgk
{

//==============================================================================
// Small fixed-capacity list of MIDI notes / intervals.
struct NoteSet
{
    std::array<int, 16> notes {};
    int size = 0;

    void add (int n) noexcept            { if (size < (int) notes.size()) notes[(size_t) size++] = n; }
    int operator[] (int i) const noexcept { return notes[(size_t) i]; }
    bool contains (int n) const noexcept;
    void removeAt (int index) noexcept;
    void sort() noexcept;
    double mean() const noexcept;
    int lowest() const noexcept;
    int highest() const noexcept;
};

//==============================================================================
// New entries are always appended so saved projects keep their settings.
enum class Extension { triad = 0, seventh, ninth, eleventh, sixNine, add9, sus, alteredDominant,
                       lushEleventh, soulThirteenth, funkySharpNine };
enum class Voicing   { close = 0, drop2, rootless, wide, quartal, shell, drop24, octaveTriads,
                       upperTriad, spreadTenths, openFifths };

constexpr int numScales     = 13;
constexpr int numExtensions = 11;
constexpr int numVoicings   = 11;

const char* scaleName (int scale) noexcept;
const std::array<int, 7>& scaleIntervals (int scale) noexcept;
const char* noteName (int pitchClass) noexcept;
/** Pitch spelling that follows the key signature (sharps in A, flats in Eb). */
const char* spelledNote (int pitchClass, int keyRoot, int scale) noexcept;
const char* extensionName (int ext) noexcept;
const char* voicingName (int voicing) noexcept;

/** True if the scale has a minor third over its tonic. */
bool scaleIsMinor (int scale) noexcept;

//==============================================================================
/** A chord described by its root and the role of each stacked tone.
    Every role holds an interval in semitones above the root (mod 12),
    or -1 if that tone is absent. */
struct Chord
{
    int rootPc = 0;
    int third = -1, fifth = -1, seventh = -1, ninth = -1, eleventh = -1, thirteenth = -1;
    int sixth = -1;            // added 6th (6 and 6/9 chords, no 7th)
    bool suspended = false;    // "third" holds the perfect 4th (sus4)

    NoteSet tones;    // intervals above the root, stacked in thirds (ascending)
    NoteSet quartal;  // diatonic stack of fourths built on the same degree
};

/** Builds the diatonic chord on a scale degree (0..6) of keyRoot/scale.
    Ugly diatonic tensions are tidied up the way a keyboard player would:
    b9 is dropped on non-dominant chords, and the 11th becomes a 13th on
    chords with a major third. A 6th that would be a b6 becomes a 7th, a sus4
    is only used where the 4th is perfect, and "altered V" turns degree V into
    a gospel dominant 13(b9) (or 7(b9,b13) in minor keys). */
Chord buildChord (int keyRoot, int scale, int degree, Extension ext) noexcept;

/** e.g. "Am9", "D13", "Fmaj9", "Bm7b5", "Em7(11)". */
std::string chordName (const Chord& chord, int keyRoot = -1, int scale = 0);

/** Roman numeral for the degree, cased by chord quality: "i", "IV", "vii°". */
std::string romanNumeral (int degree, const Chord& chord);

/** Roman numeral of any chord relative to the key, with b/# for chromatic roots: "bVI", "#iv". */
std::string chromaticNumeral (int keyRoot, const Chord& chord);

//==============================================================================
struct VoicedChord
{
    NoteSet rightHand;  // played on the right-hand (chord) rhythm
    NoteSet leftHand;   // extra low chord tones (wide voicing), also on the chord rhythm
};

/** Spreads the chord on the keyboard around `centre` (MIDI note), choosing the
    inversion that stays close to the centre and moves the least from `previous`. */
VoicedChord voiceChord (const Chord& chord, Voicing voicing, int centre,
                        const NoteSet* previousRightHand, int topTarget = -1) noexcept;

/** Moves a voicing up (positive) or down (negative) by inversions: the lowest note goes up
    an octave, or the highest note goes down an octave, `steps` times. */
void invert (NoteSet& voicing, int steps) noexcept;

//==============================================================================
/** How well chord `b` sounds after chord `a`, from 0 (clashes) to 1 (sounds great):
    shared notes, strength of the root move (down a fifth is best, a tritone worst),
    smooth voice leading between the two voicings, and how much both stay in the key. */
float transitionScore (const Chord& a, const VoicedChord& va, const Chord& b, const VoicedChord& vb,
                       int keyRoot, int scale) noexcept;

/** Share (0..1) of the chord's notes that belong to the key's scale. */
float scaleFit (const Chord& chord, int keyRoot, int scale) noexcept;

/** Pentatonic flavour of the current scale, as absolute MIDI notes in [low, high]. */
NoteSet pentatonicNotes (int keyRoot, int scale, int low, int high) noexcept;

} // namespace sgk

