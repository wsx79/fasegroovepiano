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

    bool isHit (char c) noexcept { return c == 'X' || c == 'x' || c == 'g' || c == '>' || c == 'u' || c == 'U' || c == 'd' || c == 'D'; }

    /** A push ('>') held with '=' right up to a hit: that hit is tied, not re-struck. */
    bool pushTiesIntoNextHit (const std::string& line, int pos) noexcept
    {
        const int len = (int) line.size();
        const char next = line[(size_t) ((pos + 1 + holdLength (line, pos)) % len)];
        return line[(size_t) pos] == '>' && isHit (next) && next != '>';
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
namespace
{
    /** A pianist doesn't strike a full chord on every 16th: in a burst of three or more chords
        in a row the inner ones become light ghost notes (the top of the chord), and a bar keeps
        at most 8 full chords (the weakest 16th offbeats, then 8th offbeats, go light first).
        Accents, pushes and the top-note moves stay as written. */
    void polishGroove (GroovePattern& p)
    {
        auto& r = p.rightHand;
        const int n = (int) r.size();
        auto full = [] (char c) { return isHit (c) && c != 'g'; };
        auto lighten = [&] (int i) { if (r[(size_t) i] == 'x' && (i + 1 >= n || r[(size_t) i + 1] != '=')) { r[(size_t) i] = 'g'; return true; } return false; };

        for (int i = 1; i + 1 < n; ++i)
            if (full (r[(size_t) i - 1]) && full (r[(size_t) i]) && full (r[(size_t) i + 1]))
                lighten (i);

        for (int bar = 0; bar * 16 < n; ++bar)
        {
            const int from = bar * 16, to = std::min (n, from + 16);
            auto count = [&] { int c = 0; for (int i = from; i < to; ++i) c += full (r[(size_t) i]) ? 1 : 0; return c; };
            for (int pass = 0; pass < 2 && count() > 8; ++pass)
                for (int i = from + 1; i < to && count() > 8; ++i)
                    if (pass == 0 ? i % 2 == 1 : i % 4 == 2)
                        lighten (i);
        }
    }
}

const std::vector<GroovePattern>& groovePatterns()
{
    //                         |1 e & a 2 e & a 3 e & a 4 e & a |1 ...                 bar 2
    static const std::vector<GroovePattern> written {
        { "Classic Offbeat Stabs",
          compact ("..X. ..u. ..X. ..d. ..X. ..u. ..X. ..x."),
          compact ("x==. .... x==. .... x==. .... x==. ...."), "'90s Piano House" },
        { "Soulful Push",
          compact ("X..u ..x. .d.. ..>= =..u ..x. .d.. ..>="),
          compact ("x=.. ...x ..x. .... ...x ...x ..x. ..>."), "Soulful" },
        { "Deep Chords",
          compact ("X=== ===. ..x= ===. X=== ==.x ..x. x==."),
          compact ("x=== ==== .... ..x. x=== ==== .... ...."), "Deep" },
        { "Funky 16ths",
          compact ("x.x. .x.x .xx. x.x. x.x. .x.x .x.x ..x."),
          compact ("x... ..x. x... .... x... ..x. x..x ...."), "Funky & Disco" },
        { "Gospel Bounce",
          compact ("X..u ..X. .d.. X..u ..X. .d.. X.u. .x.."),
          compact ("x..x .... x... ..x. x..x .... x.x. ...."), "Gospel" },
        { "Chicago Jack",
          compact ("..x. ..ud ..x. .u.x ..x. ..ud ..x. .xU."),
          compact ("x... x... x... x... x... x... x... x..."), "Jackin'" },
        { "Garage Skip",
          compact ("x... ..x. ...x ..x. x... ..x. ...x .x.."),
          compact ("x... .... ..x. .... x... .... ..x. ..x."), "Garage & UK" },
        { "Late Night Rhodes",
          compact ("X==. .x=. ...x =.x. X==. .x=. .x.. >==="),
          compact ("x=== .... ..x. .... x=== .... x... ...."), "Deep" },
        { "Disco Octaves",
          compact ("..x. ..x. ..x. ..x. ..x. ..x. ..x. ..x."),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x.x."), "Funky & Disco" },
        { "Long Chords",
          compact ("X=== ==== ==== ===. X=== ==== ==== ===."),
          compact ("x=== ==== ==== ===. x=== ==== ==== ===."), "Chill & Pads" },

        // ---- Funk ----
        { "Funk Clav Chops",
          compact ("x.x. .xx. x.x. .x.x x.x. .xx. x.x. .x>="),
          compact ("x... ..x. .x.. x... x... ..x. .x.. ..>="), "Funky & Disco" },
        { "Ghost Note Funk",
          compact ("X.gg .gX. g.gX .g.g X.gg .gX. g.X. gXg."),
          compact ("x... ..x. ..x. .x.. x... ..x. x... ..x."), "Funky & Disco" },
        { "Nile Chops",
          compact ("..x. x.xx ..x. x.x. ..x. x.xx .xx. x.x."),
          compact ("x... ...x ..x. .... x... ...x ..x. .x.."), "Funky & Disco" },
        { "Anticipation Funk",
          compact ("X.x. .x.. x.x. .x>= =.x. .x.. x.x. .x>="),
          compact ("x..x ..x. x... ..>= =..x ..x. x... ..>="), "Funky & Disco" },
        { "Gospel Shout 16ths",
          compact ("X..x .xX. .x.x .Xx. X..x .xX. .x.X .x>="),
          compact ("x..x ..x. .x.. x... x..x ..x. .x.. ..>="), "Gospel" },
        { "Neo Soul Lazy",
          compact ("X==. ..x= .x.. x==. ..X= =.x. .x.. >==="),
          compact ("x... ..x. .... x... x... ..x. ..x. ...."), "Neo Soul & Broken" },
        { "Boogie Stabs",
          compact ("x.x. ..x. .x.x ..x. x.x. ..x. .x.x .x.."),
          compact ("x... x.x. x... x.x. x... x.x. x... x..x"), "Funky & Disco" },
        { "Offbeat Doubles",
          compact ("..xu ..x. ..xd .x.. ..xu ..x. ..xd .x.x"),
          compact ("x... ...x x... .... x... ...x x..x ...."), "'90s Piano House" },
        { "Broken Beat",
          compact ("X... ..x. ...x .x.. ..x. x... .x.. x.x."),
          compact ("x... ...x ..x. .... .x.. ..x. ...x ...."), "Neo Soul & Broken" },
        { "Slap Bass Funk",
          compact ("..x. ...x ..x. .x.. ..x. ...x ..xg .x.."),
          compact ("x..x ..x. x... .x.. x..x ..x. x.x. .x.."), "Funky & Disco" },
        // ---- Jackin' house (Chicago bounce: offbeat stabs, pumping bass, swing it!) ----
        { "Jackin' Stabs",
          compact ("..X. .u.x ..X. .d.x ..X. .u.x ..X. u.x."),
          compact ("x..x ..x. x... ..x. x..x ..x. x... .x.."), "Jackin'" },
        { "Jack Bounce",
          compact (".x.u .x.d .x.u .X.x .x.u .x.d .x.u .U.."),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x..x"), "Jackin'" },
        { "Jackin' Push",
          compact ("X..u ..X. .d.. x.>= =..u ..X. .d.x ..>="),
          compact ("x..x .x.. x..x ..>= =..x .x.. x..x ..>="), "Jackin'" },
        { "Chicago Organ Jack",
          compact ("x.xu ..x. x.xd ..x. x.xu ..x. x.x. xU.x"),
          compact ("x... ..x. x... ..x. x... ..x. x..x ..x."), "Jackin'" },
        { "Ghetto Jack",
          compact ("X.gx ..X. .x.X ...x X.gx ..X. .xX. ..X."),
          compact ("x..x ..x. ...x ..x. x..x ..x. .x.x ..x."), "Jackin'" },
        { "Disco Jack Chops",
          compact ("..xx .x.. ..xx .x.x ..xx .x.. ..x. xx.x"),
          compact ("x... ..x. x.x. ..x. x... ..x. x.x. ..x."), "Jackin'" },

        // ---- v1.2: more soulful, deep, jackin' and funky ----
        { "Soulful Sunday",
          compact ("X==. .u.. x==. .d>= =..u ..X= =.x. x.>="),
          compact ("x... ..x. x... ..>= =..x ..x. x... ..>="), "Soulful" },
        { "Gospel Chops",
          compact ("X.x. xX.x .x.X x.>= =.x. xX.x .x.X x.>="),
          compact ("x..x ..x. x..x ..>= =..x ..x. x..x ..>="), "Gospel" },
        { "Neo Soul Comping",
          compact (".x=. ..x= .X.. x=.. .x=. ..x= .x.. >==="),
          compact ("x... .x.. ..x. .... x... .x.. ..x. ...."), "Neo Soul & Broken" },
        { "Deep Sway",
          compact ("X=== ..x= ==.. x=== X=== ..x= ==.. .x=="),
          compact ("x=== .... ..x. .... x=== .... ..x. ...."), "Deep" },
        { "Deep Dub Stabs",
          compact ("..X= ==.. .... ..x. ..X= ==.. .... x..."),
          compact ("x=== .... .... .... x=== .... ..x. ...."), "Deep" },
        { "Jackin' Shuffle",
          compact (".x.u ..x. .x.d .xx. .x.u ..x. .x.d x.x."),
          compact ("x..x ..x. x... ..x. x..x ..x. x..x ..x."), "Jackin'" },
        { "Jack Organ Pump",
          compact ("..x. ..x. ..x. ..xx ..x. ..x. ..x. .xx."),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.xx x.x."), "Jackin'" },
        { "Funky Upstrokes",
          compact (".x.u .x.d .x.U ..x. .x.u .x.d .U.. x.x."),
          compact ("x..x ..x. x... .x.. x..x ..x. x.x. ...."), "Funky & Disco" },
        // ---- v1.7: many more grooves, by genre ----
        { "Soulful Lift",
          compact ("X=.. ..x. .x=. ..x. X=.. ..x. .x.. x.>="),
          compact ("x=.. .... x... ..x. x=.. .... x... ..>="), "Soulful" },
        { "Sunday Morning Keys",
          compact ("X==. ..x= =... x=.. X==. ..x= =..x ..>="),
          compact ("x=== .... x... .... x=== .... x... ..>="), "Soulful" },
        { "Soulful Offbeat 8ths",
          compact ("..x. ..x. ..x. ..xx ..x. ..x. ..x. .x>="),
          compact ("x... ...x x... .... x... ...x x... ..>="), "Soulful" },
        { "Soul Train Comp",
          compact ("x.x. .x.x ..x. x.x. x.x. .x.x ..x. .x>="),
          compact ("x..x ..x. x... ..x. x..x ..x. x... ..>="), "Soulful" },
        { "Vocal House Stabs",
          compact ("..X. ..x= ..X. .x.. ..X. ..x= ..X. x.>="),
          compact ("x=.. ..x. x=.. .... x=.. ..x. x=.. ..>="), "Soulful" },
        { "Soulful Pianist",
          compact ("X..x .xX. .x.. X.x. X..x .xX. .x.x ..>="),
          compact ("x=.. ...x ..x. .... x=.. ...x ..x. ..>="), "Soulful" },
        { "Warm Rhodes Swing",
          compact ("X=.x ..X= .x.. x=.. X=.x ..X= .x.x .x>="),
          compact ("x=.. .... x=.. ..x. x=.. .... x=.. ..>="), "Soulful" },
        { "Deep Late Pad",
          compact ("X=== ==== ..x= ==.. X=== ==== .x== ===."),
          compact ("x=== ==== .... .... x=== ==== .... ...."), "Deep" },
        { "Deep Offbeat Chords",
          compact ("..X= ..x= ..X= ..x= ..X= ..x= ..X= .x>="),
          compact ("x=.. .... x=.. .... x=.. .... x=.. ..>="), "Deep" },
        { "Larry's Chords",
          compact ("X=== ===. ...x =... X=== ===. .x.. x==."),
          compact ("x=== ==== ..x. .... x=== ==== .... ...."), "Deep" },
        { "Deep Dorian Swing",
          compact (".x=. .x=. ..x= .x.. .x=. .x=. ..x= .x>="),
          compact ("x... ..x. x... .... x... ..x. x... ..>="), "Deep" },
        { "Midnight Stabs",
          compact ("..X. .... ..x. .x.. ..X. .... .xx. ...."),
          compact ("x=.. .... .... ..x. x=.. .... .... ...."), "Deep" },
        { "Deep Keys Rolls",
          compact ("X=.. x=.. ..X= ..x. X=.. x=.. ..x= .x>="),
          compact ("x=== .... x... .... x=== .... x... ..>="), "Deep" },
        { "Minimal Deep Pulse",
          compact ("..x. .... ..x. .... ..x. .... ..x. ..x."),
          compact ("x... .... .... .... x... .... .... ...."), "Deep" },
        { "Gospel Shout",
          compact ("X.xX .xX. X.xX .x>= =.xX .xX. X.xX .x>="),
          compact ("x..x ..x. x..x ..>= =..x ..x. x..x ..>="), "Gospel" },
        { "Church Organ Swell",
          compact ("X=== ==.x ..X= ==.. X=== ==.x ..x= .x>="),
          compact ("x=== .... x=== .... x=== .... x=.. ..>="), "Gospel" },
        { "Gospel Walk",
          compact ("X..x ..x. X..x .x.x X..x ..x. X.x. x.>="),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x.>="), "Gospel" },
        { "Praise Break",
          compact ("X.Xx .xX. .xX. .xX. X.Xx .xX. .xX. .x>="),
          compact ("x... x.x. x... x.x. x... x.x. x... ..>="), "Gospel" },
        { "Sanctified Stabs",
          compact ("..X. ..X. ..X. X.>= =.X. ..X. ..X. X.>="),
          compact ("x... ..x. x... ..>= =... ..x. x... ..>="), "Gospel" },
        { "Gospel Ballad Swing",
          compact ("X=.x .x=. X=.. .x.. X=.x .x=. X=.x ..>="),
          compact ("x=.. ..x. x=.. .... x=.. ..x. x=.. ..>="), "Gospel" },
        { "Jack Stab Skips",
          compact ("..X. .x.. ..X. x.x. ..X. .x.. ..X. .x>="),
          compact ("x..x ..x. x..x .... x..x ..x. x..x ..>="), "Jackin'" },
        { "Chicago Bounce",
          compact (".X.x .X.x .X.x .Xx. .X.x .X.x .X.x .x>="),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x.>="), "Jackin'" },
        { "Warehouse Jack",
          compact ("X..x ..x. X..x ..x. X..x ..x. X..x .xx."),
          compact ("x... x... x... x... x... x... x... x..x"), "Jackin'" },
        { "Jack Your Body",
          compact ("..xx ..xx ..xx ..x. ..xx ..xx ..xx .x>="),
          compact ("x.x. ..x. x.x. ..x. x.x. ..x. x.x. ..>="), "Jackin'" },
        { "Jackin' Organ Stabs",
          compact ("..X= ..X= ..X= ..X= ..X= ..X= ..X= .xX="),
          compact ("x... ..x. x... ..x. x... ..x. x... .x.."), "Jackin'" },
        { "Swing Jack",
          compact ("X.x. .Xx. x.X. .x>= =.x. .Xx. x.X. .x>="),
          compact ("x..x .x.. x..x ..>= =..x .x.. x..x ..>="), "Jackin'" },
        { "Ghetto House Chops",
          compact ("X.xX .x.X X.xX .xx. X.xX .x.X X.x. Xx>="),
          compact ("x... x..x x... x.x. x... x..x x... ..>="), "Jackin'" },
        { "UK Garage 2-Step",
          compact ("X..x ..x. .x.. ..x. X..x ..x. .x.x ..x."),
          compact ("x... ...x ..x. .... x... ...x ..x. ...."), "Garage & UK" },
        { "Speed Garage Organ",
          compact ("..X= ..X= ..x. X.x. ..X= ..X= ..x. X.>="),
          compact ("x... ..x. x..x ..x. x... ..x. x..x ..>="), "Garage & UK" },
        { "Shuffled 2-Step",
          compact (".x.. x..x .x.. x... .x.. x..x .x.x ..x."),
          compact ("x... ..x. ...x .... x... ..x. ...x ...."), "Garage & UK" },
        { "Garage Chord Skips",
          compact ("x=.x ..x. .x=. ..x. x=.x ..x. .x=. .x>="),
          compact ("x... ...x ..x. .... x... ...x ..x. ..>="), "Garage & UK" },
        { "Bassline House",
          compact ("..x. ..x. ..x. ..x. ..x. ..x. ..x. ..x."),
          compact ("x.x. ..x. x..x .x.. x.x. ..x. x..x .x.x"), "Garage & UK" },
        { "UK Funky Keys",
          compact ("x..x ..x. x..x .x.. x..x ..x. x.x. .x>="),
          compact ("x... x..x ..x. .... x... x..x ..x. ..>="), "Garage & UK" },
        { "Afro House Keys",
          compact ("X..x ..x. ..x. x... X..x ..x. ..x. x.>="),
          compact ("x..x ..x. .... .... x..x ..x. .... ..>="), "Afro & Latin" },
        { "Tresillo Chords",
          compact ("X..x ..x. X..x ..x. X..x ..x. X..x ..x."),
          compact ("x..x ..x. x... .... x..x ..x. x... ...."), "Afro & Latin" },
        { "Latin Montuno",
          compact ("x.x. .x.x .x.x .x.x x.x. .x.x .x.x x.x."),
          compact ("x... ..x. ...x ..x. x... ..x. ...x ..x."), "Afro & Latin" },
        { "Clave Stabs",
          compact ("X..x ..X. ..x. x... X..x ..X. ..x. x..."),
          compact ("x... .... x... .... x... .... x... ...."), "Afro & Latin" },
        { "Afro Deep Groove",
          compact ("..x= .x.. x=.. .x.. ..x= .x.. x=.. .x>="),
          compact ("x=.. ..x. ..x. .... x=.. ..x. ..x. ..>="), "Afro & Latin" },
        { "Samba House",
          compact ("x.x. .x.x ..x. .x.x x.x. .x.x ..x. .xx."),
          compact ("x..x ..x. x..x ..x. x..x ..x. x..x ..x."), "Afro & Latin" },
        { "Disco Strut",
          compact ("..x. x.x. ..x. x.x. ..x. x.x. ..x. xx>="),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x.>="), "Funky & Disco" },
        { "Nu Disco Keys",
          compact (".x.x ..x. .x.x ..x. .x.x ..x. .x.x .x>="),
          compact ("x..x ..x. x..x ..x. x..x ..x. x..x ..>="), "Funky & Disco" },
        { "Funky Clavinet 16ths",
          compact ("x.xg x.x. xgx. .xx. x.xg x.x. xgx. x.>="),
          compact ("x... ..x. .x.. x... x... ..x. .x.. ..>="), "Funky & Disco" },
        { "Philly Stabs",
          compact ("X... ..x. .x.. x... X... ..x. .x.. x.>="),
          compact ("x..x x... x..x x... x..x x... x..x ..>="), "Funky & Disco" },
        { "Boogie Wonderland",
          compact ("x.xx .x.x x.xx .x.x x.xx .x.x x.x. xx>="),
          compact ("x... x.x. x... x.x. x... x.x. x... ..>="), "Funky & Disco" },
        { "French Touch Filter",
          compact ("..xx ..xx ..xx ..xx ..xx ..xx ..xx .xx."),
          compact ("x... ..x. x... ..x. x... ..x. x... ..x."), "Funky & Disco" },
        { "M1 Organ Stabs",
          compact ("..X. ..X. ..X. ..X. ..X. ..X. ..X. .xX."),
          compact ("x... .... x... .... x... .... x... ...."), "'90s Piano House" },
        { "Italo Piano Anthem",
          compact ("X..x ..x. X..x ..x. X..x ..x. X.x. x.x."),
          compact ("x... x... x... x... x... x... x... x..."), "'90s Piano House" },
        { "Hands Up Piano",
          compact ("X.x. X.x. X.x. Xx.x X.x. X.x. X.x. x.>="),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x.>="), "'90s Piano House" },
        { "Piano Anthem Offbeats",
          compact ("..Xx ..Xx ..Xx ..X. ..Xx ..Xx ..Xx .x>="),
          compact ("x... ..x. x... ..x. x... ..x. x... ..>="), "'90s Piano House" },
        { "Show Me Stabs",
          compact ("..X. .x.. ..X. .x.. ..X. .x.. ..X. xx.."),
          compact ("x... ..x. x... ..x. x... ..x. x... ..x."), "'90s Piano House" },
        { "Rave Piano Chords",
          compact ("X.xX .xX. X.xX .x.x X.xX .xX. X.x. Xx>="),
          compact ("x... x... x... x... x... x... x... ..>="), "'90s Piano House" },
        { "Balearic Piano",
          compact ("X=.x ..x. X=.x ..x. X=.x ..x. X=.x .x>="),
          compact ("x=.. ..x. x=.. ..x. x=.. ..x. x=.. ..>="), "'90s Piano House" },
        { "Lazy Swing Keys",
          compact ("X=.. .x=. ..x= .x.. X=.. .x=. ..x. x=.."),
          compact ("x... ..x. .x.. .... x... ..x. .x.. ...."), "Neo Soul & Broken" },
        { "Broken Beat Chords",
          compact ("X..x .x.. x.X. ..x. .x.. X..x ..x. x.>="),
          compact ("x... ...x ..x. .... .x.. ..x. ...x ..>="), "Neo Soul & Broken" },
        { "Neo Soul Ghosts",
          compact ("X.g. gX.g x.g. .Xg. X.g. gX.g x.gX .g>="),
          compact ("x... .x.. ..x. .... x... .x.. ..x. ..>="), "Neo Soul & Broken" },
        { "Jazzy House Comp",
          compact (".x=. x=.. .x=. ..x= .x=. x=.. .x=. .x>="),
          compact ("x=.. ..x. x=.. .... x=.. ..x. x=.. ..>="), "Neo Soul & Broken" },
        { "Rhodes Broken Swing",
          compact ("X=.x .... x=.x ..x. .x.. X=.x .... x.>="),
          compact ("x=.. ..x. .... x... ..x. x=.. .... ..>="), "Neo Soul & Broken" },
        { "Whole Bar Pads",
          compact ("X=== ==== ==== ===. X=== ==== ==== ===."),
          compact ("x=== ==== ==== ===. x=== ==== ==== ===."), "Chill & Pads" },
        { "Half Bar Pads",
          compact ("X=== ===. X=== ===. X=== ===. X=== ===."),
          compact ("x=== ===. .... .... x=== ===. .... ...."), "Chill & Pads" },
        { "Chill Swells",
          compact ("X=== ==== ..x= ===. X=== ==== .x== ===."),
          compact ("x=== ==== .... .... x=== ==== .... ...."), "Chill & Pads" },
        { "Lounge Keys",
          compact ("X=.. ..x= ==.. .x.. X=.. ..x= ==.. x=.."),
          compact ("x=.. .... x... .... x=.. .... x... ...."), "Chill & Pads" },
        { "Sunset Chords",
          compact ("X==. .... x==. .... X==. .... x==. ..>="),
          compact ("x=== .... .... .... x=== .... .... ..>="), "Chill & Pads" },
        // ---- v1.10: jackin' piano riffs (the chord moves by inversions inside the bar) ----
        { "Chicago Piano Bounce",
          compact ("..x. .u.. ..x. .d.x ..x. .u.. ..x. u.>="),
          compact ("x... ..x. x... .... x... ..x. x... ..>="), "Jackin' Piano" },
        { "Jack Stab Melody",
          compact ("x..u ..x. .u.. x... x..u ..x. .u.U ..>="),
          compact ("x... .... x..x .... x... .... x..x ..>="), "Jackin' Piano" },
        { "Piano Call & Answer",
          compact (".x.x .x.. .... .... .u.u .U.. ..d. x.>="),
          compact ("x... ..x. .... .... x... ..x. ..x. ..>="), "Jackin' Piano" },
        { "Swing Piano Chops",
          compact ("x.xu .x.d x.xu .xU. x.xu .x.d x.x. U.>="),
          compact ("x... ..x. x... ..x. x... ..x. x... ..>="), "Jackin' Piano" },
        { "Gospel Jack Keys",
          compact ("X..u ..X. .d.x ..u. X..u ..X. .d.x .U>="),
          compact ("x..x ..x. x... .... x..x ..x. x... ..>="), "Jackin' Piano" },
        { "Offbeat Inversion Stabs",
          compact ("..x. ..u. ..x. ..d. ..x. ..u. ..x. .U>="),
          compact ("x... .... x... .... x... .... x... ..>="), "Jackin' Piano" },
        { "Three Three Two",
          compact ("X..u ..x. X..u ..x. X..u ..x. x.u. U.>="),
          compact ("x..x ..x. .... .... x..x ..x. .... ..>="), "Jackin' Piano" },
        { "Late Night Jack Piano",
          compact (".x=. .u=. ..x= .d.. .x=. .u=. ..x. U=>="),
          compact ("x... .... x... .... x... .... x... ..>="), "Jackin' Piano" },
        { "Piano Riff Runner",
          compact ("x.u. x.d. x.u. xU.. x.u. x.d. x.u. .U>="),
          compact ("x... ..x. x... ..x. x... ..x. x... ..>="), "Jackin' Piano" },
        { "Warehouse Piano",
          compact ("..X. .u.x ..X. .d.. ..X. .u.x ..X. .U>="),
          compact ("x..x ..x. x..x .... x..x ..x. x..x ..>="), "Jackin' Piano" },
        { "Jackin' Gospel Answer",
          compact ("X... ..x. .u.. u.x. X... ..x. .U.d .x>="),
          compact ("x... ...x ..x. .... x... ...x ..x. ..>="), "Jackin' Piano" },
        { "Shuffle Piano Stabs",
          compact (".x.u ..x. .x.d ..x. .x.u ..x. .x.U .x>="),
          compact ("x... ..x. x... ..x. x... ..x. x... ..x."), "Jackin' Piano" },
        { "Disco Jack Piano",
          compact ("..xu ..x. ..xd ..x. ..xu ..x. ..xU .x>="),
          compact ("x.x. x.x. x.x. x.x. x.x. x.x. x.x. x.>="), "Jackin' Piano" },
        { "Deep Jack Piano",
          compact ("X=.. .u.. x=.. .d.. X=.. .u.. x=.u ..>="),
          compact ("x=.. .... x... .... x=.. .... x... ..>="), "Jackin' Piano" },
        { "Piano Hook",
          compact (".x.. u... .x.. U.x. .x.. u... .x.. d.>="),
          compact ("x... ..x. x... .... x... ..x. x... ..>="), "Jackin' Piano" },
        { "Organ Piano Jack",
          compact ("..x= ..u= ..x= ..d= ..x= ..u= ..x= .U>="),
          compact ("x... ..x. x... ..x. x... ..x. x... ..>="), "Jackin' Piano" },
        // ---- v1.13: piano house loops (rootless chords on syncopated offbeats, octave bass) ----
        { "Dotted House Stabs",
          compact ("..X. .x.. x..x ..x. ..X. .x.. x..x ..>="),
          compact ("x... ..x. .... .... x... ..x. .... ..x."), "Piano House" },
        { "Loop Piano Classic",
          compact ("..X. ...x ...x ..x. X... ...x ..x. .x.."),
          compact ("x... ..x. .... .... x... ..x. .... x..."), "Piano House" },
        { "Rolling House Keys",
          compact ("X... x..x ..x. .x.. ..X. .x.. x..x ..x."),
          compact ("x... ..x. .... ..x. x... ..x. .... ...."), "Piano House" },
        { "Three-Three-Two Piano",
          compact ("X..x ..x. ..x. .x.. X..x ..x. ..x. .>=."),
          compact ("x... ..x. .... .... x... ..x. .... ...."), "Piano House" },
        { "Offbeat Piano Loop",
          compact (".X.. x.x. x..x ..x. ..X. ...x ...x ..x."),
          compact ("x... ..x. .... x... x... ..x. .... ..x."), "Piano House" },
        { "Upfront Piano Stabs",
          compact ("..Xx .x.. x.x. .... ..X. .x.. x..x ..x."),
          compact ("x... ..x. x... x.x. x... ..x. .... ...."), "Piano House" },
        { "Early Change Piano",
          compact ("..X. .x.. x.>= ==== ..X. .x.. x..x ..x."),
          compact ("x... ..x. ..>= .... x... ..x. .... ...."), "Piano House" },
        { "Peak Time Piano",
          compact ("X... ...x ..X. .x.. X... x..x ..X. .x.."),
          compact ("x... ..x. .... ..x. x... ..x. .... ..x."), "Piano House" },
        { "Sunset Piano Loop",
          compact ("..X= ...x =..x =.x. ..X= ...x =..x =.x."),
          compact ("x... ..x. .... .... x... ..x. .... x..."), "Piano House" },
        { "Two Chord Bounce",
          compact ("..X. .x.. x..x ..x. X... ...x ..x. .x.."),
          compact ("x... ..x. .... x... x... ..x. .... ...."), "Piano House" },
        { "Piano Pump",
          compact ("X... x..x ..x. .x.. X... x..x ..x. ..>="),
          compact ("x... ..x. x... x.x. x... ..x. x... x.>="), "Piano House" },
        { "Classic Loop Answer",
          compact ("X..x ..x. ..x. .x.. .X.. x.x. x..x ..x."),
          compact ("x... ..x. .... .... x... ..x. .... ..x."), "Piano House" },
        { "Deep Piano Loop",
          compact ("X=.. ...x =..x =... ..X= ..x= ..x= ..x."),
          compact ("x=.. ..x= .... .... x=.. ..x= .... ..x."), "Piano House" },
        { "Anthem Piano Loop",
          compact ("X..x ..x. ..x. .x.. X..x ..x. .x.. x.x."),
          compact ("x... ..x. .... ..x. x... ..x. x... x.x."), "Piano House" },
        { "Night Drive Piano",
          compact ("..X. .x.. ..x. .x.. ..X. .x.. x... x..."),
          compact ("x... ..x. .... .... x... ..x. .... ...."), "Piano House" },
        { "Skippy Piano Stabs",
          compact (".X.. x.x. x..x ..x. ..X. .x.. x..x .x.."),
          compact ("x... ..x. .... x... x... ..x. .... ..x."), "Piano House" },
        // ---- v1.14: dance stabs (short chords, 3-3-3-3-2-2 figures, bass doubling every hit) ----
        { "Future Stab 3-3-2",
          compact ("X..x ..x. .x.. x.x. X..x ..x. .x.. x.x."),
          compact ("x..x ..x. .x.. x.x. x..x ..x. .x.. x.x."), "Dance Stabs" },
        { "Tresillo Stabs",
          compact ("X... ..x. .x.. x.x. X... ..x. .x.. x.x."),
          compact ("x... ..x. .x.. x.x. x... ..x. .x.. x.x."), "Dance Stabs" },
        { "Rolling 3-3-3",
          compact ("X..x ..x. .x.. x.x. X..x ..x. .x.. x..."),
          compact ("x..x ..x. .x.. x.x. x..x ..x. .x.. x..."), "Dance Stabs" },
        { "Offbeat Push Stabs",
          compact ("X.x. x.x. .x.. x.x. X.x. x.x. .x.. x..x"),
          compact ("x.x. x.x. .x.. x.x. x.x. x.x. .x.. x..x"), "Dance Stabs" },
        { "Bounce Stabs",
          compact ("X... x..x .x.. x.x. X... x..x .x.. x.x."),
          compact ("x... x..x .x.. x.x. x... x..x .x.. x.x."), "Dance Stabs" },
        { "Call Stab Answer",
          compact ("X..x ..x. .... .... ..x. .x.. x..x .x.."),
          compact ("x..x ..x. .... .... ..x. .x.. x..x .x.."), "Dance Stabs" },
        { "Club Hook Stabs",
          compact ("X..x ..x. .x.. x... .x.. x..x ..x. x.x."),
          compact ("x..x ..x. .x.. x... .x.. x..x ..x. x.x."), "Dance Stabs" },
        { "Disco Octave Pump",
          compact ("..x. ..x. ..x. ..x. ..x. ..x. ..x. .xx."),
          compact ("x... x... x... x... x... x... x... x..."), "Dance Stabs" },
        // ---- v1.16: full piano house stabs (best with the "Piano stabs" bass) ----
        { "Stab Train 3-3-3-3-2-2",
          compact ("X..x ..x. .x.. x.x. X..x ..x. .x.. x.x."),
          compact ("x... ..x. .... .... x... ..x. .... ...."), "Piano House" },
        { "Four Stabs Push",
          compact ("X... x..x ..x. .x.. X... x..x ..x. .x.."),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Stab Run 4-3-3-2-2",
          compact ("X... x..x ..x. x.x. X... x..x ..x. x.x."),
          compact ("x... .... ..x. .... x... .... ..x. ...."), "Piano House" },
        { "Late Stab Groove",
          compact ("..X. x..x ..x. x.x. ..X. x..x ..x. x.x."),
          compact ("x... .... x... .... x... .... x... ...."), "Piano House" },
        { "Double Chop Stabs",
          compact ("X.x. xx.x x.xx .xx. X.x. xx.x x.xx .xx."),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Offbeat Roll Stabs",
          compact ("X... x.x. x..x ..x. X... x.x. x..x ..x."),
          compact ("x... .... x... .... x... .... x... ...."), "Piano House" },
        // ---- v2.1: more piano house, soulful, gospel, deep and jackin' grooves ----
        { "Classic Dotted Loop",
          compact ("X..x ..x. .x.. x.x. X..x ..x. .x.. ..>="),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Offbeat Dotted Stabs",
          compact ("..X. x..x ..x. x.x. ..X. x..x ..x. ..>="),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Late Night Piano Loop",
          compact ("X... x.x. x..x ..x. X... x..x ..x. ..x."),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Three-Three-Two Bounce",
          compact ("X..x ..x. x..x ..x. X..x ..x. x..x .x>="),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Anthem Offbeats Push",
          compact ("..X. ..x. ..X. ..x. ..X. ..x. ..x. .x>="),
          compact ("x... .... x... .... x... .... x... ...."), "Piano House" },
        { "Rolling Piano Answer",
          compact ("X... x..x ..x. x.x. X... x..x .x.. x.x."),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Syncopated Answer Stabs",
          compact ("X... x..x ..x. .x.. X... x... x..x ..x."),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Pushed Dotted Hook",
          compact (">=.x ..x. .x.. x.x. X..x ..x. .x.. ..>="),
          compact ("x... .... .... .... x... .... .... ...."), "Piano House" },
        { "Soulful Sway Comp",
          compact ("X=.x ..x= ..x. .x>= =..x ..x= ..x. .x>="),
          compact ("x=.. .... x... .... ..x. .... x... ...."), "Soulful" },
        { "Sunday Keys Push",
          compact ("X=== ..x. .x== ..>= =... ..x. .x== ..>="),
          compact ("x=== .... x... .... .... .... x... ...."), "Soulful" },
        { "Vocal House Offbeats",
          compact ("..X= ..x= ..X= ..x= ..X= ..x= ..x. x.>="),
          compact ("x... .... x... .... x... .... x... ...."), "Soulful" },
        { "Warm Soul Push",
          compact ("X=.. .x=. ..x= .x>= =... .x=. ..x= ...."),
          compact ("x=.. .... x... .... .... .... x... ...."), "Soulful" },
        { "Soulful Two-Step Keys",
          compact ("X=.. ..x= .... x=.. ..x= .... x=.. ..>="),
          compact ("x... .... .... x... .... .... x... ...."), "Soulful" },
        { "Lazy Sunday Chords",
          compact ("X=== ===x ==.. x=.. X=== ===x ==.. ..>="),
          compact ("x=== .... .... .... x=== .... .... ...."), "Soulful" },
        { "Gospel Chop Comp",
          compact ("X.xx .x.x X.x. .x>= =.xx .x.x X.x. x..."),
          compact ("x... .... x... .... .... .... x... ...."), "Gospel" },
        { "Gospel Shuffle Lift",
          compact ("X..x ..X. .x.x ..>= =..x ..X. .x.x x..."),
          compact ("x... .... x... .... .... .... x... ...."), "Gospel" },
        { "Deep Dub Chords",
          compact ("X=== ==.. ..x= ==.. X=== ==.. .... .x=="),
          compact ("x=== ==.. .... .... x=== ==.. .... ...."), "Deep" },
        { "Deep Offbeat Pads",
          compact ("..X= ==.. ..x= ==.. ..X= ==.. ..x. >==="),
          compact ("x... .... x... .... x... .... x... ...."), "Deep" },
        { "Midnight Rhodes",
          compact ("X==. ..x= =..x ==.. X==. ..x= =.x. ..>="),
          compact ("x=.. .... .... .... x=.. .... .... ...."), "Deep" },
        { "Deep Minimal Stab",
          compact ("..x. .... ..x. .... ..x. .... x... ..x."),
          compact ("x... .... .... .... x... .... .... ...."), "Deep" },
        { "Deep Swing Comp",
          compact ("X=.x =..x =.x= ..x. X=.x =..x =.x. x..."),
          compact ("x=.. .... .... .... x=.. .... .... ...."), "Deep" },
        { "Basement Chords",
          compact ("X=== .... ..x= ..x. X=== .... ..x= ..>="),
          compact ("x... .... x... .... x... .... x... ...."), "Deep" },
        { "Jack Chops",
          compact ("x.x. .x.x ..x. x.x. x.x. .x.x ..xx .x.."),
          compact ("x... .... x... .... x... .... x... ...."), "Jackin'" },
        { "Jackin' Organ Push",
          compact ("..x. ..x. ..xx ..x. ..x. ..x. ..xx .x>="),
          compact ("x... x... x... x... x... x... x... x..."), "Jackin'" },
        { "Chicago Stab Jack",
          compact ("X..x ..x. ..x. x.x. X..x ..x. ..xx .xx."),
          compact ("x... .... x... .... x... .... x... ...."), "Jackin'" },
        { "Jack Bounce 16ths",
          compact ("x.xx .x.x x.xx .x.x x.xx .x.x x.x. x.x."),
          compact ("x... ..x. x... ..x. x... ..x. x... ..x."), "Jackin'" },
        { "Ghetto Jack Skip",
          compact ("x... x..x .x.. x..x x... x..x .x.x ..x."),
          compact ("x... .... x... .... x... .... x... ...."), "Jackin'" },
        { "Disco Jack Doubles",
          compact ("..xx ..x. ..xx ..x. ..xx ..x. ..xx .x.x"),
          compact ("x... x... x... x... x... x... x... x..."), "Jackin'" },
        { "Jackin' Piano Hook",
          compact ("X.x. .x.. x.x. .x>= =.x. .x.. x.x. .x.."),
          compact ("x... .... x... .... x... .... x... ...."), "Jackin' Piano" },
        { "Jack Swing Stabs",
          compact ("X..x .x.. x..x .x.. X..x .x.. x.x. x.x."),
          compact ("x... .... x... .... x... .... x... ...."), "Jackin' Piano" },
        { "Rolling 16th Keys",
          compact ("X.x. xx.x .x.x x.xx X.x. xx.x .xx. .x.x"),
          compact ("x..x .x.x .x.. xx.. x..x .x.. x.x. xx.."), "Jackin' Piano" },
        { "Late Dotted Stabs",
          compact (".X=. x=.x =.x= .x=. .x=. x=.x =.x. x.>="),
          compact ("x... x... ..x. .x.. x... x... ..x. x..."), "Piano House" },
        { "Double Hit Stabs",
          compact ("Xx.. x.xx ..x. x.x. Xx.. x=.x =.x. x.>="),
          compact ("x... x..x ..x. .... x... x..x ..x. ..x."), "Jackin' Piano" },
        { "Pocket Three Stabs",
          compact ("X.x. x=.x =.x. x=.x X.x. x=.x =.x. x.>="),
          compact ("x... .... x..x .... x... .... x..x ..x."), "Piano House" },
        { "Offbeat Run Chops",
          compact ("..X. xx.. xx.x x.xx X..x ..xx x..x .x.x"),
          compact ("x... x... x... x... x..x .... x... x..."), "Jackin' Piano" },
        { "Front Loaded Hook",
          compact ("X.x. x.xx ==== ===. X.x. x=.x =.x. x=.x"),
          compact ("x... x.x. .... .... x... x..x ..x. ...."), "Piano House" },
        { "Answer Run Keys",
          compact ("X.x. x.xx .xx. x.xx X.x. xx.x .xx. .x.x"),
          compact ("x..x .... x..x ..x. x..x .... x... x..."), "Jackin' Piano" },
    };

    static const std::vector<GroovePattern> patterns = []
    {
        auto list = written;
        for (auto& p : list)
            polishGroove (p);
        return list;
    }();
    return patterns;
}

const std::vector<const char*>& grooveCategories()
{
    static const std::vector<const char*> categories { "Piano House", "Soulful", "Deep", "Gospel", "Jackin'", "Jackin' Piano", "Garage & UK",
                                                       "Afro & Latin", "Funky & Disco", "'90s Piano House", "Neo Soul & Broken",
                                                       "Chill & Pads", "Dance Stabs" };
    return categories;
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
                                                       "Modal & exotic", "Jackin' & funky", "Chromatic & borrowed" };
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
        { "Chicago Bounce  i - VII - i - IV",  1, { 0, 6, 0, 3 }, "Jackin' & funky" },        // v1.14: chromatic (borrowed chords, dominants)
        { "Maj9 Drift  I - bIII - bVI - bVII", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 3, 8, 10 } },
        { "Deep Planing  I - bVII - bVI - bVII", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 10, 8, 10 } },
        { "Borrowed iv  I - IV - iv - I", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 5, 17, 0 } },
        { "Minor Plagal  vi - IV - iv - I", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 21, 5, 17, 0 } },
        { "Backdoor  ii - bVII7 - I", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 14, 34, 0, 0 } },
        { "Dorian Lift  i - bIII - IV - i", 1, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 12, 3, 5, 12 } },
        { "Neapolitan Deep  i - bII - i - v", 2, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 12, 1, 12, 19 } },
        { "Minor Dominant  vi - III7 - vi - IV", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 21, 28, 21, 5 } },
        { "Floating Maj7  IV - V - iii - bIII", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 5, 7, 16, 3 } },
        { "Mediant Shift  I - iii - bIII - IV", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 16, 3, 5 } },
        { "Lydian Turn  I - II - iv - I", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 2, 17, 0 } },
        { "Picardy House  i - iv - V7 - I", 2, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 12, 17, 31, 0 } },
        { "Deep Maj9 Walk  I - bVI - IV - bVII", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 8, 5, 10 } },
        { "Gospel Flat VII  IV - bVII7 - I - vi", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 5, 34, 0, 21 } },
        { "Minor Glide  i - bVII - bVI - V7", 2, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 12, 10, 8, 31 } },
        { "Secondary Soul  I - VI7 - ii - V7", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 33, 14, 31 } },
        // v2.1: a bigger library for soulful, deep and jackin' house
        { "Soul Cycle  I - iii - vi - IV",        0, { 0, 2, 5, 3 },       "Soulful & Gospel" },
        { "Sunday Gospel  IV - iii - vi - ii - V", 0, { 3, 2, 5, 1, 4 },   "Soulful & Gospel" },
        { "Uplift  vi - V - IV - V",              0, { 5, 4, 3, 4 },       "Soulful & Gospel" },
        { "Two-Five Lift  ii - V - iii - vi",     0, { 1, 4, 2, 5 },       "Soulful & Gospel" },
        { "Gospel Walk-up  I - ii - iii - IV",    0, { 0, 1, 2, 3 },       "Soulful & Gospel" },
        { "Classic Soul  I - IV - ii - V",        0, { 0, 3, 1, 4 },       "Soulful & Gospel" },
        { "Sunrise  I - V - vi - IV",             0, { 0, 4, 5, 3 },       "Soulful & Gospel" },
        { "Deep Soul  ii - V - I - IV",           0, { 1, 4, 0, 3 },       "Soulful & Gospel" },
        { "Vocal Lift  IV - V - iii - vi - ii - V - I - I", 0, { 3, 4, 2, 5, 1, 4, 0, 0 }, "Soulful & Gospel" },
        { "Church Turn  I - I7 - IV - iv",        0, { 0, 0, 0, 0 }, "Soulful & Gospel", { 0, 24, 5, 17 } },
        { "Gospel Secondary  I - III7 - vi - IV", 0, { 0, 0, 0, 0 }, "Soulful & Gospel", { 0, 28, 21, 5 } },
        { "Deep Minor Vamp  i - VI",              2, { 0, 5 },             "Deep house" },
        { "Dorian Swing  i - IV - v - IV",        1, { 0, 3, 4, 3 },       "Deep house" },
        { "Late Night Minor  i - v - VI - iv",    2, { 0, 4, 5, 3 },       "Deep house" },
        { "Deep Float  ii - I",                   0, { 1, 0 },             "Deep house" },
        { "Deep 7ths  vi - iii - IV - I",         0, { 5, 2, 3, 0 },       "Deep house" },
        { "Minor Two  i - ii",                    1, { 0, 1 },             "Deep house" },
        { "Midnight  iv - i - VII - i",           2, { 3, 0, 6, 0 },       "Deep house" },
        { "Deep Soul Minor  i - iv - VII - VI",   2, { 0, 3, 6, 5 },       "Deep house" },
        { "Dorian Night  i - III - IV - ii",      1, { 0, 2, 3, 1 },       "Deep house" },
        { "Jack Vamp  I7 - IV7 - I7 - V7",        3, { 0, 0, 0, 0 }, "Jackin' & funky", { 24, 29, 24, 31 } },
        { "Funky Dorian  i - IV - VII - IV",      1, { 0, 3, 6, 3 },       "Jackin' & funky" },
        { "Chicago Shuffle  I - vi - IV - V",     0, { 0, 5, 3, 4 },       "Jackin' & funky" },
        { "Disco Climb  I - ii - iii - ii",       0, { 0, 1, 2, 1 },       "Jackin' & funky" },
        { "Jackin' Two  i - VII",                 2, { 0, 6 },             "Jackin' & funky" },
        { "Ghetto Dorian  i - ii - i - IV",       1, { 0, 1, 0, 3 },       "Jackin' & funky" },
        { "Funk Turn  I7 - bVII7 - IV7 - I7",     3, { 0, 0, 0, 0 }, "Jackin' & funky", { 24, 34, 29, 24 } },
        { "Long Circle  iii - vi - ii - V - I",   0, { 2, 5, 1, 4, 0 },    "Circle of 5ths" },
        { "Minor Circle Down  i-iv-VII-III-VI-ii-v-i", 2, { 0, 3, 6, 2, 5, 1, 4, 0 }, "Circle of 5ths" },
        { "Secondary Circle  III7 - VI7 - II7 - V7", 0, { 0, 0, 0, 0 }, "Circle of 5ths", { 28, 33, 26, 31 } },
        { "Soul Circle  vi - ii - V - I - IV",    0, { 5, 1, 4, 0, 3 },    "Circle of 5ths" },
        { "Soul Minor iv  I - iii - IV - iv",     0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 16, 5, 17 } },
        { "Flat Six Seven Lift  IV - bVI - bVII - I", 0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 5, 8, 10, 0 } },
        { "Tritone Soul  ii - bII7 - I",          0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 14, 25, 0, 0 } },
        { "Deep Chromatic  i - bVII - bVI - v",   2, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 12, 10, 8, 19 } },
        { "Major Minor Swap  I - i - IV - iv",    0, { 0, 0, 0, 0 }, "Chromatic & borrowed", { 0, 12, 5, 17 } },
        { "Mixo Vamp  I - v - IV - I",            3, { 0, 4, 3, 0 },       "Modal & exotic" },
        { "Lydian Glow  I - II - vii - I",        4, { 0, 1, 6, 0 },       "Modal & exotic" },
        { "Phrygian Minor  i - bII - bIII - bII", 5, { 0, 1, 2, 1 },       "Modal & exotic" },
        { "Minor Drama  i - iv - V7 - i",         2, { 0, 0, 0, 0 }, "Modal & exotic", { 12, 17, 31, 12 } },
        { "Dorian Climb  i - VII - ii - IV",         1, { 0, 6, 1, 3 }, "Deep house" },
        { "Dorian Return  i - v - IV - i",           1, { 0, 4, 3, 0 }, "Deep house" },
        { "Dorian Sway  i - VII - VII - IV",         1, { 0, 6, 6, 3 }, "Deep house" },
        { "Dorian Turn  i - IV - ii - VII",          1, { 0, 3, 1, 6 }, "Deep house" },
        { "Dorian Pull  i - ii - v",                 1, { 0, 1, 4 }, "Deep house" },
        { "Dorian Step Down  i - v - IV - III",      1, { 0, 4, 3, 2 }, "Deep house" },
        { "Dorian Glide  i - VII - IV",              1, { 0, 6, 3 }, "Deep house" },
        { "Dorian Lift  i - IV - ii - IV",           1, { 0, 3, 1, 3 }, "Deep house" },
        { "Dorian Rise  i - ii - v - v",             1, { 0, 1, 4, 4 }, "Deep house" },
        { "Dorian Walk  i - IV - ii - III",          1, { 0, 3, 1, 2 }, "Deep house" },
        { "Dorian Bloom  i - VII - III - IV",        1, { 0, 6, 2, 3 }, "Deep house" },
        { "Dorian Drift  i - III - VII - IV",        1, { 0, 2, 6, 3 }, "Deep house" },
        { "Dorian Hold  i - VII - IV - IV",          1, { 0, 6, 3, 3 }, "Deep house" },
        { "Dorian Stairs  i - ii - III - III",       1, { 0, 1, 2, 2 }, "Deep house" },
        { "Dorian Arrival  III - IV - v - i",        1, { 2, 3, 4, 0 }, "Deep house" },
        { "Dorian Fall  i - VII - v - IV",           1, { 0, 6, 4, 3 }, "Deep house" },
        { "Dorian Echo  i - ii - IV - ii",           1, { 0, 1, 3, 1 }, "Deep house" },
        { "Dorian Fold  IV - III - i",               1, { 3, 2, 0 }, "Deep house" },
    };

    return presets;
}

