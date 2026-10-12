# Guida di utilizzo – Fase Groove Piano

*🇬🇧 [English version](GUIDE.md)*

<p align="center"><img src="logo.png" alt="Fase Groove Piano" width="560"></p>

Fase Groove Piano **crea MIDI** di giri di piano funky per soulful e deep house: scegli tonalità,
accordi e groove, ascolti il riff col suono guida e poi **trascini la clip MIDI nella DAW** per
suonarla col tuo piano, Rhodes o synth preferito.

**Indice**

1. [Installazione](#1-installazione)
2. [Primo giro in 60 secondi](#2-primo-giro-in-60-secondi)
3. [L'interfaccia, sezione per sezione](#3-linterfaccia-sezione-per-sezione)
4. [Esportare il MIDI nella DAW](#4-esportare-il-midi-nella-daw)
5. [Usare il MIDI in tempo reale](#5-usare-il-midi-in-tempo-reale)
6. [Costruire un brano: intro, break, drop](#6-costruire-un-brano-intro-break-drop)
7. [Ricette soulful e jackin' house](#7-ricette-soulful-house)
8. [Domande frequenti e problemi](#8-domande-frequenti-e-problemi)
9. [Ascolta la mia musica](#9-ascolta-la-mia-musica)

---

## 1. Installazione

Scarica lo zip del tuo sistema (vedi il [README](../README.it.md#download)) ed estrailo.

| Sistema | File | Dove copiarlo |
|---|---|---|
| Windows | `Fase Groove Piano.vst3` | `C:\Program Files\Common Files\VST3\` |
| macOS | `Fase Groove Piano.vst3` | `~/Library/Audio/Plug-Ins/VST3/` |
| macOS (Logic, GarageBand) | `Fase Groove Piano.component` | `~/Library/Audio/Plug-Ins/Components/` |
| Tutti | `Fase Groove Piano` (Standalone) | dove vuoi: è un'app che funziona anche senza DAW |

Riapri la DAW e fai una nuova scansione dei plugin se non lo trova da sola.
Il plugin è uno **strumento** (instrument): caricalo su una traccia MIDI/strumento, non come effetto.

**macOS – "il plugin è danneggiato / non verificato":** il plugin non è firmato con un certificato
Apple a pagamento. Apri il Terminale e lancia:

```
xattr -cr ~/Library/Audio/Plug-Ins/VST3/"Fase Groove Piano.vst3"
xattr -cr ~/Library/Audio/Plug-Ins/Components/"Fase Groove Piano.component"
```

---

## 2. Primo giro in 60 secondi

1. Carica **Fase Groove Piano** su una traccia strumento.
2. In **PROGRESSION** apri *Load a progression…* e scegli **Dorian Vamp i – IV**.
3. In **GROOVE** scegli **Soulful Push**.
4. Premi **PREVIEW** (in alto a destra, nel riquadro *Guide sound*): senti subito il riff.
5. Cambia groove, voicing ed estensioni mentre suona, finché ti piace.
6. Trascina **DRAG MIDI TO DAW** sulla traccia del tuo piano preferito. Fatto!

Vuoi qualcosa di nuovo? Premi **DICE** (progressione casuale) e **NEW RIFF** (ritmo casuale).

---

## 3. L'interfaccia, sezione per sezione

<p align="center"><img src="screenshot.png" alt="Interfaccia" width="900"></p>

### Tempo (in alto)

| Controllo | Cosa fa |
|---|---|
| **SYNC DAW** acceso | Il groove segue BPM e posizione della DAW (consigliato). Accanto vedi il tempo della DAW. |
| **SYNC DAW** spento | Compare la barra dei BPM (60–180): trascinala o fai doppio clic per scrivere il valore. Il groove gira al suo tempo ma parte/si ferma comunque col Play della DAW o con PREVIEW. |

Il tempo scelto viene usato anche nelle clip MIDI esportate.

### Accordo corrente (in alto a destra)

Mostra l'accordo che sta suonando, il suo grado (es. *ii*) e tonalità/scala.
Con *Play mode → Hold a key* mostra anche "HOLD" e la tonalità scelta col tasto.

### Progression

| Controllo | Cosa fa |
|---|---|
| **Load a progression…** | 22 progressioni pronte (soulful, deep, jackin', funky, circolo delle quinte). Caricarne una imposta anche la scala consigliata. |
| **DICE** | Progressione casuale ma musicale, coi movimenti tipici della soulful house (ii–V, IV–iii–ii, quinte discendenti). Mantiene la lunghezza attuale. |
| **Length** | Quanti accordi (1–8). Quelli oltre la lunghezza appaiono spenti. |
| **Each chord** | Durata di ogni accordo: ½, 1, 2 o 4 battute. |
| **Slot 1–8** | Ogni slot è un grado della scala (I…VII). Sopra vedi il nome dell'accordo e il numero romano (maiuscolo = maggiore, minuscolo = minore, ø = semidiminuito). Lo slot che suona si illumina. |

I gradi sono relativi alla scala: cambiando tonalità o modo, la progressione si trasporta da sola.

### Harmony & Scales

| Controllo | Cosa fa |
|---|---|
| **Key** | Tonalità (C … B). |
| **Scale / mode** | Maggiore (Ionian), **Dorian** (il classico della deep house), Minor (Aeolian), Mixolydian, Lydian, Phrygian, Harmonic Minor, Melodic Minor. |
| **Extensions** | Triadi, 7ª, 9ª, 11ª/13ª. Le tensioni che stonano vengono gestite in automatico (es. la b9 sugli accordi non di dominante viene tolta, la 11ª sugli accordi maggiori diventa 13ª). |
| **Voicing** | *Close* (posizione stretta), *Drop 2* (più aperto, da tastierista jazz), *Rootless* (senza fondamentale, ideale con un basso separato), *Wide* (due mani: fondamentale + 7ª a sinistra), *Quartal* (accordi per quarte, sound deep/moderno). |
| **Left-hand bass** | La mano sinistra: Off, Root (fondamentale), Octaves (ottave), Root + 5th. |
| **Top +8va** | Raddoppia all'ottava superiore la nota più alta di ogni accordo: il classico suono di piano house brillante. |
| **Strum** | Arpeggia leggermente l'accordo dal basso verso l'alto, come un pianista vero, invece di suonare tutte le note insieme (0% = insieme, 100% = arpeggio evidente). Tra 15% e 35% suona naturale. |
| **Register** | Sposta in alto o in basso la mano destra. Gli accordi scelgono in automatico il rivolto più vicino al precedente (voice leading), così il giro scorre senza salti. |

### Circle of fifths

Anello esterno = tonalità maggiori, anello interno = relative minori.

- Lo spicchio arancione è la **tonalità attuale**; quelli evidenziati accanto sono le tonalità
  "vicine" (IV e V), dove le modulazioni suonano naturali.
- La linea rosa collega gli accordi della progressione: vedi a colpo d'occhio i **movimenti per
  quinte** (spicchi adiacenti), il punto bianco è l'accordo che sta suonando.
- **Clicca** uno spicchio per cambiare tonalità: esterno → modo maggiore, interno → modo minore
  (Dorian). Se la scala attuale è già dello stesso tipo, viene mantenuta.

### Groove

| Controllo | Cosa fa |
|---|---|
| **Groove** | 34 ritmi a due mani su 2 battute (vedi sotto), **Random riff** e **User pattern** (il tuo pattern modificato a mano sulla griglia). |
| **NEW RIFF** | Genera un riff funky nuovo (accordi + basso). Se hai bloccato qualche parte con **LOCK** (vedi *Rhythm grid*), quelle parti restano e cambia solo il resto. Riff e modifiche vengono salvati nel progetto. |
| **Song section** | Main, Intro, Break, Build-up, Drop, Outro: trasforma il groove per la sezione del brano (vedi [capitolo 6](#6-costruire-un-brano-intro-break-drop)). |
| **Fills** | **Spenti (Off)** di default. Ogni 2/4/8 battute l'ultimo quarto diventa un breve fill che porta all'accordo successivo. |
| **Fill style** | *Pentatonic run up* (scaletta in salita), *Pentatonic run down* (in discesa) o *Chord roll* (arpeggio dell'accordo successivo). |
| **Swing** | 0% = dritto, 100% = shuffle terzinato. 15–35% è il classico feel house. |
| **Gate** | Quanto durano i colpi brevi: basso = staccato, alto = legato. |
| **Velocity** | Dinamica generale. |
| **Humanize** | Piccole variazioni di tempo e dinamica, per un feel suonato a mano. |
| **Density** | Solo per i Random riff: da stab radi a chop di sedicesimi fitti. |

**I groove:** *Classic Offbeat Stabs*, *Soulful Push*, *Deep Chords*, *Funky 16ths*, *Gospel Bounce*,
*Chicago Jack*, *Garage Skip*, *Late Night Rhodes*, *Disco Octaves*, *Long Chords*,
*Funk Clav Chops*, *Ghost Note Funk*, *Nile Chops*, *Anticipation Funk*, *Gospel Shout 16ths*,
*Neo Soul Lazy*, *Boogie Stabs*, *Offbeat Doubles*, *Broken Beat*, *Slap Bass Funk* e i **jackin' house**:
*Jackin' Stabs*, *Jack Bounce*, *Jackin' Push*, *Chicago Organ Jack*, *Ghetto Jack*, *Disco Jack Chops*
(suonano al meglio con Swing 30–45%), e i nuovi della v1.2: *Soulful Sunday*, *Gospel Chops*, *Neo Soul Comping*, *Deep Sway*, *Deep Dub Stabs*, *Jackin' Shuffle*, *Jack Organ Pump*, *Funky Upstrokes*.

### Rhythm grid

Mostra il ritmo in sedicesimi su 2 battute: riga **CHORDS** (mano destra) e **BASS** (mano sinistra).
**Puoi modificarla a mano**:

- **Clic** su un passo: cambia in colpo → accento → ghost note → pausa.
- **Clic destro** (o Ctrl+clic su Mac): menu con *Hit*, *Accent*, *Ghost note*, *Push* (anticipo
  dell'accordo successivo), *Hold* (tieni la nota precedente), *Rest*.
- Alla prima modifica il groove diventa **User pattern**: il preset originale resta intatto, la
  tua versione viene salvata nel progetto.

**Parti A / B / C / D.** Il pattern è diviso in 4 parti da mezza battuta (le linee arancioni
sulla griglia). Sopra ogni parte trovi:

| Controllo | Cosa fa |
|---|---|
| **A / B / C / D** | Quale parte suona in quella posizione. Esempio: *A A B C* ripete la prima mezza battuta, *A B A B* crea un loop più ipnotico. |
| **RND** | Nuovo ritmo casuale solo per quella parte. |
| **LOCK** | Blocca la parte: **NEW RIFF** cambia tutto tranne le parti bloccate. |

**BASS: ON / OFF** (in alto a destra della griglia) accende e spegne al volo la mano sinistra,
senza aprire i menu, e ricorda il tipo di basso che avevi scelto. Da spento, la riga del basso
sparisce dalla griglia.

| Cella | Significato |
|---|---|
| Piena chiara | colpo accentato |
| Piena media | colpo normale |
| Piena tenue | ghost note (colpo leggerissimo) |
| Rosa | **push / anticipo**: suona in anticipo l'accordo successivo e lo lega sul battere |
| Barretta sottile | nota tenuta |
| Bordo bianco | posizione attuale |

### Guide sound & MIDI out

| Controllo | Cosa fa |
|---|---|
| **Guide sound** | Accende/spegne il suono interno. Spegnilo se usi solo il MIDI. |
| **Sound** | *Rhodes (FM)* (piano elettrico), *Soulful Grand* (pianoforte acustico), *House Piano (M1 style)* (il piano brillante della house anni '90) o *House Organ* (organo per gli stab jackin'). |
| **PREVIEW** | Fa suonare il groove anche a DAW ferma. |
| **MIDI out** | *All on channel 1* oppure *Bass on channel 2* (per mandare il basso a uno strumento diverso). |
| **Play mode** (nel riquadro Groove) | *Host transport*: suona col Play della DAW. *Hold a key*: suona mentre tieni premuto un tasto MIDI, e **il tasto sceglie la tonalità** (ottimo per jam dal vivo e nello Standalone). |
| **Tone / Reverb / Volume** | Brillantezza (per l'organo: i tiranti), riverbero e volume del suono guida (non influiscono sul MIDI). |

### MIDI clip

Vedi il prossimo capitolo.

### Dimensione della finestra

Trascina l'angolo in basso a destra per **ingrandire o rimpicciolire** l'interfaccia (dal 60% al
200%). La dimensione viene ricordata nel progetto.

### CREDITS

Il pulsante in alto a sinistra apre i crediti, la licenza, il link al codice sorgente e i
pulsanti per una donazione libera (**Ko-fi** o **PayPal**): il plugin è gratuito, se ti piace
puoi supportarne lo sviluppo.

---

## 4. Esportare il MIDI nella DAW

Nel riquadro **MIDI CLIP**:

1. **Clip length**
   - *Full loop*: la lunghezza giusta perché progressione, ritmo e fill si ripetano senza tagli
     (calcolata in automatico, es. 4 accordi da 1 battuta con fill ogni 8 battute → 8 battute).
   - *1–32 bars*: lunghezza fissa.
   - *Song: intro > drop > outro*: un arrangiamento completo di 60 battute (vedi capitolo 6).
2. **Notes**: *Chords + bass*, *Chords only* o *Bass only*.
3. **Trascina** il riquadro **DRAG MIDI TO DAW** sulla traccia o sulla timeline della DAW.
   In alternativa **Save .mid…** salva il file dove vuoi.

Con **Mini window while dragging** attivo, durante il trascinamento la finestra del plugin si
riduce a un piccolo riquadro, così vedi bene la DAW dove rilasciare la clip. Al rilascio torna
come prima. Se la tua DAW non gestisce bene il ridimensionamento dei plugin, disattiva l'opzione.

Ogni clip trascinata viene salvata anche in `Documenti/Fase Groove Piano/` con un nome che
descrive accordi, groove, lunghezza e BPM, per esempio
`Am9-D9 Soulful Push 4bars 124bpm.mid`.

**Trucco:** esporta *Chords only* e *Bass only* su due tracce diverse (es. un Rhodes e un synth bass).

---

## 5. Usare il MIDI in tempo reale

Se preferisci suonare in diretta (per cambiare parametri mentre il brano gira), spegni
*Guide sound* e manda il MIDI in uscita del plugin a un'altra traccia:

| DAW | Come fare |
|---|---|
| **Ableton Live** | Nuova traccia MIDI con il tuo strumento → *MIDI From*: la traccia di Fase Groove Piano → *Fase Groove Piano*. Monitor su *In*. |
| **FL Studio** | Nel wrapper del plugin (icona ingranaggio) imposta una *MIDI output port*; sullo strumento di destinazione imposta la stessa porta come *input port*. |
| **Reaper** | Sulla traccia di destinazione aggiungi un *Receive* MIDI dalla traccia del plugin. |
| **Bitwig** | Usa un *Note Receiver* sulla traccia di destinazione, sorgente: la traccia del plugin. |
| **Cubase / Studio One** | Seleziona come ingresso MIDI della traccia di destinazione l'uscita MIDI di Fase Groove Piano. |
| **Logic Pro** | Logic non accetta MIDI in uscita dagli strumenti AU: usa il **trascinamento della clip** (capitolo 4). |

Per **registrare** il MIDI generato, registra sulla traccia di destinazione.

---

## 6. Costruire un brano: intro, break, drop

**Song section** cambia il modo di suonare lo stesso groove:

| Sezione | Cosa succede |
|---|---|
| **Main groove** | Il groove così com'è. |
| **Intro** / **Outro** | Solo accenti e colpi sul battere, più piano, senza basso e senza fill. |
| **Break** | Accordi lunghi tipo pad, uno per battuta, morbidi, senza basso. |
| **Build-up** | Prima battuta in ottavi, seconda in sedicesimi incalzanti, basso che spinge. |
| **Drop** | Groove completo, più forte, basso in ottave. |

Due modi di usarla:

- **Esportazione automatica:** *Clip length → Song: intro > drop > outro* crea una clip di 60
  battute (Intro 8 · Main 16 · Break 8 · Build-up 4 · Drop 16 · Outro 8) con i **marker** delle
  sezioni, visibili nella timeline della maggior parte delle DAW.
- **Automazione:** *Song section* è un parametro automatizzabile. Disegna l'automazione nella DAW
  per cambiare sezione dove vuoi, in tempo reale.

---

## 7. Ricette soulful house

| Sound | Impostazioni |
|---|---|
| **Classico deep house anni '90** | A Dorian · *Dorian Vamp i – IV* · 9ths · Close · *Classic Offbeat Stabs* · Swing 20% · Gate 40% |
| **Kerri Chandler style** | F Minor (Aeolian) · *Late Night i – VII – VI – VII* · 11ths · Wide · *Deep Chords* · Swing 30% |
| **Gospel / soulful** | Eb Major · *Gospel Lift IV – V – vi – I* · 9ths · Drop 2 · *Gospel Bounce* · Fills ogni 4 battute |
| **Funky disco house** | C Mixolydian · *Mixo Groove* · 7ths · Close · *Nile Chops* o *Funk Clav Chops* · Bass: Octaves |
| **Jazzy late night** | D Major · *Circle of 5ths vi – ii – V – I* · 11ths · Rootless · *Neo Soul Lazy* · Humanize 40% |
| **Moderno / quartale** | G Dorian · *Deep Minor i – v* · 9ths · Quartal · *Broken Beat* · Swing 15% |
| **Soulful grand** | Eb Major · *Soulful Gospel I – iii – IV – V* · 9ths · Drop 2 · Sound: *Soulful Grand* · *Soulful Sunday* · Strum 25% · Reverb 40% |
| **Piano house anni '90** | C Minor (Aeolian) · *Jackin' Minor i – iv* · 7ths · Close · Top +8va · Sound: *House Piano (M1 style)* · *Classic Offbeat Stabs* |
| **Organo jackin'** | G Dorian · *Deep Dorian i – ii – IV – i* · 7ths · Close · Sound: *House Organ* · *Jack Organ Pump* · Swing 40% |
| **Jackin' house** | A Minor (Aeolian) · *Jackin' Minor i – iv* · 7ths · Close · *Jackin' Stabs* o *Jack Bounce* · Swing 40% · Gate 35% · Bass: Octaves |
| **Ispirazione casuale** | DICE + NEW RIFF più volte, Density 30–60%, poi esporta quello che ti piace |

---

## 8. Domande frequenti e problemi

**Alla fine di ogni giro c'è una piccola scaletta. Si può togliere?**
Sono i **fill**. Imposta **Fills** su **Off** (è il valore di partenza dalla v1.2) oppure scegli
un altro **Fill style**.

**Non sento nulla.**
Controlla che *Guide sound* sia acceso e premi **PREVIEW**, oppure premi Play nella DAW
(*Play mode: Host transport*). In *Hold a key* devi tenere premuto un tasto MIDI.

**Nello Standalone non parte.**
Lo Standalone non ha una DAW: usa **PREVIEW** oppure *Play mode → Hold a key*. Per cambiare il
tempo spegni *SYNC DAW* e imposta i BPM.

**Il groove non è a tempo con la DAW.**
Accendi *SYNC DAW*.

**Il trascinamento non funziona nella mia DAW.**
Usa *Save .mid…* e importa il file, oppure prendilo da `Documenti/Fase Groove Piano/`.

**La DAW non trova il plugin.**
Verifica la cartella di installazione (capitolo 1) e rifai la scansione dei plugin. Su macOS
lancia anche il comando `xattr` indicato sopra.

**I progetti si ricordano le impostazioni?**
Sì, tutti i parametri (compreso il riff casuale) vengono salvati con il progetto della DAW.

**Cosa c'è di nuovo nell'ultima versione?**
Trovi tutte le novità, versione per versione, nel [changelog](../CHANGELOG.it.md) e nella pagina
[Releases](https://github.com/wsx79/fasegroovepiano/releases).

**Il plugin si collega a internet o raccoglie dati?**
No. Vedi la [privacy policy](../PRIVACY.it.md).

---

## 9. Ascolta la mia musica

Fase Groove Piano è creato da **Fabio 'Pitch' Serra**, DJ e producer. Se ti piace il sound di
questo plugin, ti piaceranno anche le mie produzioni:

- 🎧 **Ascolta le mie produzioni su Spotify:** [Fabio 'Pitch' Serra su Spotify](https://open.spotify.com/artist/29iQGuXML8LEw3xRp1dNRP)
- 🛒 **Scarica le mie canzoni** sui maggiori distributori digitali di musica: cerca
  **Fabio 'Pitch' Serra**.

Grazie per l'ascolto e per supportare la musica indipendente! 🎹🔥
