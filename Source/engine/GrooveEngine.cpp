#include "GrooveEngine.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace sgk
{

namespace
{
    long long floorDiv (long long a, long long b) noexcept
    {
        long long q = a / b;
        if ((a % b != 0) && ((a < 0) != (b < 0))) --q;
        return q;
    }

    int mod (long long a, long long m) noexcept { return (int) (((a % m) + m) % m); }

    std::string compact (const char* s)
    {
        std::string out;
        for (; *s != 0; ++s)
            if (*s != ' ')
                out += *s;
        return out;
    }

    /** Deterministic 0..1 noise, so re-rendering a step always gives the same feel. */
    double noise (long long step, int salt) noexcept
    {
        auto x = (std::uint64_t) (step * 0x9E3779B97F4A7C15LL) ^ ((std::uint64_t) salt * 0xBF58476D1CE4E5B9ULL);
        x ^= x >> 31; x *= 0x94D049BB133111EBULL; x ^= x >> 29;
        return (double) (x >> 11) * (1.0 / 9007199254740992.0);
    }

    int holdLength (const std::string& line, int pos) noexcept
    {
        const int len = (int) line.size();
        int count = 0;
        while (count < len - 1 && line[(size_t) ((pos + 1 + count) % len)] == '=')
            ++count;
        return count;
    }

    bool isHit (char c) noexcept { return c == 'X' || c == 'x' || c == '>'; }

    /** A push ('>') held with '=' right up to a hit: that hit is tied, not re-struck. */
    bool pushTiesIntoNextHit (const std::string& line, int pos) noexcept
    {
        const int len = (int) line.size();
        const char next = line[(size_t) ((pos + 1 + holdLength (line, pos)) % len)];
        return line[(size_t) pos] == '>' && (next == 'X' || next == 'x');
    }

    bool isTiedFromPush (const std::string& line, int pos) noexcept
    {
        const int len = (int) line.size();
        for (int back = 1; back < len; ++back)
        {
            const int prev = ((pos - back) % len + len) % len;
            if (line[(size_t) prev] != '=')
                return line[(size_t) prev] == '>' && pushTiesIntoNextHit (line, prev);
        }
        return false;
    }
}

//==============================================================================
const std::vector<GroovePattern>& groovePatterns()
{
    //                         |1 e & a 2 e & a 3 e & a 4 e & a |1 ...                 bar 2
    static const std::vector<GroovePattern> patterns {
        { "Classic Offbeat Stabs",
          compact ("..X. ..x. ..X. ..x. ..X. ..x. ..X. ..x."),
          compact ("x==. .... x==. .... x==. .... x==. ....") },
        { "Soulful Push",
          compact ("X..x ..x. .x.. ..>= =..x ..x. .x.. ..>="),
          compact ("x=.. ...x ..x. .... ...x ...x ..x. ..>.") },
        { "Deep Chords",
          compact ("X=== ===. ..x= ===. X=== ==.x ..x. x==."),
          compact ("x=== ==== .... ..x. x=== ==== .... ....") },
        { "Funky 16ths",
          compact ("x.xx .x.x .xx. x.x. x.xx .x.x .x.x ..x."),
          compact ("x... ..x. x... .... x... ..x. x..x ....") },
        { "Gospel Bounce",
          compact ("X..x ..X. .x.. X..x ..X. .x.. X.x. .x.."),
          compact ("x..x .... x... ..x. x..x .... x.x. ....") },
        { "Chicago Jack",
          compact ("..x. ..xx ..x. .x.x ..x. ..xx ..x. .xx."),
          compact ("x... x... x... x... x... x... x... x...") },
        { "Garage Skip",
          compact ("x... ..x. ...x ..x. x... ..x. ...x .x.."),
          compact ("x... .... ..x. .... x... .... ..x. ..x.") },
        { "Late Night Rhodes",
          compact ("X==. .x=. ...x =.x. X==. .x=. .x.. >==="),
          compact ("x=== .... ..x. .... x=== .... x... ....") },
        { "Disco Octaves",
          compact ("..x. ..x. ..x. ..x. ..x. ..x. ..x. ..x."),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x.x.") },
        { "Long Chords",
          compact ("X=== ==== ==== ===. X=== ==== ==== ===."),
          compact ("x=== ==== ==== ===. x=== ==== ==== ===.") },
    };

    return patterns;
}

const std::vector<ProgressionPreset>& progressionPresets()
{
    static const std::vector<ProgressionPreset> presets {
        { "Dorian Vamp  i - IV",               1, { 0, 3 } },
        { "Deep Minor  i - v",                 1, { 0, 4 } },
        { "Soulful Descent  IV - iii - ii - I", 0, { 3, 2, 1, 0 } },
        { "Jazzy  ii - V",                     0, { 1, 4 } },
        { "Turnaround  I - vi - ii - V",       0, { 0, 5, 1, 4 } },
        { "Royal Road  IV - V - iii - vi",     0, { 3, 4, 2, 5 } },
        { "Minor Flow  i - VI - III - VII",    2, { 0, 5, 2, 6 } },
        { "Late Night  i - VII - VI - VII",    2, { 0, 6, 5, 6 } },
        { "Gospel Lift  IV - V - vi - I",      0, { 3, 4, 5, 0 } },
        { "Mixo Groove  I - VII - IV - I",     3, { 0, 6, 3, 0 } },
        { "Lydian Float  I - II",              4, { 0, 1 } },
        { "8-Bar Journey",                     0, { 0, 5, 3, 4, 2, 5, 1, 4 } },
    };

    return presets;
}

const char* bassModeName (int m) noexcept
{
    static const char* const names[numBassModes] { "Off", "Root", "Octaves", "Root + 5th" };
    return names[mod (m, numBassModes)];
}

//==============================================================================
GrooveEngine::GrooveEngine()
{
    offTime.fill (-1.0);
    pending.reserve (512);
    scratch.reserve (128);
}

bool GrooveEngine::anyNoteSounding() const noexcept
{
    return std::any_of (offTime.begin(), offTime.end(), [] (double t) { return t >= 0.0; });
}

int GrooveEngine::slotForStep (long long step, const GrooveSettings& s) noexcept
{
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);
    return mod (floorDiv (step, std::max (1, s.stepsPerChord)), len);
}

void GrooveEngine::voiceProgression (const GrooveSettings& s,
                                     std::array<Chord, maxProgressionLength>& chords,
                                     std::array<VoicedChord, maxProgressionLength>& voiced) noexcept
{
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);

    for (int i = 0; i < len; ++i)
        chords[(size_t) i] = buildChord (s.keyRoot, s.scale, s.progression[(size_t) i], s.extension);

    auto pass = [&] (int first, const NoteSet* previous)
    {
        for (int i = first; i < len; ++i)
        {
            voiced[(size_t) i] = voiceChord (chords[(size_t) i], s.voicing, s.centre, previous);
            previous = &voiced[(size_t) i].rightHand;
        }
    };

    pass (0, nullptr);

    // Second pass so the loop point (last -> first chord) is voice-led too.
    if (len > 1)
    {
        const auto last = voiced[(size_t) len - 1].rightHand;
        pass (0, &last);
    }
}