std::vector<PresetSlot> presetSlots (const ProgressionPreset& preset, int extension)
{
    std::vector<PresetSlot> out;
    if (preset.chords.empty())
    {
        for (int d : preset.degrees)
            out.push_back ({ d, noFreeChord });
        return out;
    }

    for (int code : preset.chords)
    {
        const int semis = code % 12;
        const int kind = code / 12;   // 0 major, 1 minor, 2 dominant
        int degree = -1;
        for (int d = 0; d < 7 && degree < 0; ++d)
        {
            const auto c = buildChord (0, preset.scale, d, (Extension) extension);
            const bool sameKind = kind == 1 ? c.third == 3 && c.fifth == 7
                                : kind == 2 ? c.third == 4 && c.seventh == 10
                                            : c.third == 4 && c.fifth == 7;   // a major chord takes the scale's own 7th
            if (c.rootPc == semis && sameKind)
                degree = d;
        }
        out.push_back ({ std::max (0, degree), degree >= 0 ? noFreeChord : code });
    }
    return out;
}

const char* fillStyleName (int style) noexcept
{
    static const char* const names[numFillStyles] { "Pentatonic run up", "Pentatonic run down", "Chord roll (next chord)",
                                                    "Gospel run (2 beats)", "Chromatic walk-up", "Push stabs", "16th chord chops",
                                                    "Gospel thirds", "Piano glissando", "Blues lick", "Mix (changes every fill)" };
    return names[mod (style, numFillStyles)];
}

