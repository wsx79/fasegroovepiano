# Changelog

*🇮🇹 [Versione italiana](CHANGELOG.it.md)*

All notable changes to **Fase Groove Piano**, version by version.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [1.5.0] - 2026-10-12

### Added
- **5 new scales:** Locrian, Harmonic Major (gospel minor iv), Lydian Dominant (funky I7#11),
  Mixolydian b6 (soulful borrowed iv) and Phrygian Dominant (dark deep house): 13 in total.
- **4 new extensions:** 6/9, Add 9, Sus (9sus4) and *9ths + altered V* (degree V becomes a gospel
  13(b9), or 7(b9,b13) in minor keys).
- **2 new voicings:** Shell (3rd, 7th and top colour note) and Drop 2+4 (open pianist voicing).
- **Approach chord:** the gospel half-step slide into each chord change, from above or from below,
  for chords and bass (only where that hand is playing).
- **8 new progressions** (30 in total), including Gospel Plagal, Lydian Dom Funk, Soul Borrowed,
  Dark Deep, Phrygian Deep, Deep House Classic, Jackin' Dorian and Neo Soul.
- Guide: **quick harmony rules for house piano** and four new recipes.

### Changed
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