//==============================================================================
void GrooveEngine::collectStep (long long step, const GrooveSettings& s,
                                const std::array<Chord, maxProgressionLength>& chords,
                                const std::array<VoicedChord, maxProgressionLength>& voiced)
{
    const auto& patterns = groovePatterns();
    const auto& pattern = patterns[(size_t) std::clamp (s.pattern, 0, (int) patterns.size() - 1)];
    const int pos = mod (step, pattern.length());

    const double swingDelay = (step % 2 != 0) ? std::clamp (s.swing, 0.0f, 1.0f) * stepLength / 3.0 : 0.0;
    const double baseTime = (double) step * stepLength + swingDelay;
    const double human = std::clamp (s.humanize, 0.0f, 1.0f);

    auto hitTime = [&] (int salt) { return baseTime + human * 0.03 * noise (step, salt); };

    auto hitVelocity = [&] (char symbol, double scale, int salt)
    {
        double v = s.velocity * scale * (symbol == 'X' ? 1.0 : symbol == '>' ? 0.95 : 0.8);
        v += human * 18.0 * (noise (step, salt + 7) - 0.5) * 2.0;
        return std::clamp ((int) std::lround (v), 1, 127);
    };

    auto durationFor = [&] (const std::string& line)
    {
        // A tied push rings slightly past the next hit, which then extends it.
        if (pushTiesIntoNextHit (line, pos))
            return (holdLength (line, pos) + 1) * stepLength + 0.06;

        const double steps = holdLength (line, pos) + std::clamp (s.gate, 0.05f, 1.0f);
        return std::max (0.04, steps * stepLength);
    };

    // Which chord a hit uses: the current one, or the next one for a push.
    auto slotFor = [&] (char symbol)
    {
        if (symbol != '>')
            return slotForStep (step, s);

        const long long grid = std::min (s.stepsPerChord, 16);
        return slotForStep ((floorDiv (step, grid) + 1) * grid, s);
    };

    // Fill: the last beat of every N bars becomes a pentatonic pick-up run.
    const long long bar = floorDiv (step, 16);
    const int stepInBar = mod (step, 16);
    const bool inFill = s.fillEveryBars > 0 && mod (bar, s.fillEveryBars) == s.fillEveryBars - 1 && stepInBar >= 12;

    // ---- right hand (chords) ----
    if (inFill)
    {
        const auto& rh = voiced[(size_t) slotForStep (step, s)].rightHand;
        const int top = rh.size > 0 ? rh.highest() : s.centre + 7;
        const auto run = pentatonicNotes (s.keyRoot, s.scale, s.centre - 5, s.centre + 19);

        if (run.size > 0)
        {
            int iTop = 0;
            for (int i = 0; i < run.size; ++i)
                if (run[i] <= top)
                    iTop = i;

            const int k = stepInBar - 12;
            const int idx = std::clamp (iTop - 2 + k, 0, run.size - 1);
            const double len = (k == 3 ? 2.0 : std::clamp (s.gate, 0.3f, 1.0f)) * stepLength;
            pending.push_back ({ hitTime (31), run[idx], hitVelocity ('x', k == 3 ? 1.0 : 0.85, 32), len });
        }
    }
    else
    {
        const char symbol = pattern.rightHand[(size_t) pos];
        if (isHit (symbol))
        {
            const auto& v = voiced[(size_t) slotFor (symbol)];
            const double len = durationFor (pattern.rightHand);
            const double t = hitTime (1);
            const bool tie = isTiedFromPush (pattern.rightHand, pos);

            for (int i = 0; i < v.rightHand.size; ++i)
                pending.push_back ({ t + 0.004 * human * noise (step, 50 + i), v.rightHand[i],
                                     hitVelocity (symbol, 1.0 - 0.03 * (v.rightHand.size - 1 - i), 60 + i), len, tie });

            for (int i = 0; i < v.leftHand.size; ++i)
                pending.push_back ({ t, v.leftHand[i], hitVelocity (symbol, 0.85, 80 + i), len, tie });
        }
    }

    // ---- left hand (bass) ----
    if (s.bass != BassMode::off)
    {
        const char symbol = pattern.leftHand[(size_t) pos];
        if (isHit (symbol))
        {
            const int root = 36 + chords[(size_t) slotFor (symbol)].rootPc;
            const double len = durationFor (pattern.leftHand);
            const double t = hitTime (90);
            const int vel = hitVelocity (symbol, 0.9, 91);
            const bool tie = isTiedFromPush (pattern.leftHand, pos);

            pending.push_back ({ t, root, vel, len, tie });
            if (s.bass == BassMode::octaves)    pending.push_back ({ t, root + 12, (int) (vel * 0.85), len, tie });
            if (s.bass == BassMode::rootFifth)  pending.push_back ({ t, root + 7,  (int) (vel * 0.85), len, tie });
        }
    }
}

