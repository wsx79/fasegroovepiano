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
4. Press **PREVIEW** (top right, in the *Guide sound* panel): you hear the riff straight away.
5. Change groove, voicing and extensions while it plays until you like it.
6. Drag **DRAG MIDI TO DAW** onto your favourite piano track. Done!

Want something new? Press **DICE** (random progression) and **NEW RIFF** (random rhythm).

**Even faster:** press **STYLE** (top left) and pick a genre, e.g. *Classic Deep House* or
*Jackin' House*: progression, voicing, groove, feel and sound are set in one click, ready to play.

---

## 3. The interface, section by section

<p align="center"><img src="screenshot.png" alt="Interface" width="900"></p>

### Tempo (top)

| Control | What it does |
|---|---|
| **SYNC DAW** on | The groove follows the DAW's tempo and position (recommended). The DAW tempo is shown next to it. |
| **SYNC DAW** off | A BPM bar appears (60–180): drag it or double-click to type a value. The groove runs at its own tempo but still starts/stops with the DAW's Play or with PREVIEW. |

The chosen tempo is also used for exported MIDI clips.

### Current chord (top right)

Shows the chord that is playing, its scale degree (e.g. *ii*) and the key/scale.
With *Play mode → Hold a key* it also shows "HOLD" and the key chosen with your keyboard.

### Progression

| Control | What it does |
|---|---|
| **Load a progression…** | 38 ready-made progressions in a three-column menu, grouped by style: *Soulful & Gospel*, *Circle of 5ths*, *Deep house*, *Modal & exotic*, *Jackin' & funky*. Loading one also sets the recommended scale. (Long menus like *Groove* also open in columns.) |
| **DICE** | A random but musical progression, using typical soulful house moves (ii–V, IV–iii–ii, falling fifths). Keeps the current length. |
| **LEARN** | Play chords on your **MIDI keyboard**: each chord you play (and release) fills the next slot, starting from slot 1, and the progression grows with it. Chords of the scale become degrees, the others free chords. Press LEARN again to stop. |
| **Length** | How many chords (1–8). Slots beyond the length are greyed out. |
| **Each chord** | How long each chord lasts: ½, 1, 2 or 4 bars. |
| **Slots 1–8** | Each slot is a scale degree (I…VII). Above it you see the chord name and the Roman numeral (upper case = major, lower case = minor, ø = half-diminished). The playing slot lights up. A coloured bar on top of each slot shows how well that chord sounds with the chords next to it (green = great, grey = clashes; hover for a hint). A slot that shows **Free** plays a free chord set from the circle of fifths (e.g. *bVI*); pick a degree from its menu to go back to the scale. The small **▲ ▼ arrows** in the top corners change the **inversion** of that chord (up to three steps each way, shown as +1, -2…) when the automatic choice doesn't convince you. |

Degrees are relative to the scale: change the key or mode and the progression follows.

### Harmony & Scales

