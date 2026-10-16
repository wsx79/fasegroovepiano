// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// Estimates key, chords and harmonic rhythm from a MIDI part (a piano riff, a chord loop...).
// The result is an estimate: inversions, passing notes and missing roots can mislead it.

#include "GrooveEngine.h"

#include <array>
#include <vector>

namespace sgk
{

struct AnalyzedNote
{
    double start = 0.0, end = 0.0;   // in beats (quarter notes)
    int note = 60;
    int velocity = 100;
};

struct HarmonyAnalysis
{
    bool ok = false;
    int keyRoot = 0;
    int scale = 0;                   // 0 Ionian, 1 Dorian, 2 Aeolian
    int chordLengthChoice = 1;       // 0 half bar, 1 bar, 2 two bars, 3 four bars
    std::vector<int> chordCodes;     // free-chord codes relative to keyRoot (major / minor / dominant)
    std::vector<PresetSlot> slots;   // the same chords as degrees of the scale or free chords
    double outOfScale = 0.0;         // share of the notes (by duration) outside the scale
    std::array<bool, 12> outsideNotes {};   // pitch classes noticeably outside the scale
    double confidence = 0.0;         // 0..1: how well the chords explain the notes
    GroovePattern groove;            // the part's rhythm (see grooveFromNotes), length 0 if none
};

HarmonyAnalysis analyzeHarmony (const std::vector<AnalyzedNote>& notes, int beatsPerBar = 4);

/** The groove of a MIDI part, as a two-bar grid (32 sixteenths, like the user pattern): the
    chords' rhythm on the right hand, the bass's (low notes under the chords) on the left, with
    accents, ghost notes and holds. Steps that play in at least half of the two-bar rounds count.
    Empty (length 0) when it can't be read: not 4/4, or fewer than three chord hits. */
GroovePattern grooveFromNotes (const std::vector<AnalyzedNote>& notes, int beatsPerBar = 4);

} // namespace sgk
