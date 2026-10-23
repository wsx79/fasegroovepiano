# Fase Groove Piano: technical and musical audit (v0.30.0)

## Confirmed problems, fixed

| # | Area | Problem | Fix |
|---|------|---------|-----|
| 1 | Thread safety / MIDI | `setStateInformation` reset the audio thread's note bookkeeping from the message thread: `pedalDown`, `answerSounding`, `bassNoteChannels`, `compingDropped`, `wasRunning`. This was a data race. When a project loaded during playback, CC64 stayed down and the guitar-lead answer notes were never released. | Load now sets an atomic `stateRestored` flag. The next `processBlock` handles a restart there, which releases the groove notes, answers and pedal. New plug-in test. |
| 2 | State | The genre a random riff is written in (`lastGenre`) was not saved. A jackin', garage, funky or other genre riff reopened as a house riff. | Saved as the `riffGenre` state property and restored with a clamp. Old projects default to 0, which is what they played before. New plug-in test. |
| 3 | Realtime | With an edited loop playing, every block pushed every note of the loop 2 to 3 times into `pending`, then threw away the notes outside the block. That is O(loop) work per block, and on long loops the reserved 512 entries ran out, so memory was allocated on the audio thread. | Only notes inside the block are queued. Output is unchanged. |
| 4 | Realtime | A replaced edited loop (`shared_ptr`) could be freed on the audio thread if the audio thread held the last reference. | `retirePlaying()` keeps replaced loops on the message thread until the audio thread lets go of them. |
| 5 | Realtime | `outMidi` had no reserved capacity. | `ensureSize` in `prepareToPlay`. |
| 6 | Randomisation | Genre riffs (every genre outside the house family) only picked 2 of 8 grids, giving 64 possible riffs. A seed and that seed + 64 gave identical riffs (48 of 48). DENSITY had no effect. Accents and ghost notes were missing, there were no pushes, and the bass was always a single hit on 1 in each bar, so the bass dice on a genre riff changed nothing. | `shapeGenreRiff()` keeps the seed's grid choice, so the core figure is unchanged. On top of it, all from the seed, it adds density thinning and filling, a last-beat turn cell per genre, pushes (held where the style holds, short where it stabs), genre accents, genre ghost notes, the garage late skip, and a bass figure table per genre with dead notes for funky and jackin'. |
| 7 | Randomisation | The gospel random progression had only 8 outcomes (`seed % 8`). | The seed also picks where in the walking row the progression starts, among points that move by step. |

## Potential issues, not changed

- On a settings change, `patternFor()` and `resolvePattern()` build `std::string` patterns on the audio thread, so they allocate on that thread. This only happens on edits, not every block. Moving it to the message thread would be a larger refactor.
- `std::atomic_load` on `shared_ptr` is not lock-free in libstdc++, because it uses a mutex pool. The critical sections are short.
- `retiredLoops` is only cleaned the next time a loop is replaced, so one old loop can stay in memory.
- A note struck by both the chords and the bass is cut and re-struck even when the bass is on channel 2. This is an existing design choice.
- While the random riff plays, a genre change sent by a linked plug-in rewrites the riff, because the riff follows the genre. This was introduced in 0.30.

## Genres analysed

All 13 groove-menu genres: Piano House, Soulful, Deep, Gospel, Jackin', Jackin' Piano, Garage & UK, Afro & Latin, Funky & Disco, '90s Piano House, Neo Soul & Broken, Chill & Pads, Dance Stabs. These cover the presets (179 grooves), the random riff, `genreRiffFeel` (voicing, extension, scale, swing, gate), the random progressions (major, minor and gospel), the 24 library bass lines and the 11 strings modes.

- **Kept (sound):**
  - The house loop figures: dotted-8th offbeat stabs, the "a"-of-4 push, a steady touch.
  - The genre comping grids.
  - Swing as a real triplet delay.
  - The extension rules: b9 only on dominants, 7#9 only on dominants, 13 on soul dominants.
  - Minor-house moves (i, iv, bVII, bVI).
  - The tresillo for Afro.
  - The skip gaps for garage.