| Control | What it does |
|---|---|
| **Key** | Key (C … B). |
| **Scale / mode** | Major (Ionian), **Dorian** (the deep house classic), Minor (Aeolian), Mixolydian, Lydian, Phrygian, Harmonic Minor, Melodic Minor, plus (v1.5) **Locrian**, **Harmonic Major** (gospel minor iv), **Lydian Dominant** (funky I7#11), **Mixolydian b6** (soulful borrowed iv) and **Phrygian Dominant** (dark, exotic deep house). |
| **Extensions** | Triads, 7ths, 9ths, 11ths/13ths, plus (v1.5): **6/9** (the warm soulful tonic chord, no 7th), **Add 9** (pop/house triad with a 9th), **Sus (9sus4)** (the open, floating deep house chord) and **9ths + altered V** (every chord a 9th, and degree V becomes a gospel **13(b9)**, or **7(b9,b13)** in minor keys). Clashing tensions are handled automatically: a b9 on a non-dominant chord is dropped, an 11th on a major chord becomes a 13th, a 6th that would be a b6 becomes a 7th, and sus4 is used only where the 4th is perfect. |
| **Voicing** | *Close* (tight position), *Drop 2* (more open, jazz-keyboard style), *Rootless* (no root, great with a separate bass), *Wide* (two hands: root + 7th in the left hand), *Quartal* (chords built in fourths, a modern deep sound), *Shell (3-7-top)* (just 3rd, 7th and the top colour note: light and jazzy, leaves room for the bass and vocals), *Drop 2+4* (open, spread pianist voicing, great for pads and breaks). Fast repeated hits use a lighter voicing (the top three notes) and ghost notes only the top two, like a pianist's touch; steady 8th-note bass lines bounce between root and octave. Every voicing follows the rules a pianist uses: no semitone rubs between neighbouring notes, no muddy small intervals in the low register, the 9th above the 3rd, and the voice that moves the least (the top note sings a smooth line). |
| **Left-hand bass** | The left hand: Off, Root, Octaves, Root + 5th, **Walking** (root, 5th, octave on the beats and a chromatic step into the next chord) or **Syncopated octaves** (the offbeats jump up an octave: the pumping house bass). |
| **Top line** | The melody drawn by the top notes of the chords: *Smooth (auto)* moves as little as possible, *Steady* keeps the top note in place, *Rising*, *Falling* or *Arch* shape it across the progression. |
| **Approach chord** | The gospel **slide**: on the last 16th before each chord change the next chord (and its bass note) is played a half step **from above** or **from below**, then resolves. Only where the hand is already playing, so a muted bass stays muted. |
| **Top +8va** | Doubles the top note of every chord an octave higher: the bright, ringing classic house piano sound. |
| **Strum** | Rolls the chord notes upwards like a real pianist instead of hitting them all at once (0% = together, 100% = clear roll). 15–35% sounds natural. |
| **Register** | Moves the right hand up or down. Chords automatically pick the inversion closest to the previous one (voice leading), so the progression flows without jumps. |

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
- Your progression is drawn as **numbered dots** (1, 2, 3…, around the wheel for major chords,
  around the centre for minor ones) joined by **arrows** that show the order of the chords. The
  dot with the white ring is the chord that is playing.
- **Harmony colours:** every dot and every arrow is coloured by **how well the chords sound
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
  Loading a preset or pressing DICE clears the free chords.

### Groove

| Control | What it does |
|---|---|
| **Groove** | 96 two-hand, two-bar rhythms grouped by genre (see below), **Random riff** and **User pattern** (your hand-edited grid). |
| **NEW RIFF** | Generates a new riff (chords + bass), built like a keyboard player builds one: a half-bar motif made of house comping figures, answered and repeated (A B A B'), often with a push into the next bar, over a simple bass figure that locks to it. If you locked some parts with **LOCK** (see *Rhythm grid*), those parts stay and only the rest changes. Riffs and edits are saved with your project. |
| **Song section** | Main, Intro, Break, Build-up, Drop, Outro: reshapes the groove for that part of the track (see [chapter 6](#6-building-a-track-intro-break-drop)). |
| **Fills** | **Off** by default. Every bar, or every 2/4/8 bars, the end of the bar becomes a short fill leading into the next chord. |
| **Fill style** | 11 styles, all aimed at the top note of the next chord: *Pentatonic run up* / *down*, *Chord roll* (arpeggio of the next chord), *Gospel run* (eight 16ths up the scale over the last two beats), *Chromatic walk-up* (four semitones into the next chord), *Push stabs* (a stab, a ghost, then the next chord pushed in on the "&" of 4), *16th chord chops* (four light chops with a crescendo), *Gospel thirds* (parallel thirds walking down), *Piano glissando* (a fast sweep up), *Blues lick* (down the blues scale) and *Mix*, which plays a different one at every fill. |
| **Variation** | Every 4 or 8 bars the last bar plays a **variation** of the groove (a hit moved, a ghost note, a push into the next bar, a bass pick-up), so a long loop never sounds static. |
| **Groove length** | *2 bars* (normal), *1 bar* (the first bar looped: tighter, more hypnotic) or *4 bars* (the groove followed by a variation of it). |
| **Swing** | 0% = straight, 100% = triplet shuffle. 15–35% is the classic house feel. |
| **Gate** | How long short hits last: low = staccato, high = legato. |
| **Velocity** | Overall dynamics. |
| **Humanize** | Small timing and velocity variations for a hand-played feel. |
| **Density** | Random riffs only: from sparse stabs to busy 16th-note chops. |

**The grooves: 96, by genre.** The *Groove* menu opens in columns with one heading per genre;
*Random riff* and *User pattern* are under **Generate**. Jackin' grooves sound best with Swing at
30–45%, UK garage ones with 45–55%.

| Genre | Grooves |
|---|---|
| **Soulful** (9) | *Soulful Push*, *Soulful Sunday*, *Soulful Lift*, *Sunday Morning Keys*, *Soulful Offbeat 8ths*, *Soul Train Comp*, *Vocal House Stabs*, *Soulful Pianist*, *Warm Rhodes Swing* |
| **Deep** (11) | *Deep Chords*, *Late Night Rhodes*, *Deep Sway*, *Deep Dub Stabs*, *Deep Late Pad*, *Deep Offbeat Chords*, *Larry's Chords*, *Deep Dorian Swing*, *Midnight Stabs*, *Deep Keys Rolls*, *Minimal Deep Pulse* |
| **Gospel** (9) | *Gospel Bounce*, *Gospel Shout 16ths*, *Gospel Chops*, *Gospel Shout*, *Church Organ Swell*, *Gospel Walk*, *Praise Break*, *Sanctified Stabs*, *Gospel Ballad Swing* |
| **Jackin'** (16) | *Chicago Jack*, *Jackin' Stabs*, *Jack Bounce*, *Jackin' Push*, *Chicago Organ Jack*, *Ghetto Jack*, *Disco Jack Chops*, *Jackin' Shuffle*, *Jack Organ Pump*, *Jack Stab Skips*, *Chicago Bounce*, *Warehouse Jack*, *Jack Your Body*, *Jackin' Organ Stabs*, *Swing Jack*, *Ghetto House Chops* |
| **Garage & UK** (7) | *Garage Skip*, *UK Garage 2-Step*, *Speed Garage Organ*, *Shuffled 2-Step*, *Garage Chord Skips*, *Bassline House*, *UK Funky Keys* |
| **Afro & Latin** (6) | *Afro House Keys*, *Tresillo Chords*, *Latin Montuno*, *Clave Stabs*, *Afro Deep Groove*, *Samba House* |
| **Funky & Disco** (15) | *Funky 16ths*, *Disco Octaves*, *Funk Clav Chops*, *Ghost Note Funk*, *Nile Chops*, *Anticipation Funk*, *Boogie Stabs*, *Slap Bass Funk*, *Funky Upstrokes*, *Disco Strut*, *Nu Disco Keys*, *Funky Clavinet 16ths*, *Philly Stabs*, *Boogie Wonderland*, *French Touch Filter* |
| **'90s Piano House** (9) | *Classic Offbeat Stabs*, *Offbeat Doubles*, *M1 Organ Stabs*, *Italo Piano Anthem*, *Hands Up Piano*, *Piano Anthem Offbeats*, *Show Me Stabs*, *Rave Piano Chords*, *Balearic Piano* |
| **Neo Soul & Broken** (8) | *Neo Soul Lazy*, *Broken Beat*, *Neo Soul Comping*, *Lazy Swing Keys*, *Broken Beat Chords*, *Neo Soul Ghosts*, *Jazzy House Comp*, *Rhodes Broken Swing* |
| **Chill & Pads** (6) | *Long Chords*, *Whole Bar Pads*, *Half Bar Pads*, *Chill Swells*, *Lounge Keys*, *Sunset Chords* |

### Rhythm grid

Shows the rhythm in 16th notes over 2 bars: the **CHORDS** row (right hand) and the **BASS** row
(left hand). **You can edit it by hand**:

- **Click** a step: it cycles hit → accent → ghost note → rest.
- **Right-click** (or Ctrl+click on Mac): a menu with *Hit*, *Accent*, *Ghost note*, *Push*
  (plays the next chord early), *Hold* (holds the previous note), *Rest*.
- On your first edit the groove becomes the **User pattern**: the original preset is untouched
  and your version is saved with the project.

**Parts A / B / C / D.** The pattern is split into 4 half-bar parts (the orange lines on the
grid). Above each part you'll find:

| Control | What it does |
|---|---|
| **A / B / C / D** | Which part plays in that position. For example *A A B C* repeats the first half bar, *A B A B* makes a more hypnotic loop. |
| **RND** | A new random rhythm for that part only. |
| **LOCK** | Locks the part: **NEW RIFF** changes everything except the locked parts. |

**BASS: ON / OFF** (top right of the grid) turns the left hand on and off instantly, without
opening any menu, and remembers the bass mode you had chosen. When it's off, the bass row
disappears from the grid.

| Cell | Meaning |
|---|---|
| Solid bright | accented hit |
| Solid medium | normal hit |
| Solid faint | ghost note (very soft hit) |
| Pink | **push / anticipation**: plays the next chord early and ties it over the downbeat |
| Thin bar | held note |
| White outline | current position |

### Guide sound & MIDI out

| Control | What it does |
|---|---|
| **Guide sound** | Turns the built-in sound on/off. Turn it off if you only use the MIDI. |
| **Sound** | *Rhodes (FM)* electric piano, *Soulful Grand* acoustic piano, *House Piano (M1 style)* – the bright '90s house piano – *House Organ* for jackin' organ stabs, *Wurlitzer* (reedy, a little driven), *Gospel Organ (Leslie)* (full drawbars through a rotating speaker) or *Warm Pad* (soft detuned pad for breaks). |
| **PREVIEW** | Plays the groove even with the DAW stopped. |
| **MIDI out** | *All on channel 1* or *Bass on channel 2* (to send the bass to a different instrument). |
| **Play mode** (in the Groove panel) | *Host transport*: plays with the DAW's Play. *Hold a key*: plays while you hold a MIDI key, and **that key sets the key** (great for live jamming and in the Standalone app). |
| **Tone / Reverb / Volume** | Brightness (or drawbars for the organ), reverb and volume of the guide sound (they don't affect the MIDI). |
| **Chorus / Pump** | *Chorus* widens the sound; *Pump* ducks it on every beat like a kick sidechain, so the riff sounds the way it will in the mix. Guide sound only. |

### MIDI clip

See the next chapter.

### Window size

Drag the bottom-right corner to **enlarge or shrink** the interface (60% to 200%). The size is
remembered with your project.

### PRESETS, undo and redo

- **PRESETS** (top of the window): *Save current settings as…* stores everything (progression,
  free chords, groove, your grid, sound…) under a name; the list below loads it in any project.
  *Delete* removes one, *Open the presets folder* shows the files (`.fgp`) so you can back them up
  or share them.
- **◀ ▶** next to it (or **Ctrl+Z / Ctrl+Shift+Z**, **Cmd** on Mac): **undo and redo** your last
  changes: chords, grid edits, styles, presets, everything except the window size.

### STYLE and RESET

- **STYLE** opens a menu of ready-made setups by genre: *Classic Deep House*, *Soulful House*,
  *Gospel House*, *Jackin' House*, *'90s Piano House*, *Organ Jack*, *Nu Disco*, *Funky Disco*,
  *UK Garage*, *Afro House*, *Neo Soul*, *Chill Lounge*. Each one sets the progression (and its
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
  - **Mini window while dragging:** the same switch you find in the MIDI clip panel.
- **ABOUT** (also opens when you click the logo): version, credits, license, source code, the
  **SPOTIFY** button for my productions and the buttons for an optional donation (**Ko-fi** or
  **PayPal**): the plug-in is free, and if you like it you can support its development.

<p align="center"><img src="options.png" alt="OPTIONS - SETTINGS" width="460"></p>

> In this guide the controls keep their English names. In another language look for the
> translated label in the same place; chord names, scale modes and groove names stay the same.

---

## 4. Exporting MIDI to your DAW

In the **MIDI CLIP** panel:

1. **Clip length**
   - *Full loop*: the right length for progression, rhythm and fills to repeat seamlessly
     (computed automatically, e.g. 4 one-bar chords with fills every 8 bars → 8 bars).
   - *1–32 bars*: a fixed length.
   - *Song: intro > drop > outro*: a full 60-bar arrangement (see chapter 6).
2. **Notes**: *Chords + bass*, *Chords only* or *Bass only*.
3. **Drag** the **DRAG MIDI TO DAW** tile onto a track or the DAW timeline.
   Or use **Save .mid…**: *Save this clip as .mid…* saves the file wherever you want, and
   **Save all parts to a folder…** writes six files at once: the loop and the whole song, each as
   full, chords only and bass only, ready to drop on separate tracks.

With **Mini window while dragging** on, the plug-in window shrinks to a small tile while you drag,
so you can see the DAW and drop the clip where you want. It comes back when you release. If your
DAW doesn't handle plug-in resizing well, turn this option off.

Every dragged clip is also saved in `Documents/Fase Groove Piano/` with a name describing
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
- **Automation:** *Song section* is an automatable parameter. Draw automation in your DAW to
  switch sections wherever you want, in real time.

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
| **Random inspiration** | DICE + NEW RIFF a few times, Density 30–60%, then export what you like |

---

## 8. FAQ and troubleshooting

**There's a little scale at the end of every few bars. Can I remove it?**
Those are the **fills**. Set **Fills** to **Off** (the default since v1.2), or pick a different
**Fill style**.

**I can't hear anything.**
Check that *Guide sound* is on and press **PREVIEW**, or press Play in your DAW
(*Play mode: Host transport*). In *Hold a key* mode you need to hold a MIDI key.

**The Standalone app doesn't start playing.**
The Standalone app has no DAW: use **PREVIEW** or *Play mode → Hold a key*. To change the tempo,
turn off *SYNC DAW* and set the BPM.

**The groove isn't in time with my DAW.**
Turn on *SYNC DAW*.

**Drag & drop doesn't work in my DAW.**
Use *Save .mid…* and import the file, or grab it from `Documents/Fase Groove Piano/`.

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
