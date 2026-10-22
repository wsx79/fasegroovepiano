// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Generators.h"
#include "GrooveEngine.h"

#include <algorithm>
#include <array>
#include <cmath>

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
        int below (int n) noexcept      { return n > 0 ? (int) (next() % (std::uint32_t) n) : 0; }

        std::uint32_t state;
    };

    bool isHit (char c) noexcept { return c == 'X' || c == 'x' || c == 'g' || c == '>' || c == 'u' || c == 'U' || c == 'd' || c == 'D'; }


}

void repairHolds (std::string& line)
{
    // A hold at the very start may continue a note from the end of the loop.
    bool sounding = false;
    for (int pass = 0; pass < 2; ++pass)
    {
        for (auto& c : line)
        {
            if (isHit (c))      sounding = true;
            else if (c == '.')  sounding = false;
            else if (c == '=' && ! sounding && pass == 1) c = '.';
        }
    }
}

GroovePattern applySegmentOrder (const GroovePattern& source, const std::array<int, numSegments>& order)
{
    const int len = source.length();
    if (len % numSegments != 0)
        return source;

    bool identity = true;
    for (int i = 0; i < numSegments; ++i)
        identity = identity && order[(size_t) i] == i;
    if (identity)
        return source;

    const int seg = len / numSegments;
    GroovePattern p = source;

    for (int i = 0; i < numSegments; ++i)
    {
        const int from = std::clamp (order[(size_t) i], 0, numSegments - 1) * seg;
        for (int k = 0; k < seg; ++k)
        {
            p.rightHand[(size_t) (i * seg + k)] = source.rightHand[(size_t) (from + k)];
            p.leftHand[(size_t) (i * seg + k)]  = source.leftHand[(size_t) (from + k)];
        }
    }

    repairHolds (p.rightHand);
    repairHolds (p.leftHand);
    return p;
}

