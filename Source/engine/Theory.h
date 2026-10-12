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
enum class Extension { triad = 0, seventh, ninth, eleventh };
enum class Voicing   { close = 0, drop2, rootless, wide, quartal };

constexpr int numScales     = 8;
constexpr int numExtensions = 4;
constexpr int numVoicings   = 5;

const char* scaleName (int scale) noexcept;
const std::array<int, 7>& scaleIntervals (int scale) noexcept;
const char* noteName (int pitchClass) noexcept;
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

    NoteSet tones;    // intervals above the root, stacked in thirds (ascending)
    NoteSet quartal;  // diatonic stack of fourths built on the same degree
};

/** Builds the diatonic chord on a scale degree (0..6) of keyRoot/scale.
    Ugly diatonic tensions are tidied up the way a keyboard player would:
    b9 is dropped on non-dominant chords, and the 11th becomes a 13th on
    chords with a major third. */
Chord buildChord (int keyRoot, int scale, int degree, Extension ext) noexcept;

/** e.g. "Am9", "D13", "Fmaj9", "Bm7b5", "Em7(11)". */
std::string chordName (const Chord& chord);

/** Roman numeral for the degree, cased by chord quality: "i", "IV", "vii°". */
std::string romanNumeral (int degree, const Chord& chord);

//==============================================================================
struct VoicedChord
{
    NoteSet rightHand;  // played on the right-hand (chord) rhythm
    NoteSet leftHand;   // extra low chord tones (wide voicing), also on the chord rhythm
};

/** Spreads the chord on the keyboard around `centre` (MIDI note), choosing the
    inversion that stays close to the centre and moves the least from `previous`. */
VoicedChord voiceChord (const Chord& chord, Voicing voicing, int centre,
                        const NoteSet* previousRightHand) noexcept;

/** Pentatonic flavour of the current scale, as absolute MIDI notes in [low, high]. */
NoteSet pentatonicNotes (int keyRoot, int scale, int low, int high) noexcept;

} // namespace sgk