int fillStartStep (FillStyle style) noexcept
{
    return style == FillStyle::gospelRun ? 8 : 12;
}

namespace
{
    /** The style a fill bar really plays: "Mix" picks a different one for each fill. */
    FillStyle effectiveFillStyle (FillStyle style, long long bar) noexcept
    {
        if (style != FillStyle::mix)
            return style;

        static constexpr FillStyle rotation[] { FillStyle::pentatonicUp, FillStyle::chordRoll, FillStyle::gospelRun,
                                                FillStyle::pushStabs, FillStyle::chromaticWalk, FillStyle::gospelThirds,
                                                FillStyle::glissando, FillStyle::chordChops, FillStyle::bluesLick };
        const auto n = (long long) (sizeof (rotation) / sizeof (*rotation));
        return rotation[(size_t) (((bar * 5 + 2) % n + n) % n)];
    }

    /** A fill that suits the groove ("Mix" and the evolution's own fill): a busy groove gets short
        fills (stabs, chops, a roll), a sparse one runs and glissandos, the rest something in
        between; a different one on each fill bar. */
    FillStyle contextualFillStyle (const std::string& rightHand, long long bar) noexcept
    {
        int hits = 0;
        for (char c : rightHand)
            hits += (c == 'X' || c == 'x' || c == '>' || c == 'u' || c == 'U' || c == 'd' || c == 'D') ? 1 : 0;
        const double perTwoBars = rightHand.empty() ? 0.0 : hits * 32.0 / (double) rightHand.size();

        static constexpr FillStyle busy[]   { FillStyle::pushStabs, FillStyle::chordChops, FillStyle::chordRoll };
        static constexpr FillStyle sparse[] { FillStyle::pentatonicUp, FillStyle::gospelRun, FillStyle::glissando, FillStyle::chromaticWalk };
        static constexpr FillStyle middle[] { FillStyle::gospelThirds, FillStyle::bluesLick, FillStyle::pentatonicDown, FillStyle::chordRoll };
        auto pick = [bar] (const FillStyle* list, long long n) { return list[(size_t) (((bar * 5 + 2) % n + n) % n)]; };
        if (perTwoBars >= 14.0) return pick (busy, 3);
        if (perTwoBars <= 9.0)  return pick (sparse, 4);
        return pick (middle, 4);
    }