void copySegment (GroovePattern& into, const GroovePattern& from, int segment)
{
    const int len = into.length();
    if (len != from.length() || len % numSegments != 0 || segment < 0 || segment >= numSegments)
        return;

    const int seg = len / numSegments;
    for (int k = segment * seg; k < (segment + 1) * seg; ++k)
    {
        into.rightHand[(size_t) k] = from.rightHand[(size_t) k];
        into.leftHand[(size_t) k]  = from.leftHand[(size_t) k];
    }

    repairHolds (into.rightHand);
    repairHolds (into.leftHand);
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
                if (c == 'g' || ((c == 'x' || c == 'u' || c == 'd') && i % 4 != 0))
                    c = '.';
                else if (c == 'X')
                    c = 'x';
                else if (c == 'U' || c == 'D')
                    c = (char) (c - 'A' + 'a');
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
namespace
{
    /** Piano house riff from the classic loop figures: syncopated offbeat stabs (often every
        three 16ths) over an octave bass on the 1 and the "&" of 2, decorated per seed. */
    GroovePattern makeLoopRiff (Random& rng, double density)
    {
        // One-bar figures: the dotted-8th (3+3+3+3+2+2) family and its relatives, the rhythms
        // that most piano house loops are built on.
        static const char* const rhBars[] {
            "x..x..x..x..x.x.", "x...x..x..x..x..", "x...x..x..x.x.x.", "..x.x..x..x.x.x.",
            "x..x..x..x..x...", "x...x.x.x..x..x.", "x...x..x..x...x.", "x...x..x.x..x.x.",
            "..x..x..x..x..x.", "x.x.x..x..x.x..x", "x...x...x..x..x.", "x..x..x.x..x..x.",
            "x..x..x...x..x..", "..x..x....x..x..", "x......x..x..x..", "..x....x...x..x.",
        };
        static const char* const lhBars[] {
            "x.....x.........", "x.....x.......x.", "x.....x.....x...", "x.....x.x...x.x.",
        };
        constexpr int numRh = (int) (sizeof (rhBars) / sizeof (*rhBars));
        constexpr int numLh = (int) (sizeof (lhBars) / sizeof (*lhBars));

        // House loops repeat their bar; now and then the second bar answers with another figure.
        const int a = rng.below (numRh);
        const int b = rng.chance (0.7) ? a : rng.below (numRh);
        std::string right = std::string (rhBars[a]) + rhBars[b];

        const int la = rng.below (numLh);
        std::string left = std::string (lhBars[la]) + lhBars[rng.chance (0.6) ? la : rng.below (numLh)];

        // Steady touch, like the loops: only the first hit of each bar is accented; sparse
        // riffs hold a stab a little longer now and then.
        for (int bar = 0; bar < 2; ++bar)
        {
            bool first = true;
            for (int i = bar * 16; i < bar * 16 + 16; ++i)
            {
                char& c = right[(size_t) i];
                if (c != 'x')
                    continue;
                if (first)
                    c = 'X';
                first = false;
                if (i + 1 < 32 && right[(size_t) i + 1] == '.' && rng.chance (0.25 * (1.0 - density)))
                    right[(size_t) i + 1] = '=';
            }
        }

        // Push into the next loop, now and then with the bass.
        if (rng.chance (0.45) && right[30] != '=')
        {
            right[30] = '>'; right[31] = '=';
            if (rng.chance (0.5)) { left[30] = '>'; left[31] = '='; }
        }

        repairHolds (right);
        repairHolds (left);

        GroovePattern p;
        p.rightHand = right;
        p.leftHand = left;
        return p;
    }
}

GroovePattern makeRandomPatternRaw (std::uint32_t seed, float densityIn);

namespace
{
    /** The last touch on a random riff: as busy as its density asks, never a long burst.
        - at most 12 (sparse) .. 20 (busy) chord hits in the two bars: the weakest ones go
          (an unaccented hit off the beat, not starting a hold or a push);
        - at most 3 hits in a row (4 when busy): the middle of a longer burst becomes a ghost. */
    void normaliseRiff (GroovePattern& p, double density)
    {
        auto& r = p.rightHand;
        const int len = (int) r.size();
        auto strongHit = [] (char c) { return c == 'X' || c == 'x' || c == '>' || c == 'u' || c == 'U' || c == 'd' || c == 'D'; };

        const int maxHits = (int) std::lround (12.0 + 8.0 * density) * std::max (1, len / 32);
        int hits = (int) std::count_if (r.begin(), r.end(), strongHit);
        for (int pass = 0; pass < 2 && hits > maxHits; ++pass)
            for (int i = len - 1; i >= 0 && hits > maxHits; --i)
            {
                const bool weak = r[(size_t) i] == 'x' && i % 4 != 0
                               && (i + 1 >= len || (r[(size_t) i + 1] != '=' && r[(size_t) i + 1] != '>'));
                // first pass: the 16ths between the 8ths; then any weak hit
                if (weak && (pass == 1 || i % 2 == 1))
                {
                    r[(size_t) i] = '.';
                    --hits;
                }
            }

        const int maxRun = density > 0.6 ? 4 : 3;
        int run = 0;
        for (int i = 0; i < len; ++i)
        {
            run = strongHit (r[(size_t) i]) ? run + 1 : 0;
            if (run > maxRun && r[(size_t) i - 1] == 'x')
            {
                r[(size_t) i - 1] = 'g';   // breathe inside the burst
                run = 1;
            }
        }
        repairHolds (r);
    }
}

GroovePattern makeRandomPattern (std::uint32_t seed, float densityIn)
{
    auto p = makeRandomPatternRaw (seed, densityIn);
    normaliseRiff (p, std::clamp ((double) densityIn, 0.0, 1.0));
    return p;
}

GroovePattern makeRandomPatternRaw (std::uint32_t seed, float densityIn)
{
    {
        // Most riffs come from the classic piano house loop figures; only busy settings
        // sometimes build one from comping cells instead.
        Random pick (seed ^ 0xC0FFEEu);
        if (pick.chance (0.95 - 0.35 * std::clamp ((double) densityIn, 0.0, 1.0)))
        {
            auto p = makeLoopRiff (pick, std::clamp ((double) densityIn, 0.0, 1.0));
            p.name = "Random Riff #" + std::to_string (seed % 100000);
            return p;
        }
    }

    // Built like a keyboard player builds a riff: a half-bar motif made of house
    // comping cells, answered by a second half-bar, then repeated with a different ending
    // that pushes into the next bar (A B A B'). The bass keeps it simple and locks to it.
    Random rng (seed);
    const double density = std::clamp ((double) densityIn, 0.0, 1.0);

    // One-beat right-hand cells (4 sixteenths), from sparse to busy.
    static const char* const sparseCells[] { "..x.", "x...", "x=..", "..x=", "x===", "....", "...x", "x.x." };
    static const char* const mediumCells[] { "..x.", ".x.x", "x..x", "..xx", "x.x.", "...x", ".xx.", "x..." , "..x=", "x=.x" };
    static const char* const busyCells[]   { "x.xx", ".x.x", "xx.x", "x.x.", ".xx.", "..xx", "x..x", ".x.." };

    auto pickCell = [&] () -> std::string
    {
        const double r = rng.uniform();
        const double busyShare = 0.1 + 0.6 * density;
        const double sparseShare = 0.55 * (1.0 - density);
        if (r < sparseShare)
            return sparseCells[rng.below ((int) (sizeof (sparseCells) / sizeof (*sparseCells)))];
        if (r < sparseShare + busyShare)
            return busyCells[rng.below ((int) (sizeof (busyCells) / sizeof (*busyCells)))];
        return mediumCells[rng.below ((int) (sizeof (mediumCells) / sizeof (*mediumCells)))];
    };

    auto countIn = [] (const std::string& line) { return (int) std::count_if (line.begin(), line.end(), isHit); };

    // A half-bar phrase (two beats) with at least one and at most six hits.
    auto makeHalf = [&] ()
    {
        for (int attempt = 0; attempt < 16; ++attempt)
        {
            const auto half = pickCell() + pickCell();
            const int hits = countIn (half);
            if (hits >= 1 && hits <= 6)
                return half;
        }
        return std::string ("..x...x.");
    };

    // Accents (first hit, and offbeat "&"s now and then) and, in busy riffs, ghost notes
    // before a hit: decided per phrase, so a repeated phrase really repeats.
    auto decorate = [&] (std::string half)
    {
        bool first = true;
        for (int i = 0; i < (int) half.size(); ++i)
        {
            char& c = half[(size_t) i];
            if (c != 'x')
                continue;
            if (first || (i % 4 == 2 && rng.chance (0.35)))
                c = 'X';
            first = false;
        }

        for (int i = 0; i + 1 < (int) half.size(); ++i)
            if (half[(size_t) i] == '.' && isHit (half[(size_t) i + 1]) && rng.chance (0.22 * density))
                half[(size_t) i] = 'g';

        return half;
    };

    const auto motif = decorate (makeHalf());      // A
    auto answer = makeHalf();                       // B
    if (answer == motif)
        answer = std::string (answer.rbegin(), answer.rend());
    answer = decorate (answer);

    // House piano keeps one voicing per chord, so riffs don't move by default
    // (the Movement control adds it when wanted).

    // B': the answer with a new last beat.
    const std::string ending = answer.substr (0, 4) + decorate (pickCell());

    std::string right = motif + answer + motif + ending;

    // The "a" of beat 4 pushes the next chord in (the classic house anticipation).
    const bool pushBar2 = rng.chance (0.55 + 0.25 * density);
    const bool pushBar1 = rng.chance (0.25 * density);
    if (pushBar2) { right[30] = '>'; right[31] = '='; }
    if (pushBar1) { right[14] = '>'; right[15] = '='; }

    if (countIn (right) < 6)
        for (int i = 2; i < 32 && countIn (right) < 6; i += 8)
            if (right[(size_t) i] == '.') right[(size_t) i] = 'x';

    // ---- bass: one simple figure, the same in both bars ----
    static const char* const bassFigures[] {
        "x=......x.......",   // 1 and 3
        "x=.......x......",   // 1 and the "e" of 3
        "x.....x.........",   // 1 and the "&" of 2
        "x==.......x.....",   // 1 held, pickup on the "&" of 3
        "x.......x.....x.",   // 1, 3 and the "&" of 4
        "x=....x=..x.....",   // syncopated house bass
        "x==.....x==.....",   // half notes
        "x...........x...",   // 1 and 4
    };
    const std::string bassBar = bassFigures[rng.below ((int) (sizeof (bassFigures) / sizeof (*bassFigures)))];
    std::string left = bassBar + bassBar;

    // The bass follows a push into bar 1 (or lands with it).
    if (pushBar2 && rng.chance (0.6)) { left[30] = '>'; left[31] = '='; left[0] = 'x'; }
    if (pushBar1 && rng.chance (0.6)) { left[14] = '>'; left[15] = '='; left[16] = 'x'; }

    repairHolds (right);
    repairHolds (left);

    GroovePattern p;
    p.name = "Random Riff #" + std::to_string (seed % 100000);
    p.rightHand = right;
    p.leftHand = left;
    return p;
}

GroovePattern morphPatterns (const GroovePattern& a, const GroovePattern& b, double amount, std::uint32_t seed)
{
    GroovePattern out = a;
    const int len = a.length();
    if (len == 0 || b.length() == 0)
        return out;
    amount = std::clamp (amount, 0.0, 1.0);

    // Each step switches at its own threshold; the downbeats of each beat follow the target a
    // little earlier, so the blend keeps a clear pulse.
    auto threshold = [seed] (int step, int salt)
    {
        auto x = (std::uint64_t) (step + 1) * 0x9E3779B97F4A7C15ULL ^ ((std::uint64_t) (seed + (std::uint32_t) salt) * 0xBF58476D1CE4E5B9ULL);
        x ^= x >> 31; x *= 0x94D049BB133111EBULL; x ^= x >> 29;
        const double r = (double) (x >> 11) * (1.0 / 9007199254740992.0);
        return step % 4 == 0 ? r * 0.8 : 0.1 + r * 0.9;
    };

    for (int i = 0; i < len; ++i)
    {
        const int j = i % b.length();
        if (amount >= 1.0 || threshold (i, 1) < amount) out.rightHand[(size_t) i] = b.rightHand[(size_t) j];
        if (amount >= 1.0 || threshold (i, 2) < amount) out.leftHand[(size_t) i]  = b.leftHand[(size_t) j];
    }
    repairHolds (out.rightHand);
    repairHolds (out.leftHand);
    if (amount > 0.0)
        out.name = a.name + " > " + b.name;
    return out;
}

const char* intentionName (int i) noexcept
{
    static const char* const names[numIntentions] { "More soulful", "Funkier", "More minimal", "More energy", "More sophisticated", "Straighter" };
    return names[std::clamp (i, 0, numIntentions - 1)];
}

GroovePattern applyIntention (const GroovePattern& source, Intention intention, std::uint32_t seed)
{
    GroovePattern p = source;
    auto& rh = p.rightHand;
    auto& lh = p.leftHand;
    const int len = p.length();
    if (len == 0)
        return p;

    auto chance = [seed] (int step, int salt)
    {
        auto x = (std::uint64_t) (step + 1) * 0x9E3779B97F4A7C15ULL ^ ((std::uint64_t) (seed * 31u + (std::uint32_t) salt) * 0xBF58476D1CE4E5B9ULL);
        x ^= x >> 31; x *= 0x94D049BB133111EBULL; x ^= x >> 29;
        return (double) (x >> 11) * (1.0 / 9007199254740992.0);
    };
    auto at = [len] (std::string& line, int i) -> char& { return line[(size_t) (((i % len) + len) % len)]; };
    auto empty = [] (char c) { return c == '.'; };

    switch (intention)
    {
        case Intention::minimal:
        {
            // Fewer notes: no ghosts, weak hits go, the strong ones stay and ring longer.
            for (int i = 0; i < len; ++i)
            {
                char& c = rh[(size_t) i];
                const bool strong = c == 'X' || c == 'U' || c == 'D' || c == '>' || i % 8 == 0;
                if (c == 'g' || (isHit (c) && ! strong && chance (i, 1) < 0.55))
                    c = '.';
            }
            for (int i = 0; i < len; ++i)
                if (isHit (rh[(size_t) i]) && i + 1 < len && empty (rh[(size_t) (i + 1)]) && chance (i, 2) < 0.6)
                    rh[(size_t) (i + 1)] = '=';
            for (int i = 0; i < len; ++i)
                if (isHit (lh[(size_t) i]) && i % 8 != 0 && chance (i, 3) < 0.6)
                    lh[(size_t) i] = '.';
            break;
        }

        case Intention::energetic:
        {
            // More drive: offbeat 8ths filled in, accents on the beat, a pick-up into each bar.
            for (int i = 0; i < len; ++i)
            {
                char& c = rh[(size_t) i];
                if (empty (c) && i % 4 == 2 && chance (i, 4) < 0.6) c = 'x';
                else if (c == 'x' && i % 4 == 0) c = 'X';
                else if (c == '=' && chance (i, 5) < 0.4) c = '.';
            }
            for (int bar = 0; bar < len / 16; ++bar)
                if (empty (at (rh, bar * 16 + 15)) && chance (bar, 6) < 0.7)
                    at (rh, bar * 16 + 15) = 'x';
            for (int i = 0; i < len; ++i)
            {
                char& c = lh[(size_t) i];
                if (empty (c) && i % 4 == 0 && chance (i, 7) < 0.5) c = 'x';
                else if (empty (c) && i % 4 == 2 && chance (i, 8) < 0.3) c = 'x';
            }
            break;
        }

        case Intention::funky:
        {
            // Ghost notes around the hits, some hits anticipated by a 16th, short notes.
            for (int i = 0; i < len; ++i)
            {
                char& c = rh[(size_t) i];
                if (c == '=') { c = '.'; continue; }
                if (empty (c) && i % 2 == 1 && (isHit (at (rh, i - 1)) || isHit (at (rh, i + 1))) && chance (i, 9) < 0.5)
                    c = 'g';
            }
            for (int i = 4; i < len; i += 4)
                if (i % 16 != 0 && (rh[(size_t) i] == 'x') && empty (at (rh, i - 1)) && chance (i, 10) < 0.35)
                {
                    at (rh, i - 1) = 'x';
                    rh[(size_t) i] = '.';
                }
            for (int i = 0; i < len; ++i)
            {
                char& c = lh[(size_t) i];
                if (c == '=') c = '.';
                else if (empty (c) && i % 4 == 3 && chance (i, 11) < 0.3) c = 'x';
            }
            break;
        }

        case Intention::soulful:
        {
            // Smoother: fewer ghosts, chords held, a push into the next bar, a calmer bass.
            for (int i = 0; i < len; ++i)
                if (rh[(size_t) i] == 'g' && chance (i, 12) < 0.7)
                    rh[(size_t) i] = '.';
            for (int i = 0; i < len; ++i)
                if (isHit (rh[(size_t) i]))
                    for (int k = 1; k <= 2 && i + k < len && empty (rh[(size_t) (i + k)])
                                    && (i + k + 1 >= len || ! isHit (rh[(size_t) (i + k + 1)]) || k == 1); ++k)
                        rh[(size_t) (i + k)] = '=';
            for (int bar = 0; bar < len / 16; ++bar)
                if (empty (at (rh, bar * 16 + 14)) && chance (bar, 13) < 0.5)
                {
                    at (rh, bar * 16 + 14) = '>';
                    if (empty (at (rh, bar * 16 + 15))) at (rh, bar * 16 + 15) = '=';
                }
            for (int i = 0; i < len; ++i)
                if (isHit (lh[(size_t) i]) && i % 2 == 1 && chance (i, 14) < 0.6)
                    lh[(size_t) i] = '.';
            break;
        }

        case Intention::straighter:
        {
            // On the grid's strong points: hits on the "e" / "a" 16ths move to the nearest 8th,
            // ghost notes and pushes go, the bar starts on its downbeat.
            for (auto* line : { &rh, &lh })
            {
                auto& l = *line;
                for (int i = 0; i < len; ++i)
                {
                    char& c = l[(size_t) i];
                    if (c == 'g') { c = '.'; continue; }
                    if (c == '>') { c = 'x'; }
                    if (i % 2 == 1 && isHit (c))
                    {
                        const char moved = c;
                        c = '.';
                        if (empty (at (l, i - 1)))      at (l, i - 1) = moved;   // a 16th earlier...
                        else if (empty (at (l, i + 1))) at (l, i + 1) = moved;   // ...or later
                    }
                }
                for (int bar = 0; bar < len / 16; ++bar)
                    if (empty (at (l, bar * 16)) && line == &rh)
                        at (l, bar * 16) = 'X';
            }
            break;
        }

        case Intention::sophisticated:
        {
            // The same rhythm, with the chord moving through its inversions inside the bar.
            for (int i = 0; i < len; ++i)
            {
                char& c = rh[(size_t) i];
                if (c == 'x' && i % 16 != 0 && chance (i, 15) < 0.3)
                    c = chance (i, 16) < 0.5 ? 'u' : 'd';
            }
            break;
        }
    }

    repairHolds (rh);
    repairHolds (lh);
    if (std::none_of (rh.begin(), rh.end(), isHit))
        rh[0] = 'X';
    return p;
}

GroovePattern addGhostNotes (const GroovePattern& source, std::uint32_t seed)
{
    Random rng (seed ^ 0x6057u);
    GroovePattern p = source;
    auto ghost = [&rng] (std::string& line, double chance)
    {
        const int len = (int) line.size();
        for (int i = 1; i < len; ++i)
            if (line[(size_t) i - 1] == '.' && (line[(size_t) i] == 'X' || line[(size_t) i] == 'x') && rng.chance (chance)
                && (i < 2 || line[(size_t) i - 2] != 'g'))
                line[(size_t) i - 1] = 'g';
    };
    ghost (p.rightHand, 0.45);
    ghost (p.leftHand, 0.2);
    return p;
}

GroovePattern makeVariation (const GroovePattern& source, std::uint32_t seed)
{
    Random rng (seed ^ 0x5EEDu);
    GroovePattern p = source;
    auto& r = p.rightHand;
    auto& l = p.leftHand;
    const int len = (int) r.size();

    for (int beat = 0; beat + 4 <= len; beat += 4)
    {
        if (! rng.chance (0.35))
            continue;

        int hits = 0;
        for (int k = 0; k < 4; ++k)
            hits += isHit (r[(size_t) (beat + k)]) ? 1 : 0;

        const int choice = rng.below (3);
        if (choice == 0)
        {
            // Delay one hit by a 16th (more syncopation).
            for (int k = 0; k < 3; ++k)
                if ((r[(size_t) (beat + k)] == 'x' || r[(size_t) (beat + k)] == 'X') && r[(size_t) (beat + k + 1)] == '.')
                {
                    std::swap (r[(size_t) (beat + k)], r[(size_t) (beat + k + 1)]);
                    break;
                }
        }
        else if (choice == 1)
        {
            // A ghost note just before a hit.
            for (int k = 1; k < 4; ++k)
                if (r[(size_t) (beat + k - 1)] == '.' && isHit (r[(size_t) (beat + k)]))
                {
                    r[(size_t) (beat + k - 1)] = 'g';
                    break;
                }
        }
        else if (hits >= 2)
        {
            // Leave a gap: drop a non-accented hit.
            for (int k = 3; k >= 0; --k)
                if (r[(size_t) (beat + k)] == 'x')
                {
                    r[(size_t) (beat + k)] = '.';
                    break;
                }
        }
    }

    // Push into the next bar at the end of each bar, and a bass pick-up.
    for (int bar = 0; bar + 16 <= len; bar += 16)
    {
        if (rng.chance (0.6) && r[(size_t) (bar + 14)] != '=' && r[(size_t) (bar + 15)] != '>')
        {
            r[(size_t) (bar + 14)] = '>';
            r[(size_t) (bar + 15)] = '=';
        }
        if (rng.chance (0.5) && l[(size_t) (bar + 14)] == '.' && l[(size_t) (bar + 13)] != '=')
            l[(size_t) (bar + 14)] = 'x';
    }

    repairHolds (r);
    repairHolds (l);
    p.name = source.name + " (variation)";
    return p;
}

GroovePattern addVoicingMovement (const GroovePattern& source, int level)
{
    if (level <= 0)
        return source;

    GroovePattern p = source;
    auto& r = p.rightHand;

    for (int half = 0; half * 8 < (int) r.size(); ++half)
    {
        const bool answer = half % 2 == 1;
        if (level == 1 && ! answer)
            continue;

        // Already moving? Leave the phrase as written.
        bool written = false;
        for (int k = half * 8; k < half * 8 + 8 && k < (int) r.size(); ++k)
            written = written || r[(size_t) k] == 'u' || r[(size_t) k] == 'd' || r[(size_t) k] == 'U' || r[(size_t) k] == 'D';
        if (written)
            continue;

        int hit = 0;
        for (int k = half * 8; k < half * 8 + 8 && k < (int) r.size(); ++k)
        {
            char& c = r[(size_t) k];
            if (c != 'x' && c != 'X')
                continue;

            // Subtle: the 2nd and 4th hits of an answer go up. Lively: base, up, base, down,
            // mirrored in the answer (down first) for a call-and-response contour.
            int move = 0;
            if (level == 1)
                move = (hit % 2 == 1) ? 1 : 0;
            else
            {
                static constexpr int contour[4] { 0, 1, 0, -1 };
                move = contour[hit % 4] * (answer ? -1 : 1);
            }

            if (move > 0) c = (c == 'X') ? 'U' : 'u';
            if (move < 0) c = (c == 'X') ? 'D' : 'd';
            ++hit;
        }
    }

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

std::vector<int> randomProgression (std::uint32_t seed, int length, int scale)
{
    if (! scaleIsMinor (scale))
        return randomProgression (seed, length);

    Random rng (seed ^ 0x5A5A5A5Au);
    length = std::clamp (length, 1, maxProgressionLength);

    // Minor moves: i iv bVII bVI, the Dorian IV, v to i, bIII as a colour.
    static const std::array<std::vector<int>, 7> moves {{
        { 3, 6, 5, 3, 2, 6 },   // i    -> iv bVII bVI iv bIII bVII
        { 4, 6, 0 },            // ii   -> v bVII i
        { 6, 3, 5 },            // bIII -> bVII iv bVI
        { 6, 0, 4, 2, 6 },      // iv   -> bVII i v bIII bVII
        { 0, 5, 3 },            // v    -> i bVI iv
        { 6, 3, 4, 2, 6 },      // bVI  -> bVII iv v bIII bVII
        { 0, 2, 5, 3, 0 },      // bVII -> i bIII bVI iv i
    }};
    static const std::array<int, 5> starts { 0, 0, 0, 5, 3 };

    std::vector<int> out;
    out.push_back (starts[(size_t) rng.below ((int) starts.size())]);
    while ((int) out.size() < length)
    {
        const auto& options = moves[(size_t) out.back()];
        int next = options[(size_t) rng.below ((int) options.size())];
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
