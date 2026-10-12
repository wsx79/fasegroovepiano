// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Generators.h"
#include "GrooveEngine.h"

#include <algorithm>
#include <array>

namespace sgk
{

namespace
{
    /** Small deterministic PRNG (xorshift32). */
    struct Random
    {
        explicit Random (std::uint32_t seed) : state (seed * 2654435761u + 0x9E3779B9u) { if (state == 0) state = 1; next(); }

        std::uint32_t next() noexcept
        {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return state;
        }

        double uniform() noexcept       { return (next() >> 8) * (1.0 / 16777216.0); }
        bool chance (double p) noexcept { return uniform() < p; }
        int below (int n) noexcept      { return (int) (next() % (std::uint32_t) n); }

        std::uint32_t state;
    };

    bool isHit (char c) noexcept { return c == 'X' || c == 'x' || c == 'g' || c == '>'; }

    /** Adds offbeat ("&") hits until the bar has at least `minimum` hits. */
    void ensureHits (std::string& bar, int minimum)
    {
        auto hits = [&bar] { return (int) std::count_if (bar.begin(), bar.end(), isHit); };
        for (int i = 2; hits() < minimum && i < (int) bar.size(); i += 4)
            if (! isHit (bar[(size_t) i]))
                bar[(size_t) i] = 'x';
    }