- **Improved, per genre:**
  - **Jackin' and Jackin' Piano:** accented "&"s, ghosts, a short "a" anticipation and offbeat or jacking basslines. A four-on-the-floor bass figure was removed because it doubles the kick.
  - **Funky & Disco:** accents on 2 and 4, ghost-heavy 16ths and octave or syncopated bass with dead notes.
  - **Gospel:** backbeat accents and a walking quarter-note bass.
  - **Garage:** 16th displacement (2-step) and skipping bass.
  - **Afro:** tresillo accents and tresillo bass.
  - **Soulful, Neo Soul and Deep:** held pushes into bar 1, sparse held basses and laid-back turn cells.
  - **Chill:** long held bass, never busy.
- The house-family path (Piano House, '90s Piano House and Dance Stabs) is untouched, so the same seed still gives the same riff.

## Dice / diversity tests (`tests/EngineTests.cpp: testGenreDiversity`)

For each of the 13 genres the test rolls 48 seeds (spread over 1..99999) at density 0.5, with that genre's progression dice. It renders 2 bars of MIDI and measures distinct chord onset sequences, bass onset sequences, velocity sequences, note-length sequences and pitch sequences. It also checks:

- the share of the single most common grid;
- seed against seed + 64 repetition;
- determinism (same seed gives byte-identical MIDI);
- sparse against busy hit counts;
- a per-genre identity rule (for example jackin' has no holds and at least 4 off-16ths, garage has a beat-long gap, Afro has a tresillo, chill has at least 6 holds and at most 8 hits).

| Genre | Before (onsets / bass / repeat64 / density) | After |
|---|---|---|
| Soulful | 41 / 1 / 48 / none | 47 / 20 / 0 / 6.5 to 8.2 |
| Deep | 39 / 1 / 48 / none | 46 / 18 / 2 / 4.1 to 6.0 |
| Gospel | 26 / 1 / 48 / none (22 progressions) | 48 / 16 / 0 / 9.9 to 14.9 (48) |
| Jackin' | 26 / 1 / 48 / none | 48 / 39 / 0 / 11.8 to 19.6 |
| Jackin' Piano | 26 / 1 / 48 / none | 48 / 14 / 0 / 10.9 to 16.2 |
| Garage | 26 / 1 / 48 / none | 40 / 14 / 0 / 6.2 to 8.5 |
| Afro | 26 / 1 / 48 / none | 47 / 12 / 0 / 10.4 to 14.8 |
| Funky & Disco | 42 / 1 / 48 / none | 48 / 45 / 0 / 13.1 to 21.6 |
| Neo Soul | 26 / 1 / 48 / none | 47 / 15 / 1 / 6.1 to 8.2 |
| Chill | 43 / 1 / 48 / none | 46 / 18 / 2 / 3.6 to 4.7 |
| House family (3) | 31 to 35 / 18 / 0 / 10.4 to 13.1 | unchanged |

Every roll in every genre passes its identity rule (48 of 48).

**Limits:** these statistics show variation and keep simple rhythmic constraints. They do not show that the music sounds good. The variation is constrained by design, using genre tables, conditional accent and ghost probabilities and turn cells, but nobody has listened to it here. House riffs have fewer distinct accent profiles by design, because they keep a steady touch.

## Build / tests

`cmake --build /tmp/b-piano -j2` succeeds. `ctest` passes 2 of 2 (engine and plugin). The existing `testGenreRiffs` still passes. New tests: `testGenreDiversity` (engine) and `testRiffGenreAndRestoreWhilePlaying` (plug-in).

## Integration notes for Fase Groove Arranger (`ThirdParty/fasegroovepiano`)

- The public API is unchanged: no signatures added or removed in `Generators.h` or `GrooveEngine.h`.
- Behaviour changes when the engine is re-vendored:
  - `makeRandomPattern(seed, density, genre)` gives different output for genres 1 to 8, 10 and 11, with richer right hands and genre left hands. The house family (0, 9, 12) is byte-identical.
  - `randomProgression(..., genre = 3)` gives different gospel walks.
  - Any Arranger snapshot or golden test of those outputs has to be updated.
- When the Arranger renders edited loops, it now gets only the notes inside the block from `GrooveEngine::render`. The note output is identical.
- Processor changes (state property `riffGenre`, the `stateRestored` flag, `retirePlaying`) only matter if the Arranger also vendors `PluginProcessor`.
