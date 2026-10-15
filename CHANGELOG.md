# Changelog

*🇮🇹 [Versione italiana](CHANGELOG.it.md)*

All notable changes to **Fase Groove Piano**, version by version.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [2.1.0] - 2026-10-15

### Added
- **A bigger library**, all for soulful, deep and jackin' house, checked for how well the chords
  sound together:
  - **40 new progressions** (94 in all): soulful and gospel (Soul Cycle, Sunday Gospel, Classic
    Soul, Sunrise, Vocal Lift, Church Turn...), deep (Late Night Minor, Deep Float, Minor Two,
    Midnight...), jackin' (Jack Vamp, Funk Turn, Chicago Shuffle...), circle of fifths, borrowed
    chords and modal.
  - **30 new grooves** (172 in all): piano house loops on the dotted-8th figures, soulful
    comping with pushes, gospel chops, deep pads and dub chords, jackin' 16ths and chops.
  - **3 new extensions**: *Lush 11ths* (m11, maj9#11, 13 on dominants), *Soulful 13ths*,
    *Funky 7#9*.
  - **3 new voicings**: *Upper triad (house stab)*, *Spread 10ths (2 hands)*, *Open 5ths (deep pad)*.
  - **12 new styles** (28 in all): Sunday Soulful, Deep Night Pads, Chicago Jack, Piano House
    Anthem, Vocal House, Deep Soul Keys, Gospel Chops, Jackin' Funk, Classic Piano House, Deep
    Minimal, Soulful Lift, Jackin' Piano Chicago.

## [2.0.0] - 2026-10-15

### Changed
- **New interface.** A clean "studio" look: graphite surfaces, one warm accent (amber: the piano
  and the main actions) and one cool accent (cyan: the bass and "playing"), violet for fills, the
  **Inter** typeface embedded (the same on every system), flat panels with hairline borders and
  small letter-spaced titles, studio knobs, slim menus, switches, tooltips and scroll bars.
- **One family of buttons with drawn icons**: lit = on, buttons that belong together are joined,
  each group has a caption.
- **Transport** at the top (⏮ back to the start, ▶ / ❚❚ play / pause, ■ stop) instead of
  PREVIEW: pause resumes where it stopped; the space bar is play / pause.
- **HEAR 🔊 PIANO / 🔊 BASS** switches above the piano roll (🔇 when muted), **NEW RHYTHM
  🎲 PIANO / 🎲 BASS**, **🎲 RANDOM** for the progression, 🎲 NEW RIFF, undo / redo, full screen
  and options with icons.
- **Piano roll editor** toolbar in captioned groups: transport, tools, draws, grid (snap magnet,
  grid size, quantize), show, hear, zoom, follow; ✕ to close.
- The current chord in a display, a top bar for the logo and the utilities.

## [1.28.0] - 2026-10-14

### Added
- **Edits follow the settings:** after editing notes in the piano roll you can still change key,
  scale, chords, voicing, Top +8va, the groove...: the notes are generated again and your edits are
  carried over (deleted notes stay deleted, moved / lengthened / softer notes change the same way,
  added notes follow the chord).
- **Multiple selection** in the piano roll: rubber-band rectangle, Ctrl / Shift + click, Ctrl + A;
  move, transpose, delete, quantize and set the velocity of all of them.
- **HEAR BOTH / PIANO / BASS** inside the piano roll editor.
- **Full screen** button (in a DAW the plug-in fills the screen).
- **The standalone app restarts by itself after an update** (portable copy: replaced and reopened;
  setup: it closes, the setup runs and opens it again).

### Fixed
- The space bar works as soon as you come back to the window, without clicking in it first.

## [1.27.0] - 2026-10-14

### Added
- **Piano roll editor tools**, each with its own cursor: DRAW (pencil), MOVE (arrow), ERASE
  (eraser: click or drag over notes).
- **Free movement** of the notes by default; **SNAP** to stick them to the grid (Alt inverts) and
  **QUANTIZE**.
- **DRAWS PIANO / BASS** in the notes' colours, **SHOW ALL / PIANO / BASS**, **FOLLOW** (the view
  follows the playhead).