    /** Notes of a scale (given as semitones above the key) between low and high, ascending. */
    NoteSet notesOf (int keyRoot, const int* degrees, int count, int low, int high) noexcept
    {
        bool member[12] {};
        for (int i = 0; i < count; ++i)
            member[mod (keyRoot + degrees[i], 12)] = true;

        NoteSet out;
        for (int n = low; n <= high && out.size < (int) out.notes.size(); ++n)
            if (member[mod (n, 12)])
                out.add (n);
        return out;
    }

    /** The notes of a run over a chord: the key's scale (or `member`), with every chord tone outside
        it taking the place of its neighbours a half step away (a run into a bVI or a chromatic chord
        plays that chord's notes, not the key's), between low and high, ascending. */
    NoteSet notesOverChord (bool member[12], const Chord& chord, int low, int high) noexcept
    {
        bool tone[12] {};
        for (int i = 0; i < chord.tones.size; ++i)
            tone[mod (chord.rootPc + chord.tones[i], 12)] = true;
        for (int pc = 0; pc < 12; ++pc)
            if (tone[pc] && ! member[pc])
            {
                for (int side : { -1, 1 })
                    if (! tone[mod (pc + side, 12)])
                        member[mod (pc + side, 12)] = false;
                member[pc] = true;
            }
        NoteSet out;
        for (int n = low; n <= high && out.size < (int) out.notes.size(); ++n)
            if (member[mod (n, 12)])
                out.add (n);
        return out;
    }

    /** Index of the highest note <= target (0 if none). */
    int indexAtOrBelow (const NoteSet& notes, int target) noexcept
    {
        int idx = 0;
        for (int i = 0; i < notes.size; ++i)
            if (notes[i] <= target)
                idx = i;
        return idx;
    }
}

const char* approachStyleName (int style) noexcept
{
    static const char* const names[numApproachStyles] { "Off", "From above", "From below" };
    return names[mod (style, numApproachStyles)];
}

const char* topLineName (int line) noexcept
{
    static const char* const names[numTopLines] { "Smooth (auto)", "Steady", "Rising", "Falling", "Arch", "House pedal" };
    return names[mod (line, numTopLines)];
}

const char* humanizeStyleName (int style) noexcept
{
    static const char* const names[numHumanizeStyles] { "Even", "Pianist", "Laid-back bass", "Tight bass" };
    return names[mod (style, numHumanizeStyles)];
}

GrooveEngine::HumanizeVoices GrooveEngine::humanizeVoices (int style) noexcept
{
    switch (mod (style, numHumanizeStyles))
    {
        case 1:  return { 1.0, 1.25, 1.0, 0.08, 0.5, 0.7, 0.0 };   // pianist: dynamic chords, singing top note, steady bass
        case 2:  return { 0.8, 0.9, 0.0, 0.0, 1.7, 1.5, 0.6 };     // laid-back bass: behind the beat, looser
        case 3:  return { 1.3, 1.2, 0.0, 0.0, 0.2, 0.4, 0.0 };     // tight bass, loose chords
        default: return { 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0 };    // even
    }
}

const std::vector<BassLine>& bassLines()
{
    // Classic bass-player figures (two bars), written as chord functions so they follow any progression.
    static const std::vector<BassLine> lines {
        { "Disco Octave Gallop", "Disco",  compact ("R.O. R.O. R.O. R.O. R.O. R.O. R.O. 5.OA") },
        { "Disco Octave Push",   "Disco",  compact ("R.O. R.OR .O.R O.RO R.O. R.OR .O.R O.5A") },
        { "Philly Walk",         "Disco",  compact ("R.R. O.5. R.R. O.7. R.R. O.5. 3.5. 6.A.") },
        { "Boogie Line",         "Disco",  compact ("R..3 5..6 7..6 5..3 R..3 5..6 7..6 5.SA") },
        { "Funk Sixteenths",     "Funky",  compact ("R..R ..O. GR.5 ..7. R..R ..O. GR.b 5.SA") },
        { "Slap & Pop",          "Funky",  compact ("R.GO .GR. O.GR 5.O. R.GO .GR. O.G7 O.SA") },
        { "Dead Note Groove",    "Funky",  compact ("RG.R G.O. GRG5 .O.A RG.R G.O. GRG5 .7.A") },
        { "Deep House Root",     "House",  compact ("R=.. ..R. ..R. .... R=.. ..R. ..5. ...A") },
        { "Offbeat House",       "House",  compact ("..R. ..R. ..R. ..O. ..R. ..R. ..R. .5.A") },
        { "Garage Skip",         "House",  compact ("R..R ...O ..R. .5.. R..R ...O ..R. 7.A.") },
        { "Soulful Walk",        "Soulful", compact ("R... 3... 5... A... R... 3... 5... 6.A.") },
        { "Gospel Walk",         "Soulful", compact ("R..5 O..5 3..R 6..A R..5 O..5 7..6 5.SA") },
        // Appended (v2.17): their own rhythms, not the groove's.
        { "Pumping Offbeat Octaves", "House",   compact ("..R. ..O. ..R. ..O. ..R. ..O. ..R. .5O.") },
        { "Chicago Root Drive",      "House",   compact ("R.R. R.R. R.R. R.O. R.R. R.R. R.R. 5.A.") },
        { "Jackin' Skip",            "House",   compact ("R..R ..R. .R.. R.O. R..R ..R. .R.. 5.SA") },
        { "Afro House Groove",       "House",   compact ("R..R .5.. R.R. .O.. R..R .5.. R.R. 7.A.") },
        { "Deep Sub Hold",           "Deep",    compact ("R=== ===. ..R= .... R=== ===. ..5= ...A") },
        { "Deep Seventh Answer",     "Deep",    compact ("R... ..7. ..R. .5.. R... ..7. ..R. .3A.") },
        { "Garage 2-Step Bounce",    "Garage",  compact ("R..O ..R. .5.. R..A R..O ..R. .5.. 7..A") },
        { "Speed Garage Octaves",    "Garage",  compact ("R.RO .R.O R.RO .R.O R.RO .R.O R.RO .5.A") },
        { "Disco Walk Up",           "Disco",   compact ("R.O. R.O. R.O. 3.5. R.O. R.O. 5.6. 7.A.") },
        { "Nu Disco Arpeggio",       "Disco",   compact ("R.3. 5.O. R.3. 5.O. R.3. 5.O. 7.5. 3.A.") },
        { "Funky Octave Skip",       "Funky",   compact ("R..O G.R. O.GR .O.. R..O G.R. O.GR 5.SA") },
        { "Soulful Hold & Push",     "Soulful", compact ("R==. ...5 R==. ..O. R==. ...5 6==. ..A.") },
    };
    return lines;
}

const char* bassModeName (int m) noexcept
{
    static const char* const names[numBassPatternModes] { "Off", "Root", "Octaves", "Root + 5th", "Walking", "Syncopated octaves",
                                                          "Piano stabs (bass in every chord)" };
    m = mod (m, numBassModes);
    if (m == stabsOwnRhythmMode)
        return "Piano stabs (own rhythm)";
    return m < numBassPatternModes ? names[m] : bassLines()[(size_t) bassLineIndex (m)].name;
}

const char* stringsModeName (int m) noexcept
{
    static const char* const names[numStringsModes] { "Off", "Pad", "High pad", "Wide pad", "Stabs with the piano",
                                                      "Offbeat stabs", "Answer the piano", "Pulsing 8ths", "Half-bar pad",
                                                      "Disco octaves", "Rise into chords" };
    return names[std::clamp (m, 0, numStringsModes - 1)];
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
    if (s.playsEditedLoop())
        return std::clamp ((int) std::lround (s.editedLoopLength * 4.0 / 16.0) * 16, 16, 64 * 16);

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
    if (s.variationEvery > 0)
        steps = lcm (steps, 16LL * s.variationEvery);
    if (s.evolveBars > 0)
        steps = lcm (steps, 16LL * s.evolveBars);
    if (s.realism)
        steps = lcm (steps, 4LL * patternLength);   // the phrase of four grooves (two other takes in it)

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

std::string bassLineRhythm (int lineIndex, int length)
{
    const auto& line = bassLines()[(size_t) std::clamp (lineIndex, 0, numBassLines - 1)].line;
    std::string rhythm ((size_t) std::max (0, length), '.');
    for (int i = 0; i < length; ++i)
    {
        const char c = line[(size_t) (i % (int) line.size())];
        rhythm[(size_t) i] = c == '.' ? '.' : c == '=' ? '=' : c == 'G' ? 'g' : (i % 4 == 0 ? 'X' : 'x');
    }
    return rhythm;
}

GroovePattern GrooveEngine::resolvePattern (const GrooveSettings& s)
{
    auto arranged = applySegmentOrder (basePattern (s), s.segmentOrder);
    if (s.sectionVariation != 0)
        arranged = makeVariation (arranged, s.sectionVariation);   // this song section, regenerated
    auto p = addVoicingMovement (applySection (arranged, s.section),
                                 s.section == Section::breakdown ? 0 : s.movement);

    // A bass line from the library has its own rhythm: show that one in the grid.
    if (isBassLine (s.bass) && (s.section == Section::main || s.section == Section::drop))
        p.leftHand = bassLineRhythm (bassLineIndex ((int) s.bass), p.length());

    if (s.grooveBars == 1 && p.length() > 16)
    {
        // One bar: the first bar of the groove, looped.
        p.rightHand.resize (16);
        p.leftHand.resize (16);
        repairHolds (p.rightHand);
        repairHolds (p.leftHand);
    }
    else if (s.grooveBars == 4)
    {
        // Four bars: the groove, then a variation of it as the answer.
        const auto answer = makeVariation (p, s.riffSeed * 31u + (std::uint32_t) s.pattern);
        p.rightHand += answer.rightHand;
        p.leftHand += answer.leftHand;
    }

    return p;
}

int GrooveEngine::patternLength (const GrooveSettings& s) noexcept
{
    const auto& patterns = groovePatterns();
    const int base = s.pattern >= randomPatternIndex() ? 32   // random riffs and the user pattern are two bars
                   : patterns[(size_t) std::clamp (s.pattern, 0, (int) patterns.size() - 1)].length();

    if (s.grooveBars == 1) return std::min (base, 16);
    if (s.grooveBars == 4) return base * 2;
    return base;
}

const GroovePattern& GrooveEngine::patternFor (const GrooveSettings& s)
{
    if (s.pattern != cachedIndex || s.riffSeed != cachedSeed || s.riffDensity != cachedDensity
        || s.section != cachedSection || s.segmentOrder != cachedOrder
        || s.userRight != cachedUserRight || s.userLeft != cachedUserLeft || s.grooveBars != cachedBars
        || s.movement != cachedMovement || s.sectionVariation != cachedSectionVariation || cachedPattern.rightHand.empty()
        || (int) s.bass != cachedBass || s.realism != cachedRealism)   // a library bass line shows its own rhythm
    {
        cachedPattern = resolvePattern (s);
        cachedVariation = makeVariation (cachedPattern, s.riffSeed * 17u + (std::uint32_t) s.pattern);
        // Evolution: the same groove, growing. Each stage keeps what the one before added.
        cachedEvolution[0] = addGhostNotes (cachedPattern, s.riffSeed * 29u + (std::uint32_t) s.pattern);
        cachedEvolution[1] = cachedEvolution[0];
        for (std::uint32_t k = 0; k < 6 && cachedEvolution[1].rightHand == cachedEvolution[0].rightHand; ++k)   // it must change
            cachedEvolution[1] = makeVariation (cachedEvolution[0], s.riffSeed * 37u + (std::uint32_t) s.pattern + k * 101u);
        cachedEvolution[2] = cachedEvolution[1];
        // Realism: two other takes of the same groove, played now and then over a phrase.
        for (std::uint32_t k = 0; k < 2; ++k)
            cachedTakes[k] = s.realism ? makeVariation (cachedPattern, s.riffSeed * 53u + (std::uint32_t) s.pattern + k * 7919u)
                                       : GroovePattern {};
        cachedBass = (int) s.bass;
        cachedRealism = s.realism;
        cachedBars = s.grooveBars;
        cachedMovement = s.movement;
        cachedIndex = s.pattern;
        cachedSeed = s.riffSeed;
        cachedDensity = s.riffDensity;
        cachedSection = s.section;
        cachedSectionVariation = s.sectionVariation;
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
        if (part.bars <= 0)
            continue;   // a section left out of the song

        auto partSettings = s;
        partSettings.section = part.section;
        partSettings.sectionVariation = part.variation;
        if (part.endFill != -1)
        {
            partSettings.forcedFillBar = (long long) std::llround (start / 4.0) + part.bars - 1;
            partSettings.forcedFillStyle = part.endFill;
        }

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

std::vector<NoteEvent> GrooveEngine::renderLoopClip (const GrooveSettings& s, double lengthInBeats)
{
    const auto all = renderClip (s, lengthInBeats + 4.0);
    std::vector<NoteEvent> out, tails;
    out.reserve (all.size());
    std::array<NoteEvent, numTracked> open {};
    std::array<bool, numTracked> isOpen {};
    for (const auto& e : all)
    {
        const auto k = (size_t) trackIndex (e.note, e.part);
        if (e.isNoteOn())
        {
            if (e.ppq >= lengthInBeats)
                continue;
            out.push_back (e);
            open[k] = e;
            isOpen[k] = true;
        }
        else if (isOpen[k])
        {
            isOpen[k] = false;
            if (e.ppq > lengthInBeats + 1.0e-9)
            {
                out.push_back ({ lengthInBeats, e.note, 0, e.part });
                tails.push_back ({ e.ppq - lengthInBeats, e.note, open[k].velocity, e.part });   // ppq: where the tail ends
            }
            else
                out.push_back (e);
        }
    }

    // The tails at the start, unless the loop strikes that note there itself; each ends before the
    // loop plays that note again.
    const auto clipEvents = out.size();
    for (const auto& t : tails)
    {
        double end = t.ppq;
        bool struck = false;
        for (size_t i = 0; i < clipEvents; ++i)
            if (out[i].isNoteOn() && out[i].note == t.note && out[i].part == t.part)
            {
                struck = struck || out[i].ppq < 0.02;
                end = std::min (end, out[i].ppq);
            }
        if (! struck && end > 0.03)
        {
            out.push_back ({ 0.0, t.note, t.velocity, t.part });
            out.push_back ({ end, t.note, 0, t.part });
        }
    }
    std::stable_sort (out.begin(), out.end(), [] (const NoteEvent& a, const NoteEvent& b)
                      { return a.ppq != b.ppq ? a.ppq < b.ppq : (! a.isNoteOn() && b.isNoteOn()); });
    return out;
}

int GrooveEngine::slotForStep (long long step, const GrooveSettings& s) noexcept
{
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);
    return mod (floorDiv (step, std::max (1, s.stepsPerChord)), len);
}

Extension GrooveEngine::sectionExtension (const GrooveSettings& s) noexcept
{
    // The song's sections colour the harmony a little: the intro and the outro play lighter chords
    // (one step simpler), the drop fuller ones (one step richer); the main loop, the break and the
    // build-up play the extension as chosen.
    const auto e = s.extension;
    if (s.section == Section::intro || s.section == Section::outro)
        switch (e)
        {
            case Extension::ninth:                                    return Extension::seventh;
            case Extension::eleventh: case Extension::lushEleventh:
            case Extension::soulThirteenth:                           return Extension::ninth;
            case Extension::sixNine:                                  return Extension::add9;
            default:                                                  return e;
        }
    if (s.section == Section::drop)
        switch (e)
        {
            case Extension::triad:   return Extension::add9;
            case Extension::seventh: return Extension::ninth;
            default:                 return e;
        }
    return e;
}

Chord GrooveEngine::chordForSlot (const GrooveSettings& s, int slot) noexcept
{
    const int i = std::clamp (slot, 0, maxProgressionLength - 1);
    const int free = s.freeChords[(size_t) i];
    if (free >= 0 && free < numFreeChordCodes)
    {
        // minor = Dorian i, major = Ionian I, dominant = Mixolydian I
        const int scale = free >= 24 ? 3 : free >= 12 ? 1 : 0;
        return buildChord ((s.keyRoot + free % 12) % 12, scale, 0, sectionExtension (s));
    }

    return buildChord (s.keyRoot, s.scale, s.progression[(size_t) i], sectionExtension (s));
}

std::string GrooveEngine::numeralForSlot (const GrooveSettings& s, int slot)
{
    const auto chord = chordForSlot (s, slot);
    if (s.freeChords[(size_t) std::clamp (slot, 0, maxProgressionLength - 1)] >= 0)
        return chromaticNumeral (s.keyRoot, chord);

    return romanNumeral (s.progression[(size_t) std::clamp (slot, 0, maxProgressionLength - 1)], chord);
}

GrooveEngine::HarmonyScores GrooveEngine::harmonyScores (const GrooveSettings& s,
                                                         const std::array<Chord, maxProgressionLength>& chords,
                                                         const std::array<VoicedChord, maxProgressionLength>& voiced) noexcept
{
    HarmonyScores h;
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);

    if (len == 1)
    {
        h.slot[0] = h.transition[0] = scaleFit (chords[0], s.keyRoot, s.scale);
        return h;
    }

    for (int i = 0; i < len; ++i)
    {
        const int next = (i + 1) % len;
        h.transition[(size_t) i] = transitionScore (chords[(size_t) i], voiced[(size_t) i],
                                                    chords[(size_t) next], voiced[(size_t) next], s.keyRoot, s.scale);
    }

    for (int i = 0; i < len; ++i)
        h.slot[(size_t) i] = 0.5f * (h.transition[(size_t) ((i + len - 1) % len)] + h.transition[(size_t) i]);

    return h;
}

void GrooveEngine::clearLowRegister (VoicedChord& v, const Chord& chord, bool bassOn) noexcept
{
    // Bass and chords together: down low (below E3) only the root and the 5th sound clear; a 3rd,
    // 7th or extension there muddies the bass, and with the bass on no chord tone goes below it.
    // Those notes move up an octave (a note already there is not doubled).
    constexpr int clearAbove = 52;
    const int bassRoot = 36 + chord.rootPc;
    for (auto* hand : { &v.leftHand, &v.rightHand })
    {
        NoteSet fixed;
        for (int i = 0; i < hand->size; ++i)
        {
            int n = (*hand)[i];
            const int interval = mod (n - chord.rootPc, 12);
            while ((n < clearAbove && interval != 0 && interval != 7) || (bassOn && n < bassRoot))
                n += 12;
            if (bassOn && n == bassRoot)
                continue;   // the bass plays that very note: a unison under it only thickens (and phases)
            if (! fixed.contains (n) && ! (hand == &v.rightHand && v.leftHand.contains (n)))
                fixed.add (n);
        }
        fixed.sort();
        *hand = fixed;
    }
}

void GrooveEngine::keepInRange (VoicedChord& v, int low, int high) noexcept
{
    // Chord range (ADVANCED): a note above the top comes down an octave, one below the bottom goes
    // up (high >= low + 12, so every note finds a place); a note already there is not doubled.
    for (auto* hand : { &v.leftHand, &v.rightHand })
    {
        NoteSet kept;
        for (int i = 0; i < hand->size; ++i)
        {
            int n = (*hand)[i];
            while (n > high) n -= 12;
            while (n < low)  n += 12;
            if (! kept.contains (n) && ! (hand == &v.rightHand && v.leftHand.contains (n)))
                kept.add (n);
        }
        kept.sort();
        *hand = kept;
    }
}

GrooveEngine::HarmonyKey GrooveEngine::harmonyKey (const GrooveSettings& s) noexcept
{
    return { s.keyRoot, s.scale, (int) s.extension, (int) s.voicing, s.centre, s.progressionLength, (int) s.topLine,
             (int) s.section, s.rangeLow, s.rangeHigh, s.bass != BassMode::off ? 1 : 0, s.bassFeel,
             s.progression, s.freeChords, s.inversions };
}

void GrooveEngine::bassRoots (const GrooveSettings& s, const std::array<Chord, maxProgressionLength>& chords,
                              std::array<int, maxProgressionLength>& out) noexcept
{
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);
    if (s.bassFeel <= 0)
    {
        for (int i = 0; i < maxProgressionLength; ++i)
            out[(size_t) i] = 36 + chords[(size_t) std::min (i, len - 1)].rootPc;
        return;
    }

    // The sub: the first root between G1 and F#2, then each one the nearest to the one before
    // (a B to a C goes up a half step, not down a major 7th), kept between E1 and A2.
    auto nearest = [] (int pc, int from)
    {
        int n = from - 6 + mod (pc - (from - 6), 12);
        while (n > 45) n -= 12;
        while (n < 28) n += 12;
        return n;
    };
    out[0] = 31 + mod (chords[0].rootPc - 31, 12);
    for (int i = 1; i < len; ++i)
        out[(size_t) i] = nearest (chords[(size_t) i].rootPc, out[(size_t) i - 1]);
    for (int i = len; i < maxProgressionLength; ++i)
        out[(size_t) i] = out[(size_t) len - 1];
}

