# Soulful Groove Keys 🎹

Plugin **VST3 / AU** (Windows e macOS) che suona **giri di piano funky e soulful house**:
scegli tonalità, scala e progressione, e il plugin genera accordi estesi (9ª, 11ª, 13ª),
voicing da tastierista, groove ritmici sincopati con swing, basso alla mano sinistra e
fill pentatonici. Ha un **piano elettrico interno** (FM stile Rhodes) e manda tutto anche
in **uscita MIDI**, così puoi usarlo per pilotare qualsiasi piano/synth della tua DAW.

![Interfaccia](docs/screenshot.png)

## Funzioni

| Sezione | Cosa fa |
|---|---|
| **Progression** | 1–8 accordi per gradi della scala (I…VII), ogni accordo dura ½, 1, 2 o 4 battute. 12 preset: *Dorian Vamp i‑IV*, *Soulful Descent IV‑iii‑ii‑I*, *ii‑V*, *Royal Road*, *Minor Flow*, *8‑Bar Journey*… |
| **Harmony & Scales** | 12 tonalità × 8 scale/modi (Maggiore, **Dorica**, Minore naturale, Misolidia, Lidia, Frigia, Minore armonica, Minore melodica). Accordi diatonici costruiti per terze: triadi, 7ª, 9ª, 11ª/13ª (le tensioni "brutte" come la b9 su accordi non di dominante vengono tolte automaticamente). |
| **Voicing** | *Close*, *Drop 2*, *Rootless* (stile Bill Evans), *Wide* (due mani: fondamentale+7ª a sinistra), *Quartal* (quarte diatoniche, sound deep house). Il **voice leading** è automatico: ogni accordo sceglie il rivolto più vicino al precedente. **Register** sposta l'altezza della mano destra. |
| **Groove** | 10 ritmi a due mani su 2 battute: *Classic Offbeat Stabs*, *Soulful Push* (anticipi legati sull'accordo successivo), *Deep Chords*, *Funky 16ths*, *Gospel Bounce*, *Chicago Jack*, *Garage Skip*, *Late Night Rhodes*, *Disco Octaves*, *Long Chords*. Swing (fino al feel terzinato), Gate, Velocity, Humanize. |
| **Fills** | Ogni 2/4/8 battute l'ultimo quarto diventa una scaletta **pentatonica** (minore o maggiore, a seconda della scala) che porta all'accordo successivo. |
| **Bass** | Mano sinistra: Off / Fondamentale / Ottave / Fondamentale + 5ª. |
| **Play mode** | *Host transport*: suona quando premi Play nella DAW, a tempo. *Hold a key*: suona mentre tieni premuto un tasto, e **il tasto sceglie la tonalità** (perfetto per suonare live o nello Standalone). |
| **Sound & MIDI** | E‑piano interno con Tone, Reverb, Volume (disattivabile). Tutte le note generate escono anche come **MIDI**. |

## Download

Ogni push compila automaticamente il plugin su GitHub Actions (Windows, macOS universal
Intel/Apple Silicon, Linux). Scarica lo zip dalla pagina **Actions → Build → Artifacts**,
oppure dalle **Releases** quando viene creato un tag `v*` (es. `git tag v1.0.0 && git push --tags`).

### Installazione

**Windows** – copia `Soulful Groove Keys.vst3` in `C:\Program Files\Common Files\VST3\`.

**macOS** – copia
- `Soulful Groove Keys.vst3` in `~/Library/Audio/Plug-Ins/VST3/`
- `Soulful Groove Keys.component` (AU, per Logic/GarageBand) in `~/Library/Audio/Plug-Ins/Components/`

Il plugin non è firmato con un certificato Apple: se macOS lo blocca, apri il Terminale e lancia
```
xattr -cr ~/Library/Audio/Plug-Ins/VST3/"Soulful Groove Keys.vst3"
xattr -cr ~/Library/Audio/Plug-Ins/Components/"Soulful Groove Keys.component"
```

## Come usarlo nella DAW

1. Carica **Soulful Groove Keys** come strumento su una traccia.
2. Scegli un preset di progressione, una scala e un groove, poi premi Play.
3. Per usare un altro suono (es. un Rhodes o un piano campionato) spegni *Internal e‑piano*
   e instrada il MIDI in uscita:
   - **Ableton Live**: su una nuova traccia MIDI imposta *MIDI From* → la traccia del plugin → *Soulful Groove Keys*.
   - **FL Studio**: nel wrapper del plugin imposta una *MIDI output port* e la stessa porta in ingresso sullo strumento di destinazione.
   - **Reaper / Bitwig / Cubase / Studio One**: invia l'uscita MIDI della traccia a un'altra traccia strumento.
   - **Logic Pro**: Logic non legge il MIDI in uscita dagli strumenti AU: usa il suono interno, oppure registra/trascina il MIDI dal VST3 in un'altra DAW.
4. Per registrare il MIDI generato in una clip, registra sulla traccia che riceve il MIDI dal plugin.

## Compilare da sorgente

Serve CMake ≥ 3.22 e un compilatore C++17 (Visual Studio 2022 su Windows, Xcode su macOS).
JUCE 8 viene scaricato automaticamente.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release        # test del motore musicale
```

I plugin compilati si trovano in `build/SoulfulGrooveKeys_artefacts/Release/`.
Per usare una copia locale di JUCE: `-DJUCE_DIR=/percorso/JUCE`.

## Struttura del codice

```
Source/engine/Theory.*        scale, accordi diatonici, nomi accordi, voicing e voice leading (C++ puro)
Source/engine/GrooveEngine.*  pattern ritmici, progressioni, swing/humanize, fill → eventi nota
Source/EPianoVoice.h          piano elettrico FM interno
Source/PluginProcessor.*      parametri, sincronizzazione col tempo della DAW, MIDI in/out, audio
Source/PluginEditor.*         interfaccia grafica
tests/EngineTests.cpp         test del motore (nessuna dipendenza da JUCE)
```

Per aggiungere un groove, aggiungi una riga in `groovePatterns()` (`GrooveEngine.cpp`):
ogni carattere è un sedicesimo — `X` accento, `x` colpo, `>` anticipo (suona l'accordo successivo),
`=` tieni, `.` pausa.

## Licenza

Il plugin usa [JUCE](https://juce.com), disponibile con licenza AGPLv3 o con licenza commerciale
JUCE (gratuita sotto una certa soglia di fatturato). Se distribuisci il plugin, verifica quale
licenza si applica al tuo caso.
