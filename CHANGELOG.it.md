# Changelog

*🇬🇧 [English version](CHANGELOG.md)*

Tutte le novità di **Fase Groove Piano**, versione per versione.
Il formato segue [Keep a Changelog](https://keepachangelog.com/it/1.1.0/) e la numerazione
[Semantic Versioning](https://semver.org/lang/it/).

## [Non ancora rilasciato]

### Aggiunto
- Sezione "Ascolta la mia musica" nella guida e nel README: Fabio 'Pitch' Serra su Spotify e sui
  maggiori distributori digitali di musica.

## [1.1.0] - 2026-10-12

Versione nata dai suggerimenti dei primi beta tester. Grazie a **John Consalvo**!

### Aggiunto
- **Griglia ritmica modificabile**: clic su un passo per colpo → accento → ghost note → pausa;
  clic destro per *push* (anticipo) e *hold* (nota tenuta). Le modifiche diventano lo
  **User pattern**, salvato nel progetto.
- **Parti A / B / C / D**: il pattern di 2 battute è diviso in 4 parti da mezza battuta.
  Per ogni posizione puoi scegliere quale parte suona (es. *A A B C*), randomizzarla da sola
  (**RND**) o bloccarla (**LOCK**) così che **NEW RIFF** la mantenga.
- **BASS: ON / OFF** sulla griglia: accende e spegne il basso al volo e ricorda il tipo di
  basso scelto. Da spento, la riga del basso sparisce dalla griglia.
- **Interfaccia ridimensionabile** dal 60% al 200% trascinando l'angolo in basso a destra;
  la dimensione viene salvata nel progetto.
- **Mini window while dragging**: durante il trascinamento della clip MIDI la finestra si riduce
  a un piccolo riquadro per lasciare libera la DAW (disattivabile).
- **6 groove jackin' house**: *Jackin' Stabs*, *Jack Bounce*, *Jackin' Push*,
  *Chicago Organ Jack*, *Ghetto Jack*, *Disco Jack Chops*.
- **2 progressioni jackin'**: *Jackin' Minor i – iv* e *Jackin' Groove i – VII – VI – v*.
- Ricetta "Jackin' house" nella guida.
- Guida d'uso, README, changelog e privacy policy anche in **inglese**.

### Modificato
- Le build automatiche su GitHub partono solo per `main`, pull request, release e avvii manuali.

## [1.0.0] - 2026-10-12

Prima versione pubblica.

### Aggiunto
- Plugin **VST3 / AU / Standalone** per Windows, macOS (Intel e Apple Silicon) e Linux.
- 12 tonalità × 8 scale/modi, accordi diatonici con 7ª, 9ª, 11ª/13ª e tensioni gestite in automatico.
- 5 voicing (Close, Drop 2, Rootless, Wide, Quartal) con voice leading automatico e Register.
- 17 progressioni pronte, compresi i giri sul **circolo delle quinte**, e **DICE** per
  progressioni casuali.
- **Circolo delle quinte** interattivo con il percorso della progressione.
- 20 groove a due mani, **NEW RIFF** (riff funky casuali) con Density, swing, gate, velocity,
  humanize e fill pentatonici.
- Basso alla mano sinistra (fondamentale, ottave, fondamentale + 5ª).
- **Sezioni del brano** (Intro, Main, Break, Build-up, Drop, Outro) automatizzabili ed
  esportazione di un arrangiamento completo di 60 battute con i marker.
- **Esportazione MIDI**: drag & drop nella DAW (completo, solo accordi, solo basso) e *Save .mid*.
- MIDI in uscita in tempo reale, con il basso eventualmente sul canale 2.
- Tempo agganciato alla DAW (**SYNC DAW**) o impostato a mano.
- Suono guida (piano elettrico FM), **PREVIEW** a DAW ferma e modalità *Hold a key*.
- Pagina **CREDITS**, logo dell'app, donazioni (Ko-fi, PayPal), licenza AGPLv3, guida d'uso e
  privacy policy.

[1.1.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.1.0
[1.0.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.0.0