- **Space bar** plays / stops the preview, and a round **▶ / ■** button next to PREVIEW.

### Fixed
- **HEAR** in VARIANTS (and in the voicing coach) did nothing with the DAW stopped and the preview
  off: it now starts the preview, and stops it again when the window closes.

## [1.26.0] - 2026-10-14

### Added
- **Piano roll editor:** **EDIT NOTES** (or a double-click on the piano roll) opens it big, with
  zoom, to edit the notes one by one: add, move, lengthen or shorten, delete, transpose with the
  arrows, and a **velocity lane**. Your notes play and are exported instead of the generated ones
  (main groove and drop) until **BACK TO GENERATED**; they are saved with the project and undone
  with ◀.
- **Hook** in the house pedal top line: every second bar the last stab steps to the next note of
  the scale and comes back, the classic piano house hook.
- **Left-hand octaves:** with the piano-stab bass the accented stab and the push play the root
  and its octave.

### Fixed
- **Dragging the whole song** from the song window: the window now shrinks to the mini tile like
  the normal clip, so you can drop it in the DAW (before, the plug-in covered the DAW).

## [1.25.0] - 2026-10-14

### Changed
- **Piano grooves in the classic house piano style.** The chords now share **one top
  note** (new *House pedal* top line, the default: their 5th, 7th or 3rd, stepping only to the
  nearest chord tone when a chord doesn't have it), so the riff no longer sounds like random
  notes jumping around inside the bar. The ▲/▼ hits became **neighbour notes** on the top voice
  only (the chord under it stays); with the pedal top line plain ▲/▼ hits keep the top note still.
- **Bass in every stab, no flams:** *Piano stabs (bass in every chord)* is the default bass; the
  bass never plays a 16th before or after a chord any more (it sounded like two hands that
  missed each other).
- **NEW RIFF / RND PIANO** build riffs from the classic piano house loop figures (the dotted-8th
  3-3-3-3-2-2 family and its relatives), repeat their bar like a house loop and keep a steady
  touch (no random accents or ghost notes).
- House styles (STYLE menu) use the pedal top line.
- **New layout of the bottom of the window** (a little taller):
  - **PIANO ROLL** (default view): every note that plays (chords, bass, fills) on a keyboard,
    the chord names above and the playhead; **EDIT GRID** switches to the rhythm grid.
  - **BOTH / PIANO / BASS** instead of the two SOLO buttons.
  - **EXPORT MIDI** panel: notes and length, a clear drag tile that shows what it holds,
    **SAVE .MID…** and **FULL SONG…** (the mini-window switch is in OPTIONS → SETTINGS).
- **Song window:** a timeline of the sections, column names, the song length in minutes, and the
  whole-song export in one place: drag it, **SAVE SONG .MID…** or **ONE .MID PER SECTION…**.
- **Circle of fifths** redrawn: bigger, flat and clean; the chords of the scale stand out, the
  others stay faint; the chords you use are tinted with their harmony colour; the progression is
  joined by soft curves under the names instead of arrows over them.
- **MORPH:** the target is chosen in the groove browser (genres, search, favourites) or with
  ◀ ▶, instead of an endless menu.
- **FEEL:** its own heading above the knobs, with the **Feel ▾** menu and **NEW FEEL**.
- *Play mode* moved next to *MIDI out*.

## [1.24.1] - 2026-10-14

### Fixed
- **Tempo of exported MIDI files:** a tempo just typed in (with the DAW sync off) was not in the
  file until the audio had run; the file now always has the tempo set now (or the DAW's).
- **Separate tracks:** the bass follows the *MIDI out* setting like the other exports (channel 2
  only when *Bass on channel 2* is chosen).

### Added
- Tests: exported tempo (manual and DAW), bass channel in every export, and a whole song with a
  different fill on every section (no hanging or doubled notes at the changes).

## [1.24.0] - 2026-10-14

### Added
- **Chord Variation Lab** (**VARIANTS** in the Progression panel): more soulful, gospel, deep
  and chromatic versions of your progression, same key and length (the rhythm stays); HEAR each
  while the loop plays, KEEP or CANCEL.
- **Humanize per voice:** *Feel* under the Humanize knob (Even, Pianist with a singing top note,
  Laid-back bass, Tight bass) and **NEW FEEL** for another repeatable humanization (saved with
  the project).
- **A fill per song section** in **SONG**: as set, none, or any fill style on the section's last
  bar; you hear it while that section loops.
- **Separate tracks** in *Notes*: one MIDI file with Piano, Bass and Fills tracks,
  tempo and section markers.

### Fixed
- Some texts of the windows added since v1.18 (browsers, Voicing Coach, SONG, MORPH, DNA) stayed
  in English after changing the language.

## [1.23.1] - 2026-10-14

### Fixed
- **Damaged presets:** a preset file that is damaged or not a Fase Groove Piano preset is no
  longer reported as loaded; nothing changes and a message says so.
- **MIDI analyzer:** a pick-up note before the loop, inversions (F/A is F, not Am), short stabs
  over a held bass, sus4 chords and root + 3rd stabs are now read correctly. The result window
  shows a **reliability** (high, medium, low).

### Added
- Tests: damaged / foreign / empty preset files, chords on channel 1 and bass on channel 2 in the
  exported MIDI, and ten tricky cases for the MIDI analyzer.

## [1.23.0] - 2026-10-14

### Added
- **Musical DNA** (**DNA** above the grid): five intentions, *More soulful*, *Funkier*, *More
  minimal*, *More energy*, *More sophisticated*; each click takes the groove one step further
  (rhythm, feel, extensions, voicing). *Keep the rhythm* / *Keep the harmony* limit what changes;
  the chords never change and locked parts stay.

## [1.22.0] - 2026-10-14

### Added
- **Groove morph** (**MORPH** above the grid): turn your groove into another one step by step with
  an *Amount* slider, live in the grid; locked parts stay; KEEP or CANCEL.
- **Song arrangement** (**SONG** in the MIDI clip panel): bars of each section (or leave it
  out), **NEW VARIATION** for one section only (e.g. a different Drop), back to ORIGINAL, and
  **SAVE EACH SECTION AS .MID…** (one file per section). Used by the *Song* clip and by *Song
  section* while playing; saved with the project.

## [1.21.0] - 2026-10-14

### Added
- **Voicing Coach** (the **A/B** button next to *Voicing*): two voicings of your progression side
  by side, drawn note by note with the top line, plus range, voice movement per chord change,
  biggest top-note leap, muddy low notes and semitone rubs. **HEAR A / HEAR B** switch between
  them while the loop plays (the chords never change), and a short verdict says which is smoother
  or more compact.

## [1.20.0] - 2026-10-14

### Added
- **Bass line library** in the *Left-hand bass* menu (now grouped): 12 bass lines that
  follow your chords, each with its own rhythm: *Disco* (Disco Octave Gallop, Disco Octave Push,
  Philly Walk, Boogie Line), *Funky* (Funk Sixteenths, Slap & Pop, Dead Note Groove), *House*
  (Deep House Root, Offbeat House, Garage Skip), *Soulful* (Soulful Walk, Gospel Walk). They use
  3rds, 5ths, 6ths, 7ths, octaves, dead notes and approach notes into the next chord.
- The grid shows the line's rhythm; **RND BASS** picks another line; editing a bass cell turns
  the line's rhythm into your own.

### Changed
- Styles *Nu Disco*, *Funky Disco*, *Gospel House* and *Disco Piano* now use a bass line.

## [1.19.0] - 2026-10-14

### Added
- **MIDI analyzer:** drop a .mid file on the window (or *Import a MIDI file…* in the progression
  browser). It estimates key (major, Dorian or minor), chords and how long each lasts, finds the
  loop and loads it as the progression, showing the notes outside the scale. It is an estimate:
  check the chords and fix them on the circle, or undo.
- **Favourites and Recent** in the groove and progression browsers: click the ☆ star to keep
  a groove or a progression; Recent lists the last twelve you tried. Saved for all projects.

## [1.18.2] - 2026-10-14

### Fixed
- **Portable updates are now all-or-nothing:** every file is checked before anything is touched,
  and if any step fails, everything already replaced is put back (no half-updated plug-in).

### Added
- **Plug-in tests**, run on every build for Windows, macOS and Linux: project state and old
  presets, undo snapshots, exported MIDI files (note pairs, channels, length, song markers), DAW
  transport (play, stop, seek, loop, tempo changes, odd block sizes, SOLO BASS) and the update
  swap with simulated failures.

## [1.18.1] - 2026-10-13

### Fixed
- The groove and progression browsers didn't open again after being closed once.

## [1.18.0] - 2026-10-13

### Added
- **Groove browser:** the *Groove* menu is now a window with genres on the left and the grooves
  on the right, each with a picture of its rhythm, a search box and your saved grooves. Click to
  hear, double-click to close. **◀ ▶** arrows step through the grooves.
- **Progression browser:** the same window for *Load a progression…*, with the chords of each
  progression in your key.
- **SOLO PIANO / SOLO BASS** above the rhythm grid (SOLO PIANO replaces BASS: ON / OFF).

### Changed
- **Smarter updates:** a copy installed with the Windows setup is updated by the setup without
  the wizard (same folders and parts); a portable copy gets the zip and its files are replaced
  where they are (Windows, macOS, Linux). Only when the files can't be written is the download
  shown to copy by hand.

## [1.17.0] - 2026-10-13

### Added
- **MY GROOVES** above the rhythm grid: save the grid (both hands) as your own groove and load
  it back on any progression.
- **One folder for your files:** `Documents/Fase Groove Piano/` with `Presets/`, `Grooves/` and
  `MIDI clips/`. Presets saved with older versions are copied there automatically.

### Changed
- **DAW automation:** all parameters were already automatable; they are now grouped (Harmony,
  Progression, Groove, Tempo & MIDI, Guide sound) so the DAW's automation list is easy to browse.
- Dragged clips go into `MIDI clips/`, which is also where *Save .mid…* starts.

## [1.16.0] - 2026-10-13

### Changed
- **Livelier piano:** notes now ring into the space before the next hit like a pianist's hand
  (Gate is the share of that space), instead of all being cut short; normal hits play just below
  the accents and the default velocity is 118, so the piano sounds bright instead of dull.
- **House piano styles** (*House Piano Loop*, *'90s Piano House*, *Future House*) now use the
  new piano-stab bass.

### Added
- **New bass mode "Piano stabs (bass in every chord)":** the root plays with every chord hit,
  plus the bass's own hits: one big, full stab, the house piano way.
- **RND PIANO and RND BASS** above the rhythm grid: a new random rhythm for one hand only.
- **6 new Piano House grooves** (142 in total), made for the piano-stab bass.

## [1.15.0] - 2026-10-13

### Added
- **Automatic updates from GitHub:** when you open the plug-in it checks (once per session)
  whether a newer version exists. If so, it downloads it and shows you what's new (the changelog,
  in Italian when the plug-in speaks Italian); nothing is installed until you confirm with
  **INSTALL NOW**. Also **LATER** and **SKIP THIS VERSION**.
- **OPTIONS → SETTINGS:** *Check for updates automatically (GitHub)* switch (on by default) and a
  **CHECK NOW** button.

### Changed
- Privacy policy updated: the update check is the only internet connection, and it can be
  switched off.

## [1.14.0] - 2026-10-13

### Added
- **"Chromatic & borrowed" progressions:** 16 new presets (54 in total) with the chords dance
  producers borrow from outside the scale: maj9 chords sliding by a tone, the minor iv, bIII / bVI /
  bVII, the Neapolitan bII, dominant chords like III7, VI7 or bVII7. They load as free chords on the
  circle of fifths; the in-scale chords stay normal degrees.
- **Free dominant chords:** a free chord can now also be a dominant 7th (V7 of a minor key, backdoor
  bVII7, secondary dominants).
- **New voicing "Octave triads (disco)":** a plain triad doubled an octave up (R 3 5 R 3 5), the
  bright disco and nu disco piano.
- **New genre "Dance Stabs"** (8 grooves, 136 in total): short chords on 3-3-3-3-2-2 figures, with
  the bass doubling every stab.
- **Two new styles:** *Future House* and *Disco Piano*.

### Changed
- The progression menu now opens in four columns.

## [1.13.0] - 2026-10-13

### Added
- **16 "Piano House" grooves** (128 in total), first in the menu: the classic house piano loop
  language, rootless chords on syncopated offbeats (often every three 16ths) over an octave bass
  on the 1 and the "&" of 2.
- **STYLE → House Piano Loop**, now the first style.

### Changed
- **House piano voicings:** the voicing engine now accepts the semitone between the 9th and the
  minor 3rd inside the chord, so it plays the classic rootless shapes (Am9 = G B C E,
  b7-9-b3-5; Gmaj9 = A B D F#, 9-3-5-7) instead of avoiding them.
- **New defaults** (new instances and RESET): Rootless voicing, register C#4, gate 70%,
  velocity 108, Movement Off (one voicing per chord, the house way).
- **Random riffs:** half of them now start from the classic piano house loop figures (with an
  octave bass), and riffs no longer add inversion moves on their own.

## [1.12.0] - 2026-10-13

### Changed
- **Strum rewritten:** the roll is now a real time (up to ~90 ms at 100%) instead of a note
  value, so it sounds the same at any tempo; it accelerates slightly like a pianist's hand, never
  takes more than half the note (short stabs stay tight) and never rolls ghost notes.
- **Strum direction:** Up, Down or Alternate (up on the beat, down off it), with the button under
  the Strum knob.
- **Chord roll fill:** a flowing arpeggio of eight 32nd notes sweeping up the next chord from an
  octave below, instead of four stiff 16ths.

## [1.11.0] - 2026-10-13

### Added
- **Tips everywhere:** rest the mouse on any knob, menu or button and a tip explains what it
  does. They can be switched off in **OPTIONS → SETTINGS** ("Show tips when the mouse rests on a
  control"); the choice is remembered for all projects.
- **Standalone app in your language:** the window's Options menu (Audio/MIDI settings, save and
  load state, reset) and the audio/MIDI settings dialog now follow the interface language.

### Changed
- **Cleaner chord movement:** hits that move up or down now play the **top three notes** of the
  chord as a clean close-position inversion, like a pianist's right hand, instead of turning the
  whole 4–5 note chord over (which caused jumps and rubs at the top).

## [1.10.0] - 2026-10-13

### Added
- **Chords that move inside the bar:** a hit can now play the same chord one inversion **up**
  or **down** (▲ / ▼ in the grid, *Hit, voicing up / down* in the right-click menu), so the top
  note draws a little melody, the way a piano player builds a riff.
- **16 new "Jackin' Piano" grooves** (112 in total) built on that movement: Chicago Piano Bounce,
  Jack Stab Melody, Piano Call & Answer, Swing Piano Chops, Gospel Jack Keys, Offbeat Inversion
  Stabs, Three Three Two, Late Night Jack Piano, Piano Riff Runner, Warehouse Piano, Jackin'
  Gospel Answer, Shuffle Piano Stabs, Disco Jack Piano, Deep Jack Piano, Piano Hook, Organ Piano Jack.
- **STYLE → Jackin' Piano:** house piano, short swung stabs, top octave, push-stab fills.
- **Movement** (Harmony panel): voicing movement for **every** groove: Off, Subtle (default) or
  Lively. The chords move by inversions instead of sounding flat; the jackin', gospel, disco and
  organ styles use Lively.

### Changed
- Twelve existing grooves now move by inversions too (Classic Offbeat Stabs, Soulful Push,
  Gospel Bounce, Chicago Jack, Jackin' Stabs, Jack Bounce, Jackin' Push, Chicago Organ Jack,
  Jackin' Shuffle, Soulful Sunday, Offbeat Doubles, Funky Upstrokes) instead of repeating the
  identical chord on every hit.
- **Random riffs:** the answer phrase now moves by inversions against the motif (call and
  response with a melody on top).

## [1.9.0] - 2026-10-13

### Added
- **Top line:** choose the melody the top notes of the chords draw: Smooth (auto), Steady,
  Rising, Falling or Arch.
- **Manual inversions:** ▲▼ arrows on every chord slot (up to three steps each way).
- **Variations:** every 4 or 8 bars the last bar plays a variation of the groove.
- **Groove length:** 1, 2 or 4 bars (4 = the groove followed by its variation).
- **Walking** and **Syncopated octaves** bass.
- **LEARN:** play chords on your MIDI keyboard and they fill the progression slot by slot
  (scale chords become degrees, the others free chords).
- **User presets:** save, load and delete your own setups (PRESETS, top of the window).
- **Undo / redo** (◀ ▶ or Ctrl+Z / Ctrl+Shift+Z) for every change except the window size.
- **Save all parts to a folder:** the loop and the whole song, each as full, chords only and bass
  only, in one click (Save .mid… menu).
- **Three new guide sounds:** Wurlitzer, Gospel Organ (Leslie) and Warm Pad.
- **Chorus** and **Pump** (sidechain-style ducking on every beat) on the guide sound.
- Automatic **crescendo** through the build-up.
- **Website** (`docs/index.html`, English/Italian) and an animated demo in the README.
- The macOS build can now be **signed with a Developer ID and notarized** automatically, as soon
  as the Apple Developer secrets are added to the repository (otherwise ad-hoc signing, as before).

## [1.8.0] - 2026-10-13

### Added
- **8 new fill styles (11 in total):** Gospel run (2 beats), Chromatic walk-up, Push stabs,
  16th chord chops, Gospel thirds, Piano glissando, Blues lick and **Mix**, which plays a different
  fill every time. Every fill aims at the top note of the next chord.
- **Fills every bar**, besides every 2, 4 or 8 bars.
- Some STYLE setups now include fills (Soulful, Gospel, Jackin', '90s Piano, Organ Jack, Nu Disco,
  Funky Disco, Neo Soul).

### Changed
- **More musical grooves:** a pianist's touch in the engine: fast repeated chord hits play a
  lighter voicing (the top three notes), ghost notes only the top two, and steady 8th-note bass
  lines bounce between root and octave like a disco/house bass player. The busiest grooves were
  rewritten with lighter, more idiomatic bass lines and chops (Slap Bass Funk, Jackin' Stabs,
  Disco Jack Chops, Jackin' Shuffle, Ghetto Jack, Praise Break, Funky Clavinet 16ths, Samba House,
  Bassline House, Jack Bounce, Funky 16ths, Funk Clav Chops, Gospel Shout 16ths).

## [1.7.0] - 2026-10-13

### Added
- **62 new grooves (96 in total) in 10 genres:** Soulful, Deep, Gospel, Jackin', Garage & UK,
  Afro & Latin, Funky & Disco, '90s Piano House, Neo Soul & Broken, Chill & Pads. The groove menu
  opens in columns with one heading per genre.
- **STYLE:** twelve one-click setups by genre (Classic Deep House, Soulful House, Gospel House,
  Jackin' House, '90s Piano House, Organ Jack, Nu Disco, Funky Disco, UK Garage, Afro House, Neo
  Soul, Chill Lounge): progression, harmony, groove, feel and sound together; the key stays.
- **RESET:** puts every setting back to its default after a confirmation (the language stays).

### Changed
- **Much more musical voicings.** Every voicing is now chosen like a pianist would: no semitone
  rubs between neighbouring notes, no muddy small intervals in the low register, no minor 9ths,
  no clusters of seconds, the 9th above the 3rd, smooth voice leading and a singing top line.
  Rootless and Shell voicings now always keep the 3rd and the 7th (e.g. Am9 = C E G B).
- **Much more musical random riffs (NEW RIFF / RND).** They are now built from house comping
  figures: a half-bar motif, an answer, the motif again and a new ending (A B A B'), with
  accents on the right beats, an optional push into the next bar and a simple bass figure that
  locks to the riff, instead of random 16ths.
- The voiced progression is cached, so the new voicing engine costs no extra CPU while playing.

## [1.6.0] - 2026-10-12

### Added
- **Harmony colours:** every chord is coloured by how well it sounds with the chords next to it
  (shared notes, voice leading, root movement, staying in the key): green = sounds great,
  amber = good, grey = clashes. You see it as a bar on top of each chord slot (with a hint on
  hover) and on the dots and arrows of the circle of fifths.

### Changed
- **New circle of fifths:** the chords of the scale are highlighted with their Roman numerals,
  the centre shows key and mode, the progression is drawn as numbered dots with arrows showing
  the order, dots sit outside the rings so the chord names stay readable, repeated chords are
  spread out so every dot can be grabbed, a colour legend sits under the wheel, and tooltips
  explain every slice.

## [1.5.0] - 2026-10-12

### Added
- **5 new scales:** Locrian, Harmonic Major (gospel minor iv), Lydian Dominant (funky I7#11),
  Mixolydian b6 (soulful borrowed iv) and Phrygian Dominant (dark deep house): 13 in total.
- **4 new extensions:** 6/9, Add 9, Sus (9sus4) and *9ths + altered V* (degree V becomes a gospel
  13(b9), or 7(b9,b13) in minor keys).
- **2 new voicings:** Shell (3rd, 7th and top colour note) and Drop 2+4 (open pianist voicing).
- **Approach chord:** the gospel half-step slide into each chord change, from above or from below,
  for chords and bass (only where that hand is playing).
- **16 new progressions** (38 in total), including Gospel Plagal, Lydian Dom Funk, Soul Borrowed,
  Dark Deep, Deep House Classic, Neo Soul and eight new jackin' & funky ones (Jackin' Stab, Jack
  Swing, Funky Dominant, Disco Jack, Ghetto Funk, Phrygian Jack, Lydian Jack, Chicago Bounce).
- **Drag the dots on the circle of fifths:** drag a chord of the progression onto another chord of
  the scale (drop targets are outlined in pink) to change it.
- **Free chords:** with the switch under the circle on, drop a dot anywhere, even outside the scale
  (e.g. bVI). The slot shows "Free" and a chromatic numeral; free chords follow key changes.
- Guide: **quick harmony rules for house piano** and four new recipes.

### Changed
- The progression menu is grouped by style (*Soulful & Gospel*, *Circle of 5ths*, *Deep house*,
  *Modal & exotic*, *Jackin' & funky*) and opens in three columns instead of one endless list.
  Long menus such as *Groove* also open in columns.
- Mode names (Dorian, Lydian…) are translated in the interface languages.

## [1.4.0] - 2026-10-12

### Added
- **Interface languages:** English, Italiano, Español, Français, Deutsch and Português.
  Choose in **OPTIONS → SETTINGS**; the change is immediate and remembered for all projects
  (the first time, the plug-in follows your computer's language).
- **OPTIONS page** with two tabs: **SETTINGS** (language, interface size, mini window while
  dragging) and **ABOUT** (credits, license, links).
- **Spotify** in the credits: a **SPOTIFY** button and "Listen to my productions on Spotify" next
  to Fabio 'Pitch' Serra.

### Changed
- The **CREDITS** button is now **OPTIONS**; clicking the logo opens the ABOUT tab directly.

## [1.3.0] - 2026-10-12

### Added
- **Windows installer** (`FaseGroovePiano-Windows-Setup.exe`): choose *VST3 + Standalone*,
  *VST3 only*, *Standalone only* or a custom selection. It installs the VST3 in the standard
  folder, adds Start-menu (and optional desktop) shortcuts, shows the license and a short
  getting-started page, speaks English and Italian, and uninstalls from *Settings → Apps*.

## [1.2.0] - 2026-10-12

### Added
- **New guide sounds** (menu *Sound*): *Soulful Grand* (acoustic piano), *House Piano (M1 style)*
  and *House Organ*, next to the *Rhodes (FM)*. All synthesized, no samples, levels matched.
- **Fill style**: *Pentatonic run up*, *Pentatonic run down* or *Chord roll* (arpeggio of the next chord).
- **Strum**: rolls the chord notes like a real pianist.
- **Top +8va**: doubles the top note of every chord an octave higher (bright house piano).
- **8 new grooves**: *Soulful Sunday*, *Gospel Chops*, *Neo Soul Comping*, *Deep Sway*,
  *Deep Dub Stabs*, *Jackin' Shuffle*, *Jack Organ Pump*, *Funky Upstrokes*.
- **3 new progressions**: *Soulful Gospel I – iii – IV – V*, *Deep Dorian i – ii – IV – i*,
  *Funky Mixo I – IV – VII – IV*.
- New recipes in the guide (soulful grand, '90s house piano, jackin' organ) and an FAQ about fills.
- "Listen to my music" section in the guide and README: Fabio 'Pitch' Serra on Spotify and on the
  major digital music stores.

### Changed
- **Fills are now off by default** (they used to play every 4 bars). Turn them on in *Fills*.
  Projects saved with older versions keep their setting.
- *Play mode* moved to the Groove panel; the window is a little taller.

## [1.1.0] - 2026-10-12

This version comes from the first beta testers' suggestions. Thanks to **John Consalvo**!

### Added
- **Editable rhythm grid**: click a step to cycle hit → accent → ghost note → rest; right-click
  for *push* (anticipation) and *hold* (tied note). Your edits become the **User pattern**,
  saved with your project.
- **Parts A / B / C / D**: the 2-bar pattern is split into 4 half-bar parts. For each position
  you choose which part plays (e.g. *A A B C*), randomize it on its own (**RND**) or lock it
  (**LOCK**) so that **NEW RIFF** keeps it.
- **BASS: ON / OFF** on the grid: turns the bass on and off instantly and remembers the bass mode.
  When it's off, the bass row disappears from the grid.
- **Resizable interface** from 60% to 200% by dragging the bottom-right corner; the size is saved
  with your project.
- **Mini window while dragging**: while you drag a MIDI clip the window shrinks to a small tile so
  you can see your DAW (optional).
- **6 jackin' house grooves**: *Jackin' Stabs*, *Jack Bounce*, *Jackin' Push*,
  *Chicago Organ Jack*, *Ghetto Jack*, *Disco Jack Chops*.
- **2 jackin' progressions**: *Jackin' Minor i – iv* and *Jackin' Groove i – VII – VI – v*.
- "Jackin' house" recipe in the user guide.
- User guide, README, changelog and privacy policy in **English** and Italian.

### Changed
- Automatic GitHub builds only run for `main`, pull requests, releases and manual runs.

## [1.0.0] - 2026-10-12

First public release.

### Added
- **VST3 / AU / Standalone** plug-in for Windows, macOS (Intel and Apple Silicon) and Linux.
- 12 keys × 8 scales/modes, diatonic chords with 7ths, 9ths, 11ths/13ths and automatic handling
  of clashing tensions.
- 5 voicings (Close, Drop 2, Rootless, Wide, Quartal) with automatic voice leading and Register.
- 17 preset progressions, including **circle-of-fifths** progressions, and **DICE** for random
  progressions.
- Interactive **circle of fifths** showing the progression's path.
- 20 two-hand grooves, **NEW RIFF** (random funky riffs) with Density, swing, gate, velocity,
  humanize and pentatonic fills.
- Left-hand bass (root, octaves, root + 5th).
- Automatable **song sections** (Intro, Main, Break, Build-up, Drop, Outro) and a full 60-bar
  arrangement export with markers.
- **MIDI export**: drag & drop into the DAW (full, chords only, bass only) and *Save .mid*.
- Live MIDI out, with the bass optionally on channel 2.
- Tempo synced to the DAW (**SYNC DAW**) or set by hand.
- Guide sound (FM electric piano), **PREVIEW** with the DAW stopped and *Hold a key* mode.
- **CREDITS** page, app logo, donations (Ko-fi, PayPal), AGPLv3 license, user guide and privacy
  policy.

[1.3.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.3.0
[1.2.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.2.0
[1.1.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.1.0
[1.0.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.0.0
