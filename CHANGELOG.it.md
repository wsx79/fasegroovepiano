# Changelog

*🇬🇧 [English version](CHANGELOG.md)*

Tutte le novità di **Fase Groove Piano**, versione per versione.
Il formato segue [Keep a Changelog](https://keepachangelog.com/it/1.1.0/) e la numerazione
[Semantic Versioning](https://semver.org/lang/it/).

## [2.5.0] - 2026-10-15

### Aggiunto
- **🎲 TUTTO** (in alto, accanto a PRESET): stile, tonalità, progressione, groove e a volte armonia,
  basso, suono e feel a caso, tutti insieme. Annulla con ↶, RESET per ripartire da zero.

### Modificato
- Launchpad / Push: i quattro pulsanti in basso della colonna a destra ora sono **NUOVO RIFF** (prima
  erano BLOCCA).
- **Consuma meno CPU**: le voci del suono guida usano seni da tabella e inviluppi calcolati al volo
  invece di sin / exp / tanh a ogni campione, in media **2,3 volte più veloci** (Rhodes, organi e
  Wurlitzer fino a 3-4 volte), con lo stesso suono. L'ingresso sidechain viene analizzato solo
  quando è la sorgente della pompa.

## [2.4.0] - 2026-10-15

### Aggiunto
- **Launchpad o Push come griglia hardware** (Novation Launchpad Mini MK3, X, Pro MK3, Ableton
  Push 2 e Push 3 in User mode; OPZIONI → IMPOSTAZIONI): i pad mostrano il groove con la testina (accordi sopra, basso sotto) e lo
  modificano come un clic sulla griglia; i pulsanti tondi suonano / fermano, creano nuovi ritmi,
  accendono il sequencer e le parti, e rinnovano o bloccano le parti A-D. Funziona anche con la
  finestra del plug-in chiusa.

## [2.3.0] - 2026-10-15

### Aggiunto
- Sulla **pagina VELOCITY** i pulsanti random e la fascia A-D cambiano **solo le velocity**:
  🎲 PIANO / 🎲 BASSO / NUOVO RIFF danno nuove velocity casuali (il ritmo resta), i menu A-D
  scelgono di quale parte sono le velocity in ogni mezza battuta (es. *A A A A*), ogni parte ha il
  suo 🎲 e il suo BLOCCA.
- Un **indicatore del sidechain** accanto al menu Sidechain mostra il segnale che arriva (e
  *Pompa 0%* quando la profondità è a zero).
- Interruttore **Sequencer** (pannello Trasporto e tempo): spento, il groove non suona con la DAW,
  così la clip MIDI trascinata sulla traccia stessa del plug-in non suona doppia. Il MIDI in
  ingresso suona comunque il suono guida (e la pompa a tempo segue la DAW); ▶ fa ancora l'anteprima.

### Modificato
- Le **note tenute** (la nota che continua a suonare, non una nota nuova) nella griglia sono una
  coda attaccata alla loro nota, con il loro suggerimento. Nella pagina VELOCITY mostrano il livello
  della loro nota, e trascinandole si regola la velocity di quella nota.

### Corretto
- Il **sidechain esterno** non compariva in Ableton Live (VST3): l'ingresso ora è un vero bus
  sidechain, attivo di default. Dopo l'aggiornamento riscansiona una volta i plug-in in Live.
- Un host che rimanda al sidechain l'uscita stessa del plug-in non lo fa più abbassare.

## [2.2.0] - 2026-10-15

### Aggiunto
- **Pagina VELOCITY** (pulsante verde accanto a GRIGLIA, nella vista griglia): ogni colpo degli
  accordi e del basso è un **fader verde** alto quanto la sua velocity. Trascinane uno (o su più
  fader) per regolarla, doppio clic per tornare automatica, clic destro per azzerare una riga o
  tutto. I livelli sono relativi alla manopola Dinamica, si salvano nel progetto e in *I MIEI
  GROOVE*, seguono l'ordine A-D e i pulsanti random azzerano quelli di ciò che cambiano.
- **Sidechain regolabile** nel pannello Suono guida:
  - **Sidechain**: *Tempo (interno)*, che abbassa a tempo, o *Ingresso sidechain (esterno)*, che
    segue una traccia mandata al nuovo **ingresso sidechain** del plug-in (es. la cassa).
  - **Ritmo** (1/4, 1/8, 1/2, 1/16) per quello interno e **Rilascio** (quanto velocemente torna su
    il suono) per entrambi. *Pompa* è la profondità.

### Modificato
- L'interruttore *Suono guida* sta accanto al menu del suono, per fare spazio ai controlli del
  sidechain.

## [2.1.0] - 2026-10-15

### Aggiunto
- **Una libreria molto più grande**, tutta per soulful, deep e jackin' house, controllata su quanto
  gli accordi suonano bene insieme:
  - **40 nuove progressioni** (94 in tutto): soulful e gospel (Soul Cycle, Sunday Gospel, Classic
    Soul, Sunrise, Vocal Lift, Church Turn...), deep (Late Night Minor, Deep Float, Minor Two,
    Midnight...), jackin' (Jack Vamp, Funk Turn, Chicago Shuffle...), circolo delle quinte, accordi
    presi in prestito e modali.
  - **30 nuovi groove** (172 in tutto): loop di piano house sulle figure di crome puntate, comping
    soulful con anticipi, chop gospel, pad e accordi dub deep, sedicesimi e chop jackin'.
  - **3 nuove estensioni**: *11e ricche* (m11, maj9#11, 13 sulle dominanti), *13e soulful*,
    *7#9 funky*.
  - **3 nuovi voicing**: *Triade superiore (stab house)*, *Decime aperte (2 mani)*, *Quinte aperte
    (pad deep)*.
  - **12 nuovi stili** (28 in tutto): Sunday Soulful, Deep Night Pads, Chicago Jack, Piano House
    Anthem, Vocal House, Deep Soul Keys, Gospel Chops, Jackin' Funk, Classic Piano House, Deep
    Minimal, Soulful Lift, Jackin' Piano Chicago.
- Modalità di basso **Stab di piano (ritmo suo)**: il basso a stab di piano sul ritmo della riga del
  basso.

### Modificato
- Tutti i pulsanti casuali (🎲 CASUALE, 🎲 NUOVO RIFF, 🎲 PIANO, 🎲 BASSO, i dadi delle parti) hanno
  lo stesso aspetto: pulsante neutro con il dado viola.

### Corretto
- **🎲 PIANO cambiava anche il basso** col basso di default *Stab di piano* (che suona sotto ogni
  stab del piano). Ora il ritmo del basso che senti viene tenuto sulla riga del basso (modalità
  *Stab di piano (ritmo suo)*) e cambia solo il piano.

## [2.0.0] - 2026-10-15

### Modificato
- **Nuova interfaccia.** Un aspetto pulito da "studio": superfici grafite, un solo colore caldo
  (ambra: il piano e le azioni principali) e uno freddo (azzurro: il basso e "sta suonando"), viola
  per i fill, il font **Inter** incorporato (uguale su ogni sistema), pannelli piatti con bordi
  sottili e titoli in piccolo maiuscoletto spaziato, manopole da studio, menu, interruttori, tooltip
  e barre di scorrimento sottili.
- **Un'unica famiglia di pulsanti con icone disegnate**: acceso = attivo, i pulsanti che vanno
  insieme sono uniti, ogni gruppo ha la sua didascalia.
- **Trasporto** in alto (⏮ torna all'inizio, ▶ / ❚❚ play / pausa, ■ stop) al posto di ASCOLTA: la
  pausa riprende da dove si era fermata; la barra spaziatrice fa play / pausa.
- Interruttori **ASCOLTA 🔊 PIANO / 🔊 BASSO** sopra il piano roll (🔇 quando è muto), **NUOVO
  RITMO 🎲 PIANO / 🎲 BASSO**, **🎲 CASUALE** per la progressione, 🎲 NUOVO RIFF, annulla / ripeti,
  schermo intero e opzioni con le icone.
- Barra strumenti dell'**editor del piano roll** a gruppi con didascalia: trasporto, strumenti,
  disegna, griglia (magnete dello snap, passo della griglia, quantizza), mostra, ascolta, zoom,
  segui; ✕ per chiudere.
- L'accordo corrente in un display, una barra in alto per il logo e i comandi.

## [1.28.0] - 2026-10-14

### Aggiunto
- **Le modifiche seguono le impostazioni:** dopo aver modificato le note nel piano roll puoi ancora
  cambiare tonalità, scala, accordi, voicing, Ottava sopra, groove...: le note vengono generate di
  nuovo e le tue modifiche riportate sopra (le note cancellate restano cancellate, quelle spostate /
  allungate / più piano cambiano allo stesso modo, quelle aggiunte seguono l'accordo).
- **Selezione multipla** nel piano roll: rettangolo di selezione, Ctrl / Shift + clic, Ctrl + A;
  sposta, trasponi, cancella, quantizza e cambia la dinamica di tutte insieme.
- **ASCOLTA TUTTI / PIANO / BASSO** dentro l'editor del piano roll.
- Pulsante **schermo intero** (in una DAW il plug-in riempie lo schermo).
- **L'app Standalone si riavvia da sola dopo un aggiornamento** (copia portable: sostituita e
  riaperta; setup: si chiude, parte il setup e la riapre).

### Corretto
- La barra spaziatrice funziona appena torni sulla finestra, senza cliccarci dentro prima.

## [1.27.0] - 2026-10-14

### Aggiunto
- **Strumenti dell'editor del piano roll**, ognuno col suo cursore: MATITA, SPOSTA (freccia),
  GOMMA (clic o trascina sopra le note).
- **Spostamento libero** delle note di default; **SNAP** per agganciarle alla griglia (Alt fa il
  contrario) e **QUANTIZZA**.
- **DISEGNA PIANO / BASSO** coi colori delle note, **MOSTRA TUTTO / PIANO / BASSO**, **SEGUI** (la
  vista segue la testina).
- La **barra spaziatrice** avvia / ferma l'ascolto, e un pulsante tondo **▶ / ■** accanto ad ASCOLTA.

### Corretto
- **ASCOLTA** in VARIANTI (e nel coach dei voicing) non faceva niente con la DAW ferma e l'ascolto
  spento: ora avvia l'ascolto, e lo ferma di nuovo quando chiudi la finestra.

## [1.26.0] - 2026-10-14

### Aggiunto
- **Editor del piano roll:** **MODIFICA NOTE** (o doppio clic sul piano roll) lo apre in grande,
  con lo zoom, per modificare le note una per una: aggiungi, sposta, allunga o accorcia, elimina,
  trasponi con le frecce, e una **corsia della dinamica**. Le tue note suonano e vengono esportate
  al posto di quelle generate (groove principale e drop) fino a **TORNA AL GENERATO**; si salvano nel
  progetto e si annullano con ◀.
- **Gancio** nella linea in alto a pedale: ogni due battute l'ultimo stab passa alla nota vicina
  della scala e torna: il classico gancio del piano house.
- **Ottave della mano sinistra:** col basso a stab di piano lo stab accentato e l'anticipo
  suonano la fondamentale e la sua ottava.

### Corretto
- **Trascinare il brano intero** dalla finestra del brano: ora la finestra si riduce al riquadro
  mini come per la clip normale, così puoi rilasciarlo nella DAW (prima il plug-in copriva la DAW).

## [1.25.0] - 2026-10-14

### Modificato
- **Groove di piano nel classico stile piano house.** Gli accordi ora condividono
  **una sola nota in alto** (nuova linea in alto *Pedale house*, il default: la loro quinta,
  settima o terza, spostandosi solo sulla nota dell'accordo più vicina quando un accordo non ce
  l'ha), così il riff non sembra più fatto di note buttate a caso che saltano dentro la battuta. I
  colpi ▲/▼ sono diventati **note vicine** sulla sola voce in alto (l'accordo sotto resta); con la
  linea in alto a pedale i colpi ▲/▼ normali tengono ferma la nota in alto.
- **Basso in ogni stab, niente flam:** *Stab di piano (basso in ogni accordo)* è il basso di
  default; il basso non suona più un sedicesimo prima o dopo un accordo (sembravano due mani che
  non si trovavano).
- **NUOVO RIFF / RND PIANO** costruiscono i riff dalle figure classiche dei loop di piano house
  (la famiglia di crome puntate 3-3-3-3-2-2 e le sue parenti), ripetono la battuta come un loop
  house e hanno un tocco regolare (niente accenti o ghost note a caso).
- Gli stili house (menu STILE) usano la linea in alto a pedale.
- **Nuova disposizione della parte bassa della finestra** (un po' più alta):
  - **PIANO ROLL** (vista di default): tutte le note che suonano (accordi, basso, fill) su una
    tastiera, i nomi degli accordi sopra e la testina; **GRIGLIA** passa alla griglia ritmica.
  - **TUTTI / PIANO / BASSO** al posto dei due pulsanti SOLO.
  - Pannello **ESPORTA MIDI**: note e lunghezza, un riquadro da trascinare chiaro che mostra cosa
    contiene, **SALVA .MID…** e **BRANO INTERO…** (l'opzione della finestra mini è in OPZIONI →
    IMPOSTAZIONI).
- **Finestra del brano:** una timeline delle sezioni, i nomi delle colonne, la durata in minuti e
  l'esportazione del brano intero in un solo posto: trascinalo, **SALVA BRANO .MID…** o **UN .MID
  PER SEZIONE…**.
- **Circolo delle quinte** ridisegnato: più grande, piatto e pulito; gli accordi della scala
  risaltano, gli altri restano tenui; gli accordi che usi sono tinti col loro colore
  dell'armonia; la progressione è unita da curve morbide sotto i nomi invece che da frecce sopra.
- **MORPH:** il groove di arrivo si sceglie nel browser dei groove (generi, ricerca, preferiti) o
  con ◀ ▶, invece che da un elenco infinito.
- **FEEL:** una sua intestazione sopra le manopole, col menu **Feel ▾** e **NUOVO FEEL**.
- *Modalità play* spostata accanto a *Uscita MIDI*.

## [1.24.1] - 2026-10-14

### Corretto
- **Tempo dei file MIDI esportati:** un tempo appena impostato (con la sync della DAW spenta) non
  finiva nel file finché l'audio non era partito; ora il file ha sempre il tempo impostato in quel
  momento (o quello della DAW).
- **Tracce separate:** il basso segue l'impostazione *Uscita MIDI* come gli altri export (canale 2
  solo se è scelto *Basso sul canale 2*).

### Aggiunto
- Test: tempo esportato (manuale e DAW), canale del basso in ogni export, e un brano intero con un
  fill diverso su ogni sezione (nessuna nota appesa o doppia ai cambi).

## [1.24.0] - 2026-10-14

### Aggiunto
- **Chord Variation Lab** (**VARIANTI** nel pannello Progressione): versioni più soulful, gospel,
  deep e cromatiche della tua progressione, stessa tonalità e lunghezza (il ritmo resta); ascolta
  ognuna mentre il loop suona, TIENI o ANNULLA.
- **Humanize per voce:** *Feel* sotto la manopola Humanize (Uniforme, Pianista con la nota in alto
  che canta, Basso rilassato, Basso stretto) e **NUOVO FEEL** per un'altra umanizzazione ripetibile
  (si salva nel progetto).
- **Un fill per ogni sezione** in **BRANO**: come impostato, nessuno, o uno stile a scelta
  sull'ultima battuta della sezione; lo senti mentre quella sezione gira.
- **Tracce separate** in *Note*: un solo file MIDI con tracce Piano, Bass e Fills, tempo
  e marker delle sezioni.

### Corretto
- Alcuni testi delle finestre aggiunte dalla v1.18 (browser, Voicing Coach, BRANO, MORPH, DNA)
  restavano in inglese dopo aver cambiato lingua.

## [1.23.1] - 2026-10-14

### Corretto
- **Preset danneggiati:** un file di preset danneggiato o che non è di Fase Groove Piano non viene
  più dato per caricato; non cambia niente e un messaggio lo dice.
- **Analizzatore MIDI:** ora legge bene una nota di pickup prima del loop, i rivolti (F/A è F, non
  Am), gli stab corti sopra un basso tenuto, gli accordi sus4 e gli stab fondamentale + terza. La
  finestra del risultato mostra l'**affidabilità** (alta, media, bassa).

### Aggiunto
- Test: file di preset danneggiati / estranei / vuoti, accordi sul canale 1 e basso sul canale 2
  nel MIDI esportato, e dieci casi difficili per l'analizzatore MIDI.

## [1.23.0] - 2026-10-14

### Aggiunto
- **Musical DNA** (**DNA** sopra la griglia): cinque intenzioni, *Più soulful*, *Più funky*, *Più
  minimale*, *Più energia*, *Più raffinato*; ogni clic porta il groove un passo più in là (ritmo,
  feel, estensioni, voicing). *Tieni il ritmo* / *Tieni l'armonia* limitano cosa cambia; gli accordi
  non cambiano mai e le parti bloccate restano.

## [1.22.0] - 2026-10-14

### Aggiunto
- **Morphing del groove** (**MORPH** sopra la griglia): trasforma il tuo groove in un altro passo
  dopo passo con lo slider *Quantità*, dal vivo nella griglia; le parti bloccate restano; TIENI o
  ANNULLA.
- **Arrangiamento del brano** (**BRANO** nel pannello MIDI clip): battute di ogni sezione (o
  toglierla), **NUOVA VARIAZIONE** per una sola sezione (per esempio un Drop diverso), ritorno
  all'ORIGINALE e **SALVA OGNI SEZIONE IN .MID…** (un file per sezione). Vale per la clip *Brano* e
  per la *Sezione del brano* mentre suona; si salva nel progetto.

## [1.21.0] - 2026-10-14

### Aggiunto
- **Voicing Coach** (il pulsante **A/B** accanto a *Voicing*): due voicing della tua progressione
  affiancati, disegnati nota per nota con la linea in alto, più estensione, movimento delle voci a
  ogni cambio d'accordo, salto più grande della nota in alto, note basse impastate e semitoni
  stridenti. **ASCOLTA A / ASCOLTA B** passano dall'uno all'altro mentre il loop suona (gli accordi
  non cambiano mai), e un breve giudizio dice quale è più morbido o più compatto.

## [1.20.0] - 2026-10-14

### Aggiunto
- **Libreria di linee di basso** nel menu *Basso mano sinistra* (ora diviso in gruppi): 12
  linee di basso che seguono i tuoi accordi, ognuna col suo ritmo: *Disco* (Disco Octave Gallop,
  Disco Octave Push, Philly Walk, Boogie Line), *Funky* (Funk Sixteenths, Slap & Pop, Dead Note
  Groove), *House* (Deep House Root, Offbeat House, Garage Skip), *Soulful* (Soulful Walk, Gospel
  Walk). Usano terze, quinte, seste, settime, ottave, note stoppate e note di passaggio verso
  l'accordo successivo.
- La griglia mostra il ritmo della linea; **RND BASSO** sceglie un'altra linea; modificando una
  casella del basso il ritmo della linea diventa tuo.

### Modificato
- Gli stili *Nu Disco*, *Funky Disco*, *Gospel House* e *Disco Piano* ora usano una linea di basso.

## [1.19.0] - 2026-10-14

### Aggiunto
- **Analizzatore MIDI:** trascina un file .mid sulla finestra (o *Importa un file MIDI…* nel
  browser delle progressioni). Stima tonalità (maggiore, dorica o minore), accordi e quanto dura
  ognuno, trova il loop e lo carica come progressione, mostrandoti le note fuori scala. È una
  stima: controlla gli accordi e correggili sul circolo, o annulla.
- **Preferiti e Recenti** nei browser di groove e progressioni: clicca la stella ☆ per tenere un
  groove o una progressione; Recenti elenca gli ultimi dodici che hai provato. Valgono per tutti i
  progetti.

## [1.18.2] - 2026-10-14

### Corretto
- **Gli aggiornamenti portable ora sono "tutto o niente":** ogni file viene controllato prima di
  toccare qualsiasi cosa e, se un passaggio fallisce, tutto quello già sostituito viene rimesso
  com'era (niente plug-in aggiornato a metà).

### Aggiunto
- **Test del plug-in**, eseguiti a ogni compilazione su Windows, macOS e Linux: stato del
  progetto e preset vecchi, annulla/ripeti, file MIDI esportati (note accoppiate, canali, durata,
  marker del brano), trasporto della DAW (play, stop, salti, loop, cambi di tempo, blocchi di
  dimensioni strane, SOLO BASSO) e la sostituzione dei file dell'aggiornamento con errori simulati.

## [1.18.1] - 2026-10-13

### Corretto
- Le finestre dei groove e delle progressioni non si riaprivano dopo averle chiuse una volta.

## [1.18.0] - 2026-10-13

### Aggiunto
- **Browser dei groove:** il menu *Groove* ora è una finestra con i generi a sinistra e i groove a
  destra, ognuno col disegno del suo ritmo, una casella di ricerca e i tuoi groove salvati.
  Clic per sentire, doppio clic per chiudere. Le frecce **◀ ▶** scorrono i groove.
- **Browser delle progressioni:** la stessa finestra per *Carica una progressione…*, con gli
  accordi di ogni progressione nella tua tonalità.
- **SOLO PIANO / SOLO BASSO** sopra la griglia ritmica (SOLO PIANO sostituisce BASSO: ON / OFF).

### Modificato
- **Aggiornamenti più furbi:** una copia installata col setup di Windows si aggiorna col setup
  senza procedura guidata (stesse cartelle e componenti); una copia portable riceve lo zip e i
  suoi file vengono sostituiti dove si trovano (Windows, macOS, Linux). Solo se i file non si
  possono scrivere ti mostra il download da copiare a mano.

## [1.17.0] - 2026-10-13

### Aggiunto
- **I MIEI GROOVE** sopra la griglia ritmica: salva la griglia (le due mani) come groove tuo e
  ricaricala su qualsiasi progressione.
- **Una cartella per i tuoi file:** `Documenti/Fase Groove Piano/` con `Presets/`, `Grooves/` e
  `MIDI clips/`. I preset salvati con le versioni vecchie vengono copiati lì in automatico.

### Modificato
- **Automazione nella DAW:** tutti i parametri erano già automatizzabili; ora sono raggruppati
  (Harmony, Progression, Groove, Tempo & MIDI, Guide sound) così la lista di automazione della
  DAW è facile da sfogliare.
- Le clip trascinate vanno in `MIDI clips/`, che è anche la cartella proposta da *Salva .mid…*.

## [1.16.0] - 2026-10-13

### Modificato
- **Piano più vivo:** le note ora suonano nello spazio prima del colpo successivo come la mano
  di un pianista (il Gate è la parte di quello spazio che riempiono), invece di essere tutte
  tagliate corte; i colpi normali suonano poco sotto gli accenti e la velocity di default è 118,
  così il piano suona brillante e non spento.
- **Stili di piano house** (*House Piano Loop*, *'90s Piano House*, *Future House*) usano il
  nuovo basso a stab.

### Aggiunto
- **Nuovo tipo di basso "Stab di piano (basso in ogni accordo)":** la fondamentale suona con
  ogni colpo di accordo, più i colpi propri del basso: un unico stab grosso e pieno, alla maniera
  del piano house.
- **RND PIANO e RND BASSO** sopra la griglia ritmica: un nuovo ritmo casuale per una sola mano.
- **6 nuovi groove Piano house** (142 in totale), pensati per il basso a stab.

## [1.15.0] - 2026-10-13

### Aggiunto
- **Aggiornamenti automatici da GitHub:** quando apri il plugin controlla (una volta per
  sessione) se esiste una versione più nuova. Se c'è la scarica e ti mostra cosa c'è di nuovo (il
  changelog, in italiano se il plugin è in italiano); non installa niente finché non confermi con
  **INSTALLA ORA**. Ci sono anche **PIÙ TARDI** e **SALTA QUESTA VERSIONE**.
- **OPZIONI → IMPOSTAZIONI:** interruttore *Controlla gli aggiornamenti in automatico (GitHub)*
  (attivo di default) e pulsante **CONTROLLA ORA**.

### Modificato
- Informativa privacy aggiornata: il controllo aggiornamenti è l'unica connessione a internet e
  si può spegnere.

## [1.14.0] - 2026-10-13

### Aggiunto
- **Progressioni "Cromatiche e prestiti":** 16 preset nuovi (54 in totale) con gli accordi che i
  produttori dance prendono da fuori scala: accordi maj9 che scivolano di un tono, la iv minore,
  bIII / bVI / bVII, la napoletana bII, dominanti come III7, VI7 o bVII7. Si caricano come accordi
  liberi sul circolo delle quinte; gli accordi in scala restano gradi normali.
- **Accordi liberi di dominante:** un accordo libero ora può essere anche una settima di dominante
  (V7 di una tonalità minore, bVII7 "backdoor", dominanti secondarie).
- **Nuovo voicing "Triadi in ottava (disco)":** una triade semplice raddoppiata all'ottava sopra
  (R 3 5 R 3 5), il piano brillante della disco e della nu disco.
- **Nuovo genere "Stab dance"** (8 groove, 136 in totale): accordi corti su figure 3-3-3-3-2-2,
  col basso che raddoppia ogni colpo.
- **Due stili nuovi:** *Future House* e *Disco Piano*.

### Modificato
- Il menu delle progressioni ora si apre in quattro colonne.

## [1.13.0] - 2026-10-13

### Aggiunto
- **16 groove "Piano house"** (128 in totale), primi nel menu: il linguaggio classico dei loop di
  piano house, accordi senza fondamentale sui controtempi sincopati (spesso ogni tre sedicesimi)
  sopra un basso in ottava sull'1 e sul "e" del 2.
- **STILE → House Piano Loop**, ora il primo stile.

### Modificato
- **Voicing da piano house:** il motore ora accetta il semitono tra la 9ª e la terza minore dentro
  l'accordo, così suona le classiche forme senza fondamentale (Am9 = Sol Si Do Mi, b7-9-b3-5;
  Gmaj9 = La Si Re Fa#, 9-3-5-7) invece di evitarle.
- **Nuovi valori iniziali** (nuove istanze e RESET): voicing Rootless, registro Do#4, gate 70%,
  velocity 108, Movimento spento (un voicing per accordo, come si fa nella house).
- **Riff casuali:** metà partono dalle figure classiche dei loop di piano house (con basso in
  ottava), e i riff non aggiungono più da soli il movimento dei rivolti.

## [1.12.0] - 2026-10-13

### Modificato
- **Arpeggiato riscritto:** l'arpeggio ora dura un tempo reale (fino a ~90 ms al 100%) invece
  di un valore di nota, quindi suona uguale a qualsiasi BPM; accelera leggermente come la mano di
  un pianista, non supera mai metà della nota (gli stab corti restano stretti) e non tocca mai le
  ghost note.
- **Direzione dell'arpeggiato:** Su, Giù o Alternato (su sui tempi, giù sui controtempi), col
  pulsante sotto la manopola Arpeggiato.
- **Fill Chord roll:** un arpeggio fluido di otto trentaduesimi che sale sull'accordo successivo
  partendo un'ottava sotto, invece di quattro sedicesimi rigidi.

## [1.11.0] - 2026-10-13

### Aggiunto
- **Suggerimenti ovunque:** tieni il mouse fermo su una manopola, un menu o un pulsante e un
  suggerimento spiega a cosa serve. Si possono spegnere in **OPZIONI → IMPOSTAZIONI** ("Mostra i
  suggerimenti quando il mouse resta fermo su un comando"); la scelta vale per tutti i progetti.
- **App Standalone nella tua lingua:** il menu Opzioni della finestra (impostazioni audio/MIDI,
  salva e carica stato, ripristina) e la finestra delle impostazioni audio/MIDI ora seguono la
  lingua dell'interfaccia.

### Modificato
- **Movimento degli accordi più pulito:** i colpi che salgono o scendono ora suonano le **tre note
  alte** dell'accordo con un rivolto pulito in posizione stretta, come la mano destra di un
  pianista, invece di rivoltare tutto l'accordo da 4–5 note (che creava salti e note che
  stridevano in alto).

## [1.10.0] - 2026-10-13

### Aggiunto
- **Accordi che si muovono dentro la battuta:** un colpo ora può suonare lo stesso accordo
  rivoltato **su** o **giù** (▲ / ▼ nella griglia, *Colpo, rivolto su / giù* nel menu del clic
  destro), così la nota in alto disegna una piccola melodia, come costruisce un riff un pianista.
- **16 nuovi groove "Piano jackin'"** (112 in totale) costruiti su questo movimento: Chicago
  Piano Bounce, Jack Stab Melody, Piano Call & Answer, Swing Piano Chops, Gospel Jack Keys,
  Offbeat Inversion Stabs, Three Three Two, Late Night Jack Piano, Piano Riff Runner, Warehouse
  Piano, Jackin' Gospel Answer, Shuffle Piano Stabs, Disco Jack Piano, Deep Jack Piano, Piano
  Hook, Organ Piano Jack.
- **STILE → Jackin' Piano:** house piano, stab corti con swing, ottava in alto, fill a stab.
- **Movimento** (pannello Armonia): movimento dei rivolti per **tutti** i groove: Spento, Leggero
  (di default) o Vivace. Gli accordi si muovono di rivolto invece di suonare piatti; gli stili
  jackin', gospel, disco e organo usano Vivace.

### Modificato
- Dodici groove esistenti ora si muovono anch'essi di rivolto (Classic Offbeat Stabs, Soulful
  Push, Gospel Bounce, Chicago Jack, Jackin' Stabs, Jack Bounce, Jackin' Push, Chicago Organ Jack,
  Jackin' Shuffle, Soulful Sunday, Offbeat Doubles, Funky Upstrokes) invece di ripetere lo stesso
  identico accordo a ogni colpo.
- **Riff casuali:** la frase di risposta ora si muove di rivolto contro il motivo (botta e
  risposta con una melodia in alto).

## [1.9.0] - 2026-10-13

### Aggiunto
- **Linea in alto:** scegli la melodia che disegnano le note alte degli accordi: Morbida (auto),
  Ferma, In salita, In discesa o Ad arco.
- **Rivolti manuali:** frecce ▲▼ su ogni slot della progressione (fino a tre passi per verso).
- **Variazioni:** ogni 4 o 8 battute l'ultima battuta suona una variazione del groove.
- **Lunghezza groove:** 1, 2 o 4 battute (4 = il groove seguito dalla sua variazione).
- Basso **Walking** e a **Ottave sincopate**.
- **IMPARA:** suona gli accordi sulla tastiera MIDI e riempiono la progressione slot dopo slot
  (gli accordi della scala diventano gradi, gli altri accordi liberi).
- **Preset personali:** salva, carica ed elimina le tue impostazioni (PRESET, in alto).
- **Annulla / ripeti** (◀ ▶ o Ctrl+Z / Ctrl+Maiusc+Z) per ogni modifica tranne la dimensione
  della finestra.
- **Salva tutte le parti in una cartella:** il loop e il brano intero, ognuno completo, solo
  accordi e solo basso, con un clic (menu Save .mid…).
- **Tre nuovi suoni guida:** Wurlitzer, Organo gospel (Leslie) e Pad caldo.
- **Chorus** e **Pompa** (abbassamento a ogni battito stile sidechain) sul suono guida.
- **Crescendo** automatico nel build-up.
- **Sito web** (`docs/index.html`, inglese/italiano) e una demo animata nel README.
- La build per macOS ora può essere **firmata con un Developer ID e notarizzata** in automatico,
  appena si aggiungono al repository i secret dell'account Apple Developer (altrimenti firma ad
  hoc, come prima).

## [1.8.0] - 2026-10-13

### Aggiunto
- **8 nuovi stili di fill (11 in totale):** Gospel run (2 tempi), Chromatic walk-up, Push stabs,
  16th chord chops, Gospel thirds, Piano glissando, Blues lick e **Mix**, che a ogni fill ne suona
  uno diverso. Ogni fill punta alla nota alta dell'accordo successivo.
- **Fill a ogni battuta**, oltre che ogni 2, 4 o 8 battute.
- Alcuni STILI ora includono i fill (Soulful, Gospel, Jackin', Piano anni '90, Organ Jack, Nu
  Disco, Funky Disco, Neo Soul).

### Modificato
- **Groove più musicali:** il tocco di un pianista nel motore: i colpi d'accordo veloci ripetuti
  suonano un voicing più leggero (le tre note alte), le ghost note solo le due note alte, e le
  linee di basso a ottavi rimbalzano tra fondamentale e ottava come un bassista disco/house. I
  groove più pesanti sono stati riscritti con bassi e chop più leggeri e idiomatici (Slap Bass
  Funk, Jackin' Stabs, Disco Jack Chops, Jackin' Shuffle, Ghetto Jack, Praise Break, Funky
  Clavinet 16ths, Samba House, Bassline House, Jack Bounce, Funky 16ths, Funk Clav Chops, Gospel
  Shout 16ths).

## [1.7.0] - 2026-10-13

### Aggiunto
- **62 nuovi groove (96 in totale) in 10 generi:** Soulful, Deep, Gospel, Jackin', Garage e UK,
  Afro e latin, Funky e disco, Piano house anni '90, Neo soul e broken, Chill e pad. Il menu dei
  groove si apre a colonne con un titolo per ogni genere.
- **STILE:** dodici impostazioni pronte per genere con un clic (Classic Deep House, Soulful House,
  Gospel House, Jackin' House, '90s Piano House, Organ Jack, Nu Disco, Funky Disco, UK Garage, Afro
  House, Neo Soul, Chill Lounge): progressione, armonia, groove, feel e suono insieme; la tonalità
  resta.
- **RESET:** riporta tutte le impostazioni ai valori iniziali dopo una conferma (la lingua resta).

### Modificato
- **Voicing molto più musicali.** Ogni voicing ora viene scelto come farebbe un pianista: niente
  semitoni stridenti tra note vicine, niente intervalli stretti e impastati nel registro basso,
  niente none minori, niente grappoli di seconde, la 9ª sopra la 3ª, voice leading morbido e una
  linea in alto che canta. I voicing Rootless e Shell ora tengono sempre 3ª e 7ª (es. Am9 = C E G B).
- **Riff casuali molto più musicali (NEW RIFF / RND).** Ora sono costruiti con figure di
  comping house: un motivo di mezza battuta, una risposta, di nuovo il motivo e un finale nuovo
  (A B A B'), con gli accenti sui tempi giusti, un eventuale anticipo sulla battuta successiva e
  una figura di basso semplice che segue il riff, invece di sedicesimi a caso.
- La progressione armonizzata viene messa in cache, così il nuovo motore dei voicing non pesa
  sulla CPU mentre suona.

## [1.6.0] - 2026-10-12

### Aggiunto
- **Colori dell'armonia:** ogni accordo è colorato in base a quanto suona bene con gli accordi
  vicini (note in comune, voice leading, movimento del basso, restare nella tonalità): verde =
  suona benissimo, ambra = bene, grigio = stride. Lo vedi come barra in alto in ogni slot della
  progressione (con un suggerimento passando il mouse) e su punti e frecce del circolo delle quinte.

### Modificato
- **Nuovo circolo delle quinte:** gli accordi della scala sono evidenziati coi numeri romani, al
  centro ci sono tonalità e modo, la progressione è disegnata con punti numerati e frecce che ne
  mostrano l'ordine, i punti stanno fuori dagli anelli così i nomi degli accordi restano leggibili,
  gli accordi ripetuti sono separati così ogni punto si può afferrare, sotto la ruota c'è la
  legenda dei colori e i suggerimenti spiegano ogni spicchio.

## [1.5.0] - 2026-10-12

### Aggiunto
- **5 nuove scale:** Locrian, Harmonic Major (il iv minore gospel), Lydian Dominant (I7#11 funky),
  Mixolydian b6 (iv soulful in prestito) e Phrygian Dominant (deep house scura): 13 in totale.
- **4 nuove estensioni:** 6/9, Add 9, Sus (9sus4) e *9ths + altered V* (il V grado diventa una
  dominante gospel 13(b9), o 7(b9,b13) nelle tonalità minori).
- **2 nuovi voicing:** Shell (3ª, 7ª e nota di colore in alto) e Drop 2+4 (voicing aperto da pianista).
- **Accordo di passaggio:** lo slide gospel di mezzo tono verso ogni cambio d'accordo, da sopra o
  da sotto, per accordi e basso (solo dove quella mano sta suonando).
- **16 nuove progressioni** (38 in totale), tra cui Gospel Plagal, Lydian Dom Funk, Soul Borrowed,
  Dark Deep, Deep House Classic, Neo Soul e otto nuove jackin' e funky (Jackin' Stab, Jack Swing,
  Funky Dominant, Disco Jack, Ghetto Funk, Phrygian Jack, Lydian Jack, Chicago Bounce).
- **Punti trascinabili nel circolo delle quinte:** trascina un accordo della progressione su un
  altro accordo della scala (i bersagli sono bordati di rosa) per cambiarlo.
- **Accordi liberi:** con l'interruttore sotto il cerchio attivo, rilascia un punto ovunque, anche
  fuori scala (es. bVI). Lo slot mostra "Libero" e un numero romano cromatico; gli accordi liberi
  seguono i cambi di tonalità.
- Guida: **regole veloci di armonia per il piano house** e quattro nuove ricette.

### Modificato
- Il menu delle progressioni è diviso per stile (*Soulful e gospel*, *Circolo delle quinte*, *Deep
  house*, *Modali ed esotiche*, *Jackin' e funky*) e si apre su tre colonne invece di una lista
  infinita. Anche i menu lunghi come *Groove* si aprono a colonne.
- I nomi dei modi (Dorian, Lydian…) ora sono tradotti nelle lingue dell'interfaccia.

## [1.4.0] - 2026-10-12

### Aggiunto
- **Lingua dell'interfaccia:** English, Italiano, Español, Français, Deutsch e Português.
  Si sceglie in **OPZIONI → IMPOSTAZIONI**; il cambio è immediato e vale per tutti i progetti
  (la prima volta il plugin segue la lingua del computer).
- **Pagina OPZIONI** con due schede: **IMPOSTAZIONI** (lingua, dimensione interfaccia, finestra
  mini durante il drag) e **INFO** (crediti, licenza, link).
- **Spotify** nei crediti: pulsante **SPOTIFY** e "Ascolta le mie produzioni su Spotify" accanto a
  Fabio 'Pitch' Serra.

### Modificato
- Il pulsante **CREDITS** ora è **OPZIONI**; cliccando il logo si apre direttamente la scheda INFO.

## [1.3.0] - 2026-10-12

### Aggiunto
- **Installer per Windows** (`FaseGroovePiano-Windows-Setup.exe`): scegli *VST3 + Standalone*,
  *solo VST3*, *solo Standalone* o una selezione personalizzata. Installa il VST3 nella cartella
  standard, crea i collegamenti nel menu Start (e sul desktop, se vuoi), mostra la licenza e una
  pagina con i primi passi, è in inglese e italiano e si disinstalla da *Impostazioni → App*.

## [1.2.0] - 2026-10-12

### Aggiunto
- **Nuovi suoni guida** (menu *Sound*): *Soulful Grand* (pianoforte acustico), *House Piano
  (stile M1)* e *House Organ*, accanto al *Rhodes (FM)*. Tutti sintetizzati, senza campioni, con
  volumi allineati.
- **Fill style**: *scaletta pentatonica in salita*, *in discesa* o *Chord roll* (arpeggio
  dell'accordo successivo).
- **Strum**: arpeggia leggermente gli accordi come un pianista vero.
- **Top +8va**: raddoppia all'ottava la nota più alta di ogni accordo (piano house brillante).
- **8 nuovi groove**: *Soulful Sunday*, *Gospel Chops*, *Neo Soul Comping*, *Deep Sway*,
  *Deep Dub Stabs*, *Jackin' Shuffle*, *Jack Organ Pump*, *Funky Upstrokes*.
- **3 nuove progressioni**: *Soulful Gospel I – iii – IV – V*, *Deep Dorian i – ii – IV – i*,
  *Funky Mixo I – IV – VII – IV*.
- Nuove ricette nella guida (soulful grand, piano house anni '90, organo jackin') e una domanda
  frequente sui fill.
- Sezione "Ascolta la mia musica" nella guida e nel README: Fabio 'Pitch' Serra su Spotify e sui
  maggiori distributori digitali di musica.

### Modificato
- **I fill ora sono spenti di default** (prima suonavano ogni 4 battute). Si accendono da *Fills*.
  I progetti salvati con le versioni precedenti mantengono la loro impostazione.
- *Play mode* è stato spostato nel riquadro Groove; la finestra è un po' più alta.

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

[1.3.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.3.0
[1.2.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.2.0
[1.1.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.1.0
[1.0.0]: https://github.com/wsx79/fasegroovepiano/releases/tag/v1.0.0
