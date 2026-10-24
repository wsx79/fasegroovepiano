// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// The piano under a leading guitar (LINK with Fase Groove Guitar on LEAD): only a few chords of
// its own groove in four bars, and answers of its own around them. Shared with Fase Groove
// Arranger (its guitar-lead songs), so both play the same comping.

#include "GrooveEngine.h"

#include <array>
#include <cstdint>
#include <vector>

namespace sgk
{

struct Comping
{
    static bool compingKeeps (double ppq, std::uint32_t seed) noexcept;
    /** One window of the four-bar comping cycle: where (beats in the cycle) and what plays there,
        the piano's own groove (figure -1) or an answer of its own (a figure). */
    struct CompingWindow { double start = 0.0; int figure = -1; };
    static int compingWindows (std::uint32_t seed, std::array<CompingWindow, 8>& out) noexcept;
    struct CompingHit { double offset, length; float level; };
    struct CompingFigure { int count; std::array<CompingHit, 3> hits; };
    static constexpr int numCompingFigures = 8;
    static const CompingFigure& compingFigure (int figure) noexcept;
    /** The answers' note-ons and note-offs that fall in [from, to), on the chords of the moment. */
    static void compingAnswers (double from, double to, std::uint32_t seed, const GrooveSettings&,
                                std::vector<NoteEvent>& out);
    /** The same on a progression already voiced (the audio thread voices it only when it changes). */
    static void compingAnswers (double from, double to, std::uint32_t seed, const GrooveSettings&,
                                const std::array<VoicedChord, maxProgressionLength>& voiced,
                                std::vector<NoteEvent>& out);
    /** One hit of the answers in the four-bar cycle (beats), ended before the next one starts. */
    struct CompingSpan { double on = 0.0, off = 0.0; float level = 1.0f; int id = 0; };
    static int compingHits (std::uint32_t seed, std::array<CompingSpan, 24>& out) noexcept;
    /** The events as the piano plays them under a leading guitar: the chord and fill notes
        outside compingKeeps() left out (each with its note-off), the bass and strings as they are. */
    static std::vector<NoteEvent> comping (const std::vector<NoteEvent>& events, std::uint32_t seed,
                                           const GrooveSettings&, double length);
    /** The groove as the grid shows it then: four bars, the right hand only on the comping beats. */
    static GroovePattern compingPattern (const GroovePattern&, std::uint32_t seed);
};

} // namespace sgk
