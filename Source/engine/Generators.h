#pragma once

// On-the-fly material: random funky riffs, random progressions and the
// song-section variations (intro / break / build-up / drop ...).

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
/** A brand new two-bar funky house riff (chords + bass) from a seed.
    density 0..1 goes from sparse stabs to busy 16th chops. Same seed = same riff. */
GroovePattern makeRandomPattern (std::uint32_t seed, float density);

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
