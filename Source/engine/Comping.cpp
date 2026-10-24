// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Comping.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>

namespace sgk
{
namespace
{
    // To the nearest integer, a half to the even one (as the plug-in rounded these before).
    int roundToInt (double x) noexcept { return (int) std::nearbyint (x); }
}

int Comping::compingWindows (std::uint32_t seed, std::array<CompingWindow, 8>& out) noexcept
{
    // The piano's answers over four bars, from the seed (another groove, NEW RHYTHM: other
    // answers). Every bar answers, the second one now and then stays quiet: one or two windows a
    // bar, on any 8th or 16th, a little apart. In about a third of them the piano plays its own
    // groove; in the others an answer of its own. The last bar often pushes into the next phrase.
    static constexpr double starts[] { 0.0, 0.25, 0.5, 0.75, 1.0, 1.5, 1.75, 2.0, 2.5, 2.75, 3.0, 3.25, 3.5 };
    std::uint64_t r = (std::uint64_t) seed * 0x9E3779B97F4A7C15ull + 0x2545F4914F6CDD1Dull;
    auto next = [&r]
    {
        // splitmix64: well spread from one seed to the next
        r += 0x9E3779B97F4A7C15ull;
        auto z = r;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        return (std::uint32_t) ((z ^ (z >> 31)) >> 16);
    };

    int count = 0;
    for (int b = 0; b < 4; ++b)
    {
        int here = 1 + (int) (next() % 2);              // one or two
        if (b == 1 && next() % 3 == 0)
            here = 0;                                   // the second bar: room for the guitar
        double previous = -2.0;
        for (int k = 0; k < here && count < (int) out.size(); ++k)
        {
            double start = starts[next() % std::size (starts)];
            if (b == 3 && k == here - 1 && next() % 2 == 0)
                start = 3.0 + 0.25 * (double) (next() % 3);   // a push into the next phrase
            if (k > 0 && start < previous + 1.0)
                start = previous + 1.0 + 0.5 * (double) (next() % 2);
            if (start > 3.5)
                break;
            previous = start;
            CompingWindow w;
            w.start = b * 4.0 + start;
            w.figure = (next() % 3 == 0) ? -1 : (int) (next() % numCompingFigures);
            out[(size_t) count++] = w;
        }
    }
    return count;
}

const Comping::CompingFigure& Comping::compingFigure (int figure) noexcept
{
    // Hits inside a window: start, length (beats), loudness.
    using H = CompingHit;
    static const CompingFigure figures[numCompingFigures]
    {
        { 1, {{ H { 0.0, 0.25, 1.0f } }} },                                              // a stab
        { 2, {{ H { 0.0, 0.2, 0.95f }, H { 0.5, 0.2, 0.8f } }} },                        // two short hits
        { 2, {{ H { 0.25, 0.2, 0.7f }, H { 0.5, 0.45, 1.0f } }} },                       // a push into a hit
        { 1, {{ H { 0.0, 0.9, 0.85f } }} },                                              // a held chord
        { 2, {{ H { 0.5, 0.2, 0.9f }, H { 0.75, 0.2, 0.7f } }} },                        // an off-beat pair
        { 3, {{ H { 0.0, 0.15, 0.9f }, H { 0.25, 0.15, 0.65f }, H { 0.5, 0.3, 0.85f } }} }, // three 16ths
        { 1, {{ H { 0.75, 0.6, 1.0f } }} },                                              // an anticipation, held over
        { 2, {{ H { 0.0, 0.2, 0.6f }, H { 0.5, 0.3, 1.0f } }} },                         // a ghost, then the hit
    };
    return figures[std::clamp (figure, 0, numCompingFigures - 1)];
}

bool Comping::compingKeeps (double ppq, std::uint32_t seed) noexcept
{
    // Only inside the windows where the piano plays its own groove (the others have their answer).
    std::array<CompingWindow, 8> windows;
    const int n = compingWindows (seed, windows);
    const double inCycle = ppq - std::floor (ppq / 16.0) * 16.0;
    for (int i = 0; i < n; ++i)
        if (windows[(size_t) i].figure < 0 && inCycle >= windows[(size_t) i].start && inCycle < windows[(size_t) i].start + 1.0)
            return true;
    return false;
}

void Comping::compingAnswers (double from, double to, std::uint32_t seed, const GrooveSettings& s,
                              std::vector<NoteEvent>& out)
{
    std::array<Chord, maxProgressionLength> chords;
    std::array<VoicedChord, maxProgressionLength> voiced;
    GrooveEngine::voiceProgression (s, chords, voiced);
    compingAnswers (from, to, seed, s, voiced, out);
}

int Comping::compingHits (std::uint32_t seed, std::array<CompingSpan, 24>& out) noexcept
{
    // Every hit of the answers in the four-bar cycle, in time order; a hit never rings into the
    // next one (two answers close together would strike the same notes twice at once).
    std::array<CompingWindow, 8> windows;
    const int n = compingWindows (seed, windows);
    int count = 0;
    for (int i = 0; i < n; ++i)
    {
        if (windows[(size_t) i].figure < 0)
            continue;
        const auto& f = compingFigure (windows[(size_t) i].figure);
        for (int h = 0; h < f.count && count < (int) out.size(); ++h)
        {
            const auto& hit = f.hits[(size_t) h];
            out[(size_t) count++] = { windows[(size_t) i].start + hit.offset, windows[(size_t) i].start + hit.offset + hit.length,
                                      hit.level, i * 4 + h };
        }
    }
    std::sort (out.begin(), out.begin() + count, [] (const CompingSpan& a, const CompingSpan& b) { return a.on < b.on; });
    for (int i = 0; i < count; ++i)
    {
        const double nextOn = i + 1 < count ? out[(size_t) i + 1].on : out[0].on + 16.0;
        out[(size_t) i].off = std::max (out[(size_t) i].on + 0.06, std::min (out[(size_t) i].off, nextOn - 0.02));
    }
    return count;
}

void Comping::compingAnswers (double from, double to, std::uint32_t seed, const GrooveSettings& s,
                              const std::array<VoicedChord, maxProgressionLength>& voiced,
                              std::vector<NoteEvent>& out)
{
    // The answers' notes whose start or end falls in [from, to): the chord of the moment, as the
    // progression voices it for the right hand, played like a pianist: a hit that rings over the
    // next chord change (an anticipation) plays the next chord, the chord rolled a hair upwards,
    // the top note a little louder and the inner ones softer, a touch late or early (Humanize).
    if (to <= from)
        return;
    std::array<CompingSpan, 24> hits;
    const int n = compingHits (seed, hits);
    const double baseVelocity = std::clamp (s.velocity, 30, 127) * sectionVelocityScale (s.section);
    const int spc = std::max (1, s.stepsPerChord);
    auto hash = [] (std::uint64_t x)
    {
        x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ull;
        x = (x ^ (x >> 27)) * 0x94D049BB133111EBull;
        return (double) ((x ^ (x >> 31)) >> 11) * (1.0 / 9007199254740992.0);
    };

    for (double cycle = std::floor ((from - 2.0) / 16.0) * 16.0; cycle < to; cycle += 16.0)
        for (int i = 0; i < n; ++i)
        {
            const auto& hit = hits[(size_t) i];
            const auto salt = (std::uint64_t) (std::llround (cycle / 16.0) * 64 + hit.id) + (std::uint64_t) seed * 977u;
            const double late = std::clamp ((double) s.humanize, 0.0, 1.0) * 0.02 * hash (salt);
            const double on = cycle + hit.on + late;
            const double off = cycle + hit.off;
            if (off < from || on >= to + 0.1)
                continue;

            auto step = (long long) std::floor (on / stepLength + 1.0e-6);
            const double change = (double) ((step >= 0 ? step / spc : (step - spc + 1) / spc) + 1) * spc * stepLength;
            if (change - on <= 0.5 && off > change + 0.05)
                step = (long long) std::llround (change / stepLength);   // the anticipation plays the next chord
            const int slot = std::clamp (GrooveEngine::slotForStep (step, s), 0, maxProgressionLength - 1);
            const auto& notes = voiced[(size_t) slot].rightHand;
            for (int k = 0; k < notes.size; ++k)
            {
                const double noteOn = on + 0.008 * k;
                if (noteOn >= off - 0.02)
                    continue;
                const double level = k == notes.size - 1 ? 1.08 : k > 0 ? 0.95 : 1.0;
                const int velocity = std::clamp (roundToInt (baseVelocity * hit.level * level
                                                             * (1.0 + 0.12 * s.humanize * (hash (salt + 11u * (std::uint64_t) k) - 0.5))), 1, 127);
                if (noteOn >= from && noteOn < to)
                    out.push_back ({ noteOn, notes[k], velocity, Part::chords });
                if (off >= from && off < to)
                    out.push_back ({ off, notes[k], 0, Part::chords });
            }
        }
}

std::vector<NoteEvent> Comping::comping (const std::vector<NoteEvent>& events, std::uint32_t seed,
                                       const GrooveSettings& settings, double length)
{
    std::vector<NoteEvent> out;
    out.reserve (events.size());
    std::map<int, std::vector<bool>> kept;   // note + part -> the notes struck, kept or not, in order
    for (const auto& e : events)
    {
        if (e.part != Part::chords && e.part != Part::fill)
        {
            out.push_back (e);
            continue;
        }
        auto& queue = kept[e.note + 128 * (int) e.part];
        if (e.isNoteOn())
        {
            const bool keep = compingKeeps (e.ppq, seed);
            queue.push_back (keep);
            if (keep)
                out.push_back (e);
        }
        else
        {
            const bool keep = queue.empty() || queue.front();   // an off with no on stays (harmless)
            if (! queue.empty())
                queue.erase (queue.begin());
            if (keep)
                out.push_back (e);
        }
    }
    // The piano's own answers: only those struck in the clip, each one closed (at the clip's end at
    // the latest, so none hangs), then everything in time order (note-offs first at the same moment).
    std::vector<NoteEvent> answers;
    compingAnswers (0.0, length + 2.0, seed, settings, answers);
    std::stable_sort (answers.begin(), answers.end(), [] (const NoteEvent& a, const NoteEvent& b)
                      { return a.ppq != b.ppq ? a.ppq < b.ppq : (! a.isNoteOn() && b.isNoteOn()); });
    std::array<bool, 128> sounding {};
    for (auto e : answers)
    {
        auto& on = sounding[(size_t) std::clamp (e.note, 0, 127)];
        if (e.isNoteOn())
        {
            if (e.ppq >= length || on)
                continue;
            on = true;
        }
        else
        {
            if (! on)
                continue;
            on = false;
            e.ppq = std::min (e.ppq, length);
        }
        out.push_back (e);
    }
    std::stable_sort (out.begin(), out.end(), [] (const NoteEvent& a, const NoteEvent& b)
                      { return a.ppq != b.ppq ? a.ppq < b.ppq : (! a.isNoteOn() && b.isNoteOn()); });

    // An answer and a chord the piano keeps can meet on one pitch (both on channel 1): the note
    // already sounding ends where the new one is struck, and its own note-off is left out.
    std::vector<NoteEvent> clean;
    clean.reserve (out.size() + 16);
    std::array<int, 128> held {}, staleOffs {};
    std::array<Part, 128> soundingPart {};
    for (const auto& e : out)
    {
        if (e.part != Part::chords && e.part != Part::fill)
        {
            clean.push_back (e);
            continue;
        }
        const auto n = (size_t) std::clamp (e.note, 0, 127);
        if (e.isNoteOn())
        {
            if (held[n] > 0)
            {
                clean.push_back ({ e.ppq, e.note, 0, soundingPart[n] });
                ++staleOffs[n];
            }
            held[n] = 1;
            soundingPart[n] = e.part;
            clean.push_back (e);
        }
        else if (staleOffs[n] > 0)
            --staleOffs[n];
        else if (held[n] > 0)
        {
            held[n] = 0;
            clean.push_back ({ e.ppq, e.note, 0, soundingPart[n] });
        }
    }
    for (int n = 0; n < 128; ++n)
        if (held[(size_t) n] > 0)
            clean.push_back ({ length, n, 0, soundingPart[(size_t) n] });
    return clean;
}

GroovePattern Comping::compingPattern (const GroovePattern& source, std::uint32_t seed)
{
    // Four bars (the comping cycle): the right hand where the piano still plays its groove, its
    // own answers drawn in their windows (X a strong hit, x a softer one, = held), the left hand
    // as it is.
    const int len = source.length();
    if (len == 0)
        return source;
    constexpr int cycle = 64;
    GroovePattern out = source;
    out.rightHand.assign ((size_t) cycle, '.');
    out.leftHand.assign ((size_t) cycle, '.');
    bool keeping = false;
    for (int i = 0; i < cycle; ++i)
    {
        const char c = source.rightHand[(size_t) (i % len)];
        if (c == '.')
            keeping = false;
        else if (c != '=')   // a hit (a push too): kept in the groove's windows only
            keeping = compingKeeps (i * stepLength, seed);
        out.rightHand[(size_t) i] = keeping ? c : '.';   // (a held note follows its hit)
        if ((int) source.leftHand.size() == len)
            out.leftHand[(size_t) i] = source.leftHand[(size_t) (i % len)];
    }
    std::array<CompingSpan, 24> hits;
    const int n = compingHits (seed, hits);
    for (int w = 0; w < n; ++w)
    {
        {
            const auto& hit = hits[(size_t) w];
            const int first = roundToInt (hit.on / stepLength);
            const int steps = std::max (1, roundToInt ((hit.off - hit.on) / stepLength));
            for (int k = 0; k < steps && first + k < cycle; ++k)
                out.rightHand[(size_t) (first + k)] = k > 0 ? '=' : (hit.level >= 0.9f ? 'X' : 'x');
        }
    }
    return out;
}

} // namespace sgk
