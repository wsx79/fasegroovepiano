<p align="center"><img src="docs/logo.png" alt="Fase Groove Piano" width="720"></p>

<p align="center">
  <b>Funky piano grooves for soulful, deep &amp; jackin' house, ready to drag into your DAW.</b><br>
  by <b>Fabio 'Pitch' Serra</b> · VST3 / AU · Windows &amp; macOS · free and open source
</p>

<p align="center"><b>🇬🇧 English</b> · <a href="README.it.md">🇮🇹 Italiano</a></p>

<p align="center">
  <a href="docs/GUIDE.md"><b>📖 User guide</b></a> ·
  <a href="CHANGELOG.md">🆕 What's new</a> ·
  <a href="#download">⬇️ Download</a> ·
  <a href="#support-the-project">☕ Support the project</a> ·
  <a href="#credits">Credits</a> ·
  <a href="PRIVACY.md">Privacy</a> ·
  <a href="#license">License</a>
</p>

---

**Fase Groove Piano** is a VST3/AU instrument that **writes MIDI** piano parts: pick a key, a
scale and a progression and you get extended chords (9ths, 11ths, 13ths), keyboard-player
voicings, syncopated grooves with swing, a left-hand bass and pentatonic fills. Listen with the
guide sound, then **drag it into your DAW as a MIDI clip** and play it with your own piano,
Rhodes or synth.

![Interface](docs/screenshot.png)

## Features

- 🎹 **Harmony:** 12 keys × 8 scales/modes (Major, Dorian, Minor, Mixolydian, Lydian, Phrygian,
  Harmonic and Melodic Minor). Diatonic chords with 3–6 voices; clashing tensions are handled
  automatically.
- 🎛️ **Voicings:** Close, Drop 2, Rootless, Wide (two hands), Quartal, with automatic voice leading.
- 🔁 **Progressions:** 1–8 chords, 19 presets (including circle-of-fifths progressions) and
  **DICE** for random but musical progressions.
