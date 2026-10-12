<p align="center"><img src="docs/logo.png" alt="Fase Groove Piano" width="720"></p>

<p align="center">
  <b>Giri di piano funky per soulful &amp; deep house, pronti da trascinare nella tua DAW.</b><br>
  by <b>Fabio 'Pitch' Serra</b> · VST3 / AU · Windows &amp; macOS · gratuito e open source
</p>

<p align="center">
  <a href="docs/GUIDA.md"><b>📖 Guida di utilizzo</b></a> ·
  <a href="#download">⬇️ Download</a> ·
  <a href="#supporta-il-progetto">☕ Supporta il progetto</a> ·
  <a href="#crediti">Crediti</a> ·
  <a href="PRIVACY.md">Privacy</a> ·
  <a href="#licenza">Licenza</a>
</p>

---

**Fase Groove Piano** è uno strumento VST3/AU che **crea MIDI** di giri di piano: scegli
tonalità, scala e progressione e ottieni accordi estesi (9ª, 11ª, 13ª), voicing da tastierista,
groove sincopati con swing, basso alla mano sinistra e fill pentatonici. Lo ascolti col suono
guida e lo **trascini nella DAW come clip MIDI**, per suonarlo col tuo piano, Rhodes o synth.

![Interfaccia](docs/screenshot.png)

## Funzioni

- 🎹 **Armonia:** 12 tonalità × 8 scale/modi (Maggiore, Dorian, Minore, Mixolydian, Lydian,
  Phrygian, Minore armonica e melodica). Accordi diatonici a 3–6 voci con tensioni gestite in automatico.
- 🎛️ **Voicing:** Close, Drop 2, Rootless, Wide (due mani), Quartal, con voice leading automatico.
- 🔁 **Progressioni:** 1–8 accordi, 17 preset (tra cui le progressioni sul circolo delle quinte)
  e **DICE** per progressioni casuali ma musicali.
- ⭕ **Circolo delle quinte** interattivo, con il percorso della progressione.
- 🥁 **20 groove** a due mani (offbeat stabs, push, clav chops, ghost notes, gospel, garage,
  disco…) più **NEW RIFF**, che genera riff funky casuali sul momento.
- 🎬 **Sezioni del brano:** Intro, Main, Break, Build-up, Drop, Outro, automatizzabili, ed
  esportazione di un **arrangiamento completo di 60 battute** con i marker.
- ⏱️ **Tempo:** agganciato alla DAW o impostato a mano.
- 📤 **MIDI:** trascina la clip nella DAW (completa, solo accordi, solo basso), salvala in
  `.mid` oppure usa il MIDI in tempo reale (basso su un canale separato, se vuoi).
- 🔊 **Suono guida** (piano elettrico) e **PREVIEW**, per ascoltare il riff anche a DAW ferma.

Tutti i dettagli sono nella **[guida di utilizzo](docs/GUIDA.md)**.

## Download