const char* bassFeelName (int feel) noexcept
{
    static const char* const names[numBassFeels] { "Classic", "Sub (voice-led)", "Sub + sidechain gaps" };
    return names[mod (feel, numBassFeels)];
}

void GrooveEngine::voiceProgression (const GrooveSettings& s,
                                     std::array<Chord, maxProgressionLength>& chords,
                                     std::array<VoicedChord, maxProgressionLength>& voiced) noexcept
{
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);

    for (int i = 0; i < len; ++i)
        chords[(size_t) i] = chordForSlot (s, i);

    // House pedal: the classic house piano way, one top note that the chords share (usually
    // their 5th, 7th or 3rd), or the smallest step away from it when a chord doesn't have it.
    std::array<int, maxProgressionLength> pedal {};
    if (s.topLine == TopLine::pedal)
    {
        auto inChord = [] (const Chord& c, int note)
        {
            for (int k = 0; k < c.tones.size; ++k)
                if (mod (c.rootPc + c.tones[k] - note, 12) == 0)
                    return true;
            return false;
        };
        auto weight = [] (const Chord& c, int note)
        {
            switch (mod (note - c.rootPc, 12))
            {
                case 7:            return 3.0;   // the 5th: the classic top note
                case 10: case 11:  return 2.5;   // the 7th
                case 3:  case 4:   return 2.5;   // the 3rd
                case 0:  case 2:   return 2.0;   // root, 9th
                default:           return 1.0;
            }
        };

        int best = s.centre + 5;
        double bestScore = -1.0e9;
        for (int note = s.centre; note <= s.centre + 9; ++note)
        {
            double score = -0.15 * std::abs (note - (s.centre + 5));
            for (int i = 0; i < len; ++i)
                score += inChord (chords[(size_t) i], note) ? weight (chords[(size_t) i], note) : -1.5;
            if (score > bestScore) { bestScore = score; best = note; }
        }
        for (int i = 0; i < len; ++i)
        {
            int target = best;
            for (int d = 1; d <= 3 && ! inChord (chords[(size_t) i], target); ++d)
                target = inChord (chords[(size_t) i], best - d) ? best - d : inChord (chords[(size_t) i], best + d) ? best + d : best;
            pedal[(size_t) i] = target;
        }
    }

    // Top line: the melody note each chord's top voice aims for.
    auto topTarget = [&] (int i) -> int
    {
        const int base = s.centre + 8;
        switch (s.topLine)
        {
            case TopLine::steady:  return base;
            case TopLine::rising:  return base - 4 + (len > 1 ? 8 * i / (len - 1) : 0);
            case TopLine::falling: return base + 4 - (len > 1 ? 8 * i / (len - 1) : 0);
            case TopLine::arch:    return base - 3 + 3 * std::min (i, len - 1 - i);
            case TopLine::pedal:   return pedal[(size_t) i];
            case TopLine::automatic:
            default:               return -1;
        }
    };

    auto pass = [&] (int first, const NoteSet* previous)
    {
        for (int i = first; i < len; ++i)
        {
            voiced[(size_t) i] = voiceChord (chords[(size_t) i], s.voicing, s.centre, previous, topTarget (i));

            // Manual inversion of this slot (the chord's ▲▼ buttons).
            const int inversion = std::clamp (s.inversions[(size_t) i], -3, 3);
            if (inversion != 0)
            {
                invert (voiced[(size_t) i].rightHand, inversion);
                while (voiced[(size_t) i].rightHand.highest() > 100) invert (voiced[(size_t) i].rightHand, -voiced[(size_t) i].rightHand.size);
                while (voiced[(size_t) i].rightHand.lowest() < 28)   invert (voiced[(size_t) i].rightHand, voiced[(size_t) i].rightHand.size);
            }

            // The low register cleared first, then the range: a note lifted out of the mud never
            // ends up above the chords' highest note.
            clearLowRegister (voiced[(size_t) i], chords[(size_t) i], s.bass != BassMode::off);
            keepInRange (voiced[(size_t) i], s.rangeLow, std::max (s.rangeHigh, s.rangeLow + 12));

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
void GrooveEngine::collectStep (long long step, const GrooveSettings& s, const GroovePattern& normalPattern, const GroovePattern& variation,
                                const std::array<Chord, maxProgressionLength>& chords,
                                const std::array<VoicedChord, maxProgressionLength>& voiced)
{
    // Every N bars the last bar plays a variation of the groove (the turnaround).
    const bool varied = s.variationEvery > 0 && mod (floorDiv (step, 16), s.variationEvery) == s.variationEvery - 1
                     && variation.length() == normalPattern.length();

    // Evolution: over 8 or 16 bars the groove grows in four stages: as written, + ghost notes,
    // + a variation (moved hits, a push into the next bar), and the last one ends with a fill.
    const int evolveStage = s.evolveBars > 0 ? (int) (mod (floorDiv (step, 16), s.evolveBars) * 4 / s.evolveBars) : 0;
    const GroovePattern* chosen = varied ? &variation : &normalPattern;
    if (evolveStage > 0 && cachedEvolution[(size_t) evolveStage - 1].length() == normalPattern.length())
        chosen = &cachedEvolution[(size_t) evolveStage - 1];
    // Realism: over a phrase of four grooves the third and the fourth are other takes of it, so a
    // long clip never repeats the same loop note for note.
    if (s.realism && ! varied && evolveStage == 0 && normalPattern.length() > 0)
        if (const int take = mod (floorDiv (step, normalPattern.length()), 4) - 2;
            take >= 0 && cachedTakes[(size_t) take].length() == normalPattern.length())
            chosen = &cachedTakes[(size_t) take];
    const GroovePattern& pattern = *chosen;
    const double evolveLift = evolveStage == 3 ? 1.06 : evolveStage == 2 ? 1.03 : 1.0;   // a little more energy

    const int pos = mod (step, pattern.length());

    const double swingDelay = (step % 2 != 0) ? std::clamp (s.swing, 0.0f, 1.5f) * stepLength / 3.0 : 0.0;
    const double baseTime = (double) step * stepLength + swingDelay;
    const double human = std::clamp (s.humanize, 0.0f, 1.0f);
    const int seedSalt = (int) (s.humanizeSeed % 100003u) * 7919;
    const auto hv = humanizeVoices (s.humanizeStyle);

    // Realism: the timing goes both ways around the beat, on a slow drift from bar to bar (a player
    // pushes a little, then sits back), and the bass (salts 90+) stays tighter than the chords.
    const long long barNow = floorDiv (step, 16);
    const double drift = s.realism ? 0.012 * 2.0 * ((noise (barNow, seedSalt + 777) * (1.0 - mod (step, 16) / 16.0)
                                                      + noise (barNow + 1, seedSalt + 777) * (mod (step, 16) / 16.0)) - 0.5)
                                   : 0.0;
    auto hitTime = [&] (int salt, double amount)
    {
        if (! s.realism)
            return baseTime + amount * 0.03 * noise (step, seedSalt + salt);
        const double a = salt >= 90 ? amount * 0.6 : amount;
        return std::max (step >= 0 ? 0.0 : -1.0e9, baseTime + a * (0.03 * (noise (step, seedSalt + salt) - 0.5) + drift));
    };

    auto hitVelocity = [&] (char symbol, double scale, int salt, double amount = -1.0)
    {
        if (amount < 0.0) amount = human * hv.chordVel;
        double level = s.velocity * (symbol == 'X' || symbol == 'U' || symbol == 'D' ? 1.0 : symbol == '>' ? 0.97 : symbol == 'g' ? 0.55 : 0.9);
        // Realism: the place in the bar shapes the touch (the beat strongest, then the "&", then
        // the 16ths in between), and the phrase of four bars breathes (the fourth leans forward).
        if (s.realism)
        {
            static constexpr double place[4] { 1.0, 0.92, 0.96, 0.91 };
            static constexpr double phrase[4] { 1.0, 0.97, 0.99, 1.03 };
            level *= place[mod (step, 4)] * phrase[mod (barNow, 4)];
        }

        // A velocity set by hand on this step of the user pattern (chord stabs: salts 60..89,
        // bass: 90..99), relative to the Velocity control (118 = as set).
        // The levels are stored per step of the user pattern as written (before the A-D order).
        if (s.pattern == userPatternIndex() && salt >= 60 && salt < 100 && ! (salt >= 90 && isBassLine (s.bass)))
        {
            const int segLength = userPatternLength / numSegments;
            const int shown = mod (pos, userPatternLength);
            const int at = std::clamp (s.velocityOrder[(size_t) (shown / segLength)], 0, numSegments - 1) * segLength + shown % segLength;
            const int index = std::clamp (s.segmentOrder[(size_t) (at / segLength)], 0, numSegments - 1) * segLength + at % segLength;
            const auto& hand = salt >= 90 ? s.userVelLeft : s.userVelRight;
            if (const int set = hand[(size_t) index]; set > 0)
                level = set * s.velocity / 118.0;
        }

        double v = level * scale * sectionVelocityScale (s.section) * evolveLift;
        if (s.section == Section::buildup)   // crescendo through the build-up
            v *= 0.72 + 0.4 * (double) pos / (double) std::max (1, pattern.length() - 1);
        v += amount * 18.0 * (noise (step, seedSalt + salt + 7) - 0.5) * 2.0;
        return std::clamp ((int) std::lround (v), 1, 127);
    };

    auto durationFor = [&] (const std::string& line)
    {
        // A tied push rings slightly past the next hit, which then extends it.
        if (pushTiesIntoNextHit (line, pos))
            return (holdLength (line, pos) + 1) * stepLength + 0.06;

        const double gate = std::clamp (s.gate, 0.05f, 1.0f);
        if (const int hold = holdLength (line, pos); hold > 0 || line[(size_t) pos] == 'g')
            return std::max (0.04, (hold + (hold > 0 ? gate : 0.6 * gate)) * stepLength);

        // Like a pianist, a hit rings into the space before the next one: Gate is the share of
        // that space it fills (a stab three 16ths before the next rings about two 16ths).
        const int n = (int) line.size();
        int gap = 1;
        while (gap < n && ! isHit (line[(size_t) ((pos + gap) % n)]))
            ++gap;
        const double steps = std::min ({ gate * gap, 4.0, gap - 0.08 });
        return std::max (0.04, std::max (steps, std::min (gate, 0.9)) * stepLength);
    };

    // Which chord a hit uses: the current one, or the next one for a push.
    auto slotFor = [&] (char symbol)
    {
        if (symbol != '>')
            return slotForStep (step, s);

        const long long grid = std::min (s.stepsPerChord, 16);
        return slotForStep ((floorDiv (step, grid) + 1) * grid, s);
    };

    // Fill: the end of every N bars becomes a pick-up into the next chord.
    const long long bar = floorDiv (step, 16);
    const int stepInBar = mod (step, 16);
    auto fillStyle = effectiveFillStyle (s.fillStyle, bar);
    bool fillBar = s.fillEveryBars > 0 && sectionAllowsFills (s.section) && mod (bar, s.fillEveryBars) == s.fillEveryBars - 1;
    const bool evolutionFill = s.evolveBars > 0 && sectionAllowsFills (s.section) && mod (bar, s.evolveBars) == s.evolveBars - 1;
    if (evolutionFill)
        fillBar = true;   // the evolution's last bar leads into the next round
    // "Mix", and the evolution's fill when the fills are off: a fill that suits this groove.
    if (s.fillStyle == FillStyle::mix || (evolutionFill && s.fillEveryBars == 0))
        fillStyle = contextualFillStyle (normalPattern.rightHand, bar);
    if (s.forcedFillBar >= 0)
    {
        // A song section with its own fill: only on its last bar.
        fillBar = bar == s.forcedFillBar && s.forcedFillStyle >= 0;
        if (fillBar)
            fillStyle = s.forcedFillStyle == (int) FillStyle::mix
                            ? contextualFillStyle (normalPattern.rightHand, bar)
                            : effectiveFillStyle ((FillStyle) std::clamp (s.forcedFillStyle, 0, numFillStyles - 1), bar);
    }
    const int fillStart = fillStartStep (fillStyle);
    const bool inFill = fillBar && stepInBar >= fillStart;
    const size_t pendingBeforeFill = pending.size();

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
        const int k = stepInBar - fillStart;
        const int lastK = 15 - fillStart;
        const double len = (k == lastK ? 2.0 : std::clamp (s.gate, 0.3f, 1.0f)) * stepLength;
        const int vel = hitVelocity ('x', k == lastK ? 1.0 : 0.85, 32);

        const long long nextBarStep = (bar + 1) * 16;
        const auto& nextChord = voiced[(size_t) slotForStep (nextBarStep, s)].rightHand;
        const auto& thisChord = voiced[(size_t) slotForStep (step, s)].rightHand;
        const int target = nextChord.size > 0 ? nextChord.highest() : s.centre + 7;   // where the fill lands
        const auto& scale = scaleIntervals (s.scale);
        bool inKey[12] {};
        for (int d : scale)
            inKey[mod (s.keyRoot + d, 12)] = true;
        const auto scaleRun = notesOverChord (inKey, chords[(size_t) slotForStep (nextBarStep, s)], target - 26, target + 14);
        const int iTarget = indexAtOrBelow (scaleRun, target);
        const double t = hitTime (31, human * hv.chordTime);

        // Top notes of a chord, as a lighter voicing for fast hits.
        auto topNotes = [] (const NoteSet& chord, int count, auto&& add)
        {
            for (int i = std::max (0, chord.size - count); i < chord.size; ++i)
                add (chord[i], i);
        };

        if (fillStyle == FillStyle::gospelRun)
        {
            // Eight 16ths up the scale, ending on the note just below where the next chord lands.
            const int idx = std::clamp (iTarget - 8 + k, 0, scaleRun.size - 1);
            pending.push_back ({ t, scaleRun[idx], hitVelocity ('x', 0.7 + 0.04 * k, 32), std::clamp (s.gate, 0.3f, 0.9f) * stepLength });
        }
        else if (fillStyle == FillStyle::chromaticWalk)
        {
            // Four chromatic steps up into the top note of the next chord.
            pending.push_back ({ t, target - 4 + k, hitVelocity ('x', 0.75 + 0.07 * k, 32), 0.8 * stepLength });
        }
        else if (fillStyle == FillStyle::pushStabs)
        {
            // Short stab of this chord, ghost, then the next chord pushed in on the "&" of 4.
            if (k == 0) topNotes (thisChord, 4, [&] (int n, int i) { pending.push_back ({ t + 0.004 * i, n, hitVelocity ('x', 0.85, 70 + i), 0.5 * stepLength }); });
            if (k == 1) topNotes (thisChord, 2, [&] (int n, int i) { pending.push_back ({ t, n, hitVelocity ('g', 1.0, 70 + i), 0.4 * stepLength }); });
            if (k == 2) topNotes (nextChord, 4, [&] (int n, int i) { pending.push_back ({ t + 0.004 * i, n, hitVelocity ('X', 1.0, 70 + i), 1.4 * stepLength }); });
        }
        else if (fillStyle == FillStyle::chordChops)
        {
            // Four 16th chops of this chord with a crescendo (light voicing: top three notes).
            topNotes (thisChord, 3, [&] (int n, int i)
            {
                pending.push_back ({ t + 0.003 * i, n, hitVelocity ('x', 0.6 + 0.12 * k, 70 + i), 0.5 * stepLength });
            });
        }
        else if (fillStyle == FillStyle::gospelThirds)
        {
            // Parallel thirds walking down the scale onto the next chord.
            const int idx = std::clamp (iTarget + 4 - k, 2, scaleRun.size - 1);
            pending.push_back ({ t, scaleRun[idx], vel, len * 0.8 });
            pending.push_back ({ t, scaleRun[idx - 2], hitVelocity ('x', 0.8, 33), len * 0.8 });
        }
        else if (fillStyle == FillStyle::glissando)
        {
            // Eight 32nd notes sweeping up the scale.
            for (int half = 0; half < 2; ++half)
            {
                const int idx = std::clamp (iTarget - 8 + 2 * k + half, 0, scaleRun.size - 1);
                pending.push_back ({ t + half * stepLength * 0.5, scaleRun[idx], hitVelocity ('x', 0.65 + 0.05 * (2 * k + half), 34 + half),
                                     (k == lastK && half == 1 ? 1.0 : 0.45) * stepLength });
            }
        }
        else if (fillStyle == FillStyle::bluesLick)
        {
            // Down the blues scale (minor pentatonic + b5), the classic house piano lick.
            static constexpr int blues[] { 0, 3, 5, 6, 7, 10 };
            const auto lick = notesOf (s.keyRoot, blues, 6, target - 14, target + 8);
            const int start = indexAtOrBelow (lick, target + 5);
            const int idx = std::clamp (start - k, 0, lick.size - 1);
            pending.push_back ({ t, lick[idx], vel, len });
        }
        else if (fillStyle == FillStyle::chordRoll)
        {
            // A flowing arpeggio of the NEXT chord: eight 32nd notes sweeping up from an
            // octave below, the last one just before the chord lands on the downbeat.
            if (nextChord.size > 0)
            {
                NoteSet sweep;
                for (int i = 0; i < nextChord.size; ++i) sweep.add (nextChord[i] - 12);
                for (int i = 0; i < nextChord.size; ++i) sweep.add (nextChord[i]);
                sweep.sort();

                for (int half = 0; half < 2; ++half)
                {
                    const int idx = std::clamp (sweep.size - 8 + 2 * k + half, 0, sweep.size - 1);
                    const bool lastNote = k == lastK && half == 1;
                    pending.push_back ({ t + half * stepLength * 0.5, sweep[idx],
                                         hitVelocity ('x', 0.62 + 0.05 * (2 * k + half), 31 + half),
                                         (lastNote ? 1.2 : 0.55) * stepLength });
                }
            }
        }
        else
        {
            const auto& rh = voiced[(size_t) slotForStep (step, s)].rightHand;
            const int top = rh.size > 0 ? rh.highest() : s.centre + 7;
            bool inPentatonic[12] {};
            const auto octave = pentatonicNotes (s.keyRoot, s.scale, 60, 71);
            for (int i = 0; i < octave.size; ++i)
                inPentatonic[mod (octave[i], 12)] = true;
            const auto run = notesOverChord (inPentatonic, chords[(size_t) slotForStep (step, s)], s.centre - 5, s.centre + 19);

            if (run.size > 0)
            {
                int iTop = 0;
                for (int i = 0; i < run.size; ++i)
                    if (run[i] <= top)
                        iTop = i;

                const int idx = fillStyle == FillStyle::pentatonicDown ? iTop + 2 - k : iTop - 2 + k;
                pending.push_back ({ hitTime (31, human * hv.chordTime), run[std::clamp (idx, 0, run.size - 1)], vel, len });
            }
        }
    }
    if (inFill)
        for (size_t k = pendingBeforeFill; k < pending.size(); ++k)
            pending[k].part = Part::fill;

    if (inFill)
    {
        // the fill replaces the right hand on this step
    }
    else if (approachHere && canApproach (pattern.rightHand))
    {
        const auto& next = voiced[(size_t) slotNext];
        const double t = hitTime (1, human * hv.chordTime);
        const double roll = std::min (std::clamp (s.strum, 0.0f, 1.0f) * 0.05 * std::max (30.0, s.bpm) / 60.0, approachLength * 0.4)
                          / std::max (1, next.rightHand.size - 1);
        for (int i = 0; i < next.rightHand.size; ++i)
            pending.push_back ({ t + i * roll, next.rightHand[i] + approachShift, hitVelocity ('x', 0.8, 60 + i), approachLength });
    }
    else
    {
        char symbol = pattern.rightHand[(size_t) pos];
        // Realism: now and then a plain hit off the beat is left out (a pianist doesn't play every
        // stab of a loop every time), never one a held note or a push leads into.
        if (s.realism && symbol == 'x' && mod (pos, 4) != 0 && ! isTiedFromPush (pattern.rightHand, pos)
            && pattern.rightHand[(size_t) mod (pos + 1, pattern.length())] != '=' && noise (step, seedSalt + 313) < 0.05)
            symbol = '.';
        // ...and the last stab of a four-bar phrase lets its top note answer, a step up.
        if (s.realism && symbol == 'x' && mod (barNow, 4) == 3 && mod (step, 16) >= 8 && s.topLine != TopLine::pedal)
        {
            bool last = true;
            for (int k = 1; k < 16 - mod (step, 16); ++k)
                last = last && ! isHit (pattern.rightHand[(size_t) mod (pos + k, pattern.length())]);
            if (last)
                symbol = 'u';
        }
        if (isHit (symbol))
        {
            // 'u' / 'd' (and accented 'U' / 'D'): the top note steps to a neighbour and back.
            // With the house pedal top line the top note holds still, as in house piano
            // loops: only the accented moves (U / D) still step away from it.
            auto v = voiced[(size_t) slotFor (symbol)];
            const bool pedal = s.topLine == TopLine::pedal;

            // The hook of a pedal loop: every second bar, its last stab (late in the bar) steps up
            // (or down) to the next note of the scale, then the next bar comes back to the pedal note. Where
            // house piano moves its top note, it is mostly there, and by a step.
            bool hook = false;
            if (pedal && (symbol == 'x' || symbol == 'X' || symbol == 'u' || symbol == 'd') && mod (bar, 2) == 1 && pos % 16 >= 8
                && (s.section == Section::main || s.section == Section::drop) && ! isTiedFromPush (pattern.rightHand, pos))
            {
                hook = true;
                const int barEnd = (pos / 16 + 1) * 16;
                for (int i = pos + 1; i < barEnd && i < (int) pattern.rightHand.size(); ++i)
                    if (isHit (pattern.rightHand[(size_t) i]) && pattern.rightHand[(size_t) i] != 'g' && pattern.rightHand[(size_t) i] != '>')
                        hook = false;   // (a push plays the next chord: the hook goes on the stab before it)
            }

            // (a 'd' written on that stab makes the hook step down instead)
            const bool moveUp = symbol == 'U' || (symbol == 'u' && ! pedal) || (hook && symbol != 'd');
            const bool moveDown = symbol == 'D' || (symbol == 'd' && (! pedal || hook));
            if ((moveUp || moveDown) && v.rightHand.size > 0)
            {
                // A neighbour note, the house piano way: only the top voice steps to the
                // next note of the scale (up or down) and comes back; the chord under it stays.
                const auto& scale = scaleIntervals (s.scale);
                auto inScale = [&] (int note)
                {
                    return std::find (scale.begin(), scale.end(), mod (note - s.keyRoot, 12)) != scale.end();
                };
                const int top = v.rightHand.highest();
                const int dir = moveUp ? 1 : -1;
                int neighbour = top + dir;
                while (! inScale (neighbour) && std::abs (neighbour - top) < 3)
                    neighbour += dir;
                const int below = v.rightHand.size > 1 ? v.rightHand[v.rightHand.size - 2] : -1;
                if (neighbour != below && neighbour > below)
                {
                    v.rightHand.removeAt (v.rightHand.size - 1);
                    v.rightHand.add (neighbour);
                }
                v.leftHand = {};
            }
            const double len = durationFor (pattern.rightHand);
            const double t = hitTime (1, human * hv.chordTime);
            const bool tie = isTiedFromPush (pattern.rightHand, pos);

            // Strum: roll the chord upwards, up to ~1/32 note between the notes.
            // A pianist's touch: ghost notes are just the top two notes, and a hit right after
            // another one (fast 16th chops) uses the top three, so busy riffs stay light.
            const bool fastRepeat = isHit (pattern.rightHand[(size_t) mod (pos - 1, pattern.length())]);
            const int firstNote = symbol == 'g' ? std::max (0, v.rightHand.size - 2)
                                : fastRepeat    ? std::max (0, v.rightHand.size - 3) : 0;

            // Strum ("Arpeggiato"): a real-time roll (up to ~90 ms at 100%, whatever the tempo),
            // slightly accelerating like a pianist's hand, never longer than half the note, and
            // never on ghost notes. Up, down, or alternating (up on the beat, down off it).
            const int count = v.rightHand.size - firstNote;
            const double rollBeats = symbol == 'g' ? 0.0
                                   : std::min (std::clamp (s.strum, 0.0f, 1.0f) * 0.09 * std::max (30.0, s.bpm) / 60.0, len * 0.45);
            const bool rollDown = s.strumDirection == 1 || (s.strumDirection == 2 && mod (pos, 4) != 0);
            auto rollOffset = [&] (int k)
            {
                if (count < 2) return 0.0;
                const int order = rollDown ? count - 1 - k : k;
                return rollBeats * std::pow ((double) order / (double) (count - 1), 0.8);
            };

            for (int i = firstNote; i < v.rightHand.size; ++i)
            {
                // Pianist: the top note sings, a hair early and a little louder.
                const bool top = i == v.rightHand.size - 1;
                // (never before the start of the song: a clip's first top note would be lost)
                pending.push_back ({ std::max (step >= 0 ? 0.0 : -1.0e9, t + 0.004 * human * hv.chordTime * noise (step, seedSalt + 50 + i) + rollOffset (i - firstNote)
                                                        - (top ? human * hv.topLead * 0.02 : 0.0)),
                                     v.rightHand[i],
                                     hitVelocity (symbol, (1.0 - 0.03 * (v.rightHand.size - 1 - i)) * (top ? 1.0 + hv.topAccent : 1.0), 60 + i),
                                     len, tie });
            }

            if (s.octaveTop && v.rightHand.size > 0 && v.rightHand.highest() + 12 <= 108)
                pending.push_back ({ t + (rollDown ? 0.0 : rollBeats), v.rightHand.highest() + 12,
                                     hitVelocity (symbol, 0.8, 70), len, tie });

            if (firstNote == 0)
                for (int i = 0; i < v.leftHand.size; ++i)
                    pending.push_back ({ t, v.leftHand[i], hitVelocity (symbol, 0.85, 80 + i), len, tie });

            // Strings stabs: the chord hits doubled by the strings (not the ghost notes).
            if (s.strings == StringsMode::stabs && symbol != 'g')
            {
                const auto& strings = cachedStrings[(size_t) slotFor (symbol)];
                for (int i = 0; i < strings.size; ++i)
                    pending.push_back ({ t, strings[i], hitVelocity (symbol, 0.78, 60 + i), len, tie, Part::strings });
            }
        }
        else if (s.strings == StringsMode::answerStabs && pos % 2 == 0 && symbol != '=')
        {
            // Strings answering the piano: a short chord on an 8th where the piano rests (and
            // doesn't play right before or after), so they fill the gaps.
            const int n = pattern.length();
            const char before = pattern.rightHand[(size_t) mod (pos - 1, n)], after = pattern.rightHand[(size_t) mod (pos + 1, n)];
            if (! isHit (before) && ! isHit (after) && before != '=')
            {
                const auto& strings = cachedStrings[(size_t) slotForStep (step, s)];
                const double t = hitTime (5, human * hv.chordTime);
                for (int i = 0; i < strings.size; ++i)
                    pending.push_back ({ t, strings[i], hitVelocity ('x', 0.72, 60 + i), 1.6 * stepLength, false, Part::strings });
            }
        }
    }

    // ---- left hand (bass) ----
    auto bassMode = s.bass;
    if (isBassLine (bassMode))
    {
        // Lines play in the main groove and the drop; the build-up pumps roots like the groove's
        // bass, and intro, outro and breakdown have no bass anyway.
        if (s.section == Section::main || s.section == Section::drop)
        {
            collectBassLine (step, s, chords, bassLines()[(size_t) std::clamp (bassLineIndex ((int) bassMode), 0, numBassLines - 1)].line);
            return;
        }
        bassMode = BassMode::root;
    }
    if (s.section == Section::drop && bassMode == BassMode::root)
        bassMode = BassMode::octaves;    // the drop hits harder

    if (bassMode != BassMode::off && approachHere && canApproach (pattern.leftHand))
    {
        const int target = cachedBassRoots[(size_t) slotNext];
        pending.push_back ({ (hitTime (90, human * hv.bassTime) + human * hv.bassLate * 0.02), target + approachShift, hitVelocity ('x', 0.8, 91, human * hv.bassVel), approachLength, false, Part::bass });
    }
    else if (bassMode == BassMode::withChords)
    {
        // House piano stabs: the left hand plays the root with every chord (one big stab, bass
        // included), plus its own hits only where they don't crowd a chord (a 16th either side
        // would sound like a flam, not a groove).
        auto rhStabAt = [&] (int i)
        {
            const auto n = (int) pattern.rightHand.size();
            const char r = pattern.rightHand[(size_t) ((i % n + n) % n)];
            return isHit (r) && r != 'g';
        };
        auto lhHitAt = [&] (int i)
        {
            const char l = pattern.leftHand[(size_t) i];
            return isHit (l) && ! rhStabAt (i) && ! rhStabAt (i - 1) && ! rhStabAt (i + 1);
        };
        // No new bass note on a fast chop (a stab right after another one) unless it is an
        // accent or a push: the bass of the stab before keeps ringing (as stabBassLine shows).
        auto bassUnder = [&] (int i)
        {
            const auto n = (int) pattern.rightHand.size();
            const char r = pattern.rightHand[(size_t) ((i % n + n) % n)];
            return rhStabAt (i) && ! (rhStabAt (i - 1) && ! (r == 'X' || r == 'U' || r == 'D' || r == '>'));
        };
        if (lhHitAt (pos) || bassUnder (pos))
        {
            // Both hands' hits on one line, so a note never rings over the next stab.
            auto& line = stabLine;
            line = pattern.leftHand;
            for (int i = 0; i < (int) line.size() && i < (int) pattern.rightHand.size(); ++i)
            {
                const char r = pattern.rightHand[(size_t) i];
                if (bassUnder (i))
                    line[(size_t) i] = r == '>' ? 'x' : (r == 'u' || r == 'd') ? 'x' : (r == 'U' || r == 'D') ? 'X' : r;
                else if (rhStabAt (i))
                    line[(size_t) i] = '.';
                else if (isHit (line[(size_t) i]) && ! lhHitAt (i))
                    line[(size_t) i] = '.';
            }
            const char symbol = line[(size_t) pos];
            const char rh = pattern.rightHand[(size_t) pos];
            const int root = cachedBassRoots[(size_t) slotFor (rh == '>' ? '>' : symbol)];
            const double t = hitTime (90, human * hv.bassTime) + human * hv.bassLate * 0.02;
            const double len = durationFor (line);
            const bool tie = isTiedFromPush (line, pos);
            pending.push_back ({ t, root, hitVelocity (symbol, 0.92, 91, human * hv.bassVel), len, tie, Part::bass });

            // The left hand plays octaves on the strong stabs (the accented one and the push),
            // a single root on the others, like a pianist's left hand.
            if (symbol == 'X' || symbol == 'U' || symbol == 'D' || rh == '>')
                pending.push_back ({ t, root + 12, hitVelocity (symbol, 0.85, 92, human * hv.bassVel), len, tie, Part::bass });
        }
    }
    else if ((int) bassMode == stabsOwnRhythmMode)
    {
        // The piano-stab bass on the bass row's own rhythm: octaves on the accents and pushes.
        const char symbol = pattern.leftHand[(size_t) pos];
        if (isHit (symbol))
        {
            auto& line = stabLine;
            line = pattern.leftHand;
            for (auto& c : line)
                if (c == '>') c = 'x';
            const int root = cachedBassRoots[(size_t) slotFor (symbol)];
            const double t = hitTime (90, human * hv.bassTime) + human * hv.bassLate * 0.02;
            const double len = durationFor (line);
            const bool tie = isTiedFromPush (line, pos);
            const char velocitySymbol = symbol == '>' ? 'x' : symbol;
            pending.push_back ({ t, root, hitVelocity (velocitySymbol, 0.92, 91, human * hv.bassVel), len, tie, Part::bass });
            if (symbol == 'X' || symbol == '>')
                pending.push_back ({ t, root + 12, hitVelocity (velocitySymbol, 0.85, 92, human * hv.bassVel), len, tie, Part::bass });
        }
    }
    else if (bassMode != BassMode::off)
    {
        const char symbol = pattern.leftHand[(size_t) pos];
        if (isHit (symbol))
        {
            const int root = cachedBassRoots[(size_t) slotFor (symbol)];
            // The chord's own 5th (a b5 on a half-diminished chord), not always a perfect one.
            const int fifthAbove = chords[(size_t) slotFor (symbol)].fifth >= 0 ? chords[(size_t) slotFor (symbol)].fifth : 7;
            const double len = durationFor (pattern.leftHand);
            const double t = (hitTime (90, human * hv.bassTime) + human * hv.bassLate * 0.02);
            const int vel = hitVelocity (symbol, 0.9, 91, human * hv.bassVel);
            const bool tie = isTiedFromPush (pattern.leftHand, pos);

            // Steady 8ths (x.x.) bounce between root and octave, like a disco / house bass player.
            const auto& lh = pattern.leftHand;
            const int n = (int) lh.size();
            const bool octaveBounce = bassMode == BassMode::root && symbol != '>' && mod (pos, 4) == 2
                                   && isHit (lh[(size_t) mod (pos - 2, n)]) && lh[(size_t) mod (pos - 1, n)] == '.';

            int note = octaveBounce ? root + 12 : root;
            int noteVel = octaveBounce ? (int) (vel * 0.9) : vel;

            if (bassMode == BassMode::walking && symbol != '>')
            {
                // Root, 5th, octave, 5th on the beats; on the last beat before a chord
                // change, a chromatic step into the next root.
                const int inChord = (int) (((step % s.stepsPerChord) + s.stepsPerChord) % s.stepsPerChord);
                const int nextSlot = slotForStep (step + (s.stepsPerChord - inChord), s);
                const int walk[4] { 0, fifthAbove, 12, fifthAbove };
                note = root + walk[(inChord / 4) % 4];
                if (s.stepsPerChord - inChord <= 4 && nextSlot != slotForStep (step, s))
                {
                    int target = cachedBassRoots[(size_t) nextSlot];
                    if (target - root > 6)  target -= 12;   // the next root in the octave closest to this one
                    if (root - target > 6)  target += 12;
                    note = target + (target - 1 >= root ? -1 : 1);
                }
                noteVel = (int) (vel * (inChord == 0 ? 1.0 : 0.85));
            }
            else if (bassMode == BassMode::syncopated && symbol != '>')
            {
                // Offbeats jump up an octave: the pumping house bass.
                if (mod (pos, 4) >= 2)
                {
                    note = root + 12;
                    noteVel = (int) (vel * 0.9);
                }
            }

            pending.push_back ({ t, note, noteVel, len, tie, Part::bass });
            if (bassMode == BassMode::octaves)    pending.push_back ({ t, root + 12, (int) (vel * 0.85), len, tie, Part::bass });
            if (bassMode == BassMode::rootFifth)  pending.push_back ({ t, root + fifthAbove, (int) (vel * 0.85), len, tie, Part::bass });
        }
    }
}

void GrooveEngine::collectBassLine (long long step, const GrooveSettings& s, const std::array<Chord, maxProgressionLength>& chords,
                                    const std::string& line)
{
    const int n = (int) line.size();
    const int pos = mod (step, n);
    const char symbol = line[(size_t) pos];
    if (symbol == '.' || symbol == '=')
        return;

    const double swingDelay = (step % 2 != 0) ? std::clamp (s.swing, 0.0f, 1.5f) * stepLength / 3.0 : 0.0;
    const double human = std::clamp (s.humanize, 0.0f, 1.0f);
    const int seedSalt = (int) (s.humanizeSeed % 100003u) * 7919;
    const auto hv = humanizeVoices (s.humanizeStyle);
    // Realism: around the beat (tighter than the chords) instead of only behind it.
    const double timing = s.realism ? 0.6 * 0.03 * (noise (step, seedSalt + 90) - 0.5) : 0.03 * noise (step, seedSalt + 90);
    const double t = std::max (step >= 0 ? 0.0 : -1.0e9, (double) step * stepLength + swingDelay + human * hv.bassTime * timing
                                    + human * hv.bassLate * 0.02);

    const int slot = slotForStep (step, s);
    const auto& chord = chords[(size_t) slot];
    const int root = cachedBassRoots[(size_t) slot];

    // The next chord's root, in the octave closest to this one.
    const int inChord = (int) (((step % s.stepsPerChord) + s.stepsPerChord) % s.stepsPerChord);
    const int nextSlot = slotForStep (step + (s.stepsPerChord - inChord), s);
    int target = cachedBassRoots[(size_t) nextSlot];
    if (target - root > 6)  target -= 12;
    if (root - target > 6)  target += 12;
    // An approach only leads somewhere when the chord changes within the beat: else it would be a
    // chromatic note against the chord still playing (the 5th, or the octave, instead).
    const bool changeSoon = s.stepsPerChord - inChord <= 4 && nextSlot != slot;

    // The chord's own 5th and 6th: a b5 on a half-diminished chord, the 6th of the scale (a b6 in
    // minor) or of the chord.
    const int fifth = chord.fifth >= 0 ? chord.fifth : 7;
    int sixth = chord.sixth >= 0 ? chord.sixth : chord.thirteenth >= 0 ? chord.thirteenth : 9;
    if (chord.sixth < 0 && chord.thirteenth < 0 && s.freeChords[(size_t) std::clamp (slot, 0, maxProgressionLength - 1)] < 0)
    {
        const auto& scale = scaleIntervals (s.scale);
        if (std::find (scale.begin(), scale.end(), mod (chord.rootPc + 9 - s.keyRoot, 12)) == scale.end())
            sixth = 8;
    }

    int note = root;
    switch (symbol)
    {
        case 'O': note = root + 12; break;
        case '5': note = root + fifth; break;
        case 'L': note = root + fifth - 12 >= 28 ? root + fifth - 12 : root + fifth; break;
        case '3': note = root + (chord.third >= 0 ? chord.third : 4); break;
        case '7': note = root + (chord.seventh >= 0 ? chord.seventh : 10); break;
        case '6': note = root + sixth; break;
        case 'b': note = root + 3; break;
        case 'A': note = changeSoon ? target - 1 : root + fifth; break;   // chromatic step into the next root
        case 'S': note = changeSoon ? target - 2 : root + 12; break;      // a whole step below it
        default: break;                        // R, G
    }
    note = std::clamp (note, 24, 64);

    // Length: like the groove's notes, a share (Gate) of the space before the next note.
    int hold = 0;
    while (hold < n - 1 && line[(size_t) ((pos + 1 + hold) % n)] == '=')
        ++hold;
    int gap = 1;
    while (gap < n && (line[(size_t) ((pos + gap) % n)] == '.' || line[(size_t) ((pos + gap) % n)] == '='))
        ++gap;
    const double gate = (double) std::clamp (s.bassGate >= 0 ? s.bassGate : s.gate, 0.05f, 1.0f);
    const double steps = symbol == 'G' ? 0.3
                       : hold > 0      ? hold + gate
                                       : std::max (std::min ({ gate * gap, 4.0, gap - 0.08 }), std::min (gate, 0.9));
    const double len = std::max (0.04, steps * stepLength);

    // A bass player's touch: strong on the beat, softer in between, dead notes very soft.
    double v = s.velocity * sectionVelocityScale (s.section) * (symbol == 'G' ? 0.45 : pos % 4 == 0 ? 0.95 : 0.82);
    v += human * hv.bassVel * 18.0 * (noise (step, seedSalt + 97) - 0.5) * 2.0;
    pending.push_back ({ t, note, std::clamp ((int) std::lround (v), 1, 127), len, false, Part::bass });
}

GrooveEngine::VoicingReport GrooveEngine::voicingReport (const GrooveSettings& s)
{
    VoicingReport r;
    std::array<Chord, maxProgressionLength> chords;
    std::array<VoicedChord, maxProgressionLength> voiced;
    voiceProgression (s, chords, voiced);

    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);
    r.lowest = 127;
    for (int i = 0; i < len; ++i)
    {
        NoteSet all;
        for (int k = 0; k < voiced[(size_t) i].leftHand.size; ++k)  all.add (voiced[(size_t) i].leftHand[k]);
        for (int k = 0; k < voiced[(size_t) i].rightHand.size; ++k) all.add (voiced[(size_t) i].rightHand[k]);
        all.sort();
        r.chordNotes.push_back (all);
        r.names.push_back (chordName (chords[(size_t) i]));
        if (all.size > 0)
        {
            r.lowest = std::min (r.lowest, all.lowest());
            r.highest = std::max (r.highest, all.highest());
        }
        for (int k = 1; k < all.size; ++k)
        {
            const int gap = all[k] - all[k - 1];
            if (gap == 1) ++r.clashes;
            if (all[k] < 52 && gap < 5) ++r.muddy;
        }
    }
    if (r.lowest > r.highest)
        r.lowest = r.highest = 0;

    // Voice movement: every note of the next chord goes to the nearest note of this one.
    if (len > 1)
    {
        double total = 0.0;
        int voices = 0;
        for (int i = 0; i < len; ++i)
        {
            const auto& a = voiced[(size_t) i].rightHand;
            const auto& b = voiced[(size_t) ((i + 1) % len)].rightHand;
            for (int k = 0; k < b.size; ++k)
            {
                int nearest = 128;
                for (int j = 0; j < a.size; ++j)
                    nearest = std::min (nearest, std::abs (b[k] - a[j]));
                if (nearest < 128) { total += nearest; ++voices; }
            }
            if (a.size > 0 && b.size > 0)
                r.biggestTopLeap = std::max (r.biggestTopLeap, std::abs (b.highest() - a.highest()));
        }
        r.averageMove = voices > 0 ? total / voices : 0.0;
    }
    return r;
}

