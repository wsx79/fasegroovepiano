// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// On-the-fly material: random funky riffs, random progressions and the
// song-section variations (intro / break / build-up / drop ...).

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace sgk
{

struct GroovePattern;

//==============================================================================
enum class Section { main = 0, intro, breakdown, buildup, drop, outro };
constexpr int numSections = 6;
const char* sectionName (int section) noexcept;

/** Rewrites a groove for a song section: sparse intro/outro, sustained
    breakdown, 8ths-then-16ths build-up, the full groove for main/drop. */
GroovePattern applySection (const GroovePattern& source, Section section);

/** Loudness and bass behaviour of a section. */
double sectionVelocityScale (Section section) noexcept;
bool sectionAllowsFills (Section section) noexcept;

//==============================================================================
//==============================================================================
/** A groove is split into four equal segments, A B C D (half a bar each for
    a two-bar groove). */
constexpr int numSegments = 4;

/** Rebuilds the groove playing the segments in the given order (e.g. A A B C). */
GroovePattern applySegmentOrder (const GroovePattern& source, const std::array<int, numSegments>& order);

/** Copies segment `segment` of `from` into `into` (same length patterns). */
void copySegment (GroovePattern& into, const GroovePattern& from, int segment);

/** Removes holds ('=') that no longer follow a note, e.g. after editing. */
void repairHolds (std::string& line);

/** A brand new two-bar funky house riff (chords + bass) from a seed.
    density 0..1 goes from sparse stabs to busy 16th chops. Same seed = same riff. */
GroovePattern makeRandomPattern (std::uint32_t seed, float density);

/** A variation of a groove for the turnaround bars: a hit moved a 16th later here, a ghost
    note there, a push into the next bar, a bass pick-up. Same seed = same variation. */
GroovePattern makeVariation (const GroovePattern& source, std::uint32_t seed);

/** The groove with a few ghost notes added (a 16th before some hits, on empty steps), and now and
    then a bass ghost before a bass hit: the first step of the groove's evolution. */
GroovePattern addGhostNotes (const GroovePattern& source, std::uint32_t seed);

/** Groove morphing: amount 0 = a, 1 = b, in between a step-by-step blend that always grows the
    same way (each step switches from a to b at its own point), with the holds repaired. */
GroovePattern morphPatterns (const GroovePattern& a, const GroovePattern& b, double amount, std::uint32_t seed);

/** Musical DNA: one step of a variation guided by an intention. */
enum class Intention { soulful = 0, funky, minimal, energetic, sophisticated, straighter };
constexpr int numIntentions = 6;
const char* intentionName (int) noexcept;
GroovePattern applyIntention (const GroovePattern&, Intention, std::uint32_t seed);

/** Voicing movement for any groove: some hits play the chord an inversion up or down so
    the top note moves. level 0 = off, 1 = subtle (the answer half-bars move), 2 = lively
    (every half-bar moves: base, up, base, down). Hits already marked u/d are kept. */
GroovePattern addVoicingMovement (const GroovePattern& source, int level);

/** A random but musical progression of scale degrees (0 = I), built from the
    moves soulful house uses most: ii-V, IV-iii-ii, falling fifths... */
std::vector<int> randomProgression (std::uint32_t seed, int length);

/** The same in a scale: the minor modes move the way minor house does (i - bVII - bVI, i - iv,
    i - IV in Dorian...), the major ones as above. */
std::vector<int> randomProgression (std::uint32_t seed, int length, int scale);

//==============================================================================
struct ArrangementPart
{
    Section section;
    int bars;
    std::uint32_t variation = 0;   // regenerated on its own (0: the groove as it is)
    int endFill = -1;              // fill on its last bar: -1 as set in the Groove panel, -2 none, else a fill style
};

/** Intro 8 | Main 16 | Break 8 | Build-up 4 | Drop 16 | Outro 8 */
const std::vector<ArrangementPart>& defaultArrangement();

} // namespace sgk
