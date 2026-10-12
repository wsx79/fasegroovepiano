<p align="center"><img src="docs/logo.png" alt="Fase Groove Piano" width="720"></p>

# Fase Groove Piano

*by **Fabio 'Pitch' Serra***

Plugin **VST3 / AU** (Windows e macOS) che **crea MIDI di giri di piano funky e soulful house**:
scegli tonalità, scala e progressione, e il plugin genera accordi estesi (9ª, 11ª, 13ª),
voicing da tastierista, groove ritmici sincopati con swing, basso alla mano sinistra e
fill pentatonici.

Il MIDI è il prodotto principale: lo **trascini nella DAW come clip** (o lo salvi in `.mid`),
oppure lo mandi in tempo reale al **tuo** piano/Rhodes/synth preferito. Il plugin ha solo un
semplice **suono guida** (e‑piano) per farti sentire subito il riff.

![Interfaccia](docs/screenshot.png)

## Funzioni

| Sezione | Cosa fa |
|---|---|
| **Tempo** | **SYNC DAW** (attivo di default): il groove segue BPM e posizione della DAW. Disattivalo per **sganciare** il plugin e impostare i BPM a mano (60–180, trascina la barra o fai doppio clic per scrivere il valore): il groove gira al suo tempo, e parte e si ferma comunque col Play della DAW (o con PREVIEW). Anche le clip MIDI esportate usano il tempo scelto. |
| **Progression** | 1–8 accordi per gradi della scala (I…VII), ogni accordo dura ½, 1, 2 o 4 battute. 17 preset: *Dorian Vamp i‑IV*, *Soulful Descent IV‑iii‑ii‑I*, *ii‑V*, *Royal Road*, *Minor Flow*, *8‑Bar Journey*, e progressioni sul **circolo delle quinte** (*vi‑ii‑V‑I*, *iii‑vi‑ii‑V*, *Full Circle I‑IV‑vii‑iii‑vi‑ii‑V‑I*…). **DICE** crea una progressione casuale ma musicale, con i movimenti tipici della soulful house (ii‑V, IV‑iii‑ii, quinte discendenti…). |
| **Circle of fifths** | Circolo delle quinte interattivo: anello esterno = tonalità maggiori, interno = relative minori. Evidenzia la tonalità e le tonalità vicine (I, IV, V), e disegna il **percorso della progressione** sul circolo con l'accordo che sta suonando. Clicca una tonalità per selezionarla (anello interno → modo minore/dorico). |
| **Harmony & Scales** | 12 tonalità × 8 scale/modi (Maggiore, **Dorica**, Minore naturale, Misolidia, Lidia, Frigia, Minore armonica, Minore melodica). Accordi diatonici costruiti per terze: triadi, 7ª, 9ª, 11ª/13ª (le tensioni "brutte" come la b9 su accordi non di dominante vengono tolte automaticamente). |
| **Voicing** | *Close*, *Drop 2*, *Rootless* (stile Bill Evans), *Wide* (due mani: fondamentale+7ª a sinistra), *Quartal* (quarte diatoniche, sound deep house). Il **voice leading** è automatico: ogni accordo sceglie il rivolto più vicino al precedente. **Register** sposta l'altezza della mano destra. |
| **Groove** | 20 ritmi a due mani su 2 battute: *Classic Offbeat Stabs*, *Soulful Push* (anticipi legati sull'accordo successivo), *Deep Chords*, *Funky 16ths*, *Gospel Bounce*, *Chicago Jack*, *Garage Skip*, *Late Night Rhodes*, *Disco Octaves*, *Long Chords* e i funk: *Funk Clav Chops*, *Ghost Note Funk*, *Nile Chops*, *Anticipation Funk*, *Gospel Shout 16ths*, *Neo Soul Lazy*, *Boogie Stabs*, *Offbeat Doubles*, *Broken Beat*, *Slap Bass Funk*. Swing (fino al feel terzinato), Gate, Velocity, Humanize. |
| **NEW RIFF** | Genera sul momento un **riff funky casuale** (accordi + basso) su 2 battute: colpi sui controtempi, ghost notes, anticipi legati sulla battuta successiva, seconda battuta come "risposta" alla prima. **Density** va da stab radi a chop di sedicesimi. Il riff viene salvato col progetto (seme del riff), quindi lo ritrovi uguale alla riapertura. |
| **Song section** | *Main groove*, *Intro* (solo accenti, niente basso), *Break* (accordi lunghi tipo pad), *Build‑up* (ottavi poi sedicesimi incalzanti), *Drop* (groove pieno, più forte, basso in ottave), *Outro*. È un parametro **automatizzabile**: puoi cambiarlo nella timeline della DAW per costruire il brano. |
| **Fills** | Ogni 2/4/8 battute l'ultimo quarto diventa una scaletta **pentatonica** (minore o maggiore, a seconda della scala) che porta all'accordo successivo. |
| **Bass** | Mano sinistra: Off / Fondamentale / Ottave / Fondamentale + 5ª. |
| **Play mode** | *Host transport*: suona quando premi Play nella DAW, a tempo. *Hold a key*: suona mentre tieni premuto un tasto, e **il tasto sceglie la tonalità** (perfetto per suonare live o nello Standalone). |
| **MIDI Clip** | **DRAG MIDI TO DAW**: trascina il riquadro su una traccia e ottieni la clip MIDI del groove. Lunghezza: *Full loop* (calcolata in automatico finché progressione, ritmo e fill si ripetono), 1–32 battute, oppure **Song: intro > drop > outro** = arrangiamento completo di 60 battute (Intro 8 · Main 16 · Break 8 · Build‑up 4 · Drop 16 · Outro 8) con i **marker** delle sezioni. Note: accordi + basso, solo accordi o solo basso. **Save .mid…** salva il file dove vuoi. Una copia di ogni clip trascinata finisce in `Documenti/Fase Groove Piano/`. |
| **Guide sound & MIDI out** | Suono guida e‑piano (Tone, Reverb, Volume, disattivabile). **PREVIEW** fa partire il groove anche a DAW ferma, per ascoltare il riff mentre cambi i parametri. *MIDI out*: tutto sul canale 1 oppure **basso sul canale 2**, per mandarlo a uno strumento diverso. |

## Download

Ogni push compila automaticamente il plugin su GitHub Actions (Windows, macOS universal
Intel/Apple Silicon, Linux). Scarica lo zip dalla pagina **Actions → Build → Artifacts**,
oppure dalle **Releases** quando viene creato un tag `v*` (es. `git tag v1.0.0 && git push --tags`).

### Installazione

**Windows** – copia `Fase Groove Piano.vst3` in `C:\Program Files\Common Files\VST3\`.

**macOS** – copia
- `Fase Groove Piano.vst3` in `~/Library/Audio/Plug-Ins/VST3/`
- `Fase Groove Piano.component` (AU, per Logic/GarageBand) in `~/Library/Audio/Plug-Ins/Components/`

Il plugin non è firmato con un certificato Apple: se macOS lo blocca, apri il Terminale e lancia
```
xattr -cr ~/Library/Audio/Plug-Ins/VST3/"Fase Groove Piano.vst3"
xattr -cr ~/Library/Audio/Plug-Ins/Components/"Fase Groove Piano.component"
```

## Come usarlo nella DAW

### Metodo 1 – Clip MIDI (consigliato, funziona in tutte le DAW, anche Logic)

1. Carica **Fase Groove Piano** come strumento su una traccia.
2. Scegli progressione, scala, voicing e groove. Premi **PREVIEW** per sentire il riff col suono guida.
3. Trascina **DRAG MIDI TO DAW** sulla traccia del tuo piano/Rhodes/synth (o sull'arrangiamento):
   ottieni una clip MIDI modificabile, già a tempo e pronta da loopare.
4. Vuoi accordi e basso su strumenti diversi? Esporta due clip: *Chords only* e *Bass only*.

### Metodo 2 – MIDI in tempo reale

Spegni *Guide sound* e instrada il MIDI in uscita verso un'altra traccia strumento:
   - **Ableton Live**: su una nuova traccia MIDI imposta *MIDI From* → la traccia del plugin → *Fase Groove Piano*.
   - **FL Studio**: nel wrapper del plugin imposta una *MIDI output port* e la stessa porta in ingresso sullo strumento di destinazione.
   - **Reaper / Bitwig / Cubase / Studio One**: invia l'uscita MIDI della traccia a un'altra traccia strumento.
   - **Logic Pro**: Logic non legge il MIDI in uscita dagli strumenti AU: usa il Metodo 1 (trascina la clip).
Per registrare il MIDI generato, registra sulla traccia che riceve il MIDI dal plugin.
Con *MIDI out → Bass on channel 2* puoi separare il basso su un altro strumento.

## Compilare da sorgente

Serve CMake ≥ 3.22 e un compilatore C++17 (Visual Studio 2022 su Windows, Xcode su macOS).
JUCE 8 viene scaricato automaticamente.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release        # test del motore musicale
```

I plugin compilati si trovano in `build/FaseGroovePiano_artefacts/Release/`.
Per usare una copia locale di JUCE: `-DJUCE_DIR=/percorso/JUCE`.

## Struttura del codice

```
Source/engine/Theory.*        scale, accordi diatonici, nomi accordi, voicing e voice leading (C++ puro)
Source/engine/GrooveEngine.*  pattern ritmici, progressioni, swing/humanize, fill → eventi nota
Source/engine/Generators.*    riff casuali, progressioni casuali, sezioni del brano, arrangiamento
Source/CircleOfFifths.*       circolo delle quinte interattivo
Source/EPianoVoice.h          piano elettrico FM interno
Source/PluginProcessor.*      parametri, sincronizzazione col tempo della DAW, MIDI in/out, audio
Source/PluginEditor.*         interfaccia grafica
tests/EngineTests.cpp         test del motore (nessuna dipendenza da JUCE)
```

Per aggiungere un groove, aggiungi una riga in `groovePatterns()` (`GrooveEngine.cpp`):
ogni carattere è un sedicesimo — `X` accento, `x` colpo, `g` ghost note, `>` anticipo (suona l'accordo
successivo), `=` tieni, `.` pausa.

## Licenza

Il plugin usa [JUCE](https://juce.com), disponibile con licenza AGPLv3 o con licenza commerciale
JUCE (gratuita sotto una certa soglia di fatturato). Se distribuisci il plugin, verifica quale
licenza si applica al tuo caso.

## Crediti

<p align="center"><img src="docs/credits.png" alt="Crediti" width="460"></p>

- **Idea, direzione musicale e sound:** Fabio 'Pitch' Serra — DJ & Producer (logo FS PITCH in `Resources/fs_pitch_logo.png`)
- **Codice:** Claude (Anthropic), assistente di programmazione AI

Nel plugin i crediti si aprono col pulsante **CREDITS** in alto a sinistra.
Logo dell'app: `Resources/fgp_icon.svg` (vettoriale) e `Resources/icon_512.png`.
