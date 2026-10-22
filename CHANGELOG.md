# Changelog

*🇮🇹 [Versione italiana](CHANGELOG.it.md)*

All notable changes to **Fase Groove Piano**, version by version.
The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and versions follow
[Semantic Versioning](https://semver.org/).

## [0.25.1] - 2026-10-22

- Big piano roll: when a linked Fase Groove Guitar is set to LEAD, the editor shows the same sparse comping notes the plug-in plays and exports (not the full groove pattern). The loop length matches the four-bar comping cycle used by the small piano-roll view.

## [0.25.0] - 2026-10-20

- Thirteen additional `Arranger:` presets coordinate palettes, articulation and balance with Fase Groove Arranger 0.4.7, including the updated Jackin in the funky/disco direction.
- Previous preset indexes and values remain available; saved projects are not rewritten and the theme and control layout stay unchanged.
- Separate keyboard, bass and strings guide rendering in new presets; centred/filtered/dry bass and an independent bass-line gate. Older states retain the original renderer.

## [0.24.0] - 2026-10-19

- Bass feel (Feel menu): Classic, Sub (the root voice-led in the low octave, E1-A2) or Sub + sidechain gaps (no bass note rings into the next beat). New instances start on Sub; older projects keep Classic.
- Groove realism (Feel menu, on in new instances): timing both ways around the beat on a slow bar-to-bar drift, the bass tighter than the chords, accents by place in the bar and over a four-bar phrase, two other takes of the groove in every phrase of four, a few off-beat stabs left out, and the last stab of a phrase answering a step up.
- Pedal & expression (Feel menu): sustain pedal (CC64) lifted on every chord change and pressed just after, expression (CC11) rising through the build-up; live and in the piano's exported clips.
- Guitar leads: the piano's answers no longer overlap or hang; an anticipation plays the next chord; answers are rolled, voiced in velocity (top note louder) and follow the section's level and Humanize; the exported clip closes every answer inside it; dropped chord notes no longer cut an answer of the same pitch.
- Fixes: a song's sections now play their own chord colours (the intro no longer sets the whole song's extensions); chord range and bass on/off revoice at once; switching from a library bass line back to a groove bass takes effect at once; the Pianist feel no longer loses top notes at block boundaries or at the clip start; the left hand no longer doubles the bass's own note, and a note struck by both belongs to the bass; an edited loop plays at the length its notes were carried to.
- Library bass lines: the 5th follows the chord (b5 on half-diminished chords), the 6th follows the key, and approach notes only play when the chord changes within the beat.
- Fills over chromatic chords run on that chord's notes; Quartal and Open 5ths voicings now move smoothly from chord to chord and follow the top line; the chord range is applied after the low-register clean-up.
- Random progressions in minor modes move like minor house (i - iv - bVII - bVI...).
- MIDI export: a single-track clip with every part puts the bass on channel 2; Separate tracks always contain Piano, Bass, Fills and Strings, so the clips of a song line up; a loop clip carries the notes still ringing at its end back to its start.
