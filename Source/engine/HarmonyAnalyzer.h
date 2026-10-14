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
};

HarmonyAnalysis analyzeHarmony (const std::vector<AnalyzedNote>& notes, int beatsPerBar = 4);

} // namespace sgk
