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
const std::vector<GroovePattern>& groovePatterns()
{
    //                         |1 e & a 2 e & a 3 e & a 4 e & a |1 ...                 bar 2
    static const std::vector<GroovePattern> patterns {
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
    };

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
    static const char* const names[numTopLines] { "Smooth (auto)", "Steady", "Rising", "Falling", "Arch" };
    return names[mod (line, numTopLines)];
}

const char* bassModeName (int m) noexcept
{
    static const char* const names[numBassModes] { "Off", "Root", "Octaves", "Root + 5th", "Walking", "Syncopated octaves" };
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
    if (s.variationEvery > 0)
        steps = lcm (steps, 16LL * s.variationEvery);

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
    auto p = addVoicingMovement (applySection (applySegmentOrder (basePattern (s), s.segmentOrder), s.section),
                                 s.section == Section::breakdown ? 0 : s.movement);

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
        || s.movement != cachedMovement || cachedPattern.rightHand.empty())
    {
        cachedPattern = resolvePattern (s);
        cachedVariation = makeVariation (cachedPattern, s.riffSeed * 17u + (std::uint32_t) s.pattern);
        cachedBars = s.grooveBars;
        cachedMovement = s.movement;
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
    if (free >= 0 && free < numFreeChordCodes)
    {
        // minor = Dorian i, major = Ionian I, dominant = Mixolydian I
        const int scale = free >= 24 ? 3 : free >= 12 ? 1 : 0;
        return buildChord ((s.keyRoot + free % 12) % 12, scale, 0, s.extension);
    }

    return buildChord (s.keyRoot, s.scale, s.progression[(size_t) i], s.extension);
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

void GrooveEngine::voiceProgression (const GrooveSettings& s,
                                     std::array<Chord, maxProgressionLength>& chords,
                                     std::array<VoicedChord, maxProgressionLength>& voiced) noexcept
{
    const int len = std::clamp (s.progressionLength, 1, maxProgressionLength);

    for (int i = 0; i < len; ++i)
        chords[(size_t) i] = chordForSlot (s, i);

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
    const GroovePattern& pattern = varied ? variation : normalPattern;

    const int pos = mod (step, pattern.length());

    const double swingDelay = (step % 2 != 0) ? std::clamp (s.swing, 0.0f, 1.0f) * stepLength / 3.0 : 0.0;
    const double baseTime = (double) step * stepLength + swingDelay;
    const double human = std::clamp (s.humanize, 0.0f, 1.0f);

    auto hitTime = [&] (int salt) { return baseTime + human * 0.03 * noise (step, salt); };

    auto hitVelocity = [&] (char symbol, double scale, int salt)
    {
        double v = s.velocity * scale * sectionVelocityScale (s.section)
                   * (symbol == 'X' || symbol == 'U' || symbol == 'D' ? 1.0 : symbol == '>' ? 0.95 : symbol == 'g' ? 0.45 : 0.8);
        if (s.section == Section::buildup)   // crescendo through the build-up
            v *= 0.72 + 0.4 * (double) pos / (double) std::max (1, pattern.length() - 1);
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

    // Fill: the end of every N bars becomes a pick-up into the next chord.
    const long long bar = floorDiv (step, 16);
    const int stepInBar = mod (step, 16);
    const auto fillStyle = effectiveFillStyle (s.fillStyle, bar);
    const int fillStart = fillStartStep (fillStyle);
    const bool inFill = s.fillEveryBars > 0 && sectionAllowsFills (s.section) && mod (bar, s.fillEveryBars) == s.fillEveryBars - 1
                     && stepInBar >= fillStart;

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
        const auto scaleRun = notesOf (s.keyRoot, scale.data(), 7, target - 26, target + 14);
        const int iTarget = indexAtOrBelow (scaleRun, target);
        const double t = hitTime (31);

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
            const auto run = pentatonicNotes (s.keyRoot, s.scale, s.centre - 5, s.centre + 19);

            if (run.size > 0)
            {
                int iTop = 0;
                for (int i = 0; i < run.size; ++i)
                    if (run[i] <= top)
                        iTop = i;

                const int idx = fillStyle == FillStyle::pentatonicDown ? iTop + 2 - k : iTop - 2 + k;
                pending.push_back ({ hitTime (31), run[std::clamp (idx, 0, run.size - 1)], vel, len });
            }
        }
    }
    else if (approachHere && canApproach (pattern.rightHand))
    {
        const auto& next = voiced[(size_t) slotNext];
        const double t = hitTime (1);
        const double roll = std::min (std::clamp (s.strum, 0.0f, 1.0f) * 0.05 * std::max (30.0, s.bpm) / 60.0, approachLength * 0.4)
                          / std::max (1, next.rightHand.size - 1);
        for (int i = 0; i < next.rightHand.size; ++i)
            pending.push_back ({ t + i * roll, next.rightHand[i] + approachShift, hitVelocity ('x', 0.8, 60 + i), approachLength });
    }
    else
    {
        const char symbol = pattern.rightHand[(size_t) pos];
        if (isHit (symbol))
        {
            // 'u' / 'd' (and accented 'U' / 'D'): the same chord one inversion up or down,
            // so the top note moves inside the bar like a real piano riff.
            auto v = voiced[(size_t) slotFor (symbol)];
            const bool moveUp = symbol == 'u' || symbol == 'U', moveDown = symbol == 'd' || symbol == 'D';
            if (moveUp || moveDown)
            {
                // Like a pianist's right hand: only the top three notes move, as a clean
                // close-position inversion (the top note steps to the next chord tone).
                NoteSet upper;
                for (int i = std::max (0, v.rightHand.size - 3); i < v.rightHand.size; ++i)
                    upper.add (v.rightHand[i]);
                invert (upper, moveUp ? 1 : -1);
                v.rightHand = upper;
                v.leftHand = {};
            }
            const double len = durationFor (pattern.rightHand);
            const double t = hitTime (1);
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
                pending.push_back ({ t + 0.004 * human * noise (step, 50 + i) + rollOffset (i - firstNote), v.rightHand[i],
                                     hitVelocity (symbol, 1.0 - 0.03 * (v.rightHand.size - 1 - i), 60 + i), len, tie });

            if (s.octaveTop && v.rightHand.size > 0 && v.rightHand.highest() + 12 <= 108)
                pending.push_back ({ t + (rollDown ? 0.0 : rollBeats), v.rightHand.highest() + 12,
                                     hitVelocity (symbol, 0.8, 70), len, tie });

            if (firstNote == 0)
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
                static constexpr int walk[4] { 0, 7, 12, 7 };
                note = root + walk[(inChord / 4) % 4];
                if (s.stepsPerChord - inChord <= 4 && nextSlot != slotForStep (step, s))
                {
                    const int target = 36 + chords[(size_t) nextSlot].rootPc;
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

    // Voicing a progression is the costly part: only redo it when the harmony changes.
    const HarmonyKey harmony { s.keyRoot, s.scale, (int) s.extension, (int) s.voicing, s.centre,
                               s.progressionLength, (int) s.topLine, s.progression, s.freeChords, s.inversions };
    if (! (harmony == cachedHarmony) || ! harmonyCached)
    {
        voiceProgression (s, cachedChords, cachedVoiced);
        cachedHarmony = harmony;
        harmonyCached = true;
    }
    const auto& chords = cachedChords;
    const auto& voiced = cachedVoiced;

    pending.clear();

    // Swing and humanize only ever delay a hit (by < 2 steps), so look back a little.
    const auto firstStep = (long long) std::floor (startPpq / stepLength) - 2;
    const auto lastStep  = (long long) std::floor (endPpq / stepLength);

    const auto& pattern = patternFor (s);
    for (auto step = firstStep; step <= lastStep; ++step)
        collectStep (step, s, pattern, cachedVariation, chords, voiced);

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
