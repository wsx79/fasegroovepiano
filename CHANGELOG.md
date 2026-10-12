# Changelog

*🇮🇹 [Versione italiana](CHANGELOG.it.md)*

All notable changes to **Fase Groove Piano**, version by version.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Added
- "Listen to my music" section in the guide and README: Fabio 'Pitch' Serra on Spotify and on the
  major digital music stores.

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

[1.1.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.1.0
[1.0.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.0.0