Le versioni compilate si trovano nella pagina
**[Releases](https://github.com/wsx79/fasegroovepiano/releases)** del repository. Le build di
sviluppo si trovano in **[Actions](https://github.com/wsx79/fasegroovepiano/actions) → Build → Artifacts**.

| Sistema | Pacchetto | Contenuto |
|---|---|---|
| Windows 10/11 (64 bit) | `FaseGroovePiano-Windows.zip` | VST3 + Standalone |
| macOS 10.13+ (Intel e Apple Silicon) | `FaseGroovePiano-macOS.zip` | VST3 + AU + Standalone |
| Linux | `FaseGroovePiano-Linux.zip` | VST3 + Standalone |

Per l'installazione segui la [guida](docs/GUIDA.md#1-installazione).

## Supporta il progetto

Fase Groove Piano è **gratuito**. Se ti è utile e vuoi sostenerne lo sviluppo puoi fare una
donazione libera:

<p align="center">
  <a href="https://ko-fi.com/fabios1979"><b>☕ Ko-fi – ko-fi.com/fabios1979</b></a> ·
  <a href="https://paypal.me/fabios79"><b>💙 PayPal – paypal.me/fabios79</b></a>
</p>

Le stesse donazioni si raggiungono dal plugin: **CREDITS → DONATE: Ko-fi / DONATE: PayPal**.

Grazie! Anche una stella ⭐ al repository o una condivisione aiutano tantissimo.

## Crediti

<p align="center"><img src="docs/credits.png" alt="Crediti" width="440"></p>

| | |
|---|---|
| **Idea, direzione musicale e sound** | **Fabio 'Pitch' Serra** – DJ & Producer |
| **Codice** | **Claude** (Anthropic) – assistente di programmazione AI |
| **Framework** | [JUCE](https://juce.com) |

Nel plugin i crediti si aprono col pulsante **CREDITS** in alto a sinistra.

## Privacy

Il plugin **non raccoglie dati e non si collega a internet**. Tutto resta sul tuo computer.
Vedi la [privacy policy](PRIVACY.md).

## Licenza

Fase Groove Piano è **software libero**: puoi usarlo, studiarlo, modificarlo e ridistribuirlo
secondo i termini della **GNU Affero General Public License v3.0 o successiva**
([LICENSE](LICENSE)).

In pratica:

- puoi usarlo **gratis**, anche per musica commerciale: la licenza riguarda il codice, non i
  brani che crei. Il MIDI e la musica che produci sono **tuoi**;
- se distribuisci una versione modificata, devi pubblicarne il codice sorgente con la stessa licenza;
- il software è fornito **"così com'è", senza garanzie**.

Il nome **Fase Groove Piano**, il logo dell'app e il logo **FS PITCH** appartengono a
Fabio 'Pitch' Serra e non sono coperti dalla licenza del codice. Se pubblichi una versione
modificata, usa un nome e un logo diversi.

Componenti di terze parti (JUCE, VST 3 SDK) e marchi: vedi
[THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).
*VST è un marchio registrato di Steinberg Media Technologies GmbH.*

---

## Per sviluppatori

### Compilare da sorgente

Servono CMake ≥ 3.22 e un compilatore C++17 (Visual Studio 2022 su Windows, Xcode su macOS).
JUCE 8 viene scaricato automaticamente.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
ctest --test-dir build -C Release        # test del motore musicale
```

I plugin compilati si trovano in `build/FaseGroovePiano_artefacts/Release/`.
Per usare una copia locale di JUCE: `-DJUCE_DIR=/percorso/JUCE`.

Su Linux servono anche: `libasound2-dev libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev
libxext-dev libfreetype-dev libfontconfig1-dev libxcomposite-dev`.

### Pubblicare una release

Crea e pubblica un tag: GitHub Actions compila per Windows, macOS e Linux e allega gli zip alla
release.

```bash
git tag v1.0.0 && git push origin v1.0.0
```

### Struttura del codice

```
Source/engine/Theory.*        scale, accordi diatonici, nomi accordi, voicing e voice leading (C++ puro)
Source/engine/GrooveEngine.*  pattern ritmici, progressioni, swing/humanize, fill → eventi nota
Source/engine/Generators.*    riff casuali, progressioni casuali, sezioni del brano, arrangiamento
Source/EPianoVoice.h          suono guida (piano elettrico FM)
Source/PluginProcessor.*      parametri, tempo/sincronizzazione, MIDI in/out, esportazione clip
Source/PluginEditor.*         interfaccia grafica e pagina crediti
Source/CircleOfFifths.*       circolo delle quinte interattivo
Source/Branding.*             logo, nome, link
Resources/                    logo dell'app (SVG), icona, logo FS PITCH
tests/EngineTests.cpp         test del motore (nessuna dipendenza da JUCE)
```

Per aggiungere un groove, aggiungi una riga in `groovePatterns()` (`GrooveEngine.cpp`):
ogni carattere è un sedicesimo: `X` accento, `x` colpo, `g` ghost note, `>` anticipo (suona
l'accordo successivo), `=` tieni, `.` pausa.

I link di donazione e del codice sorgente sono in `Source/Branding.h` (`kofiUrl`, `paypalUrl`,
`sourceUrl`). Se un link è vuoto, il pulsante corrispondente viene nascosto.
