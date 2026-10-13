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

/** A random but musical progression of scale degrees (0 = I), built from the
    moves soulful house uses most: ii-V, IV-iii-ii, falling fifths... */
std::vector<int> randomProgression (std::uint32_t seed, int length);

//==============================================================================
struct ArrangementPart
{
    Section section;
    int bars;
};

/** Intro 8 | Main 16 | Break 8 | Build-up 4 | Drop 16 | Outro 8 */
const std::vector<ArrangementPart>& defaultArrangement();

} // namespace sgk