std::vector<int> GrooveEngine::progressionCodes (const GrooveSettings& s)
{
    std::vector<int> codes;
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);
    for (int i = 0; i < len; ++i)
    {
        const auto c = chordForSlot (s, i);
        const int kind = c.third == 3 ? 1 : c.seventh == 10 ? 2 : 0;
        codes.push_back (mod (c.rootPc - s.keyRoot, 12) + 12 * kind);
    }
    return codes;
}

std::vector<GrooveEngine::ProgressionVariant> GrooveEngine::progressionVariants (const GrooveSettings& s)
{
    const auto original = progressionCodes (s);
    const int n = (int) original.size();
    auto root = [] (int code) { return code % 12; };
    auto kind = [] (int code) { return code / 12; };
    auto make = [] (int r, int k) { return mod (r, 12) + 12 * k; };
    auto next = [&] (const std::vector<int>& c, int i) { return c[(size_t) ((i + 1) % n)]; };

    std::vector<ProgressionVariant> out;
    auto add = [&] (const char* name, std::vector<int> codes)
    {
        if (codes != original)
            out.push_back ({ name, std::move (codes) });
    };

    // More soulful: ii before V, a secondary dominant into a minor chord, V7 in minor keys.
    {
        auto c = original;
        bool secondary = false;
        for (int i = 0; i < n; ++i)
        {
            const int x = original[(size_t) i], nx = next (original, i);
            if (kind (x) == 0 && root (x) == 5 && root (nx) == 7)                         c[(size_t) i] = make (2, 1);   // IV -> ii (ii - V)
            else if (! secondary && i > 0 && kind (nx) == 1 && root (nx) == mod (root (x) + 5, 12) && kind (x) != 2)
            {
                c[(size_t) i] = make (root (nx) + 7, 2);                                                               // V7 of the minor chord next
                secondary = true;
            }
            else if (kind (x) == 1 && root (x) == 7)                                    c[(size_t) i] = make (7, 2);   // minor v -> V7
        }
        if (c == original && n > 1)
            c[(size_t) (n - 1)] = make (root (original[0]) + 7, 2);   // turnaround: V7 of the first chord
        add ("More soulful", c);
    }

    // Gospel: the borrowed iv, dominant sevenths, I7 into IV.
    {
        auto c = original;
        bool borrowed = false;
        for (int i = 0; i < n; ++i)
        {
            const int x = original[(size_t) i], nx = next (original, i);
            if (! borrowed && i > 0 && kind (x) == 0 && root (x) == 5 && (root (nx) == 0 || root (nx) == 7 || i == n - 1))
            {
                c[(size_t) i] = make (5, 1);   // IV -> iv
                borrowed = true;
            }
            else if (root (x) == 7 && kind (x) == 0)                                     c[(size_t) i] = make (7, 2);   // V -> V7
            else if (root (x) == 0 && kind (x) == 0 && root (nx) == 5 && i != n - 1)     c[(size_t) i] = make (0, 2);   // I -> I7 into IV
            else if (root (x) == 9 && kind (x) == 1 && root (nx) == 2)                   c[(size_t) i] = make (9, 2);   // vi -> VI7 into ii
            else if (root (x) == 5 && kind (x) == 1)                                     c[(size_t) i] = make (5, 2);   // minor key: iv -> IV7 (Dorian)
        }
        if (c == original && n > 1)
            c[(size_t) (n - 1)] = make (5, 1);
        add ("Gospel", c);
    }

    // Deep: minor colours. v instead of V, vi instead of a returning I, bVII to close.
    {
        auto c = original;
        for (int i = 0; i < n; ++i)
        {
            const int x = original[(size_t) i];
            if (root (x) == 7 && kind (x) != 1)                         c[(size_t) i] = make (7, 1);   // V -> v
            else if (i > 0 && root (x) == 0 && kind (x) == 0)           c[(size_t) i] = make (9, 1);   // I -> vi
            else if (root (x) == 4 && kind (x) == 0)                    c[(size_t) i] = make (4, 1);   // III -> iii
        }
        if (c == original && n > 1)
            c[(size_t) (n - 1)] = make (10, 0);
        add ("Deep", c);
    }

    // Chromatic: tritone substitution, bVI for IV, the Neapolitan in minor keys.
    {
        auto c = original;
        bool tritone = false, flatSix = false;
        for (int i = 0; i < n; ++i)
        {
            const int x = original[(size_t) i], nx = next (original, i);
            if (! tritone && root (x) == 7 && kind (x) != 1 && root (nx) == 0)
            {
                c[(size_t) i] = make (1, 2);   // V -> bII7
                tritone = true;
            }
            else if (! flatSix && i > 0 && root (x) == 5 && kind (x) == 0)
            {
                c[(size_t) i] = make (8, 0);   // IV -> bVI
                flatSix = true;
            }
            else if (root (x) == 8 && kind (x) == 0 && root (nx) == 7)   c[(size_t) i] = make (1, 0);   // minor key: VI -> bII before V
        }
        for (int i = 1; i < n && ! flatSix; ++i)
            if (root (original[(size_t) i]) == 9 && kind (original[(size_t) i]) == 1)
            {
                c[(size_t) i] = make (8, 0);   // no IV: vi -> bVI
                flatSix = true;
            }
        if (c == original && n > 1)
            c[(size_t) (n - 1)] = make (root (original[0]) + 1, 2);
        add ("Chromatic", c);
    }
    return out;
}

