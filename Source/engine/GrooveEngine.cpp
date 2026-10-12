// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

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

    bool isHit (char c) noexcept { return c == 'X' || c == 'x' || c == 'g' || c == '>'; }

    /** A push ('>') held with '=' right up to a hit: that hit is tied, not re-struck. */
    bool pushTiesIntoNextHit (const std::string& line, int pos) noexcept
    {
        const int len = (int) line.size();
        const char next = line[(size_t) ((pos + 1 + holdLength (line, pos)) % len)];
        return line[(size_t) pos] == '>' && (next == 'X' || next == 'x' || next == 'g');
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

        // ---- Funk ----
        { "Funk Clav Chops",
          compact ("x.xx .xx. x.xx .x.x x.xx .xx. x.x. xx.x"),
          compact ("x... ..x. .x.. x... x... ..x. .x.. ..x.") },
        { "Ghost Note Funk",
          compact ("X.gg .gX. g.gX .g.g X.gg .gX. g.X. gXg."),
          compact ("x... ..x. ..x. .x.. x... ..x. x... ..x.") },
        { "Nile Chops",
          compact ("..x. x.xx ..x. x.x. ..x. x.xx .xx. x.x."),
          compact ("x... ...x ..x. .... x... ...x ..x. .x..") },
        { "Anticipation Funk",
          compact ("X.x. .x.. x.x. .x>= =.x. .x.. x.x. .x>="),
          compact ("x..x ..x. x... ..>= =..x ..x. x... ..>=") },
        { "Gospel Shout 16ths",
          compact ("X.xx .xX. xx.x .Xx. X.xx .xX. x.xX .x>="),
          compact ("x..x ..x. .x.. x... x..x ..x. .x.. ..>=") },
        { "Neo Soul Lazy",
          compact ("X==. ..x= .x.. x==. ..X= =.x. .x.. >==="),
          compact ("x... ..x. .... x... x... ..x. ..x. ....") },
        { "Boogie Stabs",
          compact ("x.x. ..x. .x.x ..x. x.x. ..x. .x.x .x.."),
          compact ("x... x.x. x... x.x. x... x.x. x... x..x") },
        { "Offbeat Doubles",
          compact ("..xx ..x. ..xx .x.. ..xx ..x. ..xx .x.x"),
          compact ("x... ...x x... .... x... ...x x..x ....") },
        { "Broken Beat",
          compact ("X... ..x. ...x .x.. ..x. x... .x.. x.x."),
          compact ("x... ...x ..x. .... .x.. ..x. ...x ....") },
        { "Slap Bass Funk",
          compact ("..x. ...x ..x. .x.. ..x. ...x ..xg .x.."),
          compact ("x..x .x.. x.x. ..xx x..x .x.. x.x. x.xx") },
        // ---- Jackin' house (Chicago bounce: offbeat stabs, pumping bass, swing it!) ----
        { "Jackin' Stabs",
          compact ("..X. .x.x ..X. .x.x ..X. .x.x ..X. x.x."),
          compact ("x..x ..x. x..x ..x. x..x ..x. x..x .xx.") },
        { "Jack Bounce",
          compact (".x.x .x.x .x.x .X.x .x.x .x.x .x.x .X.."),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x.xx") },
        { "Jackin' Push",
          compact ("X..x ..X. .x.. x.>= =..x ..X. .x.x ..>="),
          compact ("x..x .x.. x..x ..>= =..x .x.. x..x ..>=") },
        { "Chicago Organ Jack",
          compact ("x.xx ..x. x.xx ..x. x.xx ..x. x.x. xx.x"),
          compact ("x... ..x. x... ..x. x... ..x. x..x ..x.") },
        { "Ghetto Jack",
          compact ("X.gx .gX. gx.X .g.x X.gx .gX. gxX. .gXg"),
          compact ("x..x ..x. ...x ..x. x..x ..x. .x.x ..x.") },
        { "Disco Jack Chops",
          compact ("..xx .x.. ..xx .x.x ..xx .x.. ..x. xx.x"),
          compact ("x.x. ..x. x.x. ..xx x.x. ..x. x.x. .x.x") },

        // ---- v1.2: more soulful, deep, jackin' and funky ----
        { "Soulful Sunday",
          compact ("X==. .x.. x==. .x>= =..x ..X= =.x. x.>="),
          compact ("x... ..x. x... ..>= =..x ..x. x... ..>=") },
        { "Gospel Chops",
          compact ("X.x. xX.x .x.X x.>= =.x. xX.x .x.X x.>="),
          compact ("x..x ..x. x..x ..>= =..x ..x. x..x ..>=") },
        { "Neo Soul Comping",
          compact (".x=. ..x= .X.. x=.. .x=. ..x= .x.. >==="),
          compact ("x... .x.. ..x. .... x... .x.. ..x. ....") },
        { "Deep Sway",
          compact ("X=== ..x= ==.. x=== X=== ..x= ==.. .x=="),
          compact ("x=== .... ..x. .... x=== .... ..x. ....") },
        { "Deep Dub Stabs",
          compact ("..X= ==.. .... ..x. ..X= ==.. .... x..."),
          compact ("x=== .... .... .... x=== .... ..x. ....") },
        { "Jackin' Shuffle",
          compact (".x.x ..x. .x.x .xx. .x.x ..x. .x.x x.x."),
          compact ("x..x x... x..x x.x. x..x x... x..x .xx.") },
        { "Jack Organ Pump",
          compact ("..x. ..x. ..x. ..xx ..x. ..x. ..x. .xx."),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.xx x.x.") },
        { "Funky Upstrokes",
          compact (".x.x .x.x .x.X ..x. .x.x .x.x .X.. x.x."),
          compact ("x..x ..x. x... .x.. x..x ..x. x.x. ....") },
    };

    return patterns;
}

int randomPatternIndex() noexcept
{
    return (int) groovePatterns().size();
}

int userPatternIndex() noexcept
{
    return randomPatternIndex() + 1;
}

const std::vector<const char*>& progressionCategories()
{
    static const std::vector<const char*> categories { "Soulful & Gospel", "Circle of 5ths", "Deep house",
                                                       "Modal & exotic", "Jackin' & funky" };
    return categories;
}

const std::vector<ProgressionPreset>& progressionPresets()
{
    // New presets are appended (saved projects only store the chords, not the preset).
    static const std::vector<ProgressionPreset> presets {
        { "Dorian Vamp  i - IV",               1, { 0, 3 },       "Deep house" },
        { "Deep Minor  i - v",                 1, { 0, 4 },       "Deep house" },
        { "Soulful Descent  IV - iii - ii - I", 0, { 3, 2, 1, 0 }, "Soulful & Gospel" },
        { "Jazzy  ii - V",                     0, { 1, 4 },       "Soulful & Gospel" },
        { "Turnaround  I - vi - ii - V",       0, { 0, 5, 1, 4 }, "Soulful & Gospel" },
        { "Royal Road  IV - V - iii - vi",     0, { 3, 4, 2, 5 }, "Soulful & Gospel" },
        { "Minor Flow  i - VI - III - VII",    2, { 0, 5, 2, 6 }, "Deep house" },
        { "Late Night  i - VII - VI - VII",    2, { 0, 6, 5, 6 }, "Deep house" },
        { "Gospel Lift  IV - V - vi - I",      0, { 3, 4, 5, 0 }, "Soulful & Gospel" },
        { "Mixo Groove  I - VII - IV - I",     3, { 0, 6, 3, 0 }, "Jackin' & funky" },
        { "Lydian Float  I - II",              4, { 0, 1 },       "Modal & exotic" },
        { "8-Bar Journey",                     0, { 0, 5, 3, 4, 2, 5, 1, 4 }, "Soulful & Gospel" },
        { "Circle of 5ths  vi - ii - V - I",   0, { 5, 1, 4, 0 }, "Circle of 5ths" },
        { "Circle of 5ths  iii - vi - ii - V", 0, { 2, 5, 1, 4 }, "Circle of 5ths" },
        { "Full Circle  I-IV-vii-iii-vi-ii-V-I", 0, { 0, 3, 6, 2, 5, 1, 4, 0 }, "Circle of 5ths" },
        { "Minor Circle  i - iv - VII - III",  2, { 0, 3, 6, 2 }, "Circle of 5ths" },
        { "Dorian Circle  i - iv - VII - III", 1, { 0, 3, 6, 2 }, "Circle of 5ths" },
        { "Jackin' Minor  i - iv",             2, { 0, 3 },       "Jackin' & funky" },
        { "Jackin' Groove  i - VII - VI - v",  2, { 0, 6, 5, 4 }, "Jackin' & funky" },
        { "Soulful Gospel  I - iii - IV - V",  0, { 0, 2, 3, 4 }, "Soulful & Gospel" },
        { "Deep Dorian  i - ii - IV - i",      1, { 0, 1, 3, 0 }, "Deep house" },
        { "Funky Mixo  I - IV - VII - IV",     3, { 0, 3, 6, 3 }, "Jackin' & funky" },
        // v1.5
        { "Gospel Plagal  I - iv - V - I",     9, { 0, 3, 4, 0 }, "Soulful & Gospel" },
        { "Lydian Dom Funk  I7#11 - II",       10, { 0, 1 },      "Modal & exotic" },
        { "Soul Borrowed  I - iv - bVII - I",  11, { 0, 3, 6, 0 }, "Soulful & Gospel" },
        { "Dark Deep  I - II (Phrygian dom.)", 12, { 0, 1 },      "Modal & exotic" },
        { "Phrygian Deep  i - II - i - vii",   5, { 0, 1, 0, 6 }, "Modal & exotic" },
        { "Deep House Classic  vi - IV - I - V", 0, { 5, 3, 0, 4 }, "Deep house" },
        { "Jackin' Dorian  i - IV - i - v",    1, { 0, 3, 0, 4 }, "Jackin' & funky" },
        { "Neo Soul  ii - iii - IV - V",       0, { 1, 2, 3, 4 }, "Soulful & Gospel" },
        { "Jackin' Stab  i - III - IV",        1, { 0, 2, 3, 3 }, "Jackin' & funky" },
        { "Jack Swing  i - v - VII - IV",      1, { 0, 4, 6, 3 }, "Jackin' & funky" },
        { "Funky Dominant  I7 - IV7",          3, { 0, 3 },       "Jackin' & funky" },
        { "Disco Jack  ii - V - I - vi",       0, { 1, 4, 0, 5 }, "Jackin' & funky" },
        { "Ghetto Funk  i - iv - v - iv",      2, { 0, 3, 4, 3 }, "Jackin' & funky" },
        { "Phrygian Jack  i - bII",            5, { 0, 1 },       "Jackin' & funky" },
        { "Lydian Jack  I7 - bVII",           10, { 0, 6, 0, 6 }, "Jackin' & funky" },
        { "Chicago Bounce  i - VII - i - IV",  1, { 0, 6, 0, 3 }, "Jackin' & funky" },
    };

    return presets;
}

const char* fillStyleName (int style) noexcept
{
    static const char* const names[numFillStyles] { "Pentatonic run up", "Pentatonic run down", "Chord roll (next chord)" };
    return names[mod (style, numFillStyles)];
}

const char* approachStyleName (int style) noexcept
{
    static const char* const names[numApproachStyles] { "Off", "From above", "From below" };
    return names[mod (style, numApproachStyles)];
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

int GrooveEngine::loopLengthInSteps (const GrooveSettings& s) noexcept
{
    const int patternLength = GrooveEngine::patternLength (s);
    const int progression = std::clamp (s.progressionLength, 1, maxProgressionLength) * std::max (1, s.stepsPerChord);

    auto lcm = [] (long long a, long long b)
    {
        long long x = a, y = b;
        while (y != 0) { const auto t = x % y; x = y; y = t; }
        return a / x * b;
    };

    long long steps = lcm (progression, patternLength);
    if (s.fillEveryBars > 0 && sectionAllowsFills (s.section))
        steps = lcm (steps, 16LL * s.fillEveryBars);

    steps = lcm (steps, 16);                 // whole bars
    return (int) std::min<long long> (steps, 64 * 16);
}

GroovePattern GrooveEngine::basePattern (const GrooveSettings& s)
{
    if (s.pattern >= userPatternIndex())
    {
        GroovePattern p;
        p.name = "User Pattern";
        for (int i = 0; i < userPatternLength; ++i)
        {
            const char r = s.userRight[(size_t) i], l = s.userLeft[(size_t) i];
            p.rightHand += (r == 0 ? '.' : r);
            p.leftHand  += (l == 0 ? '.' : l);
        }
        return p;
    }

    if (s.pattern == randomPatternIndex())
        return makeRandomPattern (s.riffSeed, s.riffDensity);

    const auto& patterns = groovePatterns();
    return patterns[(size_t) std::clamp (s.pattern, 0, (int) patterns.size() - 1)];
}

GroovePattern GrooveEngine::resolvePattern (const GrooveSettings& s)
{
    return applySection (applySegmentOrder (basePattern (s), s.segmentOrder), s.section);
}

int GrooveEngine::patternLength (const GrooveSettings& s) noexcept
{
    const auto& patterns = groovePatterns();
    if (s.pattern >= randomPatternIndex())
        return 32;   // random riffs and the user pattern are always two bars

    return patterns[(size_t) std::clamp (s.pattern, 0, (int) patterns.size() - 1)].length();
}

const GroovePattern& GrooveEngine::patternFor (const GrooveSettings& s)
{
    if (s.pattern != cachedIndex || s.riffSeed != cachedSeed || s.riffDensity != cachedDensity
        || s.section != cachedSection || s.segmentOrder != cachedOrder
        || s.userRight != cachedUserRight || s.userLeft != cachedUserLeft || cachedPattern.rightHand.empty())
    {
        cachedPattern = resolvePattern (s);
        cachedIndex = s.pattern;
        cachedSeed = s.riffSeed;
        cachedDensity = s.riffDensity;
        cachedSection = s.section;
        cachedOrder = s.segmentOrder;
        cachedUserRight = s.userRight;
        cachedUserLeft = s.userLeft;
    }

    return cachedPattern;
}

std::vector<NoteEvent> GrooveEngine::renderArrangement (const GrooveSettings& s, const std::vector<ArrangementPart>& parts,
                                                        std::vector<Marker>* markers)
{
    GrooveEngine engine;
    std::vector<NoteEvent> events;
    double start = 0.0;

    for (const auto& part : parts)
    {
        auto partSettings = s;
        partSettings.section = part.section;

        if (markers != nullptr)
            markers->push_back ({ start, sectionName ((int) part.section) });

        const double end = start + part.bars * 4.0;
        engine.render (start, end, partSettings, events);
        start = end;
    }

    engine.allNotesOff (start, events);
    return events;
}

std::vector<NoteEvent> GrooveEngine::renderClip (const GrooveSettings& s, double lengthInBeats)
{
    GrooveEngine engine;
    std::vector<NoteEvent> events;
    engine.render (0.0, lengthInBeats, s, events);

    // Clip notes at the end so the clip loops cleanly.
    engine.allNotesOff (lengthInBeats, events);
    return events;
}

int GrooveEngine::slotForStep (long long step, const GrooveSettings& s) noexcept
{
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);
    return mod (floorDiv (step, std::max (1, s.stepsPerChord)), len);
}

Chord GrooveEngine::chordForSlot (const GrooveSettings& s, int slot) noexcept
{
    const int i = std::clamp (slot, 0, maxProgressionLength - 1);
    const int free = s.freeChords[(size_t) i];
    if (free >= 0 && free < 24)
        return buildChord ((s.keyRoot + free % 12) % 12, free >= 12 ? 1 : 0, 0, s.extension);   // minor = Dorian i, major = Ionian I

    return buildChord (s.keyRoot, s.scale, s.progression[(size_t) i], s.extension);
}

std::string GrooveEngine::numeralForSlot (const GrooveSettings& s, int slot)
{
    const auto chord = chordForSlot (s, slot);
    if (s.freeChords[(size_t) std::clamp (slot, 0, maxProgressionLength - 1)] >= 0)
        return chromaticNumeral (s.keyRoot, chord);

    return romanNumeral (s.progression[(size_t) std::clamp (slot, 0, maxProgressionLength - 1)], chord);
}

void GrooveEngine::voiceProgression (const GrooveSettings& s,
                                     std::array<Chord, maxProgressionLength>& chords,
                                     std::array<VoicedChord, maxProgressionLength>& voiced) noexcept
{
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);

    for (int i = 0; i < len; ++i)
        chords[(size_t) i] = chordForSlot (s, i);

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
void GrooveEngine::collectStep (long long step, const GrooveSettings& s, const GroovePattern& pattern,
                                const std::array<Chord, maxProgressionLength>& chords,
                                const std::array<VoicedChord, maxProgressionLength>& voiced)
{
    const int pos = mod (step, pattern.length());

    const double swingDelay = (step % 2 != 0) ? std::clamp (s.swing, 0.0f, 1.0f) * stepLength / 3.0 : 0.0;
    const double baseTime = (double) step * stepLength + swingDelay;
    const double human = std::clamp (s.humanize, 0.0f, 1.0f);

    auto hitTime = [&] (int salt) { return baseTime + human * 0.03 * noise (step, salt); };

    auto hitVelocity = [&] (char symbol, double scale, int salt)
    {
        double v = s.velocity * scale * sectionVelocityScale (s.section) * (symbol == 'X' ? 1.0 : symbol == '>' ? 0.95 : symbol == 'g' ? 0.45 : 0.8);
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
    const bool inFill = s.fillEveryBars > 0 && sectionAllowsFills (s.section) && mod (bar, s.fillEveryBars) == s.fillEveryBars - 1 && stepInBar >= 12;

    // Approach ("slide"): the last 16th before a real chord change.
    const int slotNow = slotForStep (step, s);
    const int slotNext = slotForStep (step + 1, s);
    const int approachShift = s.approach == ApproachStyle::fromAbove ? 1 : s.approach == ApproachStyle::fromBelow ? -1 : 0;
    const bool approachHere = approachShift != 0 && s.stepsPerChord >= 8 && slotNext != slotNow
                           && (chords[(size_t) slotNext].rootPc != chords[(size_t) slotNow].rootPc
                               || chords[(size_t) slotNext].third != chords[(size_t) slotNow].third);
    // Only where that hand is playing (a muted bass in the break stays muted).
    auto canApproach = [pos] (const std::string& line)
    {
        const char symbol = line[(size_t) pos];
        if (symbol == '>' || symbol == '=')
            return false;

        const int len = (int) line.size();
        for (int k = 1; k < 16; ++k)
            if (isHit (line[(size_t) mod (pos - k, len)]))
                return true;
        return false;
    };
    const double approachLength = std::clamp (s.gate, 0.2f, 0.6f) * stepLength;

    // ---- right hand (chords) ----
    if (inFill)
    {
        const int k = stepInBar - 12;
        const double len = (k == 3 ? 2.0 : std::clamp (s.gate, 0.3f, 1.0f)) * stepLength;
        const int vel = hitVelocity ('x', k == 3 ? 1.0 : 0.85, 32);

        if (s.fillStyle == FillStyle::chordRoll)
        {
            // Arpeggiate the NEXT chord upwards, landing on its top note.
            const long long nextBar = (floorDiv (step, 16) + 1) * 16;
            const auto& next = voiced[(size_t) slotForStep (nextBar, s)].rightHand;
            if (next.size > 0)
            {
                const int idx = next.size - 4 + k;   // last four notes of the (wrapped) chord
                const int octaveShift = idx < 0 ? -12 * ((-idx + next.size - 1) / next.size) : 0;
                const int note = next[(idx % next.size + next.size) % next.size] + octaveShift;
                pending.push_back ({ hitTime (31), note, vel, len });
            }
        }
        else
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

                const int idx = s.fillStyle == FillStyle::pentatonicDown ? iTop + 2 - k : iTop - 2 + k;
                pending.push_back ({ hitTime (31), run[std::clamp (idx, 0, run.size - 1)], vel, len });
            }
        }
    }
    else if (approachHere && canApproach (pattern.rightHand))
    {
        const auto& next = voiced[(size_t) slotNext];
        const double t = hitTime (1);
        const double roll = std::clamp (s.strum, 0.0f, 1.0f) * 0.03;
        for (int i = 0; i < next.rightHand.size; ++i)
            pending.push_back ({ t + i * roll, next.rightHand[i] + approachShift, hitVelocity ('x', 0.8, 60 + i), approachLength });
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

            // Strum: roll the chord upwards, up to ~1/32 note between the notes.
            const double roll = std::clamp (s.strum, 0.0f, 1.0f) * 0.06;

            for (int i = 0; i < v.rightHand.size; ++i)
                pending.push_back ({ t + 0.004 * human * noise (step, 50 + i) + i * roll, v.rightHand[i],
                                     hitVelocity (symbol, 1.0 - 0.03 * (v.rightHand.size - 1 - i), 60 + i), len, tie });

            if (s.octaveTop && v.rightHand.size > 0 && v.rightHand.highest() + 12 <= 108)
                pending.push_back ({ t + v.rightHand.size * roll, v.rightHand.highest() + 12,
                                     hitVelocity (symbol, 0.8, 70), len, tie });

            for (int i = 0; i < v.leftHand.size; ++i)
                pending.push_back ({ t, v.leftHand[i], hitVelocity (symbol, 0.85, 80 + i), len, tie });
        }
    }

    // ---- left hand (bass) ----
    auto bassMode = s.bass;
    if (s.section == Section::drop && bassMode == BassMode::root)
        bassMode = BassMode::octaves;    // the drop hits harder

    if (bassMode != BassMode::off && approachHere && canApproach (pattern.leftHand))
    {
        const int target = 36 + chords[(size_t) slotNext].rootPc;
        pending.push_back ({ hitTime (90), target + approachShift, hitVelocity ('x', 0.8, 91), approachLength, false, Part::bass });
    }
    else if (bassMode != BassMode::off)
    {
        const char symbol = pattern.leftHand[(size_t) pos];
        if (isHit (symbol))
        {
            const int root = 36 + chords[(size_t) slotFor (symbol)].rootPc;
            const double len = durationFor (pattern.leftHand);
            const double t = hitTime (90);
            const int vel = hitVelocity (symbol, 0.9, 91);
            const bool tie = isTiedFromPush (pattern.leftHand, pos);

            pending.push_back ({ t, root, vel, len, tie, Part::bass });
            if (bassMode == BassMode::octaves)    pending.push_back ({ t, root + 12, (int) (vel * 0.85), len, tie, Part::bass });
            if (bassMode == BassMode::rootFifth)  pending.push_back ({ t, root + 7,  (int) (vel * 0.85), len, tie, Part::bass });
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
        out.push_back ({ offTime[(size_t) n], n, 0, partOf[(size_t) n] });
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

    const auto& pattern = patternFor (s);
    for (auto step = firstStep; step <= lastStep; ++step)
        collectStep (step, s, pattern, chords, voiced);

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

            out.push_back ({ p.ppq, p.note, 0, partOf[(size_t) p.note] });  // retrigger
        }

        out.push_back ({ p.ppq, p.note, p.velocity, p.part });
        partOf[(size_t) p.note] = p.part;
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
            out.push_back ({ ppq, n, 0, partOf[(size_t) n] });
            offTime[(size_t) n] = -1.0;
        }
    }
}

} // namespace sgk
