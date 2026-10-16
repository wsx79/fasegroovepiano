# User guide – Fase Groove Piano

*🇮🇹 [Versione italiana](GUIDA.md)*

<p align="center"><img src="logo.png" alt="Fase Groove Piano" width="560"></p>

Fase Groove Piano **writes MIDI** funky piano parts for soulful, deep and jackin' house: pick a
key, chords and a groove, listen to the riff with the guide sound, then **drag the MIDI clip into
your DAW** and play it with your favourite piano, Rhodes or synth.

**Contents**

1. [Installation](#1-installation)
2. [Your first groove in 60 seconds](#2-your-first-groove-in-60-seconds)
3. [The interface, section by section](#3-the-interface-section-by-section)
4. [Exporting MIDI to your DAW](#4-exporting-midi-to-your-daw)
5. [Using live MIDI out](#5-using-live-midi-out)
6. [Building a track: intro, break, drop](#6-building-a-track-intro-break-drop)
7. [Soulful and jackin' house recipes](#7-soulful-and-jackin-house-recipes)
8. [FAQ and troubleshooting](#8-faq-and-troubleshooting)
9. [Listen to my music](#9-listen-to-my-music)

---

## 1. Installation

### Windows: the installer (easiest)

1. Download **`FaseGroovePiano-Windows-Setup.exe`** from the
   [latest release](https://github.com/wsx79/fasegroovepiano/releases/latest).
2. Run it. If Windows shows *"Windows protected your PC"*, click **More info → Run anyway**
   (the installer is free and open source but not signed with a paid certificate).
3. Choose what to install:
   - **VST3 plug-in + Standalone app** (recommended)
   - **VST3 plug-in only** (for your DAW)
   - **Standalone app only** (no DAW needed)
   - or a custom selection.
4. Done: the VST3 goes into `C:\Program Files\Common Files\VST3`, the app gets a Start-menu
   shortcut (and a desktop one if you tick it). Rescan plug-ins in your DAW.

To uninstall: **Settings → Apps → Fase Groove Piano → Uninstall**.

### Manual installation (zip, all systems)

Download the zip for your system (see the [README](../README.md#download)) and unzip it.

| System | File | Copy it to |
|---|---|---|
| Windows | `Fase Groove Piano.vst3` | `C:\Program Files\Common Files\VST3\` |
| macOS | `Fase Groove Piano.vst3` | `~/Library/Audio/Plug-Ins/VST3/` |
| macOS (Logic, GarageBand) | `Fase Groove Piano.component` | `~/Library/Audio/Plug-Ins/Components/` |
| All | `Fase Groove Piano` (Standalone) | anywhere: it's an app that also works without a DAW |

Restart your DAW and rescan plug-ins if it doesn't find it automatically.
The plug-in is an **instrument**: load it on a MIDI/instrument track, not as an effect.

**macOS – "the plug-in is damaged / can't be verified":** the plug-in isn't signed with a paid
Apple certificate. Open Terminal and run:

```
xattr -cr ~/Library/Audio/Plug-Ins/VST3/"Fase Groove Piano.vst3"
xattr -cr ~/Library/Audio/Plug-Ins/Components/"Fase Groove Piano.component"
```

---

## 2. Your first groove in 60 seconds

1. Load **Fase Groove Piano** on an instrument track.
2. In **PROGRESSION** open *Load a progression…* and pick **Dorian Vamp i – IV**.
3. In **GROOVE** pick **Soulful Push**.
4. Press **▶** (top, in *Transport & tempo*) or the space bar: you hear the riff straight away.
5. Change groove, voicing and extensions while it plays until you like it.
6. Drag **DRAG MIDI TO DAW** onto your favourite piano track. Done!

Want something new? Press **🎲 RANDOM** (random progression) and **🎲 NEW RIFF** (random rhythm).

**Even faster:** press **STYLE** (top left) and pick a genre, e.g. *Classic Deep House* or
*Jackin' House*: progression, voicing, groove, feel and sound are set in one click, ready to play.

---

## 3. The interface, section by section

<p align="center"><img src="screenshot.png" alt="Interface" width="900"></p>

### The look of version 2.0

Every button of the interface is drawn the same way: **lit (filled) = on**, raised = off. **Amber**
is the piano and the main actions, **cyan** the bass and "playing", **violet** the fills. Buttons
that belong together are joined, and every group has a small caption above it. Icons used
everywhere: ▶ play, ❚❚ pause, ■ stop, ⏮ back to the start, 🎲 random, 🔊 hear / 🔇 muted,
the keyboard = piano, the bass clef = bass, the eye = show. Every button has a tooltip.

### Simple and advanced view (v2.6)

The **Harmony**, **Groove** and **Guide sound** panels start in a **simple view** with the controls you
use all the time. **ADVANCED** (top right of each of them) shows every control of the three panels;
**SIMPLE** hides them again. The choice is saved with the project.

| Panel | Simple view | Only in ADVANCED |
|---|---|---|
| Harmony & Scales | Key, Scale, Extensions, Voicing, Left-hand bass, Strings, Register (v2.13: the captions in the colour of the instrument they change: amber the piano only, cyan the bass, red the strings; key, scale and extensions change all of them) | Top +8va, Approach chord, Top line, Movement, Strum, Chord range |
| Groove | Groove + NEW RIFF, Song section, Fills, Variation, Swing, Gate, Velocity, Humanize | Groove length, Fill style, Feel style / NEW FEEL, Density |
| Guide sound | On/off, Sound, Reverb, Volume, Pump | Tone, Chorus, MIDI out, Play mode, Sidechain, Rate, Release |

Ready-made progressions open with **BROWSE...** (it shows the name of the one you loaded).

### Transport & tempo (top)

| Control | What it does |
|---|---|
| **⏮ ▶/❚❚ ■** | The **transport** of the guide sound, to hear the groove without starting the DAW: back to the start, play / pause (cyan while it plays), stop (the next play starts from the top). The **space bar** is play / pause when the plug-in window is active. With the DAW playing, the groove follows the DAW. |
| **SYNC DAW** on | The groove follows the DAW's tempo and position (recommended). The DAW tempo is shown next to it. **Standalone app** (v2.11): it has no DAW, so it follows a **MIDI clock** - in your DAW send MIDI clock (Ableton: *Settings → Link, Tempo & MIDI*, *Sync* on for an output port) to a port the app listens to (on Windows a virtual port such as loopMIDI, on macOS the IAC bus; enable it in the app's *Options → Audio/MIDI settings*): tempo, play, stop and position follow. Without a clock it says *No DAW*. |
| **SYNC DAW** off | A BPM bar appears (60–180): drag it or double-click to type a value. The groove runs at its own tempo but still starts/stops with the DAW's Play or with ▶. |

The chosen tempo is also used for exported MIDI clips.

### Current chord (top right)

Shows the chord that is playing, its scale degree (e.g. *ii*) and the key/scale.
With *Play mode → Hold a key* it also shows "HOLD" and the key chosen with your keyboard.

### Progression

| Control | What it does |
|---|---|
| **Load a progression…** | 112 ready-made progressions in a **browser**: styles on the left, the progressions of that style on the right with their **chords in your key**, and a search box. Click one to hear it at once, double-click (or Enter) to close. Click the **☆ star** to keep it in **★ Favourites**; **Recent** lists the last ones you tried. Styles: *Soulful & Gospel*, *Circle of 5ths*, *Deep house*, *Modal & exotic*, *Jackin' & funky*, *Chromatic & borrowed*. Loading one also sets the recommended scale. The **Chromatic & borrowed** ones use chords from outside the scale, the way dance producers do (maj9 chords sliding by a tone, the borrowed minor iv, bIII / bVI / bVII, dominant chords like III7 or bVII7): they load as free chords on the circle, so you can see and move them. |
| **🎲 RANDOM** | A random but musical progression, using typical soulful house moves (ii–V, IV–iii–ii, falling fifths). Keeps the current length. |
| **VARIANTS** (next to the panel title) | The **Chord Variation Lab**: alternatives of your progression in the same key and with the same number of chords, so the rhythm stays: **More soulful** (ii before V, a secondary dominant), **Gospel** (the borrowed iv, dominant 7ths), **Deep** (minor v, vi instead of a returning I) and **Chromatic** (tritone substitution bII7, bVI). **HEAR** switches to one while the loop plays; **KEEP** or **CANCEL**. |
| **LEARN** | Play chords on your **MIDI keyboard**: each chord you play (and release) fills the next slot, starting from slot 1, and the progression grows with it. Chords of the scale become degrees, the others free chords. Press LEARN again to stop. |
| **Import a MIDI file** | Drop a **.mid** file anywhere on the window (or use *Import a MIDI file…* at the bottom of the progression browser): the plug-in **estimates the key** (major, Dorian or minor), **the chords** and **how long each lasts**, finds the loop (up to 8 chords) and loads it as your progression. A window shows what it found, the notes outside the scale and a **reliability** (high, medium, low). Drums (channel 10) are ignored. **The rhythm too** (v2.9): when the file has chords and a rhythm, you choose **Chords + rhythm**, **Chords only** or **Rhythm only** - the rhythm (chords on the upper row, the low notes as the bass) becomes your **User pattern**, with accents, ghost notes and held notes, ready to edit in the grid. It is an **estimate**: inversions, missing roots and passing notes can mislead it, so check the chords and fix them on the circle, or undo with ◀. |
| **Length** | How many chords (1–8). Slots beyond the length are greyed out. |
| **Each chord** | How long each chord lasts: ½, 1, 2 or 4 bars. |
| **Slots 1–8** | Each slot is a scale degree (I…VII). Above it you see the chord name and the Roman numeral (upper case = major, lower case = minor, ø = half-diminished). The playing slot lights up. A coloured bar on top of each slot shows how well that chord sounds with the chords next to it (green = great, grey = clashes; hover for a hint). A slot that shows **Free** plays a free chord set from the circle of fifths (e.g. *bVI*); pick a degree from its menu to go back to the scale. The small **▲ ▼ arrows** in the top corners change the **inversion** of that chord (up to three steps each way, shown as +1, -2…) when the automatic choice doesn't convince you. |

Degrees are relative to the scale: change the key or mode and the progression follows.

### Harmony & Scales

| Control | What it does |
|---|---|
| **Key** | Key (C … B). |
| **Scale / mode** | Major (Ionian), **Dorian** (the deep house classic), Minor (Aeolian), Mixolydian, Lydian, Phrygian, Harmonic Minor, Melodic Minor, plus (v1.5) **Locrian**, **Harmonic Major** (gospel minor iv), **Lydian Dominant** (funky I7#11), **Mixolydian b6** (soulful borrowed iv) and **Phrygian Dominant** (dark, exotic deep house). |
| **Extensions** | Triads, 7ths, 9ths, 11ths/13ths, plus (v1.5): **6/9** (the warm soulful tonic chord, no 7th), **Add 9** (pop/house triad with a 9th), **Sus (9sus4)** (the open, floating deep house chord) and **9ths + altered V** (every chord a 9th, and degree V becomes a gospel **13(b9)**, or **7(b9,b13)** in minor keys), plus (v2.1) **Lush 11ths** (m11 on the minor chords, maj9#11 where the #11 is in the scale, 13 on the dominants: the deep house colour), **Soulful 13ths** (every dominant becomes a 13: the gospel / soulful sound) and **Funky 7#9** (7th chords with the #9 on the dominants: jackin' and funk). Clashing tensions are handled automatically: a b9 on a non-dominant chord is dropped, an 11th on a major chord becomes a 13th, a 6th that would be a b6 becomes a 7th, and sus4 is used only where the 4th is perfect. |
| **Voicing** | *Close* (tight position), *Drop 2* (more open, jazz-keyboard style), *Rootless* (no root, great with a separate bass), *Wide* (two hands: root + 7th in the left hand), *Quartal* (chords built in fourths, a modern deep sound), *Shell (3-7-top)* (just 3rd, 7th and the top colour note: light and jazzy, leaves room for the bass and vocals), *Drop 2+4* (open, spread pianist voicing, great for pads and breaks), *Octave triads (disco)* (a plain triad doubled an octave up, R 3 5 R 3 5: the bright disco and nu disco piano; best with *Triads*). Fast repeated hits use a lighter voicing (the top three notes) and ghost notes only the top two, like a pianist's touch; steady 8th-note bass lines bounce between root and octave. Plus (v2.1): *Upper triad (house stab)* (the three highest tones of the chord as a close triad, e.g. an E minor triad over Am9: the modern house stab, the bass plays the root), *Spread 10ths (2 hands)* (left hand root and 10th, right hand 7th, 9th and colour above: the open pianist sound, great for soulful and gospel) and *Open 5ths (deep pad)* (root, 5th, 9th and 3rd on top, R 5 9 10, moving in parallel like a held deep house pad). Every voicing follows the rules a pianist uses: no semitone rubs between neighbouring notes, no muddy small intervals in the low register, the 9th above the 3rd, and the voice that moves the least (the top note sings a smooth line). **A/B** (next to the *Voicing* caption) opens the **Voicing Coach**: two voicings of your progression side by side, every chord drawn as notes on a pitch grid with the **top line** joined, plus the **range**, how much the **voices move** at each chord change, the **biggest top-note leap**, **muddy low notes** and **semitone rubs** (circled). Press **HEAR A** or **HEAR B** while the loop plays to switch between them: only the voicing changes, the chords stay. A short verdict tells you which is smoother or more compact. |
| **Left-hand bass** | The left hand: Off, Root, Octaves, Root + 5th, **Walking** (root, 5th, octave on the beats and a chromatic step into the next chord) **Syncopated octaves** (the offbeats jump up an octave: the pumping house bass) or **Piano stabs (bass in every chord)** (the default): the root plays with every chord hit, low and full, plus the bass's own hits in between, but never a 16th before or after a chord (that would sound like a flam). The accented stab and the push get the **octave** (root + root an octave up), the others a single root, like a pianist's left hand. It is how house piano loops are played: one big stab, bass included. The menu also has a **library of bass lines** (each with its own rhythm and notes, following your chords): *Disco* (Disco Octave Gallop, Disco Octave Push, Philly Walk, Boogie Line), *Funky* (Funk Sixteenths, Slap & Pop, Dead Note Groove: with dead notes), *House* (Deep House Root, Offbeat House, Garage Skip) and *Soulful* (Soulful Walk, Gospel Walk). They use 3rds, 5ths, 6ths, 7ths, octaves and **approach notes** that lead by a semitone into the next chord, like a bass player. The grid shows the line's rhythm; editing a bass cell turns it into your own rhythm on the roots. |
| **Top line** | The melody drawn by the top notes of the chords. **House pedal** (the default) is the classic house piano way: the chords share **one top note** (their 5th, 7th or 3rd) and step only to the nearest chord tone when a chord doesn't have it; the top note doesn't jump around inside the bar. Every second bar, the last stab before the end of the bar makes the **hook**: the top note steps to the next note of the scale (down when that stab is written as ▼) and comes back on the next bar. *Smooth (auto)* moves as little as possible, *Steady* aims for one height, *Rising*, *Falling* or *Arch* shape it across the progression. |
| **Movement** | **Voicing movement** for every groove: *Off* repeats the same chord on every hit, *Subtle* (default) moves the answer half-bars an inversion up, *Lively* makes every half-bar dance (chord, up, chord, down, mirrored in the answer). Grooves that already move keep their own figure; the break stays still. |
| **Strings** (v2.12) | A third part for **body and volume**: strings that follow your chords, always on **MIDI channel 3**. *Off* (the default, so old projects sound the same), **Pad** (each chord held until the next one: 3rd, 7th, 5th and root in close position around the middle of the keyboard, voice-led so the notes move as little as possible), **High pad** (three voices an octave up: air over the piano), **Wide pad** (the pad plus root and 5th an octave above the bass: the fullest) or **Stabs with the piano** (the strings double the piano's chord hits, ghost notes excluded: the classic house string stab). They sit a little under the piano's level; in the break they come up. Starting the loop in the middle of a chord, the pad comes in at once. **STRINGS** next to PIANO and BASS (under *Hear*) switches them on and off and remembers the last kind. The guide sound plays them with a string ensemble. |
| **Approach chord** | The gospel **slide**: on the last 16th before each chord change the next chord (and its bass note) is played a half step **from above** or **from below**, then resolves. Only where the hand is already playing, so a muted bass stays muted. |
| **Top +8va** | Doubles the top note of every chord an octave higher: the bright, ringing classic house piano sound. |
| **Strum** | Rolls the chord like a real pianist instead of hitting all the notes at once. The roll is a real time (up to about 90 ms at 100%, whatever the tempo), slightly accelerating like a hand, never longer than half the note, and ghost notes are never rolled. The small button under the knob sets the **direction**: *Up* (low to high), *Down* (high to low) or *Alternate* (up on the beat, down off it, the gospel way). 20–40% sounds natural. |
| **Register** | Moves the right hand up or down. Chords automatically pick the inversion closest to the previous one (voice leading), so the progression flows without jumps. Down low the chords keep clear of the bass by themselves (v2.7): below E3 only roots and 5ths, and no chord note under the bass. |
| **Chord range** (ADVANCED) | A slider with two thumbs: no chord note goes below the left one or above the right one (a note outside moves by an octave). "all" = no limit. |

#### Quick harmony rules for house piano

- **Dorian is home**: i – IV in Dorian (Am9 – D9) is the deep house sound. Minor 9ths on the i chord, dominant 9ths/13ths on IV.
- **Tonic colours**: on a major I try **6/9** or **maj9**; avoid the plain triad if you want the soulful sound.
- **Sus for space**: *Sus (9sus4)* chords float and never clash with the vocal: perfect for intros and breaks.
- **Gospel V**: use *9ths + altered V* with progressions that have a V (e.g. *Turnaround*, *Gospel Lift*): the 13(b9) pulls strongly back to I.
- **Slides**: *Approach chord → From above* is the classic gospel/soulful move; *From below* is softer and more jazzy. Use them with grooves that leave the last 16th of the bar free.
- **Voice leading beats big jumps**: keep *Register* still and let the plug-in choose the inversions; use *Shell* or *Rootless* when a bass line or a vocal needs space.
- **Jackin' = fewer notes, more rhythm**: 7ths, *Close*, Top +8va and short gate. **Deep = more colour, less rhythm**: 9ths/11ths, *Wide* or *Drop 2+4*, longer gate.

### Circle of fifths

Outer ring = major keys, inner ring = relative minors.

- The orange slice is the **current key**. The **bright slices are the chords of the scale**,
  each with its Roman numeral (I, ii, iii…); the dark ones are outside the scale. The centre shows
  the key and the mode.
- The slices of the chords you use are tinted with their harmony colour, and the one playing
  lights up. Your progression is drawn as **numbered dots** (1, 2, 3…, on the rim for major
  chords, around the centre for minor ones) joined by soft **curves** (under the names, so they
  stay readable) in the order of the chords. The dot with the white ring is the chord that is
  playing.
- **Harmony colours:** every dot and every curve is coloured by **how well the chords sound
  together** (shared notes, smooth voice leading, strong root moves like down a fifth, staying in
  the key): **green = sounds great**, **amber = good**, **grey = clashes**. The legend is under
  the wheel. The same colour appears as a bar on top of each chord slot.
- Hover over a slice to see the chord, its numeral and whether it is in the scale; hover over
  the centre for a reminder of what the rings mean.
- **Click** a slice to change key: outer ring → major mode, inner ring → minor mode (Dorian). If
  the current scale is already of the same kind, it is kept.
- **Drag a pink dot** onto another slice to change that chord of the progression. While you drag,
  the chords of the current scale are outlined in pink (the others fade out); drop the dot on one
  of them and the chord slot updates (the big dot shows the slot number). Dropping it anywhere
  else leaves the chord as it was. When the same chord appears twice, the first slot moves.
- **Free chords** (switch under the circle): every slice becomes a target, so you can drop a dot
  anywhere, even on chords outside the scale (e.g. Ab in C major = *bVI*, the soulful borrowed
  chord). Major slices give a major 7/9 chord, minor slices a minor 7/9 chord, with the current
  *Extensions*. Free chords are stored relative to the key, so they follow a key change. The slot
  shows **Free** and a chromatic numeral (bVI, #iv…). It may sound "wrong": your choice!
  Loading a preset or pressing 🎲 RANDOM clears the free chords.

### Groove

| Control | What it does |
|---|---|
| **Groove** | 179 two-hand, two-bar rhythms grouped by genre (see below), **Random riff** and **User pattern** (your hand-edited grid). Clicking it opens the **groove browser**: genres on the left, the grooves of that genre on the right with a **picture of their rhythm** (chords on top, bass below), a search box, **MY GROOVES** (your saved ones), **★ Favourites** (click the ☆ star on any groove) and **Recent** (the last twelve you tried). Click a groove to hear it while the loop plays, double-click (or Enter) to close. The **◀ ▶** arrows next to the menu step to the previous / next groove. |
| **NEW RIFF** | Generates a new riff (chords + bass), mostly from the classic piano house loop figures (the dotted-8th 3-3-3-3-2-2 family and its relatives), usually the same bar twice like a house loop, with a steady touch, over a simple bass figure that locks to it. If you locked some parts with **LOCK** (see *Piano roll and rhythm grid*), those parts stay and only the rest changes. Riffs and edits are saved with your project. |
| **Song section** | Main, Intro, Break, Build-up, Drop, Outro: reshapes the groove for that part of the track (see [chapter 6](#6-building-a-track-intro-break-drop)). The harmony follows too (v2.9): lighter chords in the intro and outro (9ths become 7ths...), fuller ones in the drop (7ths become 9ths). |
| **Fills** | **Off** by default. Every bar, or every 2/4/8 bars, the end of the bar becomes a short fill leading into the next chord. |
| **Fill style** | 11 styles, all aimed at the top note of the next chord: *Pentatonic run up* / *down*, *Chord roll* (a flowing 32nd-note arpeggio of the next chord, from an octave below), *Gospel run* (eight 16ths up the scale over the last two beats), *Chromatic walk-up* (four semitones into the next chord), *Push stabs* (a stab, a ghost, then the next chord pushed in on the "&" of 4), *16th chord chops* (four light chops with a crescendo), *Gospel thirds* (parallel thirds walking down), *Piano glissando* (a fast sweep up), *Blues lick* (down the blues scale) and *Mix*, which plays a different one at every fill, **chosen for the groove** (v2.8): a busy groove gets short fills (stabs, chops, a roll), a sparse one runs and glissandos. The fill that closes an *Evolve* cycle with the fills off is chosen the same way. |
| **Variation** | Every 4 or 8 bars the last bar plays a **variation** of the groove (a hit moved, a ghost note, a push into the next bar, a bass pick-up), so a long loop never sounds static. **Evolve over 8 / 16 bars** (v2.7): the same groove **grows** in four steps - as written, then with ghost notes, then a variation with a little more energy, and the last bar plays a fill into the next round. In the simple view too. |
| **Groove length** | *2 bars* (normal), *1 bar* (the first bar looped: tighter, more hypnotic) or *4 bars* (the groove followed by a variation of it). |
| **Swing** | 0% = straight, 100% = triplet shuffle. 15–35% is the classic house feel. |
| **Gate** | How much of the space before the next hit each note fills, like a pianist's hand: low = short stabs, high = almost legato. At the default 65% a stab three 16ths before the next one rings about two 16ths. Notes held with *=* in the grid keep their length. |
| **Velocity** | Overall dynamics (default 118: a piano played softly sounds dull). Accents play at this value, normal hits just below, ghost notes about half. |
| **Humanize** | Small timing and velocity variations for a hand-played feel. The **FEEL** heading above the knobs has a **Feel ▾** menu that shares it between the voices: *Even*, *Pianist* (the top note sings, a hair early and louder; steady bass), *Laid-back bass* (the bass a little behind the beat) or *Tight bass* (bass locked, chords looser), and **NEW FEEL**, another humanization of the same groove (repeatable and saved with the project). |
| **Density** | Random riffs only: from sparse stabs to busy 16th-note chops. |

**The grooves: 179, by genre.** New users: start from **Piano House**, the classic house piano loops (rootless chords on syncopated offbeats over an octave bass). The *Groove* menu opens in columns with one heading per genre;
*Random riff* and *User pattern* are under **Generate**. Jackin' grooves sound best with Swing at
30–45%, UK garage ones with 45–55%.

| Genre | Grooves |
|---|---|
| **Piano House** (30) | *Dotted House Stabs*, *Loop Piano Classic*, *Rolling House Keys*, *Three-Three-Two Piano*, *Offbeat Piano Loop*, *Upfront Piano Stabs*, *Early Change Piano*, *Peak Time Piano*, *Sunset Piano Loop*, *Two Chord Bounce*, *Piano Pump*, *Classic Loop Answer*, *Deep Piano Loop*, *Anthem Piano Loop*, *Night Drive Piano*, *Skippy Piano Stabs*, *Stab Train 3-3-3-3-2-2*, *Four Stabs Push*, *Stab Run 4-3-3-2-2*, *Late Stab Groove*, *Double Chop Stabs*, *Offbeat Roll Stabs*, *Classic Dotted Loop*, *Offbeat Dotted Stabs*, *Late Night Piano Loop*, *Three-Three-Two Bounce*, *Anthem Offbeats Push*, *Rolling Piano Answer*, *Syncopated Answer Stabs*, *Pushed Dotted Hook* |
| **Soulful** (15) | *Soulful Push*, *Soulful Sunday*, *Soulful Lift*, *Sunday Morning Keys*, *Soulful Offbeat 8ths*, *Soul Train Comp*, *Vocal House Stabs*, *Soulful Pianist*, *Warm Rhodes Swing*, *Soulful Sway Comp*, *Sunday Keys Push*, *Vocal House Offbeats*, *Warm Soul Push*, *Soulful Two-Step Keys*, *Lazy Sunday Chords* |
| **Deep** (17) | *Deep Chords*, *Late Night Rhodes*, *Deep Sway*, *Deep Dub Stabs*, *Deep Late Pad*, *Deep Offbeat Chords*, *Larry's Chords*, *Deep Dorian Swing*, *Midnight Stabs*, *Deep Keys Rolls*, *Minimal Deep Pulse*, *Deep Dub Chords*, *Deep Offbeat Pads*, *Midnight Rhodes*, *Deep Minimal Stab*, *Deep Swing Comp*, *Basement Chords* |
| **Gospel** (11) | *Gospel Bounce*, *Gospel Shout 16ths*, *Gospel Chops*, *Gospel Shout*, *Church Organ Swell*, *Gospel Walk*, *Praise Break*, *Sanctified Stabs*, *Gospel Ballad Swing*, *Gospel Chop Comp*, *Gospel Shuffle Lift* |
| **Jackin'** (22) | *Chicago Jack*, *Jackin' Stabs*, *Jack Bounce*, *Jackin' Push*, *Chicago Organ Jack*, *Ghetto Jack*, *Disco Jack Chops*, *Jackin' Shuffle*, *Jack Organ Pump*, *Jack Stab Skips*, *Chicago Bounce*, *Warehouse Jack*, *Jack Your Body*, *Jackin' Organ Stabs*, *Swing Jack*, *Ghetto House Chops*, *Jack Chops*, *Jackin' Organ Push*, *Chicago Stab Jack*, *Jack Bounce 16ths*, *Ghetto Jack Skip*, *Disco Jack Doubles* |
| **Jackin' Piano** (18) | *Chicago Piano Bounce*, *Jack Stab Melody*, *Piano Call & Answer*, *Swing Piano Chops*, *Gospel Jack Keys*, *Offbeat Inversion Stabs*, *Three Three Two*, *Late Night Jack Piano*, *Piano Riff Runner*, *Warehouse Piano*, *Jackin' Gospel Answer*, *Shuffle Piano Stabs*, *Disco Jack Piano*, *Deep Jack Piano*, *Piano Hook*, *Organ Piano Jack*, *Jackin' Piano Hook*, *Jack Swing Stabs* |
| **Garage & UK** (7) | *Garage Skip*, *UK Garage 2-Step*, *Speed Garage Organ*, *Shuffled 2-Step*, *Garage Chord Skips*, *Bassline House*, *UK Funky Keys* |
| **Afro & Latin** (6) | *Afro House Keys*, *Tresillo Chords*, *Latin Montuno*, *Clave Stabs*, *Afro Deep Groove*, *Samba House* |
| **Funky & Disco** (15) | *Funky 16ths*, *Disco Octaves*, *Funk Clav Chops*, *Ghost Note Funk*, *Nile Chops*, *Anticipation Funk*, *Boogie Stabs*, *Slap Bass Funk*, *Funky Upstrokes*, *Disco Strut*, *Nu Disco Keys*, *Funky Clavinet 16ths*, *Philly Stabs*, *Boogie Wonderland*, *French Touch Filter* |
| **'90s Piano House** (9) | *Classic Offbeat Stabs*, *Offbeat Doubles*, *M1 Organ Stabs*, *Italo Piano Anthem*, *Hands Up Piano*, *Piano Anthem Offbeats*, *Show Me Stabs*, *Rave Piano Chords*, *Balearic Piano* |
| **Neo Soul & Broken** (8) | *Neo Soul Lazy*, *Broken Beat*, *Neo Soul Comping*, *Lazy Swing Keys*, *Broken Beat Chords*, *Neo Soul Ghosts*, *Jazzy House Comp*, *Rhodes Broken Swing* |
| **Chill & Pads** (6) | *Long Chords*, *Whole Bar Pads*, *Half Bar Pads*, *Chill Swells*, *Lounge Keys*, *Sunset Chords* |
| **Dance Stabs** (8) | *Future Stab 3-3-2*, *Tresillo Stabs*, *Rolling 3-3-3*, *Offbeat Push Stabs*, *Bounce Stabs*, *Call Stab Answer*, *Club Hook Stabs*, *Disco Octave Pump*: short chords on 3-3-3-3-2-2 figures with the bass doubling every hit |

### Piano roll and rhythm grid

The big panel at the bottom has two views (tabs at the top left):

- **PIANO ROLL** (the default): **the notes the plug-in plays**, on a keyboard, **one instrument at
  a time** (v2.13): **PIANO** (amber, fills in violet), **BASS** (cyan) or **STRINGS** (red), the
  buttons at the bottom right; the piano is shown first. Notes are brighter while they sound. Above the notes, the **chord names** of
  the loop (the playing one lit), and the white playhead. It shows the whole loop (until the groove
  and the progression come round together, up to 8 bars) exactly as it will be exported.
- **EDIT GRID**: the rhythm in 16th notes over 2 bars, to edit by hand (below).
  - **VELOCITY** (green, next to EDIT GRID, only in the grid view): the **velocity page**, see
    *The velocity page* below.

**EDIT NOTES** (next to the tabs, or double-click the piano roll) opens the piano roll **big**, to
edit the **notes one by one**:

The toolbar is in groups, each with its caption: **TRANSPORT** (⏮ ▶ ■) | **TOOLS** | **EDIT** |
**GRID** | **HEAR** | **ZOOM** | **FOLLOW**; **BACK TO GENERATED** and ✕ (close) on the
right; **?** (bottom right) recalls the mouse and keyboard commands.

- **Tools** (each with its own mouse cursor): the **pencil** (click to add a note, drag to make
  it longer; you can still drag a note to move it), the **arrow** (drag a note to move it, its
  **right edge** to change its length) and the **eraser** (click a note, or drag over several,
  to delete them). Double-click or right-click also deletes a note.
- **EDIT** **PIANO** (amber) | **BASS** (cyan) | **STRINGS** (red) (v2.13): the instrument you edit,
  one at a time. Its notes are full and can be touched (the pencil draws them); the other two stay
  see-through in their colours, so you see what they play, but can't be touched. It opens on the
  instrument shown in the small piano roll. The **strings** are edited note by note like the others.
- **HEAR** (🔊 + keyboard, 🔊 + bass clef, 🔊 STRINGS): lit = you hear it, 🔇 = muted (as under the
  main piano roll).
- **Select several notes**: drag a rectangle with MOVE (or Shift + drag with the pencil),
  Ctrl / Shift + click to add or remove a note, Ctrl + A for all; Esc clears the selection. Dragging,
  the arrows, Delete, QUANTIZE and the velocity lane then work on all of them.
- Notes move **freely** (no jumps). The **magnet** (SNAP) sticks them to the grid (**1/8, 1/16 or
  1/32**; Alt while dragging does the opposite) and **QUANTIZE** (the icon next to it) puts the
  selected notes (or every note shown) on the grid.
- **Zoom** with the magnifiers, **fit** (the whole loop) or Ctrl + mouse wheel; the mouse wheel and
  the bar at the bottom scroll. **FOLLOW** (on by default) turns the page with the playhead.
- The arrows move the selected note (up / down a semitone, Shift an octave; left / right one grid
  step), Delete removes it. The **space bar** plays / stops the preview.
- The **lane at the bottom** is the **velocity**: drag on it to set how hard a stab plays (the
  notes of a chord move together; Shift: only the selected note).
- The chord names stay on top and the playhead runs while the loop plays.

From the first edit **your notes play and are exported instead of the generated ones** (in the
main groove and the drop; the other song sections keep being generated), and the small piano roll
shows **EDITED NOTES**. **Your edits follow everything else**: change the key, the scale, the
chords, the voicing, Top +8va, the groove... and the notes are generated again with your edits
carried over (the notes you deleted stay deleted, the ones you moved, lengthened or played softer
change the same way, the ones you added follow the chord). **BACK TO GENERATED** forgets the edits. They are saved with the project and
undone with ◀.

Above the piano roll the buttons are in four captioned groups:

- **VIEW**: PIANO ROLL / GRID, then EDIT NOTES (piano roll) or VELOCITY (grid).
- **HEAR**: **🔊 PIANO** and **🔊 BASS**, two switches: lit = you hear it, grey with **🔇** = muted.
  They can't both be off (switching off the only one you hear switches on the other). Muting the
  piano only affects what plays: dragged and saved clips still contain everything (use *Notes* in
  the EXPORT MIDI panel to export one part); switching the bass off remembers the bass mode you had.
- **NEW RHYTHM**: **🎲 PIANO** and **🎲 BASS** (see below).
- **GROOVE**: MY GROOVES, MORPH, DNA.

The grid shows the **CHORDS** row (right hand) and the **BASS** row (left hand). **You can edit it
by hand**:

- **Click** a step: it cycles hit → accent → ghost note → rest.
- **Right-click** (or Ctrl+click on Mac): a menu with *Hit*, *Accent*, *Ghost note*, *Push*
  (plays the next chord early), *Hold* (holds the previous note), *Rest*.
- A **hold** is drawn as a short tail joined to the note before: it is not a new note (and not a
  ghost note), the note before just rings longer. On the VELOCITY page it shows that note's level.
- On your first edit the groove becomes the **User pattern**: the original preset is untouched
  and your version is saved with the project.

#### The velocity page

**VELOCITY** (green, in the VIEW group when the grid is shown) turns the grid into **faders**: every
hit of the **CHORDS** and **BASS** rows is a green bar as tall as its velocity.

- **Drag** a fader up or down to set that hit's velocity; drag **across several** to draw a
  crescendo or an accent pattern in one go. The number appears while you point at it.
- Faint bars are **automatic** (hit 118, accent 118, push 114, short hit 106, ghost note 65);
  bright bars are set by hand. **Double-click** a fader: automatic again. **Right-click**: all the
  chords, all the bass or everything back to automatic.
- 118 means *exactly the Velocity knob* (Groove panel): the knob still scales everything, your
  faders keep their balance.
- Like a grid edit, the first fader you move turns the groove into the **User pattern** (same
  rhythm). The levels are saved with the project, with undo, and in *MY GROOVES*; they follow the
  A-D order; 🎲 PIANO / 🎲 BASS / RND / NEW RIFF clear the levels of what they change.
- With the *Piano stabs* bass the BASS row shows the stabs it plays; a bass line from the library
  has its own levels (write the bass in the grid to set them).
- On this page the random buttons and the strip above the faders **change only the velocities,
  never the rhythm** (the caption NEW RHYTHM becomes NEW VELOCITY, the strip turns green):
  - **🎲 PIANO / 🎲 BASS**: new random velocities for the chords / the bass (a shape per half bar:
    loose, on the beat, rising or falling); **NEW RIFF** both at once.
  - **A / B / C / D**: whose velocities play in each half bar, independent from the rhythm's
    order: *A A A A* gives every half bar the accents of the first.
  - **🎲** of a part: new velocities for that part only. **LOCK**: the random buttons keep that
    part's velocities.

**Parts A / B / C / D.** The pattern is split into 4 half-bar parts (the orange lines on the
grid). Above each part you'll find:

| Control | What it does |
|---|---|
| **A / B / C / D** | Which part plays in that position. For example *A A B C* repeats the first half bar, *A B A B* makes a more hypnotic loop. |
| **RND** | A new random rhythm for that part only. |
| **LOCK** | Locks the part: **NEW RIFF** changes everything except the locked parts. |

**DNA** (top of the grid) opens **Musical DNA**: six buttons (v2.9: **Straighter** - every hit on the
8ths, no ghost notes or pushes, less swing) that each take your groove **one
step** towards an intention: **More soulful** (held chords, a push into the next bar, fewer ghost
notes, 9ths, rootless voicing, a passing chord), **Funkier** (ghost notes, 16th anticipations,
short notes, more swing), **More minimal** (fewer notes that ring longer, shell voicing, no fills),
**More energy** (offbeats filled, accents, pick-ups, octave on top, fills) and **More sophisticated**
(the chord moves through its inversions, 11ths/13ths or 6/9, drop voicings, a passing chord from
below). Click again to go further. **Keep the rhythm** changes only extensions and voicing; **Keep
the harmony** changes only the grid and its feel. The chords of the progression never change and
**locked** parts stay. Undo with ◀.

**MORPH** (top of the grid) turns your groove into another one **step by step**: click the
target to choose it in the **groove browser** (genres, search, favourites: no endless menu) or
step through the grooves with **◀ ▶**, then move the **Amount** slider; the grid changes as you move it, so you can stop
anywhere in between (each step switches at its own point, so more amount only adds more of the
target). **Locked** parts stay as they are. **KEEP** keeps the blend (as your *User pattern*),
**CANCEL** puts everything back.

**NEW RHYTHM 🎲 PIANO** and **🎲 BASS** give a new random rhythm to **one hand only**: the
piano (chords row) or the bass, while the other row stays as it is. Locked parts (**LOCK**) are
kept. With a bass line from the library, **🎲 BASS** picks another line. With the *Piano stabs*
bass (which plays under every piano stab), **🎲 PIANO** first writes the bass rhythm you hear on
the bass row and switches the bass to *Piano stabs (own rhythm)*: the piano changes, the bass
stays exactly as it was. Press them a few times until the two hands lock together.

| Cell | Meaning |
|---|---|
| With a small ▲ / ▼ | a **neighbour note**: the top voice steps to the next scale note up / down while the chord under it stays (choose *Hit, voicing up / down* with a right-click). With the *House pedal* top line the top note stays put on these hits; accented ones still step |
| Solid bright | accented hit |
| Solid medium | normal hit |
| Solid faint | ghost note (very soft hit) |
| Pink | **push / anticipation**: plays the next chord early and ties it over the downbeat |
| Tail joined to the note before | held note (the note before rings on) |
| White outline | current position |

### Guide sound & MIDI out

| Control | What it does |
|---|---|
| **Guide sound** | Turns the built-in sound on/off. Turn it off if you only use the MIDI. |
| **Sound** | *Rhodes (FM)* electric piano, *Soulful Grand* acoustic piano, *House Piano (M1 style)* – the bright '90s house piano – *House Organ* for jackin' organ stabs, *Wurlitzer* (reedy, a little driven), *Gospel Organ (Leslie)* (full drawbars through a rotating speaker) or *Warm Pad* (soft detuned pad for breaks). |
| **Sequencer** (Transport, top right) | On (default): the groove plays with the DAW. **Off**: it doesn't, e.g. when you dragged the MIDI clip onto the plug-in's own track and play that (otherwise you hear it twice). Incoming MIDI still plays the guide sound; ▶ still previews. |
| **▶ (Transport, top)** | Plays the groove even with the DAW stopped (also the **space bar**; a plug-in can't start the DAW's own transport). **HEAR** in VARIANTS and in the voicing coach starts it by itself when nothing is playing. |
| **MIDI out** | *All on channel 1* or *Bass on channel 2* (to send the bass to a different instrument). The **strings** are always on **channel 3**: in Ableton Live add a MIDI track with a string instrument, set *MIDI From* to this track and the channel to *Ch. 3* (and on the piano track, *Ch. 1*, so it doesn't play the strings too). |
| **Play mode** (in the Groove panel) | *Host transport*: plays with the DAW's Play. *Hold a key*: plays while you hold a MIDI key, and **that key sets the key** (great for live jamming and in the Standalone app). |
| **Tone / Reverb / Volume** | Brightness (or drawbars for the organ), reverb and volume of the guide sound (they don't affect the MIDI). |
| **Chorus / Pump** | *Chorus* widens the sound; *Pump* is the **sidechain** depth: it ducks the sound like a kick sidechain, so the riff sounds the way it will in the mix. Guide sound only. |
| **Sidechain / Rate / Release** | Where the duck comes from. **Tempo (internal)**: on the tempo while the groove plays, every **Rate** (1/4 = every beat, 1/8, 1/2, 1/16). **Sidechain input (external)**: it follows a track you send to the plug-in's **sidechain input** (usually the kick); a small meter next to *Sidechain* shows the signal arriving (grey = nothing arrives; *Pump 0%* in amber = raise **Pump**). **Release**: how fast the sound comes back. See *Sidechain from your DAW* below. |

#### Sidechain from your DAW

The guide sound is ducked only when **Guide sound** is on and **Pump** is above 0%. Then send the kick:

- **Ableton Live** (VST3): after updating, rescan the plug-ins once (*Settings → Plug-Ins → Rescan*;
  hold Alt for a full rescan) and load the plug-in again. Unfold the device (the ▸ triangle in its
  title bar) to show the **Sidechain** section, switch it on and pick the kick track in *Audio From*.
- **Logic Pro** (AU): the *Side Chain* menu at the top of the plug-in window.
- **Cubase / Studio One / Reaper**: activate the plug-in's side-chain input and send the kick to it.
- **FL Studio**: route the kick's mixer track to the plug-in (sidechain to this track).

The standalone app has no sidechain input (use *Tempo (internal)*).

### Launchpad or Push as a hardware grid

A **Novation Launchpad** (Mini MK3, X or Pro MK3) or an **Ableton Push 2 / Push 3** can be the rhythm
grid in your hands. Switch it on in **OPTIONS → SETTINGS → Launchpad / Push as a hardware grid** (it
stays on for all your projects); the line below says whether it is connected.

| On the Launchpad | What it does |
|---|---|
| **Top 4 rows of pads** | The **chords**: rows 1-2 = steps 1-16, rows 3-4 = steps 17-32. Amber = hit (bright = accent, dark = ghost note), yellow = push, dark red = hold. |
| **Bottom 4 rows** | The **bass**, the same way, in cyan (blue = push, dark blue = hold). |
| A **pad** | Edits that step like a click on the grid: rest → hit → accent → ghost → rest. The white pad is the playhead; grey pads mark the beats. |
| Round buttons, **top row** (left to right) | ▶ play / pause · ■ stop · 🎲 PIANO · 🎲 BASS · NEW RIFF · **Sequencer** on/off (green / red) · HEAR PIANO · HEAR BASS |
| **Right column** | Top four: a new rhythm for part **A, B, C, D**. Bottom four: **NEW RIFF** (parts locked in the plug-in stay). |

- The plug-in talks to the Launchpad directly, so it works with the window closed; one Fase Groove
  Piano at a time drives it.
- In your DAW the Launchpad must be free: in **Ableton Live** set its *Control Surface* to *None*
  (Settings → Link, Tempo & MIDI) and switch off *Track* / *Remote* for its ports; on Windows a MIDI
  port can be used by one program at a time.
- Switching the option off puts the Launchpad back in its normal mode.

**Ableton Push 2 / Push 3** (connected to the computer, in Control Mode) works the same way in its
**User mode**: the 8x8 pads are the grid, the **8 buttons above the display** are the Launchpad's top
row, the **8 scene buttons** on the right its right column, and **Play** plays / pauses.

- Live keeps Push as its control surface: the plug-in uses only Push's **User Port** (in Live's
  settings leave *Track* and *Remote* off on the User Port) and switches Push to User mode; switching
  the option off gives Push back to Live.
- If the pads stay dark, press **User** on Push. On **Push 3** first set the pads to *Poly Aftertouch*
  in Control Mode (Ableton's advice for User mode).
- Push 1 and Push 3 in Standalone mode are not supported.

#### Live keys (v2.9)

**OPTIONS → SETTINGS → Live keys** turns the lowest octave of your MIDI keyboard (MIDI notes
24-35, the octave from C1 - C0 in Ableton's names) into controls; those keys don't sound.

| Key | Does |
|---|---|
| C · D · E · F · G · A | Section: Intro · Main · Break · Build-up · Drop · Outro |
| C# | A fill in the next bar (your fill style, or one that suits the groove) |
| D# | New riff |
| F# · G# | Hear piano · hear bass |
| A# | Sequencer on / off |
| B | Play / pause |

### Export MIDI

See the next chapter.

### Full screen

The **corners button** (next to STYLE) puts the interface full screen; click it again to come back.
Inside a DAW the plug-in window grows to the size of the screen (the DAW owns its window). In the
standalone app the window can also be maximised or go full screen with any shape: the interface
keeps its proportions, centred, and always stays inside the screen; full screen fills the usable
area, so the Windows taskbar stays visible and nothing is cut off at the bottom (v2.12.1).

### Window size

Drag the bottom-right corner to **enlarge or shrink** the interface (60% to 200%). The size is
remembered with your project.

### PRESETS, undo and redo

- **PRESETS** (top of the window): *Save current settings as…* stores everything (progression,
  free chords, groove, your grid, sound…) under a name; the list below loads it in any project.
  *Delete* removes one, *Open the presets folder* shows the files (`.fgp`, in
  `Documents/Fase Groove Piano/Presets`) so you can back them up or share them.
- **◀ ▶** next to it (or **Ctrl+Z / Ctrl+Shift+Z**, **Cmd** on Mac): **undo and redo** your last
  changes: chords, grid edits, styles, presets, everything except the window size.

### Your Fase Groove Piano folder

Everything you save goes into **`Documents/Fase Groove Piano/`**, easy to find and back up:

| Folder | What |
|---|---|
| `Presets/` | Your presets (`.fgp`): the whole setup. Presets saved with older versions are copied here automatically. |
| `Grooves/` | Your grooves (`.fgg`): the rhythm grid only, see **MY GROOVES**. |
| `MIDI clips/` | Every clip you drag into the DAW, and the default place for *Save .mid…*. |

**MY GROOVES** (above the rhythm grid): *Save this groove as…* stores the grid as you hear it
(both hands, parts in the A B C D order you chose) under a name. The list loads it back as
the *User pattern*, keeping the chords and everything else. Use it to build your own library of
grooves and try them on any progression.

### STYLE, 🎲 TOTAL and RESET

- **🎲 TOTAL** (top, next to PRESETS) shakes up everything at once: a random style, then a random
  key, progression and groove, and sometimes another extension, voicing, bass, guide sound, fills,
  swing and humanize. Always inside choices that sound good. Didn't like it? **↶** undoes it,
  **RESET** starts over.

- **STYLE** opens a menu of ready-made setups by genre: *House Piano Loop*, *Classic Deep House*, *Soulful House*,
  *Gospel House*, *Jackin' House*, *'90s Piano House*, *Organ Jack*, *Nu Disco*, *Funky Disco*,
  *UK Garage*, *Afro House*, *Neo Soul*, *Jackin' Piano*, *Chill Lounge*, *Future House*, *Disco Piano*, plus (v2.1) *Sunday Soulful*, *Deep Night Pads*, *Chicago Jack*, *Piano House Anthem*, *Vocal House*, *Deep Soul Keys*, *Gospel Chops*, *Jackin' Funk*, *Classic Piano House*, *Deep Minimal*, *Soulful Lift*, *Jackin' Piano Chicago*. Each one sets the progression (and its
  scale), extensions, voicing, groove, swing, gate, humanize, strum, Top +8va, bass, approach chord,
  register and guide sound. **The key stays the one you chose.** Then tweak anything you like.
- **RESET** puts **every setting back to its default** (after asking you to confirm): harmony,
  progression, groove, user pattern, locks, free chords, sound, tempo, interface size. The
  **language** is not touched.

### OPTIONS: settings, language and credits

The **OPTIONS** button at the top left opens a page with two tabs (click outside it or **CLOSE**
to go back):

- **SETTINGS**
  - **Language:** the interface speaks **English, Italiano, Español, Français, Deutsch and
    Português**. The change is immediate and is remembered for all your projects. The first time,
    the plug-in picks your computer's language.
  - **Interface size:** 60% to 200%, the same as dragging the corner.
  - **Mini window while dragging:** the plug-in window shrinks to a small tile while you drag a clip.
  - **Show tips when the mouse rests on a control:** on by default. Rest the mouse on any knob,
    menu or button for a moment and a tip explains what it does. Switch it off once you know
    the plug-in.
  - **Check for updates automatically (GitHub):** on by default. When you open the plug-in it
    asks GitHub, once per session, whether there is a newer version. If there is, it downloads it
    into *Downloads/Fase Groove Piano updates* and shows you **what's new** (the changelog, in
    Italian when the plug-in speaks Italian). Nothing is installed until you confirm:
    **INSTALL NOW** updates the copy you have, the way it was installed:
    - installed with the **Windows setup**: it downloads the setup and runs it **without the
      wizard**, keeping your folders and parts (confirm the Windows prompt; close the DAW if it
      asks);
    - **portable** copy (files copied by hand, on Windows, macOS or Linux): it downloads the zip
      and **replaces the files where they are** (the running one and the other copies in the usual
      plug-in folders); then restart the DAW;
    - if the files can't be written (e.g. a system folder), it shows the download to copy by hand.

    In the **Standalone app** the update finishes by itself: the app closes and opens again in the
    new version (with the setup, the setup reopens it when it is done). Inside a DAW, restart the
    DAW: a plug-in can't restart its host.

    **LATER** asks again
    next time; **SKIP THIS VERSION** stops asking for that version. **CHECK NOW** checks
    straight away. Switch it off and the plug-in never connects to the internet.
  - In the **Standalone** app, the window's *Options* menu (Audio/MIDI settings, save/load
    state, reset) and the audio settings also follow the chosen language.
- **ABOUT** (also opens when you click the logo): version, credits, license, source code, the
  **SPOTIFY** button for my productions and the buttons for an optional donation (**Ko-fi** or
  **PayPal**): the plug-in is free, and if you like it you can support its development.

<p align="center"><img src="options.png" alt="OPTIONS - SETTINGS" width="460"></p>

> In this guide the controls keep their English names. In another language look for the
> translated label in the same place; chord names, scale modes and groove names stay the same.

---

## 4. Exporting MIDI to your DAW

In the **EXPORT MIDI** panel:

1. **Clip length**
   - *Full loop*: the right length for progression, rhythm and fills to repeat seamlessly
     (computed automatically, e.g. 4 one-bar chords with fills every 8 bars → 8 bars).
   - *1–32 bars*: a fixed length.
   - *Song: intro > drop > outro*: a full 60-bar arrangement (see chapter 6).
2. **Notes**: *Everything* (strings included when on), *Piano only*, *Bass only*, **Strings only** (clips named *Strings ...*) or **Separate tracks** (one MIDI file with a Piano track, a Bass track (on channel 2 when *MIDI out* is set to *Bass on channel 2*), a Strings track when they are on and, if there are fills, a Fills track, plus tempo and section markers: most DAWs open it as separate tracks).
3. **Drag** the **DRAG MIDI TO DAW** tile onto a track or the DAW timeline (the tile shows what
   it holds, e.g. *Chords + bass · Full loop*).
   Or use **SAVE .MID…**: *Save this clip as .mid…* saves the file wherever you want, and
   **Save all parts to a folder…** writes six files at once: the loop and the whole song, each as
   full, chords only and bass only, ready to drop on separate tracks.

4. **FULL SONG…** opens the song window (chapter 6): there you drag or save the **whole song**.

With **Mini window while dragging** on (in **OPTIONS → SETTINGS**; it works for the whole song too), the plug-in window shrinks to a small tile while you drag,
so you can see the DAW and drop the clip where you want. It comes back when you release. If your
DAW doesn't handle plug-in resizing well, turn this option off.

Every dragged clip is also saved in `Documents/Fase Groove Piano/MIDI clips/` with a name describing
chords, groove, length and BPM, for example `Am9-D9 Soulful Push 4bars 124bpm.mid`.

**Tip:** export *Chords only* and *Bass only* to two different tracks (e.g. a Rhodes and a synth bass).

---

## 5. Using live MIDI out

If you'd rather play live (to tweak parameters while the track runs), turn off *Guide sound*
and route the plug-in's MIDI output to another track:

| DAW | How |
|---|---|
| **Ableton Live** | New MIDI track with your instrument → *MIDI From*: the Fase Groove Piano track → *Fase Groove Piano*. Monitor: *In*. |
| **FL Studio** | In the plug-in wrapper (gear icon) set a *MIDI output port*; on the target instrument set the same port as *input port*. |
| **Reaper** | On the target track add a MIDI *Receive* from the plug-in track. |
| **Bitwig** | Use a *Note Receiver* on the target track, with the plug-in track as source. |
| **Cubase / Studio One** | Select Fase Groove Piano's MIDI output as the MIDI input of the target track. |
| **Logic Pro** | Logic doesn't accept MIDI output from AU instruments: use **clip drag & drop** (chapter 4). |

To **record** the generated MIDI, record on the target track.

---

## 6. Building a track: intro, break, drop

**Song section** changes how the same groove is played:

| Section | What happens |
|---|---|
| **Main groove** | The groove as it is. |
| **Intro** / **Outro** | Only accents and on-beat hits, softer, no bass, no fills. |
| **Break** | Long pad-like chords, one per bar, soft, no bass. |
| **Build-up** | First bar in 8th notes, second bar in driving 16ths, pushing bass. |
| **Drop** | The full groove, louder, bass in octaves. |

Two ways to use it:

- **Automatic export:** *Clip length → Song: intro > drop > outro* creates a 60-bar clip
  (Intro 8 · Main 16 · Break 8 · Build-up 4 · Drop 16 · Outro 8) with section **markers**,
  visible in the timeline of most DAWs.
- **FULL SONG…** (in the EXPORT MIDI panel) opens the **song arrangement**. On top, a
  **timeline** shows the sections as long as their bars; below, for each section: the bars of each
  section (2 to 32, or **-** to leave it out), **NEW VARIATION** to regenerate the groove of **one
  section only** (for example a different Drop) while the others stay, **ORIGINAL** to go back,
  and the **fill** that closes each section (as set in the Groove panel, none, or any fill style:
  you hear it while that *Song section* loops).
  The arrangement is used by *Clip length → Song*, by *Song section* while you play, and by
  the export at the bottom: **DRAG THE WHOLE SONG** (with section markers, as one clip), **DRAG
  THIS SECTION** (v2.11: click a section on the timeline - it gets a white outline - and drag just
  that one: the clip is named after it, *Piano Intro*, *Piano Main groove*, *Piano Drop*...),
  **SAVE SONG .MID…** and **ONE .MID PER SECTION…** (one MIDI file per section to drop on
  separate clips), all with the instrument chosen in the **INSTRUMENT** row right above them (v2.13:
  **ALL** | **PIANO** | **BASS** | **STRINGS** | **TRACKS**, without leaving the window; it is the
  same choice as *Notes* in EXPORT MIDI). Clips dragged from the main
  window are named after the instrument and the *Song section* too (*Piano Intro - ...*). The total length is shown in
  bars and minutes. It is saved with the project.
- **Automation:** every knob, menu and switch is a DAW parameter, so you can automate anything:
  song section, groove, fills, gate, swing, chords of the progression, sound… In the DAW's
  automation list the parameters are grouped (*Harmony*, *Progression*, *Groove*, *Tempo & MIDI*,
  *Guide sound*). For example, draw *Song section* to switch sections wherever you want, in real
  time. (Free chords, LEARN and your edited grid are saved with the project but are not
  automatable.)

---

## 7. Soulful and jackin' house recipes

| Sound | Settings |
|---|---|
| **Classic '90s deep house** | A Dorian · *Dorian Vamp i – IV* · 9ths · Close · *Classic Offbeat Stabs* · Swing 20% · Gate 40% |
| **Kerri Chandler style** | F Minor (Aeolian) · *Late Night i – VII – VI – VII* · 11ths · Wide · *Deep Chords* · Swing 30% |
| **Gospel / soulful** | Eb Major · *Gospel Lift IV – V – vi – I* · 9ths · Drop 2 · *Gospel Bounce* · Fills every 4 bars |
| **Funky disco house** | C Mixolydian · *Mixo Groove* · 7ths · Close · *Nile Chops* or *Funk Clav Chops* · Bass: Octaves |
| **Jazzy late night** | D Major · *Circle of 5ths vi – ii – V – I* · 11ths · Rootless · *Neo Soul Lazy* · Humanize 40% |
| **Modern / quartal** | G Dorian · *Deep Minor i – v* · 9ths · Quartal · *Broken Beat* · Swing 15% |
| **Soulful grand** | Eb Major · *Soulful Gospel I – iii – IV – V* · 9ths · Drop 2 · Sound: *Soulful Grand* · *Soulful Sunday* · Strum 25% · Reverb 40% |
| **'90s house piano** | C Minor (Aeolian) · *Jackin' Minor i – iv* · 7ths · Close · Top +8va · Sound: *House Piano (M1 style)* · *Classic Offbeat Stabs* |
| **Jackin' organ** | G Dorian · *Deep Dorian i – ii – IV – i* · 7ths · Close · Sound: *House Organ* · *Jack Organ Pump* · Swing 40% |
| **Jackin' house** | A Minor (Aeolian) · *Jackin' Minor i – iv* · 7ths · Close · *Jackin' Stabs* or *Jack Bounce* · Swing 40% · Gate 35% · Bass: Octaves |
| **Gospel slide** | Eb Major · *Turnaround I – vi – ii – V* · 9ths + altered V · Drop 2 · Approach chord: From above · *Gospel Bounce* · Strum 20% |
| **Floating deep sus** | F Dorian · *Dorian Vamp i – IV* · Sus (9sus4) · Wide · *Deep Chords* · Reverb 40% |
| **Lydian dominant funk** | E Lydian Dominant · *Lydian Dom Funk I7#11 – II* · 9ths · Shell · *Funky Upstrokes* · Swing 30% |
| **Dark Afro-deep** | A Phrygian Dominant · *Dark Deep I – II* · 7ths · Drop 2+4 · *Deep Sway* · Gate 70% |
| **Random inspiration** | 🎲 RANDOM + 🎲 NEW RIFF a few times, Density 30–60%, then export what you like |

---

## 8. FAQ and troubleshooting

**There's a little scale at the end of every few bars. Can I remove it?**
Those are the **fills**. Set **Fills** to **Off** (the default since v1.2), or pick a different
**Fill style**.

**I can't hear anything.**
Check that *Guide sound* is on and press **▶** (top), or press Play in your DAW
(*Play mode: Host transport*). In *Hold a key* mode you need to hold a MIDI key.

**The Standalone app doesn't start playing.**
The Standalone app has no DAW: use **▶** (or the space bar) or *Play mode → Hold a key*. To change the tempo,
turn off *SYNC DAW* and set the BPM.

**The groove isn't in time with my DAW.**
Turn on *SYNC DAW*.

**Drag & drop doesn't work in my DAW.**
Use *Save .mid…* and import the file, or grab it from `Documents/Fase Groove Piano/MIDI clips/`.

**My DAW can't find the plug-in.**
Check the install folder (chapter 1) and rescan plug-ins. On macOS also run the `xattr` command
shown above.

**Does my project remember the settings?**
Yes, every parameter (including the random riff and your edited grid) is saved with your DAW
project.

**What's new in the latest version?**
See the [changelog](../CHANGELOG.md) and the
[Releases](https://github.com/wsx79/fasegroovepiano/releases) page.

**Does the plug-in connect to the internet or collect data?**
No. See the [privacy policy](../PRIVACY.md).

---

## 9. Listen to my music

Fase Groove Piano is made by **Fabio 'Pitch' Serra**, DJ and producer. If you like the sound of
this plug-in, you'll probably like my tracks too:

- 🎧 **Listen to my productions on Spotify:** [Fabio 'Pitch' Serra on Spotify](https://open.spotify.com/artist/29iQGuXML8LEw3xRp1dNRP)
- 🛒 **Download my tracks** from all the major digital music stores: just search for
  **Fabio 'Pitch' Serra**.

Thanks for listening and for supporting independent music! 🎹🔥