int GrooveEngine::stringsVelocity (const GrooveSettings& s) noexcept
{
    // Under the piano, not over it; the break leans on them a little more.
    const double lift = s.section == Section::breakdown ? 1.2 : 1.0;
    return std::clamp ((int) std::lround (s.velocity * 0.72 * sectionVelocityScale (s.section) * lift), 1, 127);
}

void GrooveEngine::voiceStrings (const GrooveSettings& s, const std::array<Chord, maxProgressionLength>& chords,
                                 std::array<NoteSet, maxProgressionLength>& out) noexcept
{
    for (auto& n : out)
        n = {};
    if (s.strings == StringsMode::off)
        return;

    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);
    const bool high = s.strings == StringsMode::highPad, wide = s.strings == StringsMode::widePad;
    const int low = high ? 64 : wide ? 57 : 55;   // the lowest note of the close voicing starts here (or up to an octave above)
    const int voices = high ? 3 : 4;

    NoteSet previous;
    for (int slot = 0; slot < len; ++slot)
    {
        const auto& c = chords[(size_t) slot];

        // The colours first: the 3rd, the 7th (or 6th, or 9th), the 5th, then the root.
        std::array<int, 4> pcs {};
        int count = 0;
        auto addInterval = [&] (int interval)
        {
            if (interval < 0 || count >= voices)
                return;
            const int pc = mod (c.rootPc + interval, 12);
            for (int i = 0; i < count; ++i)
                if (pcs[(size_t) i] == pc)
                    return;
            pcs[(size_t) count++] = pc;
        };
        addInterval (c.third);
        addInterval (c.seventh >= 0 ? c.seventh : c.sixth >= 0 ? c.sixth : c.ninth);
        addInterval (c.fifth);
        addInterval (0);

        // Close position, every inversion in the octave above `low`: the one that moves least
        // from the previous chord (and stays near the middle of the range).
        NoteSet best;
        double bestCost = 1.0e9;
        for (int base = low; base < low + 12; ++base)
        {
            if (std::find (pcs.begin(), pcs.begin() + count, mod (base, 12)) == pcs.begin() + count)
                continue;
            NoteSet v;
            for (int i = 0; i < count; ++i)
                v.add (base + mod (pcs[(size_t) i] - base, 12));
            v.sort();

            double cost = 0.4 * std::abs (v.mean() - (low + 9.0));
            if (previous.size == v.size)
                for (int i = 0; i < v.size; ++i)
                    cost += std::abs (v[i] - previous[i]);
            else if (previous.size > 0)
                cost += std::abs (v.mean() - previous.mean()) * v.size;
            if (cost < bestCost)
            {
                bestCost = cost;
                best = v;
            }
        }
        previous = best;

        if (wide && best.size > 0)
        {
            // The root and its fifth an octave above the bass, under the pad.
            int root = 48 + c.rootPc;
            if (root > best.lowest() - 3)
                root -= 12;
            NoteSet full;
            full.add (root);
            if (root + 7 < best.lowest() - 2)
                full.add (root + 7);
            for (int i = 0; i < best.size; ++i)
                full.add (best[i]);
            best = full;
        }
        out[(size_t) slot] = best;
    }
}

namespace
{
    /** Strings that hold a chord (and come in at once when playing starts in the middle of one). */
    bool stringsHold (StringsMode m) noexcept
    {
        return m == StringsMode::pad || m == StringsMode::highPad || m == StringsMode::widePad
            || m == StringsMode::discoOctaves || m == StringsMode::riseIntoChords;
    }
    /** Strings played from the piano's own hits (over a loop edited by hand they hold like a pad). */
    bool stringsFollowPiano (StringsMode m) noexcept
    {
        return m == StringsMode::stabs || m == StringsMode::answerStabs;
    }
}

