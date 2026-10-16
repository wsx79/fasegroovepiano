// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "HarmonyAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <map>

namespace sgk
{
namespace
{
    int pcOf (int n) noexcept { return ((n % 12) + 12) % 12; }

    struct WindowChord { int root = -1; int kind = 0; double fit = 0.0; };

    /** The best major / minor / dominant chord for a chroma (weights per pitch class). */
    WindowChord bestChord (const std::array<double, 12>& w, int bassPc)
    {
        double total = 0.0;
        for (double x : w) total += x;
        if (total <= 1.0e-9)
            return {};

        // Chord tones count; colour tones a pianist adds (9ths, 6ths, maj7 / m7, 11ths) are free;
        // anything else counts against the chord.
        struct Kind { int tones[4]; double weights[4]; int numTones; int colours[3]; };
        static constexpr Kind kinds[3] {
            { { 0, 4, 7, 0 },  { 1.0, 1.0, 0.7, 0.0 }, 3, { 2, 9, 11 } },   // major (add9, 6, maj7)
            { { 0, 3, 7, 0 },  { 1.0, 1.0, 0.7, 0.0 }, 3, { 2, 10, 5 } },   // minor (9, m7, 11)
            { { 0, 4, 7, 10 }, { 1.0, 1.0, 0.6, 0.9 }, 4, { 2, 9, 2 } },    // dominant 7 (9, 13)
        };

        WindowChord best;
        double bestScore = -1.0e9;
        for (int root = 0; root < 12; ++root)
            for (int k = 0; k < 3; ++k)
            {
                const auto& kind = kinds[k];
                bool used[12] {};
                double score = 0.0;
                for (int t = 0; t < kind.numTones; ++t)
                {
                    const int pc = (root + kind.tones[t]) % 12;
                    used[pc] = true;
                    score += kind.weights[t] * w[(size_t) pc];
                }
                for (int c : kind.colours)
                    used[(root + c) % 12] = true;
                if (k == 0 && w[(size_t) ((root + 4) % 12)] <= 1.0e-9)
                    used[(root + 5) % 12] = used[(root + 2) % 12] = true;   // sus4 / sus2: a major chord, suspended

                for (int pc = 0; pc < 12; ++pc)
                    if (! used[pc])
                        score -= 0.6 * w[(size_t) pc];

                if (k == 2 && w[(size_t) ((root + 10) % 12)] < 0.35 * w[(size_t) ((root + 4) % 12)])
                    score -= 0.3 * total;   // a dominant needs its b7
                const bool hasThird = w[(size_t) ((root + kind.tones[1]) % 12)] > 1.0e-9;
                const bool hasFifth = w[(size_t) ((root + 7) % 12)] > 1.0e-9;
                if (! hasThird)
                    score -= (k == 1 ? 0.55 : 0.5) * total;   // every chord needs its 3rd; without it (sus) read it as major
                if (w[(size_t) root] <= 1.0e-9)
                    score -= 0.4 * total;   // rootless shapes are possible, but rarer
                if (w[(size_t) root] > 1.0e-9 && hasThird && hasFifth)
                    score += 0.1 * total;   // a complete triad beats a half one over the same bass (F/A is F, not Am)
                if (root == bassPc)
                    score += 0.25 * total;  // the bass usually plays the root

                if (score > bestScore)
                {
                    bestScore = score;
                    best = { root, k, 0.0 };
                }
            }

        best.fit = std::clamp (bestScore / (total * 1.35), 0.0, 1.0);
        return best;
    }