    /** Removes '=' that no longer follow a note (after we deleted hits). */
    void repairHolds (std::string& line)
    {
        bool sounding = false;
        for (auto& c : line)
        {
            if (isHit (c))      sounding = true;
            else if (c == '.')  sounding = false;
            else if (c == '=' && ! sounding) c = '.';
        }
    }
}

//==============================================================================
const char* sectionName (int section) noexcept
{
    static const char* const names[numSections] { "Main groove", "Intro", "Break", "Build-up", "Drop", "Outro" };
    return names[std::clamp (section, 0, numSections - 1)];
}

double sectionVelocityScale (Section section) noexcept
{
    switch (section)
    {
        case Section::intro:     return 0.8;
        case Section::breakdown: return 0.72;
        case Section::buildup:   return 0.9;
        case Section::drop:      return 1.1;
        case Section::outro:     return 0.8;
        case Section::main:
        default:                 return 1.0;
    }
}

bool sectionAllowsFills (Section section) noexcept
{
    return section == Section::main || section == Section::drop;
}

GroovePattern applySection (const GroovePattern& source, Section section)
{
    GroovePattern p = source;
    const int len = p.length();

    switch (section)
    {
        case Section::intro:
        case Section::outro:
        {
            // Keep the accents, pushes and on-beat hits, softened; no bass.
            for (int i = 0; i < len; ++i)
            {
                char& c = p.rightHand[(size_t) i];
                if (c == 'g' || (c == 'x' && i % 4 != 0))
                    c = '.';
                else if (c == 'X')
                    c = 'x';
            }

            repairHolds (p.rightHand);

            bool any = std::any_of (p.rightHand.begin(), p.rightHand.end(), isHit);
            if (! any)
                p.rightHand[0] = 'x';

            p.leftHand.assign ((size_t) len, '.');
            break;
        }

        case Section::breakdown:
        {
            // Long, soft pads: one chord per bar, held almost to the end.
            for (int i = 0; i < len; ++i)
            {
                const int inBar = i % 16;
                p.rightHand[(size_t) i] = inBar == 0 ? 'X' : inBar < 15 ? '=' : '.';
            }

            p.leftHand.assign ((size_t) len, '.');
            break;
        }

        case Section::buildup:
        {
            // Bar 1: straight 8ths, bar 2: driving 16ths; bass pumping on quarters then 8ths.
            for (int i = 0; i < len; ++i)
            {
                const bool secondHalf = i >= len / 2;
                p.rightHand[(size_t) i] = secondHalf ? (i % 4 == 0 ? 'X' : 'x') : (i % 2 == 0 ? (i % 4 == 0 ? 'X' : 'x') : '.');
                p.leftHand[(size_t) i]  = secondHalf ? (i % 2 == 0 ? 'x' : '.') : (i % 4 == 0 ? 'x' : '.');
            }
            break;
        }

        case Section::drop:
        case Section::main:
        default:
            break;
    }

    if (section != Section::main)
        p.name = source.name + " / " + sectionName ((int) section);

    return p;
}

//==============================================================================
GroovePattern makeRandomPattern (std::uint32_t seed, float densityIn)
{
    Random rng (seed);
    const double density = std::clamp ((double) densityIn, 0.0, 1.0);

    // Hit probability for the four 16ths of a beat: 1, e, &, a.
    // Funky house lives on the "&" and the "a".
    const std::array<double, 4> weight { 0.35, 0.18, 0.75, 0.45 };
    const double busy = 0.45 + 0.75 * density;

    std::string rh (16, '.'), lh (16, '.');

    // ---- right hand, bar 1 ----
    for (int i = 0; i < 16; ++i)
        if (rng.chance (std::min (0.95, weight[(size_t) (i % 4)] * busy)))
            rh[(size_t) i] = 'x';

    if (rng.chance (0.6)) rh[0] = 'x';

    // No more than three 16ths in a row: funk needs air.
    for (int i = 3; i < 16; ++i)
        if (rh[(size_t) i] == 'x' && rh[(size_t) i - 1] == 'x' && rh[(size_t) i - 2] == 'x' && rh[(size_t) i - 3] == 'x')
            rh[(size_t) i] = '.';

    ensureHits (rh, 4);

    // Accents and holds.
    for (int i = 0; i < 16; ++i)
    {
        if (rh[(size_t) i] != 'x')
            continue;

        if (rng.chance (0.3) || (i % 4 == 2 && rng.chance (0.3)))
            rh[(size_t) i] = 'X';

        const double holdChance = 0.35 * (1.0 - density);
        for (int k = i + 1; k < std::min (16, i + 3) && rh[(size_t) k] == '.' && rng.chance (holdChance); ++k)
            rh[(size_t) k] = '=';
    }

    // Ghost notes in the gaps.
    for (int i = 0; i < 16; ++i)
        if (rh[(size_t) i] == '.' && rng.chance (0.18 * density))
            rh[(size_t) i] = 'g';

    // ---- bass, bar 1 ----
    lh[0] = 'x';
    const std::array<double, 4> bassWeight { 0.3, 0.12, 0.3, 0.35 };
    for (int i = 1; i < 16; ++i)
        if (rng.chance (bassWeight[(size_t) (i % 4)] * (0.6 + 0.6 * density)))
            lh[(size_t) i] = 'x';

    // ---- bar 2 = bar 1 with variations (call & response) ----
    std::string rh2 = rh, lh2 = lh;
    for (int i = 0; i < 16; ++i)
    {
        if (rng.chance (0.25))
        {
            const char was = rh2[(size_t) i];
            rh2[(size_t) i] = was == '.' ? (rng.chance (weight[(size_t) (i % 4)]) ? 'x' : '.') : (was == '=' ? '=' : '.');
        }
        if (i > 0 && rng.chance (0.15))
            lh2[(size_t) i] = lh2[(size_t) i] == '.' ? 'x' : '.';
    }

    ensureHits (rh2, 4);

    std::string right = rh + rh2, left = lh + lh2;

    // ---- pushes into the next bar ----
    auto addPush = [&] (int at)   // at = step of the push (14 or 30)
    {
        right[(size_t) at] = '>';
        right[(size_t) at + 1] = '=';
        const int next = (at + 2) % 32;
        if (right[(size_t) next] == '.' || right[(size_t) next] == 'g')
            right[(size_t) next] = 'x';     // tied by the engine

        if (rng.chance (0.5))
        {
            left[(size_t) at] = '>';
            left[(size_t) at + 1] = '=';
            left[(size_t) next] = 'x';
        }
    };

    if (rng.chance (0.35 + 0.3 * density)) addPush (14);
    if (rng.chance (0.5))                  addPush (30);

    repairHolds (right);
    repairHolds (left);

    GroovePattern p;
    p.name = "Random Riff #" + std::to_string (seed % 100000);
    p.rightHand = right;
    p.leftHand = left;
    return p;
}

//==============================================================================
std::vector<int> randomProgression (std::uint32_t seed, int length)
{
    Random rng (seed ^ 0xA5A5A5A5u);
    length = std::clamp (length, 1, maxProgressionLength);

    // Typical soulful / deep house moves from each degree.
    static const std::array<std::vector<int>, 7> moves {{
        { 3, 5, 1, 2, 4, 3 },   // I   -> IV vi ii iii V (IV twice: favourite)
        { 4, 4, 2, 3, 0 },      // ii  -> V V iii IV I
        { 5, 5, 3, 1 },         // iii -> vi vi IV ii
        { 2, 4, 0, 1, 2 },      // IV  -> iii V I ii iii
        { 0, 5, 2, 3, 0 },      // V   -> I vi iii IV I
        { 1, 1, 3, 4, 2 },      // vi  -> ii ii IV V iii
        { 2, 0 },               // vii -> iii I
    }};

    static const std::array<int, 6> starts { 0, 0, 5, 1, 3, 0 };

    std::vector<int> out;
    out.push_back (starts[(size_t) rng.below ((int) starts.size())]);

    while ((int) out.size() < length)
    {
        const auto& options = moves[(size_t) out.back()];
        int next = options[(size_t) rng.below ((int) options.size())];

        // Avoid ending on the first chord, or it would repeat at the loop point.
        if ((int) out.size() == length - 1 && length > 2 && next == out.front())
            next = options[(size_t) rng.below ((int) options.size())];

        out.push_back (next);
    }

    return out;
}

const std::vector<ArrangementPart>& defaultArrangement()
{
    static const std::vector<ArrangementPart> parts {
        { Section::intro, 8 }, { Section::main, 16 }, { Section::breakdown, 8 },
        { Section::buildup, 4 }, { Section::drop, 16 }, { Section::outro, 8 },
    };
    return parts;
}

} // namespace sgk