- ⭕ Interactive **circle of fifths** showing the path of your progression.
- 🥁 **26 two-hand grooves** (offbeat stabs, pushes, clav chops, ghost notes, gospel, garage,
  disco, **jackin' house**…) plus **NEW RIFF**, a random funky riff generator.
- ✏️ **Editable rhythm grid** split into parts **A/B/C/D** you can reorder, randomize (RND) and
  lock (LOCK), plus a quick **BASS ON/OFF**.
- 🔍 **Resizable UI** and a mini window while you drag a clip.
- 🎬 **Song sections:** Intro, Main, Break, Build-up, Drop, Outro (automatable), plus a **full
  60-bar arrangement** export with markers.
- ⏱️ **Tempo:** synced to the DAW or set by hand.
- 📤 **MIDI:** drag the clip into your DAW (full, chords only, bass only), save it as `.mid`, or
  use live MIDI out (bass on its own channel if you like).
- 🔊 **Guide sound** (electric piano) and **PREVIEW** to hear the riff with the DAW stopped.

Everything is explained in the **[user guide](docs/GUIDE.md)**.

## Download

Ready-to-use builds are on the **[Releases](https://github.com/wsx79/fasegroovepiano/releases/latest)**
page. Development builds are under **[Actions](https://github.com/wsx79/fasegroovepiano/actions) → Build → Artifacts**.

| System | Package | Contents |
|---|---|---|
| Windows 10/11 (64-bit) | `FaseGroovePiano-Windows.zip` | VST3 + Standalone |
| macOS 10.13+ (Intel & Apple Silicon) | `FaseGroovePiano-macOS.zip` | VST3 + AU + Standalone |
| Linux | `FaseGroovePiano-Linux.zip` | VST3 + Standalone |

To install, follow the [guide](docs/GUIDE.md#1-installation).

## Support the project

Fase Groove Piano is **free**. If you find it useful and want to support its development, you can
make a donation of any amount:

<p align="center">
  <a href="https://ko-fi.com/fabios1979"><b>☕ Ko-fi – ko-fi.com/fabios1979</b></a> ·
  <a href="https://paypal.me/fabios79"><b>💙 PayPal – paypal.me/fabios79</b></a>
</p>

The same links are in the plug-in: **CREDITS → DONATE: Ko-fi / DONATE: PayPal**.

Thank you! A star ⭐ on the repository or a share helps a lot too.

## Credits

<p align="center"><img src="docs/credits.png" alt="Credits" width="440"></p>

| | |
|---|---|
| **Idea, music direction and sound** | **Fabio 'Pitch' Serra** – DJ & Producer |
| **Code** | **Claude** (Anthropic) – AI coding assistant |
| **Framework** | [JUCE](https://juce.com) |

In the plug-in, open the credits with the **CREDITS** button at the top left.

## Privacy

The plug-in **collects no data and never connects to the internet**. Everything stays on your
computer. See the [privacy policy](PRIVACY.md).

## License

Fase Groove Piano is **free software**: you can use, study, modify and redistribute it under the
terms of the **GNU Affero General Public License v3.0 or later** ([LICENSE](LICENSE)).

In short:

- you can use it **for free**, including for commercial music: the license covers the code, not
  the songs you make. The MIDI and the music you create are **yours**;
- if you distribute a modified version, you must publish its source code under the same license;
- the software is provided **"as is", without warranty**.

The name **Fase Groove Piano**, the app logo and the **FS PITCH** logo belong to
Fabio 'Pitch' Serra and are not covered by the code license. If you publish a modified version,
please use a different name and logo.

Third-party components (JUCE, VST 3 SDK) and trademarks: see
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
*VST is a registered trademark of Steinberg Media Technologies GmbH.*

---

## For developers

### Building from source

You need CMake ≥ 3.22 and a C++17 compiler (Visual Studio 2022 on Windows, Xcode on macOS).
JUCE 8 is downloaded automatically.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release        # music engine tests
```

The built plug-ins are in `build/FaseGroovePiano_artefacts/Release/`.
To use a local copy of JUCE: `-DJUCE_DIR=/path/to/JUCE`.

On Linux you also need: `libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev
libxext-dev libfreetype-dev libfontconfig1-dev libxcomposite-dev`.

### Publishing a release

1. Add the new version at the top of [CHANGELOG.md](CHANGELOG.md) (English, used for the release
   text) and [CHANGELOG.it.md](CHANGELOG.it.md) (e.g. `## [1.2.0] - date`), and update `VERSION`
   in `CMakeLists.txt`, the guides (EN/IT) and the READMEs (EN/IT).
2. On GitHub open **Actions → Build → Run workflow**, enter the version in *release_tag*
   (e.g. `v1.2.0`) and run it. GitHub builds Windows, macOS and Linux and publishes a Release with
   the three zips; the release text is taken from the English changelog section.

Instead of step 2 you can also push a tag:

```bash
git tag v1.2.0 && git push origin v1.2.0
```

To fix the text of an existing release without rebuilding:
**Actions → Update release notes → Run workflow** with the tag (e.g. `v1.1.0`).

### Code layout

```
Source/engine/Theory.*        scales, diatonic chords, chord names, voicings and voice leading (pure C++)
Source/engine/GrooveEngine.*  rhythm patterns, progressions, swing/humanize, fills -> note events
Source/engine/Generators.*    random riffs, random progressions, song sections, arrangement, A/B/C/D parts
Source/EPianoVoice.h          guide sound (FM electric piano)
Source/PluginProcessor.*      parameters, tempo sync, MIDI in/out, clip export, user pattern
Source/PluginEditor.*         user interface and credits page
Source/CircleOfFifths.*       interactive circle of fifths
Source/Branding.*             logo, name, links
Resources/                    app logo (SVG), icon, FS PITCH logo
tests/EngineTests.cpp         engine tests (no JUCE dependency)
```

To add a groove, add a line to `groovePatterns()` in `GrooveEngine.cpp`: each character is a
16th note: `X` accent, `x` hit, `g` ghost note, `>` push (plays the next chord early),
`=` hold, `.` rest.

Donation and source-code links live in `Source/Branding.h` (`kofiUrl`, `paypalUrl`, `sourceUrl`);
an empty link hides its button.