NoteSet GrooveEngine::heldStrings (int slot, const GrooveSettings& s) const noexcept
{
    const auto& pad = cachedStrings[(size_t) std::clamp (slot, 0, maxProgressionLength - 1)];
    if (s.strings != StringsMode::discoOctaves || pad.size == 0)
        return pad;
    // Disco strings: the melody note (the pad's top) in two octaves.
    NoteSet octaves;
    octaves.add (pad.highest());
    octaves.add (pad.highest() + 12);
    return octaves;
}

void GrooveEngine::collectStrings (long long step, const GrooveSettings& s, double startPpq, double endPpq)
{
    auto mode = s.strings;
    if (mode == StringsMode::off)
        return;
    if (stringsFollowPiano (mode))
    {
        if (! s.playsEditedLoop())
            return;   // played in collectStep, with the piano's hits
        mode = StringsMode::pad;
    }

    const int spc = std::max (1, s.stepsPerChord);
    const double t = (double) step * stepLength;
    if (t < startPpq || t >= endPpq)
        return;

    const int slot = slotForStep (step, s);
    const int inChord = mod (step, spc);
    const int vel = stringsVelocity (s);
    auto play = [&] (const NoteSet& notes, double length, int velocity)
    {
        for (int i = 0; i < notes.size; ++i)
            pending.push_back ({ t, notes[i], std::clamp (velocity, 1, 127), std::max (0.05, length), false, Part::strings });
    };

    switch (mode)
    {
        case StringsMode::offbeatStabs:
            // Short chords on every "&" of the beat, the disco / house string skank.
            if (mod (step, 4) == 2)
                play (cachedStrings[(size_t) slot], 1.5 * stepLength, vel + 4);
            return;

        case StringsMode::pulse8ths:
            // The chord struck again on every 8th, almost legato; the beats a little louder.
            if (mod (step, 2) == 0)
                play (cachedStrings[(size_t) slot], 1.8 * stepLength, mod (step, 4) == 0 ? vel + 6 : vel - 8);
            return;

        case StringsMode::halfBarPad:
            // Held, struck again every half bar (beats 1 and 3).
            if (mod (step, 8) == 0)
                play (cachedStrings[(size_t) slot], std::min (8, spc - inChord) * stepLength - 0.02, inChord == 0 ? vel : vel - 10);
            return;

        case StringsMode::riseIntoChords:
        {
            // The pad, then on the last beat before the next chord a run of 16ths up to its top note.
            const bool runs = spc >= 8;
            if (inChord == 0)
            {
                play (cachedStrings[(size_t) slot], (spc - (runs ? 4 : 0)) * stepLength - 0.02, vel);
                stringsUntil = t + (spc - (runs ? 4 : 0)) * stepLength;
            }
            else if (runs && inChord >= spc - 4)
            {
                const auto& next = cachedStrings[(size_t) slotForStep (step + spc - inChord, s)];
                if (next.size > 0)
                {
                    // Four notes of the scale climbing into the next chord's top note.
                    const auto& scale = scaleIntervals (s.scale);
                    auto inScale = [&] (int n) { return std::find (scale.begin(), scale.end(), mod (n - s.keyRoot, 12)) != scale.end(); };
                    int n = next.highest();
                    for (int k = 0; k < spc - inChord; ++k)
                        do --n; while (! inScale (n));
                    NoteSet one;
                    one.add (n);
                    play (one, 0.9 * stepLength, vel - 6 + 4 * (inChord - (spc - 4)));
                }
            }
            return;
        }

        case StringsMode::pad:
        case StringsMode::highPad:
        case StringsMode::widePad:
        case StringsMode::discoOctaves:
        default:
            if (inChord == 0)
            {
                play (heldStrings (slot, s), spc * stepLength - 0.02, mode == StringsMode::discoOctaves ? vel + 4 : vel);
                stringsUntil = t + spc * stepLength;
            }
            return;
    }
}

void GrooveEngine::flushOffsUpTo (double ppq, bool inclusive, std::vector<NoteEvent>& out)
{
    scratch.clear();
    for (int n = 0; n < numTracked; ++n)
    {
        const double t = offTime[(size_t) n];
        if (t >= 0.0 && (t < ppq || (inclusive && t <= ppq)))
            scratch.push_back (n);
    }

    std::sort (scratch.begin(), scratch.end(),
               [this] (int a, int b) { return offTime[(size_t) a] < offTime[(size_t) b]; });

    for (int n : scratch)
    {
        out.push_back ({ offTime[(size_t) n], n & 127, 0, partOf[(size_t) n] });
        offTime[(size_t) n] = -1.0;
    }
}

void GrooveEngine::render (double startPpq, double endPpq, const GrooveSettings& s,
                           std::vector<NoteEvent>& out)
{
    if (endPpq <= startPpq)
        return;

    // Voicing a progression is the costly part: only redo it when the harmony changes.
    const auto harmony = harmonyKey (s);
    if (! (harmony == cachedHarmony) || ! harmonyCached)
    {
        voiceProgression (s, cachedChords, cachedVoiced);
        bassRoots (s, cachedChords, cachedBassRoots);
        cachedHarmony = harmony;
        harmonyCached = true;
        cachedStringsMode = -1;
    }
    if (cachedStringsMode != (int) s.strings)
    {
        voiceStrings (s, cachedChords, cachedStrings);
        cachedStringsMode = (int) s.strings;
    }
    const auto& chords = cachedChords;
    const auto& voiced = cachedVoiced;

    pending.clear();

    // Swing and humanize delay a hit by < 2 steps, so look back a little; the pianist's singing top
    // note and the realism's timing can be a hair early, so look one step ahead too.
    const auto firstStep = (long long) std::floor (startPpq / stepLength) - 2;
    const auto lastStep  = (long long) std::floor (endPpq / stepLength) + 1;

    // The strings' pad starts with each chord; starting (or turning them on) in the middle of a
    // chord, it comes in at once for the rest of it.
    // A loop edited by hand carries its own string notes (edited in the piano roll); one edited
    // before the strings were switched on gets them generated.
    const bool loopHasStrings = s.playsEditedLoop()
        && std::any_of (s.editedLoop->begin(), s.editedLoop->end(), [] (const LoopNote& n) { return n.part == Part::strings; });
    if (s.strings != StringsMode::off && ! loopHasStrings
        && (stringsHold (s.strings) || (stringsFollowPiano (s.strings) && s.playsEditedLoop()))
        && stringsUntil < startPpq - 1.0e-6)
    {
        const int spc = std::max (1, s.stepsPerChord);
        const auto now = (long long) std::floor (startPpq / stepLength + 1.0e-9);
        const double chordEnd = (double) (floorDiv (now, spc) + 1) * spc * stepLength;
        if (std::abs ((double) now * stepLength - startPpq) > 1.0e-9 || mod (now, spc) != 0)
            if (chordEnd - startPpq > stepLength)
            {
                const auto notes = heldStrings (slotForStep (now, s), s);
                for (int i = 0; i < notes.size; ++i)
                    pending.push_back ({ startPpq, notes[i], stringsVelocity (s), chordEnd - startPpq - 0.02, false, Part::strings });
                stringsUntil = chordEnd;
            }
    }

    if (s.playsEditedLoop())
    {
        // The loop edited in the piano roll (already carried over to the current settings),
        // repeated; Velocity and bass off still apply to it.
        const double length = s.editedLoopLength;
        const double scale = s.velocity / 118.0 * sectionVelocityScale (s.section);
        for (auto k = (long long) std::floor (startPpq / length) - 1; k <= (long long) std::floor (endPpq / length); ++k)
            for (const auto& n : *s.editedLoop)
                if ((n.part != Part::bass || s.bass != BassMode::off)   // bass off (PIANO only) silences the edited bass too
                    && (n.part != Part::strings || s.strings != StringsMode::off))
                    pending.push_back ({ (double) k * length + n.start, n.note,
                                         std::clamp ((int) std::lround (n.velocity * scale), 1, 127),
                                         std::max (0.02, n.length), false, n.part });
        if (! loopHasStrings)
            for (auto step = firstStep; step <= lastStep; ++step)
                collectStrings (step, s, startPpq, endPpq);
    }
    else
    {
        const auto& pattern = patternFor (s);
        for (auto step = firstStep; step <= lastStep; ++step)
        {
            collectStep (step, s, pattern, cachedVariation, chords, voiced);
            collectStrings (step, s, startPpq, endPpq);
        }
    }

    pending.erase (std::remove_if (pending.begin(), pending.end(),
                                   [&] (const PendingOn& p) { return p.ppq < startPpq || p.ppq >= endPpq
                                                                   || p.note < 0 || p.note > 127; }),
                   pending.end());

    // Sidechain gaps: a bass note never rings over the next beat (where the kick ducks it anyway):
    // it ends a 32nd before, so the kick has the low end to itself.
    if (s.bassFeel == 2)
        for (auto& p : pending)
            if (p.part == Part::bass)
            {
                const double nextBeat = std::floor (p.ppq + 0.02) + 1.0;
                if (p.ppq + p.duration > nextBeat - 0.125)
                    p.duration = std::max (0.06, nextBeat - 0.125 - p.ppq);
            }

    for (size_t i = 0; i < pending.size(); ++i)
        pending[i].order = i;
    std::sort (pending.begin(), pending.end(), [] (const PendingOn& a, const PendingOn& b)
    { return a.ppq < b.ppq || (a.ppq == b.ppq && a.order < b.order); });

    std::array<double, numTracked> onTime;
    onTime.fill (-1.0);

    for (const auto& p : pending)
    {
        flushOffsUpTo (p.ppq, true, out);

        const auto k = (size_t) trackIndex (p.note, p.part);
        auto& off = offTime[k];

        if (off >= 0.0)
        {
            if (p.tie || onTime[k] == p.ppq)   // tied, or same note twice at once: extend
            {
                off = std::max (off, p.ppq + p.duration);
                if (onTime[k] == p.ppq && p.part == Part::bass && partOf[k] != Part::bass)
                    for (auto it = out.rbegin(); it != out.rend(); ++it)
                        if (it->isNoteOn() && it->note == p.note && it->ppq == p.ppq && it->part != Part::strings)
                        {
                            it->part = Part::bass;   // the bass's note (on its channel, in its track)
                            partOf[k] = Part::bass;
                            break;
                        }
                continue;
            }

            out.push_back ({ p.ppq, p.note, 0, partOf[k] });  // retrigger
        }

        out.push_back ({ p.ppq, p.note, p.velocity, p.part });
        partOf[k] = p.part;
        off = p.ppq + p.duration;
        onTime[k] = p.ppq;
    }

    flushOffsUpTo (endPpq, false, out);
}

void GrooveEngine::allNotesOff (double ppq, std::vector<NoteEvent>& out)
{
    for (int n = 0; n < numTracked; ++n)
    {
        if (offTime[(size_t) n] >= 0.0)
        {
            out.push_back ({ ppq, n & 127, 0, partOf[(size_t) n] });
            offTime[(size_t) n] = -1.0;
        }
    }
    stringsUntil = -1.0;
}


//==============================================================================
std::string stabBassLine (const GroovePattern& pattern)
{
    const int n = (int) pattern.rightHand.size();
    auto rhStabAt = [&] (int i)
    {
        const char r = pattern.rightHand[(size_t) ((i % n + n) % n)];
        return isHit (r) && r != 'g';
    };
    std::string line = pattern.leftHand;
    for (int i = 0; i < n && i < (int) line.size(); ++i)
    {
        const char r = pattern.rightHand[(size_t) i];
        const bool ownHit = isHit (pattern.leftHand[(size_t) i]) && ! rhStabAt (i) && ! rhStabAt (i - 1) && ! rhStabAt (i + 1);
        // The left hand doesn't strike again on a fast chop (a stab right after another one):
        // only accents and pushes get the bass there, so busy riffs don't turn into a muddy bass.
        const bool chop = rhStabAt (i) && rhStabAt (i - 1) && ! (r == 'X' || r == 'U' || r == 'D' || r == '>');
        if (chop)
            line[(size_t) i] = '.';
        else if (rhStabAt (i))
            line[(size_t) i] = r == '>' ? '>' : (r == 'X' || r == 'U' || r == 'D') ? 'X' : (r == 'u' || r == 'd') ? 'x' : r;
        else if (isHit (line[(size_t) i]) && ! ownHit)
            line[(size_t) i] = '.';
    }
    return line;
}

//==============================================================================
std::vector<LoopNote> carryEdits (const std::vector<LoopNote>& base, const std::vector<LoopNote>& edited, double baseLength,
                                  const std::vector<LoopNote>& now, double nowLength)
{
    // The instrument a note belongs to (a fill is the piano's right hand).
    auto lane = [] (Part p) { return p == Part::fill ? Part::chords : p; };
    auto same = [] (const LoopNote& a, const LoopNote& b)
    {
        return std::abs (a.start - b.start) < 1.0e-6 && std::abs (a.length - b.length) < 1.0e-6
               && a.note == b.note && a.velocity == b.velocity && a.part == b.part;
    };
    if (std::abs (baseLength - nowLength) < 1.0e-6 && base.size() == now.size()
        && std::equal (base.begin(), base.end(), now.begin(), same))
        return edited;   // nothing changed

    constexpr double together = 0.03, nearby = 0.12;   // beats

    // Place of a note in its chord: 0 = lowest note of that part struck at that moment.
    auto rankIn = [&] (const std::vector<LoopNote>& notes, size_t i)
    {
        int rank = 0;
        for (size_t j = 0; j < notes.size(); ++j)
            if (j != i && notes[j].part == notes[i].part && std::abs (notes[j].start - notes[i].start) < together
                && (notes[j].note < notes[i].note || (notes[j].note == notes[i].note && j < i)))
                ++rank;
        return rank;
    };

    std::vector<const LoopNote*> editOf (base.size(), nullptr);
    std::vector<const LoopNote*> added;
    for (const auto& e : edited)
    {
        if (e.origin >= 0 && e.origin < (int) base.size() && editOf[(size_t) e.origin] == nullptr)
            editOf[(size_t) e.origin] = &e;
        else
            added.push_back (&e);
    }

    std::vector<int> baseRank (base.size());
    for (size_t i = 0; i < base.size(); ++i)
        baseRank[i] = rankIn (base, i);
    std::vector<int> nowRank (now.size());
    for (size_t j = 0; j < now.size(); ++j)
        nowRank[j] = rankIn (now, j);

    std::vector<LoopNote> result;
    const double loop = std::max (0.25, baseLength);
    const int repeats = std::max (1, (int) std::ceil (nowLength / loop - 1.0e-6));

    for (int r = 0; r < repeats; ++r)
    {
        const double offset = r * loop;
        std::vector<int> match (base.size(), -1);
        std::vector<char> used (now.size(), 0), removed (now.size(), 0);

        // Each generated note of before, found again now: same part, nearest moment, same rank.
        for (size_t i = 0; i < base.size(); ++i)
        {
            int best = -1;
            double bestDistance = nearby;
            for (size_t j = 0; j < now.size(); ++j)
            {
                if (used[j] || now[j].part != base[i].part || nowRank[j] != baseRank[i])
                    continue;
                const double d = std::abs (now[j].start - (offset + base[i].start));
                if (d < bestDistance) { bestDistance = d; best = (int) j; }
            }
            if (best >= 0)
            {
                match[i] = best;
                used[(size_t) best] = 1;
            }
        }

        std::vector<char> carried (now.size(), 0);
        for (size_t i = 0; i < base.size(); ++i)
        {
            const int m = match[i];
            if (m < 0)
                continue;
            const auto* e = editOf[i];
            if (e == nullptr)
            {
                removed[(size_t) m] = 1;   // deleted by hand: stays deleted
                continue;
            }
            const auto& b = base[i];
            const bool untouched = std::abs (e->start - b.start) < 1.0e-6 && std::abs (e->length - b.length) < 1.0e-6
                                   && e->note == b.note && e->velocity == b.velocity;
            if (untouched)
                continue;   // the new note as it is
            auto n = now[(size_t) m];
            n.start = std::clamp (n.start + (e->start - b.start), 0.0, nowLength - 0.001);
            n.note = std::clamp (n.note + (e->note - b.note), 0, 127);
            n.length = std::max (0.02, n.length * (e->length / std::max (0.001, b.length)));
            n.velocity = std::clamp (n.velocity + (e->velocity - b.velocity), 1, 127);
            n.origin = m;
            result.push_back (n);
            carried[(size_t) m] = 1;
        }

        for (size_t j = 0; j < now.size(); ++j)
            if (now[j].start >= offset - 1.0e-9 && now[j].start < offset + loop - 1.0e-9 && ! removed[j] && ! carried[j])
            {
                auto n = now[j];
                n.origin = (int) j;
                result.push_back (n);
            }

        // Notes added by hand: they move with the chord around them (the nearest generated note).
        for (const auto* e : added)
        {
            if (offset + e->start >= nowLength)
                continue;
            int shift = 0;
            double bestDistance = 1.0e9;
            for (size_t i = 0; i < base.size(); ++i)
            {
                if (match[i] < 0 || lane (base[i].part) != lane (e->part))
                    continue;
                const double d = std::abs (base[i].start - e->start) * 24.0 + std::abs (base[i].note - e->note);
                if (d < bestDistance) { bestDistance = d; shift = now[(size_t) match[i]].note - base[i].note; }
            }
            auto n = *e;
            n.start = offset + e->start;
            n.note = std::clamp (e->note + shift, 0, 127);
            n.origin = -1;
            result.push_back (n);
        }
    }

    std::sort (result.begin(), result.end(), [] (const LoopNote& a, const LoopNote& b)
    {
        return std::tie (a.start, a.note, a.part) < std::tie (b.start, b.note, b.part);
    });
    return result;
}

} // namespace sgk
