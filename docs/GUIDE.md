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
| **Load a progression…** | 19 ready-made progressions (including jackin' house). Loading one also sets the recommended scale. |
| **DICE** | A random but musical progression, using typical soulful house moves (ii–V, IV–iii–ii, falling fifths). Keeps the current length. |
| **Length** | How many chords (1–8). Slots beyond the length are greyed out. |
| **Each chord** | How long each chord lasts: ½, 1, 2 or 4 bars. |
| **Slots 1–8** | Each slot is a scale degree (I…VII). Above it you see the chord name and the Roman numeral (upper case = major, lower case = minor, ø = half-diminished). The playing slot lights up. |

Degrees are relative to the scale: change the key or mode and the progression follows.

### Harmony & Scales

| Control | What it does |
|---|---|
| **Key** | Key (C … B). |
| **Scale / mode** | Major (Ionian), **Dorian** (the deep house classic), Minor (Aeolian), Mixolydian, Lydian, Phrygian, Harmonic Minor, Melodic Minor. |
| **Extensions** | Triads, 7ths, 9ths, 11ths/13ths. Clashing tensions are handled automatically (e.g. a b9 on a non-dominant chord is dropped, an 11th on a major chord becomes a 13th). |
| **Voicing** | *Close* (tight position), *Drop 2* (more open, jazz-keyboard style), *Rootless* (no root, great with a separate bass), *Wide* (two hands: root + 7th in the left hand), *Quartal* (chords built in fourths, a modern deep sound). |
| **Left-hand bass** | The left hand: Off, Root, Octaves, Root + 5th. |
| **Register** | Moves the right hand up or down. Chords automatically pick the inversion closest to the previous one (voice leading), so the progression flows without jumps. |

### Circle of fifths

Outer ring = major keys, inner ring = relative minors.

- The orange slice is the **current key**; the highlighted neighbours are the "closest" keys
  (IV and V), where modulations sound natural.
- The pink line connects the chords of your progression: you can see the **moves by fifths**
  (adjacent slices) at a glance; the white dot is the chord that is playing.
- **Click** a slice to change key: outer ring → major mode, inner ring → minor mode (Dorian). If
  the current scale is already of the same kind, it is kept.

### Groove

| Control | What it does |
|---|---|
| **Groove** | 26 two-hand, two-bar rhythms (see below), **Random riff** and **User pattern** (your hand-edited grid). |
| **NEW RIFF** | Generates a new funky riff (chords + bass). If you locked some parts with **LOCK** (see *Rhythm grid*), those parts stay and only the rest changes. Riffs and edits are saved with your project. |
| **Song section** | Main, Intro, Break, Build-up, Drop, Outro: reshapes the groove for that part of the track (see [chapter 6](#6-building-a-track-intro-break-drop)). |
| **Pentatonic fills** | Every 2/4/8 bars the last beat becomes a pentatonic run leading into the next chord. |
| **Swing** | 0% = straight, 100% = triplet shuffle. 15–35% is the classic house feel. |
| **Gate** | How long short hits last: low = staccato, high = legato. |
| **Velocity** | Overall dynamics. |
| **Humanize** | Small timing and velocity variations for a hand-played feel. |
| **Density** | Random riffs only: from sparse stabs to busy 16th-note chops. |

**The grooves:** *Classic Offbeat Stabs*, *Soulful Push*, *Deep Chords*, *Funky 16ths*,
*Gospel Bounce*, *Chicago Jack*, *Garage Skip*, *Late Night Rhodes*, *Disco Octaves*,
*Long Chords*, *Funk Clav Chops*, *Ghost Note Funk*, *Nile Chops*, *Anticipation Funk*,
*Gospel Shout 16ths*, *Neo Soul Lazy*, *Boogie Stabs*, *Offbeat Doubles*, *Broken Beat*,
*Slap Bass Funk* and the **jackin' house** ones: *Jackin' Stabs*, *Jack Bounce*, *Jackin' Push*,
*Chicago Organ Jack*, *Ghetto Jack*, *Disco Jack Chops* (best with Swing at 30–45%).

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
| **Guide sound** | A simple electric piano to hear the riff. Turn it off if you only use the MIDI. |
| **PREVIEW** | Plays the groove even with the DAW stopped. |
| **MIDI out** | *All on channel 1* or *Bass on channel 2* (to send the bass to a different instrument). |
| **Play mode** | *Host transport*: plays with the DAW's Play. *Hold a key*: plays while you hold a MIDI key, and **that key sets the key** (great for live jamming and in the Standalone app). |
| **Tone / Reverb / Volume** | Brightness, reverb and volume of the guide sound (they don't affect the MIDI). |

### MIDI clip

See the next chapter.

### Window size

Drag the bottom-right corner to **enlarge or shrink** the interface (60% to 200%). The size is
remembered with your project.

### CREDITS

The button at the top left opens the credits, the license, the source-code link and the
buttons for an optional donation (**Ko-fi** or **PayPal**): the plug-in is free, and if you like it
you can support its development.

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
   Or use **Save .mid…** to save the file wherever you want.

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
| **Jackin' house** | A Minor (Aeolian) · *Jackin' Minor i – iv* · 7ths · Close · *Jackin' Stabs* or *Jack Bounce* · Swing 40% · Gate 35% · Bass: Octaves |
| **Random inspiration** | DICE + NEW RIFF a few times, Density 30–60%, then export what you like |

---

## 8. FAQ and troubleshooting

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

- 🎧 **Listen to my productions on Spotify:** [Fabio 'Pitch' Serra on Spotify](https://open.spotify.com/search/Fabio%20%27Pitch%27%20Serra)
- 🛒 **Download my tracks** from all the major digital music stores: just search for
  **Fabio 'Pitch' Serra**.

Thanks for listening and for supporting independent music! 🎹🔥