    /** Krumhansl-Kessler key profiles: the best of the 24 major and minor keys. */
    void estimateKey (const std::array<double, 12>& w, int& keyRoot, bool& minor)
    {
        static constexpr double majorProfile[12] { 6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88 };
        static constexpr double minorProfile[12] { 6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17 };

        auto correlation = [&w] (const double* profile, int root)
        {
            double mw = 0.0, mp = 0.0;
            for (int i = 0; i < 12; ++i) { mw += w[(size_t) i]; mp += profile[i]; }
            mw /= 12.0; mp /= 12.0;
            double num = 0.0, dw = 0.0, dp = 0.0;
            for (int i = 0; i < 12; ++i)
            {
                const double a = w[(size_t) ((root + i) % 12)] - mw, b = profile[i] - mp;
                num += a * b; dw += a * a; dp += b * b;
            }
            return dw > 0.0 && dp > 0.0 ? num / std::sqrt (dw * dp) : -1.0;
        };

        double best = -2.0;
        for (int root = 0; root < 12; ++root)
            for (int m = 0; m < 2; ++m)
            {
                const double c = correlation (m == 0 ? majorProfile : minorProfile, root);
                if (c > best)
                {
                    best = c;
                    keyRoot = root;
                    minor = m == 1;
                }
            }
    }
}

HarmonyAnalysis analyzeHarmony (const std::vector<AnalyzedNote>& notes, int beatsPerBar)
{
    HarmonyAnalysis result;
    beatsPerBar = std::clamp (beatsPerBar, 2, 12);

    double end = 0.0, start = 1.0e12;
    for (const auto& n : notes)
        if (n.end > n.start)
        {
            end = std::max (end, n.end);
            start = std::min (start, n.start);
        }
    if (notes.empty() || end <= 0.0)
        return result;

    auto weightBetween = [&notes] (double a, double b)
    {
        double weight = 0.0;
        for (const auto& n : notes)
            if (const double overlap = std::min (b, n.end) - std::max (a, n.start); overlap > 0.0)
                weight += std::sqrt (overlap);
        return weight;
    };

    // Start on the first bar line that has notes (a clip may begin with silence), and skip a
    // bar that only holds a pick-up (a few notes leading into the loop).
    double origin = std::floor (std::max (0.0, start) / beatsPerBar) * beatsPerBar;
    {
        std::vector<double> bars;
        for (double t = origin; t < end; t += beatsPerBar)
            bars.push_back (weightBetween (t, t + beatsPerBar));
        auto sorted = bars;
        std::sort (sorted.begin(), sorted.end());
        const double median = sorted.empty() ? 0.0 : sorted[sorted.size() / 2];
        size_t skip = 0;
        while (skip + 1 < bars.size() && bars[skip] < 0.25 * median)
            ++skip;
        origin += (double) skip * beatsPerBar;
    }
    const double window = beatsPerBar / 2.0;   // half a bar
    const int numWindows = std::max (1, (int) std::ceil ((end - origin) / window - 1.0e-6));
    if (numWindows > 512)
        return result;   // far too long for a loop

    // ---- Chroma and bass of every half bar ----
    std::array<double, 12> totalChroma {};
    std::vector<WindowChord> chords ((size_t) numWindows);
    for (int i = 0; i < numWindows; ++i)
    {
        const double a = origin + i * window, b = a + window;
        std::array<double, 12> chroma {};
        int lowest = 128;
        for (const auto& n : notes)
        {
            const double overlap = std::min (b, n.end) - std::max (a, n.start);
            if (overlap <= 0.0)
                continue;
            // Short notes still count (stabs), long ones count more.
            const double weight = std::sqrt (overlap) * (n.start >= a - 1.0e-6 ? 1.0 : 0.7);
            chroma[(size_t) pcOf (n.note)] += weight;
            if (overlap >= 0.2)
                lowest = std::min (lowest, n.note);
        }

        // The bass: among the lowest notes, the one that sounds longest (a pick-up note on the
        // downbeat is not the root of the chord that follows).
        int bassNote = 128;
        double bassTime = 0.0;
        std::array<double, 128> lowTime {};
        for (const auto& n : notes)
        {
            const double overlap = std::min (b, n.end) - std::max (a, n.start);
            if (overlap > 0.0 && n.note >= 0 && n.note < 128 && n.note <= lowest + 7)
                lowTime[(size_t) n.note] += overlap;
        }
        for (int note = 0; note < 128; ++note)
            if (lowTime[(size_t) note] > bassTime + 1.0e-9)
            {
                bassTime = lowTime[(size_t) note];
                bassNote = note;
            }
        for (int pc = 0; pc < 12; ++pc)
            totalChroma[(size_t) pc] += chroma[(size_t) pc];

        // Too little to name a chord (only a held bass or a single note): it goes on with the
        // chord before it, like silence. Two notes (root + 3rd, a common house stab) are enough.
        double windowTotal = 0.0;
        for (double x : chroma) windowTotal += x;
        int distinct = 0;
        for (double x : chroma) distinct += x > 0.04 * windowTotal ? 1 : 0;
        chords[(size_t) i] = distinct >= 2 ? bestChord (chroma, bassNote < 128 ? pcOf (bassNote) : -1) : WindowChord {};
    }

    // Silent half bars keep the chord before them.
    for (size_t i = 1; i < chords.size(); ++i)
        if (chords[i].root < 0)
            chords[i] = chords[i - 1];
    if (chords.front().root < 0)
        for (const auto& c : chords)
            if (c.root >= 0) { chords.front() = c; break; }
    for (size_t i = 1; i < chords.size(); ++i)
        if (chords[i].root < 0)
            chords[i] = chords[i - 1];
    if (chords.front().root < 0)
        return result;

    // ---- Key: from the notes and, above all, from the chords heard (a riff may leave out
    // the notes that tell the key, but its chords don't) ----
    std::array<double, 12> keyChroma = totalChroma;
    double sum = 0.0;
    for (double x : totalChroma) sum += x;
    for (const auto& c : chords)
    {
        static constexpr int third[3] { 4, 3, 4 };
        const double w = sum / (double) chords.size();
        keyChroma[(size_t) c.root] += 1.5 * w;
        keyChroma[(size_t) ((c.root + third[c.kind]) % 12)] += 1.0 * w;
        keyChroma[(size_t) ((c.root + 7) % 12)] += 1.0 * w;
        if (c.kind == 2) keyChroma[(size_t) ((c.root + 10) % 12)] += 0.5 * w;
    }
    bool minor = false;
    estimateKey (keyChroma, result.keyRoot, minor);

    // Major or minor is decided by the chord on the tonic when the loop plays it (a Dorian vamp
    // has no b6 and can look "major" to the profiles).
    int tonicMinor = 0, tonicMajor = 0;
    for (const auto& c : chords)
        if (c.root == result.keyRoot)
            (c.kind == 1 ? tonicMinor : tonicMajor) += 1;
    if (tonicMinor > tonicMajor) minor = true;
    else if (tonicMajor > tonicMinor) minor = false;
    result.scale = 0;
    if (minor)
    {
        // Minor: Dorian when the major 6th is clearly there (very common in house), else Aeolian.
        const double major6 = keyChroma[(size_t) ((result.keyRoot + 9) % 12)];
        const double minor6 = keyChroma[(size_t) ((result.keyRoot + 8) % 12)];
        result.scale = major6 > 1.5 * minor6 + 1.0e-9 ? 1 : 2;
    }

    // ---- Harmonic rhythm: how many half bars a chord usually lasts ----
    auto same = [] (const WindowChord& x, const WindowChord& y) { return x.root == y.root && x.kind == y.kind; };
    std::vector<int> runs;
    for (size_t i = 0; i < chords.size();)
    {
        size_t j = i;
        while (j < chords.size() && same (chords[j], chords[i])) ++j;
        runs.push_back ((int) (j - i));
        i = j;
    }
    // The unit: the largest of 1, 2, 4, 8 half bars that most runs are a multiple of.
    int unit = 1;
    for (int candidate : { 8, 4, 2 })
    {
        int fits = 0;
        for (int r : runs) fits += r % candidate == 0 ? 1 : 0;
        if (fits * 10 >= (int) runs.size() * 8 && candidate <= numWindows)
        {
            unit = candidate;
            break;
        }
    }
    result.chordLengthChoice = unit == 1 ? 0 : unit == 2 ? 1 : unit == 4 ? 2 : 3;

    // One chord per unit (the one heard most in it).
    std::vector<WindowChord> sequence;
    for (int i = 0; i < numWindows; i += unit)
    {
        std::map<int, int> votes;
        WindowChord pick = chords[(size_t) i];
        int bestVotes = 0;
        for (int k = i; k < std::min (numWindows, i + unit); ++k)
        {
            const int key = chords[(size_t) k].root * 3 + chords[(size_t) k].kind;
            if (++votes[key] > bestVotes) { bestVotes = votes[key]; pick = chords[(size_t) k]; }
        }
        sequence.push_back (pick);
    }

    // ---- The loop: the shortest period that repeats (up to 8 chords) ----
    const int m = (int) sequence.size();
    int period = std::min (m, maxProgressionLength);
    for (int p = 1; p <= std::min (m, maxProgressionLength); ++p)
    {
        if (p == m) { period = p; break; }
        int mismatches = 0;
        for (int i = 0; i + p < m; ++i)
            mismatches += same (sequence[(size_t) i], sequence[(size_t) (i + p)]) ? 0 : 1;
        if (mismatches * 10 <= (m - p))   // up to 10% of differences (variations, fills)
        {
            period = p;
            break;
        }
    }

    double fit = 0.0;
    for (int i = 0; i < period; ++i)
    {
        const auto& c = sequence[(size_t) i];
        result.chordCodes.push_back (((c.root - result.keyRoot + 12) % 12) + 12 * c.kind);
        fit += c.fit;
    }
    result.confidence = fit / std::max (1, period);

    ProgressionPreset asPreset { "Imported", result.scale, {}, "", result.chordCodes };
    result.slots = presetSlots (asPreset);

    // ---- Notes outside the scale ----
    const auto& intervals = scaleIntervals (result.scale);
    double total = 0.0, outside = 0.0;
    for (int pc = 0; pc < 12; ++pc)
    {
        total += totalChroma[(size_t) pc];
        const bool inScale = std::find (intervals.begin(), intervals.end(), (pc - result.keyRoot + 12) % 12) != intervals.end();
        if (! inScale)
            outside += totalChroma[(size_t) pc];
    }
    result.outOfScale = total > 0.0 ? outside / total : 0.0;
    for (int pc = 0; pc < 12; ++pc)
    {
        const bool inScale = std::find (intervals.begin(), intervals.end(), (pc - result.keyRoot + 12) % 12) != intervals.end();
        result.outsideNotes[(size_t) pc] = ! inScale && total > 0.0 && totalChroma[(size_t) pc] / total > 0.03;
    }

    result.ok = true;
    return result;
}

//==============================================================================
GroovePattern grooveFromNotes (const std::vector<AnalyzedNote>& notes, int beatsPerBar)
{
    constexpr int steps = 32;                      // two bars of 16ths
    if (beatsPerBar != 4 || notes.empty())
        return {};

    // Onsets on the 16th grid; within each onset (notes starting together) the bass is a low
    // note well under the others (or anything very low).
    struct Onset { int step; bool bass; int velocity; double length; };
    std::vector<Onset> onsets;
    {
        auto sorted = notes;
        std::sort (sorted.begin(), sorted.end(), [] (const AnalyzedNote& a, const AnalyzedNote& b)
                   { return a.start != b.start ? a.start < b.start : a.note < b.note; });
        for (size_t i = 0; i < sorted.size();)
        {
            size_t j = i;
            while (j < sorted.size() && sorted[j].start - sorted[i].start < 0.06)   // a chord (even rolled a little)
                ++j;
            const int step = (int) std::lround (sorted[i].start * 4.0);
            const bool lowAlone = j - i >= 2 && sorted[i].note < 52 && sorted[i + 1].note - sorted[i].note >= 5;
            for (size_t k = i; k < j; ++k)
            {
                const bool bass = sorted[k].note < 45 || (k == i && lowAlone) || (j - i == 1 && sorted[k].note < 50);
                onsets.push_back ({ step, bass, sorted[k].velocity, sorted[k].end - sorted[k].start });
            }
            i = j;
        }
    }
    if (onsets.empty())
        return {};

    // Fold the part into two-bar rounds and keep what plays in at least half of them.
    const int lastStep = std::max_element (onsets.begin(), onsets.end(), [] (const Onset& a, const Onset& b) { return a.step < b.step; })->step;
    const int firstRound = std::min_element (onsets.begin(), onsets.end(), [] (const Onset& a, const Onset& b) { return a.step < b.step; })->step / steps;
    const int rounds = std::max (1, lastStep / steps - firstRound + 1);
    const bool oneBar = lastStep - firstRound * steps < 16;   // a one-bar loop: both bars the same

    struct Cell { std::vector<int> roundsSeen; double velocity = 0.0; double length = 0.0; int count = 0; };
    std::array<Cell, steps> chordCells, bassCells;
    int maxVelocity = 1;
    for (const auto& o : onsets)
    {
        const int local = o.step - firstRound * steps;
        if (local < 0)
            continue;
        const int round = local / steps;
        const int at = local % steps;
        auto& cell = (o.bass ? bassCells : chordCells)[(size_t) at];
        if (std::find (cell.roundsSeen.begin(), cell.roundsSeen.end(), round) == cell.roundsSeen.end())
            cell.roundsSeen.push_back (round);
        cell.velocity += o.velocity;
        cell.length += o.length;
        ++cell.count;
        if (! o.bass) maxVelocity = std::max (maxVelocity, o.velocity);
    }

    auto line = [&] (std::array<Cell, steps>& cells, int peak)
    {
        std::string out (steps, '.');
        for (int i = 0; i < steps; ++i)
        {
            const auto& c = cells[(size_t) (oneBar ? i % 16 : i)];
            if (c.count == 0 || (int) c.roundsSeen.size() * 2 < rounds)
                continue;
            const double velocity = c.velocity / c.count;
            out[(size_t) i] = velocity < 0.55 * peak ? 'g' : velocity >= 0.9 * peak ? 'X' : 'x';
        }
        // Holds: a note that rings on to the next steps (until the next hit).
        for (int i = 0; i < steps; ++i)
        {
            const auto& c = cells[(size_t) (oneBar ? i % 16 : i)];
            if (out[(size_t) i] == '.' || out[(size_t) i] == 'g' || c.count == 0)
                continue;
            const int length = (int) std::lround (c.length / c.count * 4.0);
            for (int k = 1; k < length && i + k < steps && out[(size_t) (i + k)] == '.'; ++k)
                out[(size_t) (i + k)] = '=';
        }
        return out;
    };

    int bassPeak = 1;
    for (const auto& o : onsets)
        if (o.bass) bassPeak = std::max (bassPeak, o.velocity);

    GroovePattern p;
    p.name = "From MIDI";
    p.rightHand = line (chordCells, maxVelocity);
    p.leftHand = line (bassCells, bassPeak);
    if (std::count_if (p.rightHand.begin(), p.rightHand.end(), [] (char c) { return c == 'x' || c == 'X'; }) < 3)
        return {};
    return p;
}

} // namespace sgk