void GrooveEngine::flushOffsUpTo (double ppq, bool inclusive, std::vector<NoteEvent>& out)
{
    scratch.clear();
    for (int n = 0; n < 128; ++n)
    {
        const double t = offTime[(size_t) n];
        if (t >= 0.0 && (t < ppq || (inclusive && t <= ppq)))
            scratch.push_back (n);
    }

    std::sort (scratch.begin(), scratch.end(),
               [this] (int a, int b) { return offTime[(size_t) a] < offTime[(size_t) b]; });

    for (int n : scratch)
    {
        out.push_back ({ offTime[(size_t) n], n, 0 });
        offTime[(size_t) n] = -1.0;
    }
}

void GrooveEngine::render (double startPpq, double endPpq, const GrooveSettings& s,
                           std::vector<NoteEvent>& out)
{
    if (endPpq <= startPpq)
        return;

    std::array<Chord, maxProgressionLength> chords;
    std::array<VoicedChord, maxProgressionLength> voiced;
    voiceProgression (s, chords, voiced);

    pending.clear();

    // Swing and humanize only ever delay a hit (by < 2 steps), so look back a little.
    const auto firstStep = (long long) std::floor (startPpq / stepLength) - 2;
    const auto lastStep  = (long long) std::floor (endPpq / stepLength);

    for (auto step = firstStep; step <= lastStep; ++step)
        collectStep (step, s, chords, voiced);

    pending.erase (std::remove_if (pending.begin(), pending.end(),
                                   [&] (const PendingOn& p) { return p.ppq < startPpq || p.ppq >= endPpq
                                                                   || p.note < 0 || p.note > 127; }),
                   pending.end());

    std::stable_sort (pending.begin(), pending.end(),
                      [] (const PendingOn& a, const PendingOn& b) { return a.ppq < b.ppq; });

    std::array<double, 128> onTime;
    onTime.fill (-1.0);

    for (const auto& p : pending)
    {
        flushOffsUpTo (p.ppq, true, out);

        auto& off = offTime[(size_t) p.note];

        if (off >= 0.0)
        {
            if (p.tie || onTime[(size_t) p.note] == p.ppq)   // tied, or same note twice at once: extend
            {
                off = std::max (off, p.ppq + p.duration);
                continue;
            }

            out.push_back ({ p.ppq, p.note, 0 });  // retrigger
        }

        out.push_back ({ p.ppq, p.note, p.velocity });
        off = p.ppq + p.duration;
        onTime[(size_t) p.note] = p.ppq;
    }

    flushOffsUpTo (endPpq, false, out);
}

void GrooveEngine::allNotesOff (double ppq, std::vector<NoteEvent>& out)
{
    for (int n = 0; n < 128; ++n)
    {
        if (offTime[(size_t) n] >= 0.0)
        {
            out.push_back ({ ppq, n, 0 });
            offTime[(size_t) n] = -1.0;
        }
    }
}

} // namespace sgk
