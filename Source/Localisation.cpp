// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Localisation.h"

#include <atomic>
#include <unordered_map>

namespace Localisation
{
namespace
{
    struct Entry { const char* en; const char* it; const char* es; const char* fr; const char* de; const char* pt; };

    // clang-format off
    const Entry table[] {
        // ---- Panels ----
        { "TEMPO", "TEMPO", "TEMPO", "TEMPO", "TEMPO", "ANDAMENTO" },
        { "PROGRESSION", "PROGRESSIONE", "PROGRESIÓN", "PROGRESSION", "AKKORDFOLGE", "PROGRESSÃO" },
        { "HARMONY & SCALES", "ARMONIA E SCALE", "ARMONÍA Y ESCALAS", "HARMONIE ET GAMMES", "HARMONIE & SKALEN", "HARMONIA E ESCALAS" },
        { "CIRCLE OF FIFTHS", "CIRCOLO DELLE QUINTE", "CÍRCULO DE QUINTAS", "CYCLE DES QUINTES", "QUINTENZIRKEL", "CÍRCULO DAS QUINTAS" },
        { "GROOVE", "GROOVE", "GROOVE", "GROOVE", "GROOVE", "GROOVE" },
        { "GUIDE SOUND & MIDI OUT", "SUONO GUIDA E MIDI OUT", "SONIDO GUÍA Y SALIDA MIDI", "SON GUIDE ET SORTIE MIDI", "GUIDE-SOUND & MIDI-AUSGANG", "SOM GUIA E SAÍDA MIDI" },
        { "RHYTHM GRID", "GRIGLIA RITMICA", "REJILLA RÍTMICA", "GRILLE RYTHMIQUE", "RHYTHMUSRASTER", "GRADE RÍTMICA" },
        { "MIDI CLIP", "CLIP MIDI", "CLIP MIDI", "CLIP MIDI", "MIDI-CLIP", "CLIPE MIDI" },

        // ---- Captions ----
        { "Progression presets", "Progressioni pronte", "Progresiones", "Progressions prêtes", "Akkordfolgen", "Progressões prontas" },
        { "Length", "Lunghezza", "Longitud", "Longueur", "Länge", "Duração" },
        { "Each chord", "Ogni accordo", "Cada acorde", "Chaque accord", "Pro Akkord", "Cada acorde" },
        { "Key", "Tonalità", "Tonalidad", "Tonalité", "Tonart", "Tom" },
        { "Scale / mode", "Scala / modo", "Escala / modo", "Gamme / mode", "Skala / Modus", "Escala / modo" },
        { "Extensions", "Estensioni", "Extensiones", "Extensions", "Erweiterungen", "Extensões" },
        { "Voicing", "Voicing", "Voicing", "Voicing", "Voicing", "Voicing" },
        { "Left-hand bass", "Basso mano sinistra", "Bajo mano izquierda", "Basse main gauche", "Bass linke Hand", "Baixo mão esquerda" },
        { "Register", "Registro", "Registro", "Registre", "Lage", "Registro" },
        { "Strum", "Arpeggiato", "Rasgueo", "Arpège", "Anschlag", "Dedilhado" },
        { "Groove", "Groove", "Groove", "Groove", "Groove", "Groove" },
        { "Song section", "Sezione del brano", "Sección", "Section du morceau", "Song-Teil", "Seção da música" },
        { "Fills", "Fill", "Rellenos", "Fills", "Fills", "Viradas" },
        { "Fill style", "Stile fill", "Estilo de relleno", "Style de fill", "Fill-Stil", "Estilo da virada" },
        { "Play mode", "Modalità play", "Modo de reproducción", "Mode de lecture", "Wiedergabe", "Modo de reprodução" },
        { "Swing", "Swing", "Swing", "Swing", "Swing", "Swing" },
        { "Gate", "Gate", "Gate", "Gate", "Gate", "Gate" },
        { "Velocity", "Dinamica", "Dinámica", "Vélocité", "Dynamik", "Velocidade" },
        { "Humanize", "Umanizza", "Humano", "Humain", "Human", "Humano" },
        { "Density", "Densità", "Densidad", "Densité", "Dichte", "Densidade" },
        { "Sound", "Suono", "Sonido", "Son", "Klang", "Som" },
        { "MIDI out", "Uscita MIDI", "Salida MIDI", "Sortie MIDI", "MIDI-Ausgang", "Saída MIDI" },
        { "Tone", "Timbro", "Tono", "Timbre", "Klangfarbe", "Timbre" },
        { "Reverb", "Riverbero", "Reverb", "Réverb", "Hall", "Reverb" },
        { "Volume", "Volume", "Volumen", "Volume", "Lautstärke", "Volume" },
        { "Clip length", "Lunghezza clip", "Longitud del clip", "Longueur du clip", "Clip-Länge", "Duração do clipe" },
        { "Notes", "Note", "Notas", "Notes", "Noten", "Notas" },
        { "chord", "accordo", "acorde", "accord", "Akkord", "acorde" },
        { "chords", "accordi", "acordes", "accords", "Akkorde", "acordes" },
        { "Language", "Lingua", "Idioma", "Langue", "Sprache", "Idioma" },
        { "Interface size", "Dimensione interfaccia", "Tamaño de la interfaz", "Taille de l'interface", "Größe der Oberfläche", "Tamanho da interface" },

        // ---- Buttons ----
        { "DICE", "DADO", "DADO", "DÉ", "WÜRFEL", "DADO" },
        { "NEW RIFF", "NUOVO RIFF", "NUEVO RIFF", "NOUVEAU RIFF", "NEUER RIFF", "NOVO RIFF" },
        { "PREVIEW", "ASCOLTA", "ESCUCHAR", "ÉCOUTER", "VORHÖREN", "OUVIR" },
        { "SYNC DAW", "SYNC DAW", "SYNC DAW", "SYNC DAW", "SYNC DAW", "SYNC DAW" },
        { "Guide sound", "Suono guida", "Sonido guía", "Son guide", "Guide-Sound", "Som guia" },
        { "Top +8va", "Ottava sopra", "Octava arriba", "Octave haute", "Oktave oben", "Oitava acima" },
        { "Save .mid...", "Salva .mid...", "Guardar .mid...", "Enregistrer .mid...", ".mid speichern...", "Salvar .mid..." },
        { "Mini window while dragging", "Finestra mini durante il drag", "Ventana mini al arrastrar", "Mini fenêtre pendant le glisser", "Mini-Fenster beim Ziehen", "Janela mini ao arrastar" },
        { "OPTIONS", "OPZIONI", "OPCIONES", "OPTIONS", "OPTIONEN", "OPÇÕES" },
        { "SETTINGS", "IMPOSTAZIONI", "AJUSTES", "RÉGLAGES", "EINSTELLUNGEN", "AJUSTES" },
        { "ABOUT", "INFO", "ACERCA DE", "À PROPOS", "ÜBER", "SOBRE" },
        { "CLOSE", "CHIUDI", "CERRAR", "FERMER", "SCHLIESSEN", "FECHAR" },
        { "RND", "RND", "ALE", "ALÉ", "ZUF", "ALE" },
        { "LOCK", "BLOCCA", "FIJAR", "BLOQ.", "SPERRE", "TRAVAR" },
        { "BASS: ON", "BASSO: ON", "BAJO: ON", "BASSE : ON", "BASS: AN", "BAIXO: ON" },
        { "BASS: OFF", "BASSO: OFF", "BAJO: OFF", "BASSE : OFF", "BASS: AUS", "BAIXO: OFF" },
        { "DRAG MIDI TO DAW", "TRASCINA MIDI NELLA DAW", "ARRASTRA EL MIDI A LA DAW", "GLISSER LE MIDI DANS LA DAW", "MIDI IN DIE DAW ZIEHEN", "ARRASTE O MIDI PARA A DAW" },
        { "SOURCE CODE", "CODICE SORGENTE", "CÓDIGO FUENTE", "CODE SOURCE", "QUELLCODE", "CÓDIGO-FONTE" },
        { "DONATE: Ko-fi", "DONA: Ko-fi", "DONAR: Ko-fi", "DON : Ko-fi", "SPENDEN: Ko-fi", "DOAR: Ko-fi" },
        { "DONATE: PayPal", "DONA: PayPal", "DONAR: PayPal", "DON : PayPal", "SPENDEN: PayPal", "DOAR: PayPal" },
        { "SPOTIFY", "SPOTIFY", "SPOTIFY", "SPOTIFY", "SPOTIFY", "SPOTIFY" },

        // ---- Texts ----
        { "Load a progression...", "Carica una progressione...", "Carga una progresión...", "Charger une progression...", "Akkordfolge laden...", "Carregar uma progressão..." },
        { "Drop it on any instrument track (your own piano / Rhodes)", "Rilasciala su una traccia strumento (il tuo piano / Rhodes)", "Suéltalo en una pista de instrumento (tu piano / Rhodes)", "Déposez-le sur une piste d'instrument (votre piano / Rhodes)", "Auf eine Instrumentenspur ziehen (dein Piano / Rhodes)", "Solte numa faixa de instrumento (seu piano / Rhodes)" },
        { "Drop the clip on a track in your DAW", "Rilascia la clip su una traccia della DAW", "Suelta el clip en una pista de tu DAW", "Déposez le clip sur une piste de la DAW", "Clip auf eine Spur der DAW ziehen", "Solte o clipe numa faixa da DAW" },
        { "Bass off  -  click BASS to turn it back on", "Basso spento  -  clicca BASSO per riaccenderlo", "Bajo apagado  -  pulsa BAJO para activarlo", "Basse coupée  -  cliquez sur BASSE pour la rallumer", "Bass aus  -  BASS klicken zum Einschalten", "Baixo desligado  -  clique em BAIXO para ligar" },
        { "CHORDS", "ACCORDI", "ACORDES", "ACCORDS", "AKKORDE", "ACORDES" },
        { "BASS", "BASSO", "BAJO", "BASSE", "BASS", "BAIXO" },
        { "by", "di", "de", "par", "von", "por" },
        { "HOLD: ", "TIENI: ", "MANTENER: ", "MAINTIEN : ", "HALTEN: ", "SEGURAR: " },
        { "(DAW)", "(DAW)", "(DAW)", "(DAW)", "(DAW)", "(DAW)" },
        { "Settings are saved for all your projects.", "Le impostazioni valgono per tutti i tuoi progetti.", "Los ajustes se guardan para todos tus proyectos.", "Les réglages s'appliquent à tous vos projets.", "Einstellungen gelten für alle Projekte.", "Os ajustes valem para todos os seus projetos." },

        // ---- Grid menu ----
        { "Bass", "Basso", "Bajo", "Basse", "Bass", "Baixo" },
        { "Chords", "Accordi", "Acordes", "Accords", "Akkorde", "Acordes" },
        { "step", "passo", "paso", "pas", "Schritt", "passo" },
        { "Hit", "Colpo", "Golpe", "Coup", "Anschlag", "Toque" },
        { "Accent", "Accento", "Acento", "Accent", "Akzent", "Acento" },
        { "Ghost note", "Ghost note", "Nota fantasma", "Ghost note", "Ghost Note", "Ghost note" },
        { "Push (anticipate the next chord)", "Push (anticipa l'accordo successivo)", "Push (anticipa el siguiente acorde)", "Push (anticipe l'accord suivant)", "Push (nächsten Akkord vorziehen)", "Push (antecipa o próximo acorde)" },
        { "Hold (tie from the previous note)", "Tieni (lega la nota precedente)", "Mantener (liga la nota anterior)", "Tenir (lie la note précédente)", "Halten (vorherige Note binden)", "Segurar (liga a nota anterior)" },
        { "Rest", "Pausa", "Silencio", "Silence", "Pause", "Pausa" },

        // ---- Tooltips ----
        { "Double the top note of every chord an octave higher (bright house piano)", "Raddoppia all'ottava la nota alta di ogni accordo (piano house brillante)", "Duplica una octava arriba la nota más alta de cada acorde (piano house brillante)", "Double la note aiguë de chaque accord à l'octave (piano house brillant)", "Verdoppelt den höchsten Ton jedes Akkords eine Oktave höher (helles House-Piano)", "Dobra a nota mais aguda de cada acorde uma oitava acima (piano house brilhante)" },
        { "New random funky riff (locked parts are kept)", "Nuovo riff funky casuale (le parti bloccate restano)", "Nuevo riff funky aleatorio (las partes fijadas se mantienen)", "Nouveau riff funky aléatoire (les parties bloquées restent)", "Neuer zufälliger Funky-Riff (gesperrte Teile bleiben)", "Novo riff funky aleatório (as partes travadas ficam)" },
        { "Random soulful-house progression", "Progressione soulful house casuale", "Progresión soulful house aleatoria", "Progression soulful house aléatoire", "Zufällige Soulful-House-Akkordfolge", "Progressão soulful house aleatória" },
        { "On: follow the DAW tempo. Off: set your own BPM.", "Acceso: segue il tempo della DAW. Spento: imposti tu i BPM.", "Activado: sigue el tempo de la DAW. Desactivado: tus propios BPM.", "Activé : suit le tempo de la DAW. Désactivé : vos propres BPM.", "An: folgt dem DAW-Tempo. Aus: eigene BPM.", "Ligado: segue o andamento da DAW. Desligado: seus próprios BPM." },
        { "Drag, or double-click to type the BPM", "Trascina o fai doppio clic per scrivere i BPM", "Arrastra o haz doble clic para escribir los BPM", "Glissez ou double-cliquez pour saisir les BPM", "Ziehen oder doppelklicken, um BPM einzugeben", "Arraste ou clique duas vezes para digitar os BPM" },
        { "Play the groove with the guide sound without starting the DAW", "Suona il groove col suono guida senza avviare la DAW", "Toca el groove con el sonido guía sin iniciar la DAW", "Joue le groove avec le son guide sans lancer la DAW", "Spielt den Groove mit dem Guide-Sound ohne DAW-Start", "Toca o groove com o som guia sem iniciar a DAW" },
        { "Shrink the plug-in window while you drag a clip, so you can see your DAW", "Riduce la finestra mentre trascini una clip, così vedi la DAW", "Reduce la ventana al arrastrar un clip para ver tu DAW", "Réduit la fenêtre pendant le glisser pour voir votre DAW", "Verkleinert das Fenster beim Ziehen, damit die DAW sichtbar ist", "Reduz a janela ao arrastar um clipe para ver a DAW" },
        { "Click a step: hit > accent > ghost > rest.  Right-click: more options", "Clic su un passo: colpo > accento > ghost > pausa.  Clic destro: altre opzioni", "Clic en un paso: golpe > acento > fantasma > silencio.  Clic derecho: más opciones", "Clic sur un pas : coup > accent > ghost > silence.  Clic droit : plus d'options", "Klick: Anschlag > Akzent > Ghost > Pause.  Rechtsklick: mehr Optionen", "Clique num passo: toque > acento > ghost > pausa.  Botão direito: mais opções" },
        { "Left-hand bass on / off", "Basso mano sinistra acceso / spento", "Bajo de la mano izquierda on / off", "Basse main gauche on / off", "Bass der linken Hand an / aus", "Baixo da mão esquerda liga / desliga" },
        { "Which part (A-D) plays here: combine parts like A A B C", "Quale parte (A-D) suona qui: combinale, es. A A B C", "Qué parte (A-D) suena aquí: combínalas, p. ej. A A B C", "Quelle partie (A-D) joue ici : combinez, p. ex. A A B C", "Welcher Teil (A-D) hier spielt: kombiniere z. B. A A B C", "Qual parte (A-D) toca aqui: combine, ex. A A B C" },
        { "New random rhythm for this part", "Nuovo ritmo casuale per questa parte", "Nuevo ritmo aleatorio para esta parte", "Nouveau rythme aléatoire pour cette partie", "Neuer Zufallsrhythmus für diesen Teil", "Novo ritmo aleatório para esta parte" },
        { "Keep this part when you press NEW RIFF", "Mantieni questa parte quando premi NUOVO RIFF", "Mantén esta parte al pulsar NUEVO RIFF", "Garde cette partie avec NOUVEAU RIFF", "Diesen Teil bei NEUER RIFF behalten", "Mantém esta parte ao premir NOVO RIFF" },

        // ---- About ----
        { "Version", "Versione", "Versión", "Version", "Version", "Versão" },
        { "funky piano grooves for soulful & deep house", "groove di piano funky per soulful e deep house", "grooves de piano funky para soulful y deep house", "grooves de piano funky pour la soulful et la deep house", "Funky-Piano-Grooves für Soulful & Deep House", "grooves de piano funky para soulful e deep house" },
        { "CREATED BY", "CREATO DA", "CREADO POR", "CRÉÉ PAR", "ERSTELLT VON", "CRIADO POR" },
        { "DJ & Producer", "DJ e Producer", "DJ y productor", "DJ et producteur", "DJ & Produzent", "DJ e produtor" },
        { "CODE", "CODICE", "CÓDIGO", "CODE", "CODE", "CÓDIGO" },
        { "AI coding assistant", "assistente di programmazione AI", "asistente de programación IA", "assistant de programmation IA", "KI-Programmierassistent", "assistente de programação IA" },
        { "Free & open source - GNU AGPL v3", "Gratuito e open source - GNU AGPL v3", "Gratis y de código abierto - GNU AGPL v3", "Gratuit et open source - GNU AGPL v3", "Kostenlos & Open Source - GNU AGPL v3", "Gratuito e open source - GNU AGPL v3" },
        { "Listen to my productions on Spotify", "Ascolta le mie produzioni su Spotify", "Escucha mis producciones en Spotify", "Écoutez mes productions sur Spotify", "Hör meine Produktionen auf Spotify", "Ouça as minhas produções no Spotify" },
        { "Built with JUCE", "Realizzato con JUCE", "Hecho con JUCE", "Réalisé avec JUCE", "Erstellt mit JUCE", "Feito com JUCE" },

        // ---- Menu items (parameter choices) ----
        { "Off", "Spenti", "Desactivado", "Désactivé", "Aus", "Desligado" },
        { "Every 2 bars", "Ogni 2 battute", "Cada 2 compases", "Toutes les 2 mesures", "Alle 2 Takte", "A cada 2 compassos" },
        { "Every 4 bars", "Ogni 4 battute", "Cada 4 compases", "Toutes les 4 mesures", "Alle 4 Takte", "A cada 4 compassos" },
        { "Every 8 bars", "Ogni 8 battute", "Cada 8 compases", "Toutes les 8 mesures", "Alle 8 Takte", "A cada 8 compassos" },
        { "1/2 bar", "1/2 battuta", "1/2 compás", "1/2 mesure", "1/2 Takt", "1/2 compasso" },
        { "1 bar", "1 battuta", "1 compás", "1 mesure", "1 Takt", "1 compasso" },
        { "2 bars", "2 battute", "2 compases", "2 mesures", "2 Takte", "2 compassos" },
        { "4 bars", "4 battute", "4 compases", "4 mesures", "4 Takte", "4 compassos" },
        { "8 bars", "8 battute", "8 compases", "8 mesures", "8 Takte", "8 compassos" },
        { "16 bars", "16 battute", "16 compases", "16 mesures", "16 Takte", "16 compassos" },
        { "32 bars", "32 battute", "32 compases", "32 mesures", "32 Takte", "32 compassos" },
        { "Full loop", "Loop completo", "Loop completo", "Boucle complète", "Ganzer Loop", "Loop completo" },
        { "Song: intro > drop > outro", "Brano: intro > drop > outro", "Tema: intro > drop > outro", "Morceau : intro > drop > outro", "Song: Intro > Drop > Outro", "Música: intro > drop > outro" },
        { "Chords + bass", "Accordi + basso", "Acordes + bajo", "Accords + basse", "Akkorde + Bass", "Acordes + baixo" },
        { "Chords only", "Solo accordi", "Solo acordes", "Accords seuls", "Nur Akkorde", "Só acordes" },
        { "Bass only", "Solo basso", "Solo bajo", "Basse seule", "Nur Bass", "Só baixo" },
        { "Host transport", "Trasporto DAW", "Transporte de la DAW", "Transport de la DAW", "DAW-Transport", "Transporte da DAW" },
        { "Hold a key", "Tieni premuto un tasto", "Mantén una tecla", "Maintenir une touche", "Taste halten", "Segure uma tecla" },
        { "All on channel 1", "Tutto sul canale 1", "Todo en el canal 1", "Tout sur le canal 1", "Alles auf Kanal 1", "Tudo no canal 1" },
        { "Bass on channel 2", "Basso sul canale 2", "Bajo en el canal 2", "Basse sur le canal 2", "Bass auf Kanal 2", "Baixo no canal 2" },
        { "Main groove", "Groove principale", "Groove principal", "Groove principal", "Haupt-Groove", "Groove principal" },
        { "Break", "Break", "Break", "Break", "Break", "Break" },
        { "Build-up", "Build-up", "Subida", "Montée", "Build-up", "Subida" },
        { "Root", "Fondamentale", "Fundamental", "Fondamentale", "Grundton", "Fundamental" },
        { "Octaves", "Ottave", "Octavas", "Octaves", "Oktaven", "Oitavas" },
        { "Root + 5th", "Fondamentale + 5ª", "Fundamental + 5ª", "Fondamentale + quinte", "Grundton + Quinte", "Fundamental + 5ª" },
        { "Pentatonic run up", "Scaletta pentatonica in salita", "Escala pentatónica ascendente", "Gamme pentatonique montante", "Pentatonik-Lauf aufwärts", "Escala pentatônica subindo" },
        { "Pentatonic run down", "Scaletta pentatonica in discesa", "Escala pentatónica descendente", "Gamme pentatonique descendante", "Pentatonik-Lauf abwärts", "Escala pentatônica descendo" },
        { "Chord roll (next chord)", "Arpeggio (accordo successivo)", "Arpegio (siguiente acorde)", "Arpège (accord suivant)", "Arpeggio (nächster Akkord)", "Arpejo (próximo acorde)" },
        { "Triads", "Triadi", "Tríadas", "Triades", "Dreiklänge", "Tríades" },
        { "7ths", "Settime", "Séptimas", "Septièmes", "Septimen", "Sétimas" },
        { "9ths", "None", "Novenas", "Neuvièmes", "Nonen", "Nonas" },
        { "11ths / 13ths", "Undicesime / tredicesime", "Oncenas / trecenas", "Onzièmes / treizièmes", "Undezimen / Tredezimen", "Décimas primeiras / terceiras" },
        { "Wide (2 hands)", "Wide (2 mani)", "Wide (2 manos)", "Wide (2 mains)", "Wide (2 Hände)", "Wide (2 mãos)" },
        { "Quartal (4ths)", "Quartale (quarte)", "Cuartal (cuartas)", "Quartal (quartes)", "Quartal (Quarten)", "Quartal (quartas)" },
        { "Major (Ionian)", "Maggiore (ionica)", "Mayor (jónica)", "Majeur (ionien)", "Dur (ionisch)", "Maior (jônio)" },
        { "Minor (Aeolian)", "Minore (eolia)", "Menor (eólica)", "Mineur (éolien)", "Moll (äolisch)", "Menor (eólio)" },
        { "Harmonic Minor", "Minore armonica", "Menor armónica", "Mineur harmonique", "Harmonisch Moll", "Menor harmônica" },
        { "Melodic Minor", "Minore melodica", "Menor melódica", "Mineur mélodique", "Melodisch Moll", "Menor melódica" },
        { "* Random riff *", "* Riff casuale *", "* Riff aleatorio *", "* Riff aléatoire *", "* Zufallsriff *", "* Riff aleatório *" },
        { "* User pattern *", "* Pattern utente *", "* Patrón de usuario *", "* Motif utilisateur *", "* Eigenes Pattern *", "* Padrão do usuário *" },

        // ---- v1.11: standalone window (JUCE texts) and tips ----
        { "Audio/MIDI Settings...", "Impostazioni audio/MIDI...", "Ajustes de audio/MIDI...", "Réglages audio/MIDI...", "Audio/MIDI-Einstellungen...", "Configurações de áudio/MIDI..." },
        { "Audio/MIDI Settings", "Impostazioni audio/MIDI", "Ajustes de audio/MIDI", "Réglages audio/MIDI", "Audio/MIDI-Einstellungen", "Configurações de áudio/MIDI" },
        { "Save current state...", "Salva lo stato attuale...", "Guardar el estado actual...", "Enregistrer l'état actuel...", "Aktuellen Zustand speichern...", "Salvar o estado atual..." },
        { "Load a saved state...", "Carica uno stato salvato...", "Cargar un estado guardado...", "Charger un état enregistré...", "Gespeicherten Zustand laden...", "Carregar um estado salvo..." },
        { "Reset to default state", "Ripristina lo stato iniziale", "Restablecer el estado inicial", "Rétablir l'état par défaut", "Auf Standard zurücksetzen", "Restaurar o estado padrão" },
        { "Save current state", "Salva lo stato attuale", "Guardar el estado actual", "Enregistrer l'état actuel", "Aktuellen Zustand speichern", "Salvar o estado atual" },
        { "Load a saved state", "Carica uno stato salvato", "Cargar un estado guardado", "Charger un état enregistré", "Gespeicherten Zustand laden", "Carregar um estado salvo" },
        { "Error whilst saving", "Errore durante il salvataggio", "Error al guardar", "Erreur lors de l'enregistrement", "Fehler beim Speichern", "Erro ao salvar" },
        { "Error whilst loading", "Errore durante il caricamento", "Error al cargar", "Erreur lors du chargement", "Fehler beim Laden", "Erro ao carregar" },
        { "Couldn't write to the specified file!", "Impossibile scrivere il file indicato!", "¡No se pudo escribir el archivo!", "Impossible d'écrire le fichier !", "Datei konnte nicht geschrieben werden!", "Não foi possível gravar o arquivo!" },
        { "Couldn't read from the specified file!", "Impossibile leggere il file indicato!", "¡No se pudo leer el archivo!", "Impossible de lire le fichier !", "Datei konnte nicht gelesen werden!", "Não foi possível ler o arquivo!" },
        { "Audio device type:", "Tipo di dispositivo audio:", "Tipo de dispositivo de audio:", "Type de périphérique audio :", "Audiogerät-Typ:", "Tipo de dispositivo de áudio:" },
        { "Device:", "Dispositivo:", "Dispositivo:", "Périphérique :", "Gerät:", "Dispositivo:" },
        { "Output:", "Uscita:", "Salida:", "Sortie :", "Ausgang:", "Saída:" },
        { "Input:", "Ingresso:", "Entrada:", "Entrée :", "Eingang:", "Entrada:" },
        { "Sample rate:", "Frequenza di campionamento:", "Frecuencia de muestreo:", "Fréquence d'échantillonnage :", "Abtastrate:", "Taxa de amostragem:" },
        { "Audio buffer size:", "Dimensione del buffer audio:", "Tamaño del búfer de audio:", "Taille du tampon audio :", "Audio-Puffergröße:", "Tamanho do buffer de áudio:" },
        { "Active MIDI inputs:", "Ingressi MIDI attivi:", "Entradas MIDI activas:", "Entrées MIDI actives :", "Aktive MIDI-Eingänge:", "Entradas MIDI ativas:" },
        { "MIDI Output:", "Uscita MIDI:", "Salida MIDI:", "Sortie MIDI :", "MIDI-Ausgang:", "Saída MIDI:" },
        { "Active output channels:", "Canali di uscita attivi:", "Canales de salida activos:", "Canaux de sortie actifs :", "Aktive Ausgangskanäle:", "Canais de saída ativos:" },
        { "Active input channels:", "Canali di ingresso attivi:", "Canales de entrada activos:", "Canaux d'entrée actifs :", "Aktive Eingangskanäle:", "Canais de entrada ativos:" },
        { "Test", "Prova", "Probar", "Tester", "Test", "Testar" },
        { "Plays a test tone", "Suona un tono di prova", "Reproduce un tono de prueba", "Joue un son de test", "Spielt einen Testton", "Toca um tom de teste" },
        { "Control Panel", "Pannello di controllo", "Panel de control", "Panneau de configuration", "Systemsteuerung", "Painel de controle" },
        { "Reset Device", "Reimposta dispositivo", "Reiniciar dispositivo", "Réinitialiser le périphérique", "Gerät zurücksetzen", "Reiniciar dispositivo" },
        { "Show advanced settings...", "Mostra impostazioni avanzate...", "Mostrar ajustes avanzados...", "Afficher les réglages avancés...", "Erweiterte Einstellungen...", "Mostrar configurações avançadas..." },
        { "No MIDI inputs available", "Nessun ingresso MIDI disponibile", "No hay entradas MIDI", "Aucune entrée MIDI disponible", "Keine MIDI-Eingänge verfügbar", "Nenhuma entrada MIDI disponível" },
        { "none", "nessuno", "ninguno", "aucun", "keiner", "nenhum" },
        { "OK", "OK", "Aceptar", "OK", "OK", "OK" },
        { "Options", "Opzioni", "Opciones", "Options", "Optionen", "Opções" },
        { "Mute audio input", "Silenzia l'ingresso audio", "Silenciar la entrada de audio", "Couper l'entrée audio", "Audioeingang stummschalten", "Silenciar a entrada de áudio" },
        { "Feedback Loop:", "Rischio di feedback:", "Bucle de realimentación:", "Boucle de larsen :", "Rückkopplung:", "Realimentação:" },
        { "Audio input is muted to avoid feedback loop", "L'ingresso audio è silenziato per evitare il feedback", "La entrada de audio está silenciada para evitar realimentación", "L'entrée audio est coupée pour éviter le larsen", "Der Audioeingang ist stummgeschaltet, um Rückkopplung zu vermeiden", "A entrada de áudio está silenciada para evitar realimentação" },
        { "Show tips when the mouse rests on a control", "Mostra i suggerimenti quando il mouse resta fermo su un comando", "Mostrar consejos al dejar el ratón sobre un control", "Afficher des astuces quand la souris reste sur une commande", "Tipps zeigen, wenn die Maus auf einem Element ruht", "Mostrar dicas quando o mouse para sobre um controle" },

        // ---- v1.11: tips ----
        { "How many chords the progression has (1 to 8)", "Quanti accordi ha la progressione (da 1 a 8)", "Cuántos acordes tiene la progresión (de 1 a 8)", "Nombre d'accords de la progression (1 à 8)", "Wie viele Akkorde die Folge hat (1 bis 8)", "Quantos acordes tem a progressão (de 1 a 8)" },
        { "How long each chord lasts, from half a bar to 4 bars", "Quanto dura ogni accordo, da mezza battuta a 4 battute", "Cuánto dura cada acorde, de medio compás a 4 compases", "Durée de chaque accord, d'une demi-mesure à 4 mesures", "Wie lange jeder Akkord dauert, von einem halben bis 4 Takten", "Quanto dura cada acorde, de meio compasso a 4 compassos" },
        { "The key of the song", "La tonalità del brano", "La tonalidad del tema", "La tonalité du morceau", "Die Tonart des Songs", "O tom da música" },
        { "Scale or mode: Dorian is the deep house classic", "Scala o modo: il Dorian è il classico della deep house", "Escala o modo: el dórico es el clásico del deep house", "Gamme ou mode : le dorien est le classique de la deep house", "Skala oder Modus: Dorisch ist der Deep-House-Klassiker", "Escala ou modo: o dórico é o clássico do deep house" },
        { "How rich the chords are: triads, 7ths, 9ths, 11ths/13ths, 6/9, sus...", "Quanto sono ricchi gli accordi: triadi, settime, none, undicesime/tredicesime, 6/9, sus...", "Riqueza de los acordes: tríadas, séptimas, novenas, oncenas/trecenas, 6/9, sus...", "Richesse des accords : triades, septièmes, neuvièmes, onzièmes/treizièmes, 6/9, sus...", "Wie reich die Akkorde sind: Dreiklänge, Septimen, Nonen, Undezimen/Tredezimen, 6/9, sus...", "Riqueza dos acordes: tríades, sétimas, nonas, décimas primeiras/terceiras, 6/9, sus..." },
        { "How the chord notes are spread on the keyboard", "Come sono distribuite le note dell'accordo sulla tastiera", "Cómo se reparten las notas del acorde en el teclado", "Comment les notes de l'accord sont réparties sur le clavier", "Wie die Akkordtöne auf der Tastatur verteilt sind", "Como as notas do acorde se distribuem no teclado" },
        { "What the left hand plays", "Cosa suona la mano sinistra", "Qué toca la mano izquierda", "Ce que joue la main gauche", "Was die linke Hand spielt", "O que a mão esquerda toca" },
        { "Moves the right hand up or down the keyboard", "Sposta la mano destra in alto o in basso sulla tastiera", "Mueve la mano derecha arriba o abajo en el teclado", "Déplace la main droite vers le haut ou le bas du clavier", "Bewegt die rechte Hand auf der Tastatur nach oben oder unten", "Move a mão direita para cima ou para baixo no teclado" },
        { "Rolls the chord notes like a pianist instead of hitting them all together", "Arpeggia le note dell'accordo come un pianista invece di suonarle tutte insieme", "Arpegia las notas del acorde como un pianista en vez de tocarlas a la vez", "Arpège les notes de l'accord comme un pianiste au lieu de les jouer ensemble", "Bricht den Akkord wie ein Pianist, statt alle Töne zugleich anzuschlagen", "Arpeja as notas do acorde como um pianista em vez de tocá-las juntas" },
        { "The rhythm of both hands, grouped by genre", "Il ritmo delle due mani, diviso per genere", "El ritmo de las dos manos, agrupado por género", "Le rythme des deux mains, classé par genre", "Der Rhythmus beider Hände, nach Genre geordnet", "O ritmo das duas mãos, agrupado por gênero" },
        { "Reshapes the groove for intro, main, break, build-up, drop and outro", "Adatta il groove a intro, main, break, build-up, drop e outro", "Adapta el groove a intro, main, break, build-up, drop y outro", "Adapte le groove à l'intro, main, break, montée, drop et outro", "Passt den Groove an Intro, Main, Break, Build-up, Drop und Outro an", "Adapta o groove a intro, main, break, build-up, drop e outro" },
        { "How often a fill leads into the next chord", "Ogni quanto un fill porta all'accordo successivo", "Cada cuánto un relleno lleva al siguiente acorde", "À quelle fréquence un fill mène à l'accord suivant", "Wie oft ein Fill zum nächsten Akkord führt", "Com que frequência uma virada leva ao próximo acorde" },
        { "What the fill plays", "Cosa suona il fill", "Qué toca el relleno", "Ce que joue le fill", "Was das Fill spielt", "O que a virada toca" },
        { "Play with the DAW, or while you hold a key (the key you hold sets the key)", "Suona con la DAW, o mentre tieni premuto un tasto (il tasto sceglie la tonalità)", "Toca con la DAW, o mientras mantienes una tecla (la tecla elige la tonalidad)", "Joue avec la DAW, ou tant que tu tiens une touche (la touche choisit la tonalité)", "Spielt mit der DAW oder solange du eine Taste hältst (die Taste bestimmt die Tonart)", "Toca com a DAW, ou enquanto você segura uma tecla (a tecla escolhe o tom)" },
        { "Delays the off-beat 16ths for a shuffled, human feel", "Ritarda i sedicesimi in levare per un feel swingato e umano", "Retrasa las semicorcheas a contratiempo para un feel con swing", "Retarde les doubles croches à contretemps pour un feel swing", "Verzögert die Off-Beat-16tel für ein geshuffeltes, menschliches Gefühl", "Atrasa as semicolcheias no contratempo para um feel com swing" },
        { "How long the chords ring: from short stabs to long chords", "Quanto suonano gli accordi: da stab corti ad accordi lunghi", "Cuánto suenan los acordes: de stabs cortos a acordes largos", "Durée des accords : de stabs courts à accords longs", "Wie lange die Akkorde klingen: von kurzen Stabs bis zu langen Akkorden", "Quanto os acordes soam: de stabs curtos a acordes longos" },
        { "How hard the notes are played", "Con quanta forza sono suonate le note", "Con qué fuerza se tocan las notas", "Avec quelle force les notes sont jouées", "Wie stark die Töne angeschlagen werden", "Com que força as notas são tocadas" },
        { "Small timing and dynamics variations, like a real player", "Piccole variazioni di tempo e dinamica, come un musicista vero", "Pequeñas variaciones de tiempo y dinámica, como un músico real", "Petites variations de timing et de dynamique, comme un vrai musicien", "Kleine Timing- und Dynamikschwankungen wie bei einem echten Spieler", "Pequenas variações de tempo e dinâmica, como um músico real" },
        { "Random riffs only: from sparse stabs to busy chops", "Solo per i riff casuali: da stab radi a chop fitti", "Solo para riffs aleatorios: de stabs escasos a chops densos", "Riffs aléatoires seulement : de stabs espacés à des chops denses", "Nur für Zufallsriffs: von spärlichen Stabs bis zu dichten Chops", "Só para riffs aleatórios: de stabs esparsos a chops densos" },
        { "The guide sound you hear (the MIDI stays the same)", "Il suono guida che senti (il MIDI resta uguale)", "El sonido guía que oyes (el MIDI no cambia)", "Le son guide que tu entends (le MIDI ne change pas)", "Der Guide-Sound, den du hörst (das MIDI bleibt gleich)", "O som guia que você ouve (o MIDI continua igual)" },
        { "Bass on the same MIDI channel, or on channel 2 to play it with another instrument", "Basso sullo stesso canale MIDI, o sul canale 2 per suonarlo con un altro strumento", "Bajo en el mismo canal MIDI, o en el canal 2 para tocarlo con otro instrumento", "Basse sur le même canal MIDI, ou sur le canal 2 pour la jouer avec un autre instrument", "Bass auf demselben MIDI-Kanal oder auf Kanal 2 für ein anderes Instrument", "Baixo no mesmo canal MIDI, ou no canal 2 para tocá-lo com outro instrumento" },
        { "Brightness of the guide sound", "Brillantezza del suono guida", "Brillo del sonido guía", "Brillance du son guide", "Helligkeit des Guide-Sounds", "Brilho do som guia" },
        { "Reverb on the guide sound", "Riverbero sul suono guida", "Reverb en el sonido guía", "Réverbération sur le son guide", "Hall auf dem Guide-Sound", "Reverb no som guia" },
        { "Volume of the guide sound", "Volume del suono guida", "Volumen del sonido guía", "Volume du son guide", "Lautstärke des Guide-Sounds", "Volume do som guia" },
        { "Widens the guide sound", "Allarga il suono guida", "Ensancha el sonido guía", "Élargit le son guide", "Verbreitert den Guide-Sound", "Alarga o som guia" },
        { "Ducks the guide sound on every beat, like a kick sidechain", "Abbassa il suono guida a ogni battito, come un sidechain sulla cassa", "Baja el sonido guía en cada pulso, como un sidechain con el bombo", "Baisse le son guide à chaque temps, comme un sidechain sur le kick", "Duckt den Guide-Sound auf jedem Schlag wie ein Kick-Sidechain", "Abaixa o som guia a cada batida, como um sidechain no bumbo" },
        { "Turns the built-in guide sound on or off (the MIDI keeps playing)", "Accende o spegne il suono guida interno (il MIDI continua a suonare)", "Activa o desactiva el sonido guía (el MIDI sigue sonando)", "Active ou coupe le son guide intégré (le MIDI continue)", "Schaltet den eingebauten Guide-Sound ein oder aus (das MIDI läuft weiter)", "Liga ou desliga o som guia interno (o MIDI continua tocando)" },
        { "Ready-made progressions, grouped by style (they also set the scale)", "Progressioni pronte, divise per stile (impostano anche la scala)", "Progresiones listas, agrupadas por estilo (también ajustan la escala)", "Progressions prêtes, classées par style (elles règlent aussi la gamme)", "Fertige Akkordfolgen nach Stil (sie setzen auch die Skala)", "Progressões prontas, agrupadas por estilo (também definem a escala)" },
        { "How long the exported MIDI clip is", "Quanto è lunga la clip MIDI esportata", "Duración del clip MIDI exportado", "Durée du clip MIDI exporté", "Wie lang der exportierte MIDI-Clip ist", "Duração do clipe MIDI exportado" },
        { "Which notes go into the clip: chords and bass, chords only or bass only", "Quali note vanno nella clip: accordi e basso, solo accordi o solo basso", "Qué notas van en el clip: acordes y bajo, solo acordes o solo bajo", "Quelles notes vont dans le clip : accords et basse, accords seuls ou basse seule", "Welche Töne in den Clip kommen: Akkorde und Bass, nur Akkorde oder nur Bass", "Quais notas vão no clipe: acordes e baixo, só acordes ou só baixo" },
        { "Drag this onto a track in your DAW to drop the groove as a MIDI clip", "Trascinalo su una traccia della DAW per inserire il groove come clip MIDI", "Arrástralo a una pista de tu DAW para soltar el groove como clip MIDI", "Fais-le glisser sur une piste de ta DAW pour déposer le groove en clip MIDI", "Auf eine Spur in deiner DAW ziehen, um den Groove als MIDI-Clip abzulegen", "Arraste para uma faixa da DAW para soltar o groove como clipe MIDI" },
        { "Save the clip, or every part at once, as .mid files", "Salva la clip, o tutte le parti insieme, come file .mid", "Guarda el clip, o todas las partes a la vez, como archivos .mid", "Enregistre le clip, ou toutes les parties d'un coup, en fichiers .mid", "Speichert den Clip oder alle Teile auf einmal als .mid-Dateien", "Salva o clipe, ou todas as partes de uma vez, como arquivos .mid" },
        { "Language, interface size, tips and credits", "Lingua, dimensione dell'interfaccia, suggerimenti e crediti", "Idioma, tamaño de la interfaz, consejos y créditos", "Langue, taille de l'interface, astuces et crédits", "Sprache, Fenstergröße, Tipps und Credits", "Idioma, tamanho da interface, dicas e créditos" },
        { "Scale degree of this chord; the small arrows change its inversion", "Grado della scala di questo accordo; le frecce piccole ne cambiano il rivolto", "Grado de la escala de este acorde; las flechas cambian su inversión", "Degré de la gamme de cet accord ; les petites flèches changent son renversement", "Stufe dieses Akkords; die kleinen Pfeile ändern seine Umkehrung", "Grau da escala deste acorde; as setinhas mudam sua inversão" },

        // ---- v1.12: strum ----
        { "Up", "Su", "Arriba", "Haut", "Hoch", "Cima" },
        { "Down", "Giù", "Abajo", "Bas", "Runter", "Baixo" },
        { "Alternate", "Alternato", "Alterno", "Alterné", "Abwechselnd", "Alternado" },
        { "Direction of the roll: up, down, or alternating (up on the beat, down off it). Click to change", "Direzione dell'arpeggiato: su, giù o alternato (su sui tempi, giù sui controtempi). Clicca per cambiare", "Dirección del arpegiado: arriba, abajo o alterno (arriba en el tiempo, abajo a contratiempo). Haz clic para cambiar", "Sens de l'arpège : haut, bas ou alterné (haut sur le temps, bas à contretemps). Clique pour changer", "Richtung des Anschlags: hoch, runter oder abwechselnd (hoch auf dem Schlag, runter dazwischen). Klicken zum Wechseln", "Direção do dedilhado: cima, baixo ou alternado (cima no tempo, baixo no contratempo). Clique para mudar" },

        { "Piano House", "Piano house", "Piano house", "Piano house", "Piano House", "Piano house" },
        { "House Piano Loop", "Loop di piano house", "Loop de piano house", "Boucle de piano house", "House-Piano-Loop", "Loop de piano house" },

        // ---- v1.10 ----
        { "Jackin' Piano", "Piano jackin'", "Piano jackin'", "Piano jackin'", "Jackin' Piano", "Piano jackin'" },
        { "Hit, voicing up", "Colpo, rivolto su", "Golpe, inversión arriba", "Coup, renversement vers le haut", "Anschlag, Umkehrung hoch", "Toque, inversão acima" },
        { "Hit, voicing down", "Colpo, rivolto giù", "Golpe, inversión abajo", "Coup, renversement vers le bas", "Anschlag, Umkehrung runter", "Toque, inversão abaixo" },

        { "Movement", "Movimento", "Movimiento", "Mouvement", "Bewegung", "Movimento" },
        { "Subtle", "Leggero", "Suave", "Léger", "Dezent", "Leve" },
        { "Lively", "Vivace", "Vivo", "Vif", "Lebhaft", "Vivo" },
        { "Voicing movement: some hits play the chord an inversion up or down so the riff moves instead of repeating the same chord",
          "Movimento: alcuni colpi suonano l'accordo rivoltato su o giù, così il riff si muove invece di ripetere lo stesso accordo",
          "Movimiento: algunos golpes tocan el acorde invertido arriba o abajo, así el riff se mueve en vez de repetir el mismo acorde",
          "Mouvement : certains coups jouent l'accord renversé vers le haut ou le bas, le riff bouge au lieu de répéter le même accord",
          "Bewegung: manche Anschläge spielen den Akkord eine Umkehrung höher oder tiefer, so bewegt sich das Riff",
          "Movimento: alguns toques tocam o acorde invertido acima ou abaixo, assim o riff se move em vez de repetir o mesmo acorde" },

        // ---- v1.9 ----
        { "Top line", "Linea in alto", "Línea superior", "Ligne du haut", "Oberstimme", "Linha superior" },
        { "Smooth (auto)", "Morbida (auto)", "Suave (auto)", "Douce (auto)", "Weich (auto)", "Suave (auto)" },
        { "Steady", "Ferma", "Fija", "Stable", "Gleichbleibend", "Fixa" },
        { "Rising", "In salita", "Ascendente", "Montante", "Steigend", "Subindo" },
        { "Falling", "In discesa", "Descendente", "Descendante", "Fallend", "Descendo" },
        { "Arch", "Ad arco", "En arco", "En arche", "Bogen", "Em arco" },
        { "Variation", "Variazione", "Variación", "Variation", "Variation", "Variação" },
        { "Groove length", "Lunghezza groove", "Duración del groove", "Longueur du groove", "Groove-Länge", "Duração do groove" },
        { "Chorus", "Chorus", "Chorus", "Chorus", "Chorus", "Chorus" },
        { "Pump", "Pompa", "Bombeo", "Pompage", "Pumpen", "Pump" },
        { "Walking", "Walking", "Walking", "Walking", "Walking", "Walking" },
        { "Syncopated octaves", "Ottave sincopate", "Octavas sincopadas", "Octaves syncopées", "Synkopierte Oktaven", "Oitavas sincopadas" },
        { "Wurlitzer", "Wurlitzer", "Wurlitzer", "Wurlitzer", "Wurlitzer", "Wurlitzer" },
        { "Gospel Organ (Leslie)", "Organo gospel (Leslie)", "Órgano gospel (Leslie)", "Orgue gospel (Leslie)", "Gospel-Orgel (Leslie)", "Órgão gospel (Leslie)" },
        { "Warm Pad", "Pad caldo", "Pad cálido", "Nappe chaude", "Warmes Pad", "Pad quente" },
        { "LEARN", "IMPARA", "APRENDER", "APPRENDRE", "LERNEN", "APRENDER" },
        { "PRESETS", "PRESET", "PRESETS", "PRESETS", "PRESETS", "PRESETS" },
        { "Save your own setups and load them in any project", "Salva le tue impostazioni e caricale in qualsiasi progetto", "Guarda tus ajustes y cárgalos en cualquier proyecto", "Enregistre tes réglages et charge-les dans n'importe quel projet", "Eigene Einstellungen speichern und in jedem Projekt laden", "Salve seus ajustes e carregue-os em qualquer projeto" },
        { "Learn: play chords on your MIDI keyboard, they fill the progression slot by slot (press again to stop)", "Impara: suona gli accordi sulla tastiera MIDI, riempiono la progressione slot dopo slot (premi di nuovo per fermare)", "Aprender: toca acordes en tu teclado MIDI, llenan la progresión uno a uno (pulsa otra vez para parar)", "Apprendre : joue des accords sur ton clavier MIDI, ils remplissent la progression case par case (appuie à nouveau pour arrêter)", "Lernen: Akkorde auf dem MIDI-Keyboard spielen, sie füllen die Akkordfolge Feld für Feld (nochmals drücken zum Stoppen)", "Aprender: toque acordes no teclado MIDI, eles preenchem a progressão um a um (pressione de novo para parar)" },
        { "Undo (Ctrl+Z)", "Annulla (Ctrl+Z)", "Deshacer (Ctrl+Z)", "Annuler (Ctrl+Z)", "Rückgängig (Strg+Z)", "Desfazer (Ctrl+Z)" },
        { "Redo (Ctrl+Shift+Z)", "Ripeti (Ctrl+Maiusc+Z)", "Rehacer (Ctrl+Mayús+Z)", "Rétablir (Ctrl+Maj+Z)", "Wiederholen (Strg+Umschalt+Z)", "Refazer (Ctrl+Shift+Z)" },
        { "The melody the top notes of the chords draw across the progression", "La melodia che disegnano le note alte degli accordi lungo la progressione", "La melodía que dibujan las notas altas de los acordes en la progresión", "La mélodie que dessinent les notes aiguës des accords dans la progression", "Die Melodie der obersten Akkordtöne über die Akkordfolge", "A melodia que as notas agudas dos acordes desenham na progressão" },
        { "Every 4 or 8 bars the last bar plays a variation of the groove, so the loop never feels static. Evolve: over 8 or 16 bars the same groove grows - as written, then ghost notes, then a variation, then a fill into the next round", "Ogni 4 o 8 battute l'ultima battuta suona una variazione del groove, così il loop non è mai statico. Evoluzione: in 8 o 16 battute lo stesso groove cresce - com'è scritto, poi ghost note, poi una variazione, poi un fill verso il giro successivo", "Cada 4 u 8 compases el último compás toca una variación del groove, así el loop nunca es estático. Evolución: en 8 o 16 compases el mismo groove crece - tal cual, luego ghost notes, luego una variación, luego un fill hacia la vuelta siguiente", "Toutes les 4 ou 8 mesures la dernière joue une variation du groove, la boucle n'est jamais statique. Évolution : sur 8 ou 16 mesures le même groove grandit - tel quel, puis des ghost notes, puis une variation, puis un fill vers le tour suivant", "Alle 4 oder 8 Takte spielt der letzte Takt eine Variation, so wirkt der Loop nie statisch. Entwicklung: über 8 oder 16 Takte wächst derselbe Groove - wie geschrieben, dann Ghost Notes, dann eine Variation, dann ein Fill in die nächste Runde", "A cada 4 ou 8 compassos o último toca uma variação do groove, o loop nunca fica estático. Evolução: em 8 ou 16 compassos o mesmo groove cresce - como escrito, depois ghost notes, depois uma variação, depois um fill para a próxima volta" },
        { "1 bar, 2 bars, or 4 bars (the groove followed by a variation of it)", "1 battuta, 2 battute o 4 battute (il groove seguito da una sua variazione)", "1 compás, 2 compases o 4 compases (el groove seguido de una variación)", "1 mesure, 2 mesures ou 4 mesures (le groove suivi d'une variation)", "1 Takt, 2 Takte oder 4 Takte (der Groove und dann eine Variation)", "1 compasso, 2 compassos ou 4 compassos (o groove seguido de uma variação)" },
        { "Save current settings as...", "Salva le impostazioni attuali come...", "Guardar los ajustes actuales como...", "Enregistrer les réglages actuels sous...", "Aktuelle Einstellungen speichern als...", "Salvar os ajustes atuais como..." },
        { "No presets yet", "Ancora nessun preset", "Aún no hay presets", "Aucun preset pour l'instant", "Noch keine Presets", "Ainda sem presets" },
        { "Delete", "Elimina", "Eliminar", "Supprimer", "Löschen", "Excluir" },
        { "Open the presets folder", "Apri la cartella dei preset", "Abrir la carpeta de presets", "Ouvrir le dossier des presets", "Preset-Ordner öffnen", "Abrir a pasta de presets" },
        { "Save preset", "Salva preset", "Guardar preset", "Enregistrer le preset", "Preset speichern", "Salvar preset" },
        { "Name:", "Nome:", "Nombre:", "Nom :", "Name:", "Nome:" },
        { "My groove", "Il mio groove", "Mi groove", "Mon groove", "Mein Groove", "Meu groove" },
        { "Save", "Salva", "Guardar", "Enregistrer", "Speichern", "Salvar" },
        { "Save this clip as .mid...", "Salva questa clip come .mid...", "Guardar este clip como .mid...", "Enregistrer ce clip en .mid...", "Diesen Clip als .mid speichern...", "Salvar este clipe como .mid..." },
        { "Save all parts to a folder...", "Salva tutte le parti in una cartella...", "Guardar todas las partes en una carpeta...", "Enregistrer toutes les parties dans un dossier...", "Alle Teile in einen Ordner speichern...", "Salvar todas as partes numa pasta..." },
        { "Choose a folder", "Scegli una cartella", "Elige una carpeta", "Choisis un dossier", "Ordner wählen", "Escolha uma pasta" },

        // ---- v1.8 fills ----
        { "Every bar", "Ogni battuta", "Cada compás", "Chaque mesure", "Jeden Takt", "Todo compasso" },
        { "Gospel run (2 beats)", "Scala gospel (2 tempi)", "Escala gospel (2 tiempos)", "Gamme gospel (2 temps)", "Gospel-Lauf (2 Schläge)", "Escala gospel (2 tempos)" },
        { "Chromatic walk-up", "Salita cromatica", "Subida cromática", "Montée chromatique", "Chromatischer Aufgang", "Subida cromática" },
        { "Push stabs", "Stab di anticipo", "Stabs de anticipo", "Stabs d'anticipation", "Push-Stabs", "Stabs de antecipação" },
        { "16th chord chops", "Accordi a sedicesimi", "Acordes en semicorcheas", "Accords en doubles croches", "Akkord-Chops in 16teln", "Acordes em semicolcheias" },
        { "Gospel thirds", "Terze gospel", "Terceras gospel", "Tierces gospel", "Gospel-Terzen", "Terças gospel" },
        { "Piano glissando", "Glissato di piano", "Glissando de piano", "Glissando de piano", "Klavier-Glissando", "Glissando de piano" },
        { "Blues lick", "Fraseggio blues", "Frase blues", "Plan blues", "Blues-Lick", "Frase blues" },
        { "Mix (changes every fill)", "Misto (cambia a ogni fill)", "Mezcla (cambia en cada relleno)", "Mélange (change à chaque fill)", "Mix (wechselt bei jedem Fill)", "Mistura (muda a cada virada)" },

        // ---- v1.7 styles, reset, genres ----
        { "STYLE", "STILE", "ESTILO", "STYLE", "STIL", "ESTILO" },
        { "RESET", "RESET", "REINICIAR", "RÉINIT.", "RESET", "REINICIAR" },
        { "Cancel", "Annulla", "Cancelar", "Annuler", "Abbrechen", "Cancelar" },
        { "Ready-made styles (the key stays)", "Stili pronti (la tonalità resta)", "Estilos listos (la tonalidad se mantiene)", "Styles prêts (la tonalité reste)", "Fertige Stile (die Tonart bleibt)", "Estilos prontos (o tom fica)" },
        { "Ready-made setups by genre: progression, voicing, groove, feel and sound in one click (the key stays)",
          "Impostazioni pronte per genere: progressione, voicing, groove, feel e suono in un clic (la tonalità resta)",
          "Ajustes listos por género: progresión, voicing, groove, feel y sonido en un clic (la tonalidad se mantiene)",
          "Réglages prêts par genre : progression, voicing, groove, feel et son en un clic (la tonalité reste)",
          "Fertige Einstellungen pro Genre: Akkordfolge, Voicing, Groove, Feel und Sound mit einem Klick (die Tonart bleibt)",
          "Ajustes prontos por gênero: progressão, voicing, groove, feel e som num clique (o tom fica)" },
        { "Put every setting back to its default (the language stays)",
          "Riporta tutte le impostazioni ai valori iniziali (la lingua resta)",
          "Devuelve todos los ajustes a sus valores iniciales (el idioma se mantiene)",
          "Remet tous les réglages par défaut (la langue reste)",
          "Setzt alle Einstellungen zurück (die Sprache bleibt)",
          "Volta todos os ajustes ao padrão (o idioma fica)" },
        { "Put every setting of the plug-in back to its default? The language stays as it is.",
          "Riportare tutte le impostazioni del plugin ai valori iniziali? La lingua resta com'è.",
          "¿Devolver todos los ajustes del plugin a sus valores iniciales? El idioma se mantiene.",
          "Remettre tous les réglages du plug-in par défaut ? La langue reste la même.",
          "Alle Einstellungen des Plug-ins zurücksetzen? Die Sprache bleibt.",
          "Voltar todos os ajustes do plug-in ao padrão? O idioma fica como está." },
        { "Soulful", "Soulful", "Soulful", "Soulful", "Soulful", "Soulful" },
        { "Deep", "Deep", "Deep", "Deep", "Deep", "Deep" },
        { "Gospel", "Gospel", "Gospel", "Gospel", "Gospel", "Gospel" },
        { "Jackin'", "Jackin'", "Jackin'", "Jackin'", "Jackin'", "Jackin'" },
        { "Garage & UK", "Garage e UK", "Garage y UK", "Garage et UK", "Garage & UK", "Garage e UK" },
        { "Afro & Latin", "Afro e latin", "Afro y latino", "Afro et latin", "Afro & Latin", "Afro e latino" },
        { "Funky & Disco", "Funky e disco", "Funky y disco", "Funky et disco", "Funky & Disco", "Funky e disco" },
        { "'90s Piano House", "Piano house anni '90", "Piano house de los 90", "Piano house années 90", "90er Piano House", "Piano house anos 90" },
        { "Neo Soul & Broken", "Neo soul e broken", "Neo soul y broken", "Neo soul et broken", "Neo Soul & Broken", "Neo soul e broken" },
        { "Chill & Pads", "Chill e pad", "Chill y pads", "Chill et nappes", "Chill & Pads", "Chill e pads" },
        { "Generate", "Genera", "Generar", "Générer", "Erzeugen", "Gerar" },

        // ---- v1.6 harmony colours & circle ----
        { "Chord", "Accordo", "Acorde", "Accord", "Akkord", "Acorde" },
        { "drag it onto another chord", "trascinalo su un altro accordo", "arrástralo a otro acorde", "fais-le glisser sur un autre accord", "auf einen anderen Akkord ziehen", "arraste-o para outro acorde" },
        { "Outer ring: major chords. Inner ring: minor chords. Bright slices are the chords of the scale.",
          "Anello esterno: accordi maggiori. Anello interno: accordi minori. Gli spicchi chiari sono gli accordi della scala.",
          "Anillo exterior: acordes mayores. Anillo interior: acordes menores. Las porciones claras son los acordes de la escala.",
          "Anneau extérieur : accords majeurs. Anneau intérieur : accords mineurs. Les parts claires sont les accords de la gamme.",
          "Außenring: Dur-Akkorde. Innenring: Moll-Akkorde. Helle Felder sind die Akkorde der Skala.",
          "Anel externo: acordes maiores. Anel interno: acordes menores. As fatias claras são os acordes da escala." },
        { "in the scale", "nella scala", "en la escala", "dans la gamme", "in der Skala", "na escala" },
        { "outside the scale", "fuori scala", "fuera de la escala", "hors de la gamme", "außerhalb der Skala", "fora da escala" },
        { "Click: change key", "Clic: cambia tonalità", "Clic: cambia la tonalidad", "Clic : change de tonalité", "Klick: Tonart wechseln", "Clique: muda o tom" },
        { "sounds great", "suona benissimo", "suena genial", "sonne très bien", "klingt super", "soa muito bem" },
        { "clashes", "stride", "choca", "dissonant", "reibt sich", "destoa" },
        { "Sounds great with the chords next to it", "Suona benissimo con gli accordi vicini", "Suena genial con los acordes vecinos", "Sonne très bien avec les accords voisins", "Klingt super mit den Nachbarakkorden", "Soa muito bem com os acordes vizinhos" },
        { "Sounds good with the chords next to it", "Suona bene con gli accordi vicini", "Suena bien con los acordes vecinos", "Sonne bien avec les accords voisins", "Klingt gut mit den Nachbarakkorden", "Soa bem com os acordes vizinhos" },
        { "Bold: it clashes with the chords next to it", "Azzardato: stride con gli accordi vicini", "Atrevido: choca con los acordes vecinos", "Osé : dissonant avec les accords voisins", "Gewagt: reibt sich mit den Nachbarakkorden", "Ousado: destoa dos acordes vizinhos" },

        // ---- v1.5 harmony pack ----
        { "Soulful & Gospel", "Soulful e gospel", "Soulful y gospel", "Soulful et gospel", "Soulful & Gospel", "Soulful e gospel" },
        { "Circle of 5ths", "Circolo delle quinte", "Círculo de quintas", "Cycle des quintes", "Quintenzirkel", "Círculo das quintas" },
        { "Deep house", "Deep house", "Deep house", "Deep house", "Deep House", "Deep house" },
        { "Modal & exotic", "Modali ed esotiche", "Modales y exóticas", "Modales et exotiques", "Modal & exotisch", "Modais e exóticas" },
        { "Jackin' & funky", "Jackin' e funky", "Jackin' y funky", "Jackin' et funky", "Jackin' & Funky", "Jackin' e funky" },
        // ---- v1.16 living grooves ----
        { "Piano stabs (bass in every chord)", "Stab di piano (basso in ogni accordo)", "Stabs de piano (bajo en cada acorde)",
          "Stabs de piano (basse dans chaque accord)", "Piano-Stabs (Bass in jedem Akkord)", "Stabs de piano (baixo em cada acorde)" },
        { "RND PIANO", "RND PIANO", "RND PIANO", "RND PIANO", "RND PIANO", "RND PIANO" },
        { "RND BASS", "RND BASSO", "RND BAJO", "RND BASSE", "RND BASS", "RND BAIXO" },
        { "New random rhythm for the piano (chords) only: the bass stays",
          "Nuovo ritmo casuale solo per il piano (accordi): il basso resta com'è",
          "Nuevo ritmo aleatorio solo para el piano (acordes): el bajo se queda",
          "Nouveau rythme aléatoire pour le piano (accords) seulement : la basse reste",
          "Neuer Zufallsrhythmus nur für das Piano (Akkorde): der Bass bleibt",
          "Novo ritmo aleatório só para o piano (acordes): o baixo fica" },
        { "New random rhythm for the bass only: the piano stays",
          "Nuovo ritmo casuale solo per il basso: il piano resta com'è",
          "Nuevo ritmo aleatorio solo para el bajo: el piano se queda",
          "Nouveau rythme aléatoire pour la basse seulement : le piano reste",
          "Neuer Zufallsrhythmus nur für den Bass: das Piano bleibt",
          "Novo ritmo aleatório só para o baixo: o piano fica" },
        // ---- v1.17 my grooves ----
        { "MY GROOVES", "I MIEI GROOVE", "MIS GROOVES", "MES GROOVES", "MEINE GROOVES", "MEUS GROOVES" },
        { "Save the rhythm grid as your own groove, or load one you saved (Documents/Fase Groove Piano/Grooves)",
          "Salva la griglia ritmica come groove tuo, o carica uno che hai salvato (Documenti/Fase Groove Piano/Grooves)",
          "Guarda la cuadrícula rítmica como tu propio groove, o carga uno guardado (Documentos/Fase Groove Piano/Grooves)",
          "Enregistrez la grille rythmique comme votre groove, ou chargez-en un (Documents/Fase Groove Piano/Grooves)",
          "Speichere das Rhythmusraster als eigenen Groove oder lade einen gespeicherten (Dokumente/Fase Groove Piano/Grooves)",
          "Salve a grade rítmica como seu groove, ou carregue um salvo (Documentos/Fase Groove Piano/Grooves)" },
        { "Save this groove as...", "Salva questo groove come...", "Guardar este groove como...", "Enregistrer ce groove sous...",
          "Diesen Groove speichern als...", "Salvar este groove como..." },
        { "No saved grooves yet", "Nessun groove salvato", "Aún no hay grooves guardados", "Aucun groove enregistré",
          "Noch keine gespeicherten Grooves", "Nenhum groove salvo ainda" },
        { "Open the Fase Groove Piano folder", "Apri la cartella Fase Groove Piano", "Abrir la carpeta Fase Groove Piano",
          "Ouvrir le dossier Fase Groove Piano", "Ordner Fase Groove Piano öffnen", "Abrir a pasta Fase Groove Piano" },
        { "Save groove", "Salva groove", "Guardar groove", "Enregistrer le groove", "Groove speichern", "Salvar groove" },
        // ---- v1.18 solo ----
        { "SOLO PIANO", "SOLO PIANO", "SOLO PIANO", "SOLO PIANO", "SOLO PIANO", "SÓ PIANO" },
        { "SOLO BASS", "SOLO BASSO", "SOLO BAJO", "SOLO BASSE", "SOLO BASS", "SÓ BAIXO" },
        { "Piano only: the bass is switched off (click again to hear both)",
          "Solo piano: il basso è spento (clicca di nuovo per sentire entrambi)",
          "Solo piano: el bajo se apaga (pulsa otra vez para oír ambos)",
          "Piano seul : la basse est coupée (cliquez à nouveau pour entendre les deux)",
          "Nur Piano: der Bass ist aus (erneut klicken, um beides zu hören)",
          "Só piano: o baixo é desligado (clique de novo para ouvir os dois)" },
        { "Bass only: the chords are silent while you play (exported clips keep everything)",
          "Solo basso: gli accordi tacciono mentre suoni (le clip esportate contengono tutto)",
          "Solo bajo: los acordes callan mientras tocas (los clips exportados lo tienen todo)",
          "Basse seule : les accords se taisent pendant la lecture (les clips exportés gardent tout)",
          "Nur Bass: die Akkorde schweigen beim Spielen (exportierte Clips enthalten alles)",
          "Só baixo: os acordes ficam em silêncio enquanto toca (os clipes exportados têm tudo)" },
        // ---- v1.18 groove browser ----
        { "Search a groove...", "Cerca un groove...", "Buscar un groove...", "Chercher un groove...", "Groove suchen...", "Procurar um groove..." },
        { "Saved groove", "Groove salvato", "Groove guardado", "Groove enregistré", "Gespeicherter Groove", "Groove salvo" },
        { "NEW RIFF makes a new one", "NUOVO RIFF ne crea uno nuovo", "NUEVO RIFF crea otro", "NOUVEAU RIFF en crée un autre", "NEUER RIFF erzeugt einen neuen", "NOVO RIFF cria outro" },
        { "Your edited grid", "La tua griglia modificata", "Tu cuadrícula editada", "Votre grille modifiée", "Dein bearbeitetes Raster", "Sua grade editada" },
        { "Click a groove to hear it, double-click or Enter to close. The star keeps it in your favourites.",
          "Clicca un groove per sentirlo, doppio clic o Invio per chiudere. La stella lo mette tra i preferiti.",
          "Haz clic en un groove para oírlo, doble clic o Intro para cerrar. La estrella lo guarda en favoritos.",
          "Cliquez sur un groove pour l'entendre, double-clic ou Entrée pour fermer. L'étoile le garde en favori.",
          "Klicke auf einen Groove, um ihn zu hören; Doppelklick oder Enter schließt. Der Stern macht ihn zum Favoriten.",
          "Clique num groove para ouvi-lo, duplo clique ou Enter para fechar. A estrela guarda-o nos favoritos." },
        { "Previous groove", "Groove precedente", "Groove anterior", "Groove précédent", "Vorheriger Groove", "Groove anterior" },
        { "Next groove", "Groove successivo", "Groove siguiente", "Groove suivant", "Nächster Groove", "Próximo groove" },
        // ---- v1.18 browsers ----
        { "Search a progression...", "Cerca una progressione...", "Buscar una progresión...", "Chercher une progression...", "Progression suchen...", "Procurar uma progressão..." },
        { "Click a groove to hear it, double-click or Enter to close",
          "Clicca un groove per sentirlo, doppio clic o Invio per chiudere",
          "Haz clic en un groove para oírlo, doble clic o Intro para cerrar",
          "Cliquez sur un groove pour l'entendre, double-clic ou Entrée pour fermer",
          "Klicke auf einen Groove, um ihn zu hören; Doppelklick oder Enter schließt",
          "Clique num groove para ouvi-lo, duplo clique ou Enter para fechar" },
        { "Click a progression to hear it, double-click or Enter to close. The star keeps it in your favourites.",
          "Clicca una progressione per sentirla, doppio clic o Invio per chiudere. La stella la mette tra i preferiti.",
          "Haz clic en una progresión para oírla, doble clic o Intro para cerrar. La estrella la guarda en favoritos.",
          "Cliquez sur une progression pour l'entendre, double-clic ou Entrée pour fermer. L'étoile la garde en favori.",
          "Klicke auf eine Progression, um sie zu hören; Doppelklick oder Enter schließt. Der Stern macht sie zum Favoriten.",
          "Clique numa progressão para ouvi-la, duplo clique ou Enter para fechar. A estrela guarda-a nos favoritos." },
        { "Progressions", "Progressioni", "Progresiones", "Progressions", "Progressionen", "Progressões" },
        // ---- v1.18 smart update ----
        { "INSTALL NOW updates it with the setup, without the wizard (same folders and parts). Close your DAW if it asks.",
          "INSTALLA ORA aggiorna con il setup, senza procedura guidata (stesse cartelle e componenti). Chiudi la DAW se te lo chiede.",
          "INSTALAR AHORA actualiza con el instalador, sin asistente (mismas carpetas y partes). Cierra tu DAW si lo pide.",
          "INSTALLER met à jour avec l'installateur, sans assistant (mêmes dossiers et composants). Fermez votre DAW s'il le demande.",
          "JETZT INSTALLIEREN aktualisiert mit dem Setup, ohne Assistent (gleiche Ordner und Teile). Schließe die DAW, wenn es fragt.",
          "INSTALAR AGORA atualiza com o instalador, sem assistente (mesmas pastas e partes). Feche sua DAW se ele pedir." },
        { "INSTALL NOW replaces the files of this copy where they are. Then restart your DAW.",
          "INSTALLA ORA sostituisce i file di questa copia dove si trovano. Poi riavvia la DAW.",
          "INSTALAR AHORA reemplaza los archivos de esta copia donde están. Luego reinicia tu DAW.",
          "INSTALLER remplace les fichiers de cette copie là où ils sont. Puis redémarrez votre DAW.",
          "JETZT INSTALLIEREN ersetzt die Dateien dieser Kopie an Ort und Stelle. Dann die DAW neu starten.",
          "INSTALAR AGORA substitui os arquivos desta cópia onde estão. Depois reinicie sua DAW." },
        { "Updated to version", "Aggiornato alla versione", "Actualizado a la versión", "Mis à jour vers la version", "Aktualisiert auf Version", "Atualizado para a versão" },
        { "Restart your DAW (or the app) to use the new version.", "Riavvia la DAW (o l'app) per usare la nuova versione.",
          "Reinicia tu DAW (o la app) para usar la nueva versión.", "Redémarrez votre DAW (ou l'app) pour utiliser la nouvelle version.",
          "Starte die DAW (oder die App) neu, um die neue Version zu nutzen.", "Reinicie sua DAW (ou o app) para usar a nova versão." },
        { "The files could not be replaced automatically: the download is shown, copy it as the first time.",
          "Non è stato possibile sostituire i file in automatico: ti mostro il download, copialo come la prima volta.",
          "No se pudieron reemplazar los archivos automáticamente: se muestra la descarga, cópiala como la primera vez.",
          "Les fichiers n'ont pas pu être remplacés automatiquement : le téléchargement est affiché, copiez-le comme la première fois.",
          "Die Dateien konnten nicht automatisch ersetzt werden: der Download wird angezeigt, kopiere ihn wie beim ersten Mal.",
          "Não foi possível substituir os arquivos automaticamente: o download é mostrado, copie-o como da primeira vez." },
        // ---- v1.19 favourites and MIDI analyzer ----
        { "Favourites", "Preferiti", "Favoritos", "Favoris", "Favoriten", "Favoritos" },
        { "Recent", "Recenti", "Recientes", "Récents", "Zuletzt", "Recentes" },
        { "Import a MIDI file...", "Importa un file MIDI...", "Importar un archivo MIDI...", "Importer un fichier MIDI...", "MIDI-Datei importieren...", "Importar um arquivo MIDI..." },
        { "No chords found in this MIDI file.", "Nessun accordo trovato in questo file MIDI.", "No se encontraron acordes en este archivo MIDI.",
          "Aucun accord trouvé dans ce fichier MIDI.", "Keine Akkorde in dieser MIDI-Datei gefunden.", "Nenhum acorde encontrado neste arquivo MIDI." },
        { "Notes outside the scale", "Note fuori scala", "Notas fuera de la escala", "Notes hors gamme", "Töne außerhalb der Skala", "Notas fora da escala" },
        { "This is an estimate: inversions and passing notes can mislead it. Fix any chord on the circle, or undo with the arrow.",
          "È una stima: rivolti e note di passaggio possono ingannarla. Correggi gli accordi sul circolo, o annulla con la freccia.",
          "Es una estimación: inversiones y notas de paso pueden confundirla. Corrige los acordes en el círculo o deshaz con la flecha.",
          "C'est une estimation : renversements et notes de passage peuvent la tromper. Corrigez les accords sur le cercle ou annulez avec la flèche.",
          "Das ist eine Schätzung: Umkehrungen und Durchgangstöne können täuschen. Korrigiere Akkorde am Kreis oder mache es mit dem Pfeil rückgängig.",
          "É uma estimativa: inversões e notas de passagem podem enganá-la. Corrija os acordes no círculo ou desfaça com a seta." },
        // ---- v1.20 bass lines ----
        { "Follow the groove", "Seguono il groove", "Siguen el groove", "Suivent le groove", "Folgen dem Groove", "Seguem o groove" },
        { "Bass lines: Disco", "Linee di basso: Disco", "Líneas de bajo: Disco", "Lignes de basse : Disco", "Basslinien: Disco", "Linhas de baixo: Disco" },
        { "Bass lines: Funky", "Linee di basso: Funky", "Líneas de bajo: Funky", "Lignes de basse : Funky", "Basslinien: Funky", "Linhas de baixo: Funky" },
        { "Bass lines: House", "Linee di basso: House", "Líneas de bajo: House", "Lignes de basse : House", "Basslinien: House", "Linhas de baixo: House" },
        { "Bass lines: Soulful", "Linee di basso: Soulful", "Líneas de bajo: Soulful", "Lignes de basse : Soulful", "Basslinien: Soulful", "Linhas de baixo: Soulful" },
        // ---- v1.21 voicing coach ----
        { "Voicing Coach", "Voicing Coach", "Voicing Coach", "Voicing Coach", "Voicing-Coach", "Voicing Coach" },
        { "HEAR A", "ASCOLTA A", "ESCUCHAR A", "ÉCOUTER A", "A HÖREN", "OUVIR A" },
        { "HEAR B", "ASCOLTA B", "ESCUCHAR B", "ÉCOUTER B", "B HÖREN", "OUVIR B" },
        { "Range", "Estensione", "Registro", "Tessiture", "Umfang", "Extensão" },
        { "semitones", "semitoni", "semitonos", "demi-tons", "Halbtöne", "semitons" },
        { "Voices move", "Movimento delle voci", "Las voces se mueven", "Mouvement des voix", "Stimmen bewegen sich", "As vozes movem-se" },
        { "semitones per chord change", "semitoni a ogni cambio d'accordo", "semitonos por cambio de acorde", "demi-tons par changement d'accord",
          "Halbtöne pro Akkordwechsel", "semitons por troca de acorde" },
        { "Biggest top-note leap", "Salto più grande della nota in alto", "Mayor salto de la nota superior", "Plus grand saut de la note du haut",
          "Größter Sprung der Oberstimme", "Maior salto da nota de cima" },
        { "Muddy low notes", "Note basse impastate", "Notas graves empastadas", "Graves boueux", "Matschige tiefe Töne", "Graves embolados" },
        { "Semitone rubs", "Semitoni stridenti", "Roces de semitono", "Frottements de demi-ton", "Halbton-Reibungen", "Atritos de semitom" },
        { "Two voicings of your progression: same chords, different notes. Press HEAR A or HEAR B while the loop plays.",
          "Due voicing della tua progressione: stessi accordi, note diverse. Premi ASCOLTA A o ASCOLTA B mentre il loop suona.",
          "Dos voicings de tu progresión: mismos acordes, notas distintas. Pulsa ESCUCHAR A o ESCUCHAR B mientras suena el loop.",
          "Deux voicings de votre progression : mêmes accords, notes différentes. Appuyez sur ÉCOUTER A ou B pendant la boucle.",
          "Zwei Voicings deiner Progression: gleiche Akkorde, andere Töne. Drücke A HÖREN oder B HÖREN, während der Loop läuft.",
          "Dois voicings da sua progressão: mesmos acordes, notas diferentes. Prima OUVIR A ou OUVIR B enquanto o loop toca." },
        { "is smoother: its voices move less.", "è più morbido: le sue voci si muovono meno.", "es más suave: sus voces se mueven menos.",
          "est plus fluide : ses voix bougent moins.", "ist geschmeidiger: seine Stimmen bewegen sich weniger.", "é mais suave: as vozes movem-se menos." },
        { "Both lead the voices about the same: choose by ear.", "Muovono le voci più o meno allo stesso modo: scegli a orecchio.",
          "Ambos conducen las voces casi igual: elige de oído.", "Les deux conduisent les voix à peu près pareil : choisissez à l'oreille.",
          "Beide führen die Stimmen etwa gleich: entscheide nach Gehör.", "Ambos conduzem as vozes quase igual: escolha de ouvido." },
        { "is muddier in the low register: raise the Register.", "è più impastato nel registro basso: alza il Registro.",
          "está más empastado en el grave: sube el Registro.", "est plus boueux dans le grave : montez le Registre.",
          "klingt tief matschiger: erhöhe das Register.", "está mais embolado no grave: suba o Registro." },
        { "is more compact (leaves room for vocals and bass).", "è più compatto (lascia spazio a voce e basso).",
          "es más compacto (deja sitio a la voz y al bajo).", "est plus compact (laisse de la place à la voix et à la basse).",
          "ist kompakter (lässt Platz für Gesang und Bass).", "é mais compacto (deixa espaço para voz e baixo)." },
        { "Voicing Coach: compare two voicings of your progression", "Voicing Coach: confronta due voicing della tua progressione",
          "Voicing Coach: compara dos voicings de tu progresión", "Voicing Coach : comparez deux voicings de votre progression",
          "Voicing-Coach: vergleiche zwei Voicings deiner Progression", "Voicing Coach: compare dois voicings da sua progressão" },
        // ---- v1.22 song and morph ----
        { "NEW VARIATION", "NUOVA VARIAZIONE", "NUEVA VARIACIÓN", "NOUVELLE VARIATION", "NEUE VARIATION", "NOVA VARIAÇÃO" },
        { "ORIGINAL", "ORIGINALE", "ORIGINAL", "ORIGINAL", "ORIGINAL", "ORIGINAL" },
        { "SAVE EACH SECTION AS .MID...", "SALVA OGNI SEZIONE IN .MID...", "GUARDAR CADA SECCIÓN EN .MID...", "ENREGISTRER CHAQUE SECTION EN .MID...", "JEDEN TEIL ALS .MID SPEICHERN...", "SALVAR CADA SEÇÃO EM .MID..." },
        { "Song arrangement", "Arrangiamento del brano", "Arreglo de la canción", "Arrangement du morceau", "Song-Arrangement", "Arranjo da música" },
        { "Bars of each section (- leaves it out) and a NEW VARIATION of the groove for one section only: the others stay as they are. Used by Clip length > Song, by Song section while playing, and by SAVE EACH SECTION.", "Battute di ogni sezione (- la toglie) e una NUOVA VARIAZIONE del groove per una sola sezione: le altre restano com'erano. Vale per Lunghezza clip > Brano, per Sezione del brano mentre suona e per SALVA OGNI SEZIONE.", "Compases de cada sección (- la quita) y una NUEVA VARIACIÓN del groove solo para una sección: las demás no cambian. Se usa en Longitud del clip > Canción, en Sección mientras suena y en GUARDAR CADA SECCIÓN.", "Mesures de chaque section (- la retire) et une NOUVELLE VARIATION du groove pour une seule section : les autres ne changent pas. Utilisé par Longueur du clip > Morceau, par Section en lecture et par ENREGISTRER CHAQUE SECTION.", "Takte jedes Teils (- lässt ihn weg) und eine NEUE VARIATION des Grooves nur für einen Teil: die anderen bleiben. Gilt für Clip-Länge > Song, für den Song-Teil beim Spielen und für JEDEN TEIL SPEICHERN.", "Compassos de cada seção (- tira-a) e uma NOVA VARIAÇÃO do groove só para uma seção: as outras ficam iguais. Vale para Duração do clip > Música, para Seção durante a reprodução e para SALVAR CADA SEÇÃO." },
        { "left out", "tolta", "quitada", "retirée", "weggelassen", "retirada" },
        { "own variation", "variazione propria", "variación propia", "variation propre", "eigene Variation", "variação própria" },
        { "the groove as it is", "il groove così com'è", "el groove tal cual", "le groove tel quel", "der Groove wie er ist", "o groove como está" },
        { "Total", "Totale", "Total", "Total", "Gesamt", "Total" },
        { "bars", "battute", "compases", "mesures", "Takte", "compassos" },
        { "KEEP", "TIENI", "MANTENER", "GARDER", "BEHALTEN", "MANTER" },
        { "CANCEL", "ANNULLA", "CANCELAR", "ANNULER", "ABBRECHEN", "CANCELAR" },
        { "Groove morph", "Morphing del groove", "Morphing del groove", "Morphing du groove", "Groove-Morphing", "Morphing do groove" },
        { "From", "Da", "Desde", "De", "Von", "De" },
        { "(locked parts stay as they are)", "(le parti bloccate restano com'erano)", "(las partes bloqueadas no cambian)", "(les parties verrouillées ne changent pas)", "(gesperrte Teile bleiben)", "(as partes bloqueadas ficam iguais)" },
        { "Towards", "Verso", "Hacia", "Vers", "Zu", "Para" },
        { "Amount", "Quantità", "Cantidad", "Quantité", "Menge", "Quantidade" },
        { "SONG", "BRANO", "CANCIÓN", "MORCEAU", "SONG", "MÚSICA" },
        { "MORPH", "MORPH", "MORPH", "MORPH", "MORPH", "MORPH" },
        { "Song arrangement: bars of each section, a new variation for one section, one .mid per section", "Arrangiamento del brano: battute di ogni sezione, una nuova variazione per una sezione, un .mid per sezione", "Arreglo: compases de cada sección, una nueva variación para una sección, un .mid por sección", "Arrangement : mesures de chaque section, une nouvelle variation pour une section, un .mid par section", "Arrangement: Takte pro Teil, eine neue Variation für einen Teil, eine .mid pro Teil", "Arranjo: compassos de cada seção, uma nova variação para uma seção, um .mid por seção" },
        { "Morph the groove step by step towards another one (locked parts stay)", "Trasforma il groove passo passo verso un altro (le parti bloccate restano)", "Transforma el groove paso a paso hacia otro (las partes bloqueadas se quedan)", "Transforme le groove pas à pas vers un autre (les parties verrouillées restent)", "Verwandle den Groove Schritt für Schritt in einen anderen (gesperrte Teile bleiben)", "Transforma o groove passo a passo noutro (as partes bloqueadas ficam)" },
        { "MIDI files saved in", "file MIDI salvati in", "archivos MIDI guardados en", "fichiers MIDI enregistrés dans", "MIDI-Dateien gespeichert in", "arquivos MIDI salvos em" },
        // ---- v1.23 musical DNA ----
        { "More soulful", "Più soulful", "Más soulful", "Plus soulful", "Mehr Soul", "Mais soulful" },
        { "Funkier", "Più funky", "Más funky", "Plus funky", "Funkiger", "Mais funky" },
        { "More minimal", "Più minimale", "Más minimal", "Plus minimal", "Minimaler", "Mais minimal" },
        { "More energy", "Più energia", "Más energía", "Plus d'énergie", "Mehr Energie", "Mais energia" },
        { "More sophisticated", "Più raffinato", "Más sofisticado", "Plus raffiné", "Raffinierter", "Mais sofisticado" },
        { "Keep the rhythm (grid and feel)", "Tieni il ritmo (griglia e feel)", "Mantener el ritmo (cuadrícula y feel)", "Garder le rythme (grille et feel)", "Rhythmus behalten (Raster und Feel)", "Manter o ritmo (grade e feel)" },
        { "Keep the harmony (extensions and voicing)", "Tieni l'armonia (estensioni e voicing)", "Mantener la armonía (extensiones y voicing)", "Garder l'harmonie (extensions et voicing)", "Harmonie behalten (Erweiterungen und Voicing)", "Manter a harmonia (extensões e voicing)" },
        { "Each click goes one step further. The chords never change, locked parts stay; undo with the arrow.", "Ogni clic fa un passo in più. Gli accordi non cambiano mai, le parti bloccate restano; annulla con la freccia.", "Cada clic da un paso más. Los acordes nunca cambian, las partes bloqueadas se quedan; deshaz con la flecha.", "Chaque clic va un pas plus loin. Les accords ne changent jamais, les parties verrouillées restent ; annulez avec la flèche.", "Jeder Klick geht einen Schritt weiter. Die Akkorde ändern sich nie, gesperrte Teile bleiben; rückgängig mit dem Pfeil.", "Cada clique vai um passo mais longe. Os acordes nunca mudam, as partes bloqueadas ficam; desfaça com a seta." },
        { "steps applied", "passi applicati", "pasos aplicados", "pas appliqués", "Schritte angewendet", "passos aplicados" },
        { "Musical DNA: variations by intention (more soulful, funkier, more minimal, more energy, more sophisticated)", "Musical DNA: variazioni per intenzione (più soulful, più funky, più minimale, più energia, più raffinato)", "Musical DNA: variaciones por intención (más soulful, más funky, más minimal, más energía, más sofisticado)", "Musical DNA : variations par intention (plus soulful, plus funky, plus minimal, plus d'énergie, plus raffiné)", "Musical DNA: Variationen nach Absicht (mehr Soul, funkiger, minimaler, mehr Energie, raffinierter)", "Musical DNA: variações por intenção (mais soulful, mais funky, mais minimal, mais energia, mais sofisticado)" },
        // ---- v1.23.1 ----
        { "This preset file is damaged or is not a Fase Groove Piano preset: nothing was changed.",
          "Questo file di preset è danneggiato o non è un preset di Fase Groove Piano: non è stato cambiato niente.",
          "Este archivo de preset está dañado o no es un preset de Fase Groove Piano: no se ha cambiado nada.",
          "Ce fichier de preset est endommagé ou n'est pas un preset Fase Groove Piano : rien n'a été modifié.",
          "Diese Preset-Datei ist beschädigt oder kein Fase-Groove-Piano-Preset: nichts wurde geändert.",
          "Este arquivo de preset está danificado ou não é um preset do Fase Groove Piano: nada foi alterado." },
        { "Reliability", "Affidabilità", "Fiabilidad", "Fiabilité", "Zuverlässigkeit", "Confiabilidade" },
        { "high", "alta", "alta", "élevée", "hoch", "alta" },
        { "medium", "media", "media", "moyenne", "mittel", "média" },
        { "low: check the chords", "bassa: controlla gli accordi", "baja: revisa los acordes", "faible : vérifiez les accords", "niedrig: prüfe die Akkorde", "baixa: confira os acordes" },
        // ---- v1.24 ----
        { "VARIANTS", "VARIANTI", "VARIANTES", "VARIANTES", "VARIANTEN", "VARIANTES" },
        { "Chord variations", "Varianti armoniche", "Variaciones de acordes", "Variations d'accords", "Akkord-Variationen", "Variações de acordes" },
        { "Alternatives of your progression in the same key, same number of chords (the rhythm stays). HEAR them while the loop plays.", "Alternative della tua progressione nella stessa tonalità, con lo stesso numero di accordi (il ritmo resta). ASCOLTALE mentre il loop suona.", "Alternativas de tu progresión en la misma tonalidad y con el mismo número de acordes (el ritmo se queda). ESCÚCHALAS mientras suena el loop.", "Des alternatives de votre progression dans la même tonalité, même nombre d'accords (le rythme reste). ÉCOUTEZ-les pendant la boucle.", "Alternativen deiner Progression in derselben Tonart, gleich viele Akkorde (der Rhythmus bleibt). HÖRE sie, während der Loop läuft.", "Alternativas da sua progressão no mesmo tom e com o mesmo número de acordes (o ritmo fica). OUÇA-as enquanto o loop toca." },
        { "Chord variations: more soulful, gospel, deep or chromatic versions of your progression, to hear and keep", "Varianti armoniche: versioni più soulful, gospel, deep o cromatiche della tua progressione, da ascoltare e tenere", "Variaciones: versiones más soulful, gospel, deep o cromáticas de tu progresión, para oír y quedarte", "Variations : versions plus soulful, gospel, deep ou chromatiques de votre progression, à écouter et garder", "Variationen: soulige, Gospel-, Deep- oder chromatische Versionen deiner Progression zum Anhören und Behalten", "Variações: versões mais soulful, gospel, deep ou cromáticas da sua progressão, para ouvir e manter" },
        { "Original", "Originale", "Original", "Original", "Original", "Original" },
        { "HEAR", "ASCOLTA", "ESCUCHAR", "ÉCOUTER", "HÖREN", "OUVIR" },
        { "Chromatic", "Cromatica", "Cromática", "Chromatique", "Chromatisch", "Cromática" },
        { "NEW FEEL", "NUOVO FEEL", "NUEVO FEEL", "NOUVEAU FEEL", "NEUES FEEL", "NOVO FEEL" },
        { "Another humanization of the same groove (repeatable: saved with the project)", "Un'altra umanizzazione dello stesso groove (ripetibile: si salva nel progetto)", "Otra humanización del mismo groove (repetible: se guarda en el proyecto)", "Une autre humanisation du même groove (répétable : enregistrée avec le projet)", "Eine andere Humanisierung desselben Grooves (wiederholbar: im Projekt gespeichert)", "Outra humanização do mesmo groove (repetível: salva no projeto)" },
        { "How Humanize is shared: Even, Pianist (the top note sings), Laid-back bass, Tight bass. Click to change", "Come si divide l'umanizzazione: Uniforme, Pianista (la nota in alto canta), Basso rilassato, Basso stretto. Clicca per cambiare", "Cómo se reparte la humanización: Uniforme, Pianista (la nota alta canta), Bajo relajado, Bajo justo. Haz clic para cambiar", "Répartition de l'humanisation : Égale, Pianiste (la note du haut chante), Basse décontractée, Basse serrée. Cliquez pour changer", "Wie Humanize verteilt wird: Gleichmäßig, Pianist (die Oberstimme singt), Lässiger Bass, Straffer Bass. Klicken zum Wechseln", "Como a humanização é dividida: Uniforme, Pianista (a nota de cima canta), Baixo relaxado, Baixo justo. Clique para mudar" },
        { "Feel", "Feel", "Feel", "Feel", "Feel", "Feel" },
        { "Even", "Uniforme", "Uniforme", "Égale", "Gleichmäßig", "Uniforme" },
        { "Pianist", "Pianista", "Pianista", "Pianiste", "Pianist", "Pianista" },
        { "Laid-back bass", "Basso rilassato", "Bajo relajado", "Basse décontractée", "Lässiger Bass", "Baixo relaxado" },
        { "Tight bass", "Basso stretto", "Bajo justo", "Basse serrée", "Straffer Bass", "Baixo justo" },
        { "Fill: as set", "Fill: come impostato", "Fill: como esté", "Fill : comme réglé", "Fill: wie eingestellt", "Fill: como definido" },
        { "No fill", "Nessun fill", "Sin fill", "Pas de fill", "Kein Fill", "Sem fill" },
        { "Fill", "Fill", "Fill", "Fill", "Fill", "Fill" },
        { "Bars of each section (- leaves it out), a NEW VARIATION of the groove for one section only, and the fill that leads out of it. Used by Clip length > Song, by Song section while playing (you hear the fill as it loops), and by SAVE EACH SECTION.", "Battute di ogni sezione (- la toglie), una NUOVA VARIAZIONE del groove per una sola sezione e il fill che la chiude. Vale per Lunghezza clip > Brano, per la Sezione del brano mentre suona (senti il fill mentre gira) e per SALVA OGNI SEZIONE.", "Compases de cada sección (- la quita), una NUEVA VARIACIÓN del groove para una sola sección y el fill que la cierra. Se usa en Longitud del clip > Canción, en Sección mientras suena (oyes el fill en bucle) y en GUARDAR CADA SECCIÓN.", "Mesures de chaque section (- la retire), une NOUVELLE VARIATION du groove pour une seule section et le fill qui la termine. Utilisé par Longueur du clip > Morceau, par Section en lecture (vous entendez le fill en boucle) et par ENREGISTRER CHAQUE SECTION.", "Takte jedes Teils (- lässt ihn weg), eine NEUE VARIATION des Grooves nur für einen Teil und der Fill, der ihn abschließt. Gilt für Clip-Länge > Song, für den Song-Teil beim Spielen (du hörst den Fill im Loop) und für JEDEN TEIL SPEICHERN.", "Compassos de cada seção (- tira-a), uma NOVA VARIAÇÃO do groove só para uma seção e o fill que a fecha. Vale para Duração do clip > Música, para Seção durante a reprodução (ouve o fill em loop) e para SALVAR CADA SEÇÃO." },
        { "Separate tracks", "Tracce separate", "Pistas separadas", "Pistes séparées", "Getrennte Spuren", "Faixas separadas" },
        // ---- v1.15 updates ----
        { "Check for updates automatically (GitHub)", "Controlla gli aggiornamenti in automatico (GitHub)", "Buscar actualizaciones automáticamente (GitHub)",
          "Rechercher les mises à jour automatiquement (GitHub)", "Automatisch nach Updates suchen (GitHub)", "Procurar atualizações automaticamente (GitHub)" },
        { "CHECK NOW", "CONTROLLA ORA", "BUSCAR AHORA", "VÉRIFIER", "JETZT PRÜFEN", "VERIFICAR AGORA" },
        { "CHECKING...", "CONTROLLO...", "BUSCANDO...", "VÉRIFICATION...", "PRÜFE...", "VERIFICANDO..." },
        { "INSTALL NOW", "INSTALLA ORA", "INSTALAR AHORA", "INSTALLER", "JETZT INSTALLIEREN", "INSTALAR AGORA" },
        { "LATER", "PIÙ TARDI", "MÁS TARDE", "PLUS TARD", "SPÄTER", "MAIS TARDE" },
        { "SKIP THIS VERSION", "SALTA QUESTA VERSIONE", "OMITIR ESTA VERSIÓN", "IGNORER CETTE VERSION", "VERSION ÜBERSPRINGEN", "PULAR ESTA VERSÃO" },
        { "New version", "Nuova versione", "Nueva versión", "Nouvelle version", "Neue Version", "Nova versão" },
        { "downloaded", "scaricata", "descargada", "téléchargée", "heruntergeladen", "baixada" },
        { "You have", "Hai la", "Tienes la", "Vous avez la", "Du hast", "Você tem a" },
        { "What's new:", "Cosa c'è di nuovo:", "Novedades:", "Nouveautés :", "Neu:", "Novidades:" },
        { "You have the latest version.", "Hai già l'ultima versione.", "Ya tienes la última versión.", "Vous avez déjà la dernière version.",
          "Du hast bereits die neueste Version.", "Você já tem a versão mais recente." },
        { "Could not check for updates. Check your internet connection and try again.",
          "Impossibile controllare gli aggiornamenti. Controlla la connessione a internet e riprova.",
          "No se pudo buscar actualizaciones. Comprueba tu conexión a internet e inténtalo de nuevo.",
          "Impossible de vérifier les mises à jour. Vérifiez votre connexion internet et réessayez.",
          "Updates konnten nicht geprüft werden. Prüfe deine Internetverbindung und versuche es erneut.",
          "Não foi possível verificar atualizações. Verifique sua conexão com a internet e tente novamente." },
        { "INSTALL NOW starts the installer. Close your DAW when it asks, then reopen it.",
          "INSTALLA ORA avvia l'installer. Chiudi la DAW quando te lo chiede, poi riaprila.",
          "INSTALAR AHORA abre el instalador. Cierra tu DAW cuando te lo pida y vuelve a abrirla.",
          "INSTALLER lance l'installateur. Fermez votre DAW quand il le demande, puis rouvrez-la.",
          "JETZT INSTALLIEREN startet das Setup. Schließe deine DAW, wenn es danach fragt, und öffne sie dann wieder.",
          "INSTALAR AGORA abre o instalador. Feche sua DAW quando ele pedir e depois abra de novo." },
        { "INSTALL NOW opens the zip: copy the plug-ins to the same folders as the first time, then restart your DAW.",
          "INSTALLA ORA apre lo zip: copia i plug-in nelle stesse cartelle della prima volta, poi riavvia la DAW.",
          "INSTALAR AHORA abre el zip: copia los plug-ins en las mismas carpetas que la primera vez y reinicia tu DAW.",
          "INSTALLER ouvre le zip : copiez les plug-ins dans les mêmes dossiers que la première fois, puis redémarrez votre DAW.",
          "JETZT INSTALLIEREN öffnet das Zip: kopiere die Plug-ins in dieselben Ordner wie beim ersten Mal und starte die DAW neu.",
          "INSTALAR AGORA abre o zip: copie os plug-ins para as mesmas pastas da primeira vez e reinicie sua DAW." },
        { "INSTALL NOW shows the downloaded zip: copy the plug-ins to the same folders as the first time.",
          "INSTALLA ORA mostra lo zip scaricato: copia i plug-in nelle stesse cartelle della prima volta.",
          "INSTALAR AHORA muestra el zip descargado: copia los plug-ins en las mismas carpetas que la primera vez.",
          "INSTALLER affiche le zip téléchargé : copiez les plug-ins dans les mêmes dossiers que la première fois.",
          "JETZT INSTALLIEREN zeigt das heruntergeladene Zip: kopiere die Plug-ins in dieselben Ordner wie beim ersten Mal.",
          "INSTALAR AGORA mostra o zip baixado: copie os plug-ins para as mesmas pastas da primeira vez." },
        // ---- v1.14 dance pack ----
        { "Chromatic & borrowed", "Cromatiche e prestiti", "Cromáticas y prestadas", "Chromatiques et empruntées", "Chromatisch & entlehnt", "Cromáticas e emprestadas" },
        { "Octave triads (disco)", "Triadi in ottava (disco)", "Tríadas en octava (disco)", "Triades à l'octave (disco)", "Oktav-Dreiklänge (Disco)", "Tríades em oitava (disco)" },
        { "Dance Stabs", "Stab dance", "Stabs dance", "Stabs dance", "Dance-Stabs", "Stabs dance" },
        { "Future House", "Future house", "Future house", "Future house", "Future House", "Future house" },
        { "Disco Piano", "Piano disco", "Piano disco", "Piano disco", "Disco-Piano", "Piano disco" },
        { "Free", "Libero", "Libre", "Libre", "Frei", "Livre" },
        { "Free chords", "Accordi liberi", "Acordes libres", "Accords libres", "Freie Akkorde", "Acordes livres" },
        { "Free chords: drag the dots anywhere on the circle, even outside the scale",
          "Accordi liberi: trascina i punti ovunque nel circolo, anche fuori scala",
          "Acordes libres: arrastra los puntos a cualquier lugar del círculo, incluso fuera de la escala",
          "Accords libres : fais glisser les points n'importe où sur le cercle, même hors de la gamme",
          "Freie Akkorde: Punkte überall auf den Kreis ziehen, auch außerhalb der Skala",
          "Acordes livres: arraste os pontos para qualquer lugar do círculo, mesmo fora da escala" },
        { "Approach chord", "Accordo di passaggio", "Acorde de aproximación", "Accord d'approche", "Annäherungsakkord", "Acorde de aproximação" },
        { "From above", "Da sopra", "Desde arriba", "Par le haut", "Von oben", "De cima" },
        { "From below", "Da sotto", "Desde abajo", "Par le bas", "Von unten", "De baixo" },
        { "Gospel slide: on the last 16th before a chord change, play the next chord a half step above or below",
          "Slide gospel: sull'ultimo sedicesimo prima del cambio, suona l'accordo successivo mezzo tono sopra o sotto",
          "Slide gospel: en la última semicorchea antes del cambio, toca el acorde siguiente medio tono arriba o abajo",
          "Glissé gospel : sur la dernière double croche avant le changement, joue l'accord suivant un demi-ton au-dessus ou en dessous",
          "Gospel-Slide: auf der letzten Sechzehntel vor dem Wechsel den nächsten Akkord einen Halbton höher oder tiefer spielen",
          "Slide gospel: na última semicolcheia antes da troca, toca o próximo acorde meio tom acima ou abaixo" },
        { "Add 9", "Add 9", "Add 9", "Add 9", "Add 9", "Add 9" },
        { "9ths + altered V", "None + V alterata", "Novenas + V alterado", "Neuvièmes + V altéré", "Nonen + alteriertes V", "Nonas + V alterado" },
        { "Shell (3-7-top)", "Shell (3-7-nota alta)", "Shell (3-7-arriba)", "Shell (3-7-aigu)", "Shell (3-7-oben)", "Shell (3-7-topo)" },
        { "Locrian", "Locrio", "Locrio", "Locrien", "Lokrisch", "Lócrio" },
        { "Harmonic Major", "Maggiore armonica", "Mayor armónica", "Majeur harmonique", "Harmonisch Dur", "Maior harmônica" },
        { "Lydian Dominant", "Lidia dominante", "Lidia dominante", "Lydien dominant", "Lydisch dominant", "Lídia dominante" },
        { "Mixolydian b6", "Misolidia b6", "Mixolidia b6", "Mixolydien b6", "Mixolydisch b6", "Mixolídia b6" },
        { "Phrygian Dominant", "Frigia dominante", "Frigia dominante", "Phrygien dominant", "Phrygisch dominant", "Frígia dominante" },
        { "Dorian", "Dorica", "Dórica", "Dorien", "Dorisch", "Dórica" },
        { "Mixolydian", "Misolidia", "Mixolidia", "Mixolydien", "Mixolydisch", "Mixolídia" },
        { "Lydian", "Lidia", "Lidia", "Lydien", "Lydisch", "Lídia" },
        { "Phrygian", "Frigia", "Frigia", "Phrygien", "Phrygisch", "Frígia" },
        // ---- v1.25: piano roll, export, song, morph ----
        { "House pedal", "Pedale house", "Pedal house", "Pédale house", "House-Orgelpunkt", "Pedal house" },
        { "PIANO ROLL", "PIANO ROLL", "PIANO ROLL", "PIANO ROLL", "PIANO ROLL", "PIANO ROLL" },
        { "EDIT GRID", "GRIGLIA", "REJILLA", "GRILLE", "RASTER", "GRADE" },
        { "BOTH", "TUTTI", "AMBOS", "LES DEUX", "BEIDE", "AMBOS" },
        { "PIANO", "PIANO", "PIANO", "PIANO", "PIANO", "PIANO" },
        { "EXPORT MIDI", "ESPORTA MIDI", "EXPORTAR MIDI", "EXPORTER LE MIDI", "MIDI EXPORTIEREN", "EXPORTAR MIDI" },
        { "FULL SONG...", "BRANO INTERO...", "CANCIÓN ENTERA...", "MORCEAU ENTIER...", "GANZER SONG...", "MÚSICA INTEIRA..." },
        { "SAVE .MID...", "SALVA .MID...", "GUARDAR .MID...", "ENREGISTRER .MID...", ".MID SPEICHERN...", "SALVAR .MID..." },
        { "Every note the plug-in plays: chords, bass and fills, with the chord names", "Tutte le note che suona il plug-in: accordi, basso e fill, con i nomi degli accordi", "Todas las notas que toca el plug-in: acordes, bajo y rellenos, con los nombres de los acordes", "Toutes les notes jouées par le plug-in : accords, basse et fills, avec le nom des accords", "Alle Noten, die das Plug-in spielt: Akkorde, Bass und Fills, mit den Akkordnamen", "Todas as notas que o plug-in toca: acordes, baixo e viradas, com os nomes dos acordes" },
        { "The rhythm grid: click the steps to write your own groove", "La griglia ritmica: clicca sui passi per scrivere il tuo groove", "La rejilla rítmica: haz clic en los pasos para escribir tu groove", "La grille rythmique : cliquez sur les pas pour écrire votre groove", "Das Rhythmusraster: Klicke auf die Schritte, um deinen Groove zu schreiben", "A grade rítmica: clique nos passos para escrever o seu groove" },
        { "Hear the piano and the bass", "Ascolta piano e basso", "Escucha el piano y el bajo", "Écouter le piano et la basse", "Klavier und Bass hören", "Ouvir o piano e o baixo" },
        { "Piano only: the bass is switched off", "Solo piano: il basso è spento", "Solo piano: el bajo está apagado", "Piano seul : la basse est coupée", "Nur Klavier: der Bass ist aus", "Só piano: o baixo está desligado" },
        { "The whole song: arrange the sections, then drag or save it as one .mid, or one .mid per section", "Il brano intero: organizza le sezioni, poi trascinalo o salvalo come un unico .mid, o un .mid per sezione", "La canción entera: ordena las secciones y arrástrala o guárdala como un .mid, o un .mid por sección", "Le morceau entier : organisez les sections, puis glissez-le ou enregistrez-le en un .mid, ou un .mid par section", "Der ganze Song: Teile anordnen, dann als eine .mid ziehen oder speichern, oder eine .mid pro Teil", "A música inteira: organize as seções e arraste ou salve como um .mid, ou um .mid por seção" },
        { "No notes: the piano and the bass are both silent", "Nessuna nota: piano e basso sono entrambi muti", "Sin notas: el piano y el bajo están en silencio", "Aucune note : le piano et la basse sont muets", "Keine Noten: Klavier und Bass sind stumm", "Sem notas: o piano e o baixo estão em silêncio" },
        { "Morph towards...", "Morph verso...", "Morph hacia...", "Morph vers...", "Morph zu...", "Morph para..." },
        { "Click the groove to morph towards: genres on the left, or type its name.", "Clicca il groove verso cui fare il morph: generi a sinistra, oppure scrivi il nome.", "Haz clic en el groove de destino: géneros a la izquierda, o escribe su nombre.", "Cliquez sur le groove visé : genres à gauche, ou tapez son nom.", "Klicke auf den Ziel-Groove: Genres links, oder tippe seinen Namen.", "Clique no groove de destino: gêneros à esquerda, ou digite o nome." },
        { "ONE .MID PER SECTION...", "UN .MID PER SEZIONE...", "UN .MID POR SECCIÓN...", "UN .MID PAR SECTION...", "EINE .MID PRO TEIL...", "UM .MID POR SEÇÃO..." },
        { "SAVE SONG .MID...", "SALVA BRANO .MID...", "GUARDAR CANCIÓN .MID...", "ENREGISTRER LE MORCEAU .MID...", "SONG ALS .MID SPEICHERN...", "SALVAR MÚSICA .MID..." },
        { "Drag the whole song (every section, with markers) onto a track in your DAW", "Trascina il brano intero (tutte le sezioni, con i marker) su una traccia della DAW", "Arrastra la canción entera (todas las secciones, con marcadores) a una pista de tu DAW", "Glissez le morceau entier (toutes les sections, avec marqueurs) sur une piste de votre DAW", "Ziehe den ganzen Song (alle Teile, mit Markern) auf eine Spur deiner DAW", "Arraste a música inteira (todas as seções, com marcadores) para uma faixa da sua DAW" },
        { "Bars of each section (- leaves it out), its own variation of the groove and the fill that leads out of it. Then drag or save the whole song.", "Battute di ogni sezione (- la toglie), la sua variazione del groove e il fill con cui esce. Poi trascina o salva il brano intero.", "Compases de cada sección (- la quita), su propia variación del groove y el relleno de salida. Luego arrastra o guarda la canción entera.", "Mesures de chaque section (- la retire), sa variation du groove et le fill de sortie. Puis glissez ou enregistrez le morceau entier.", "Takte jedes Teils (- lässt ihn weg), seine eigene Groove-Variation und das Fill am Ende. Dann den ganzen Song ziehen oder speichern.", "Compassos de cada seção (- remove), a sua variação do groove e a virada de saída. Depois arraste ou salve a música inteira." },
        { "Section", "Sezione", "Sección", "Section", "Teil", "Seção" },
        { "Bars", "Battute", "Compases", "Mesures", "Takte", "Compassos" },
        { "Fill out", "Fill in uscita", "Relleno de salida", "Fill de sortie", "Fill am Ende", "Virada de saída" },
        { "Song", "Brano", "Canción", "Morceau", "Song", "Música" },
        { "Save MIDI clip", "Salva clip MIDI", "Guardar clip MIDI", "Enregistrer le clip MIDI", "MIDI-Clip speichern", "Salvar clipe MIDI" },
        { "Edited notes", "Note modificate", "Notas editadas", "Notes modifiées", "Bearbeitete Noten", "Notas editadas" },
        { "EDIT NOTES", "MODIFICA NOTE", "EDITAR NOTAS", "ÉDITER NOTES", "NOTEN BEARBEITEN", "EDITAR NOTAS" },
        { "Open the piano roll big, zoom in and edit the notes one by one (or double-click the piano roll)", "Apre il piano roll in grande: zoom e modifica delle note una per una (o doppio clic sul piano roll)", "Abre el piano roll en grande: zoom y edición nota por nota (o doble clic en el piano roll)", "Ouvre le piano roll en grand : zoom et édition note par note (ou double-clic sur le piano roll)", "Öffnet die Pianorolle groß: zoomen und Noten einzeln bearbeiten (oder Doppelklick auf die Pianorolle)", "Abre o piano roll em tamanho grande: zoom e edição nota a nota (ou clique duplo no piano roll)" },
        { "BACK TO GENERATED", "TORNA AL GENERATO", "VOLVER A LO GENERADO", "REVENIR AU GÉNÉRÉ", "ZURÜCK ZUM GENERIERTEN", "VOLTAR AO GERADO" },
        { "Forget your edits: the plug-in plays the generated notes again", "Dimentica le modifiche: il plug-in torna a suonare le note generate", "Olvida tus cambios: el plug-in vuelve a tocar las notas generadas", "Oublie vos modifications : le plug-in rejoue les notes générées", "Verwirft deine Änderungen: das Plug-in spielt wieder die generierten Noten", "Esquece as suas edições: o plug-in volta a tocar as notas geradas" },
        { "FIT", "ADATTA", "AJUSTAR", "AJUSTER", "EINPASSEN", "AJUSTAR" },
        { "Grid", "Griglia", "Rejilla", "Grille", "Raster", "Grade" },
        { "New notes", "Nuove note", "Notas nuevas", "Nouvelles notes", "Neue Noten", "Notas novas" },
        { "Zoom out (Ctrl + mouse wheel)", "Riduci (Ctrl + rotella)", "Alejar (Ctrl + rueda)", "Dézoomer (Ctrl + molette)", "Verkleinern (Strg + Mausrad)", "Afastar (Ctrl + roda)" },
        { "Zoom in (Ctrl + mouse wheel)", "Ingrandisci (Ctrl + rotella)", "Acercar (Ctrl + rueda)", "Zoomer (Ctrl + molette)", "Vergrößern (Strg + Mausrad)", "Aproximar (Ctrl + roda)" },
        { "Whether the notes you add are piano or bass (exported on their own track)", "Se le note che aggiungi sono di piano o di basso (esportate sulla loro traccia)", "Si las notas que añades son de piano o de bajo (exportadas en su pista)", "Si les notes ajoutées sont du piano ou de la basse (exportées sur leur piste)", "Ob neue Noten Klavier oder Bass sind (eigene Spur beim Export)", "Se as notas que adiciona são de piano ou de baixo (exportadas na sua faixa)" },
        { "Generated notes (edit one to start)", "Note generate (modificane una per iniziare)", "Notas generadas (edita una para empezar)", "Notes générées (modifiez-en une pour commencer)", "Generierte Noten (bearbeite eine, um zu beginnen)", "Notas geradas (edite uma para começar)" },
        { "Piano roll", "Piano roll", "Piano roll", "Piano roll", "Pianorolle", "Piano roll" },
        { "Click: add a note  |  drag: move (freely, or on the grid with SNAP)  |  drag the right edge: length  |  double-click or right-click: delete  |  arrows: move the selected note (Shift: octave)  |  bottom lane: velocity", "Clic: aggiungi una nota  |  trascina: sposta (libera, o sulla griglia con SNAP)  |  trascina il bordo destro: durata  |  doppio clic o clic destro: elimina  |  frecce: sposta la nota scelta (Shift: ottava)  |  corsia in basso: velocity", "Clic: añade una nota  |  arrastra: mueve (libre, o en la rejilla con SNAP)  |  arrastra el borde derecho: duración  |  doble clic o clic derecho: borra  |  flechas: mueve la nota elegida (Shift: octava)  |  carril inferior: velocity", "Clic : ajouter une note  |  glisser : déplacer (librement, ou sur la grille avec SNAP)  |  bord droit : durée  |  double-clic ou clic droit : supprimer  |  flèches : déplacer la note (Maj : octave)  |  bande du bas : vélocité", "Klick: Note hinzufügen  |  ziehen: verschieben (frei, oder im Raster mit SNAP)  |  rechter Rand: Länge  |  Doppelklick oder Rechtsklick: löschen  |  Pfeile: gewählte Note bewegen (Umschalt: Oktave)  |  untere Spur: Velocity", "Clique: adiciona uma nota  |  arraste: move (livre, ou na grade com SNAP)  |  borda direita: duração  |  clique duplo ou direito: apaga  |  setas: move a nota escolhida (Shift: oitava)  |  faixa inferior: velocity" },
        { "SNAP", "SNAP", "SNAP", "SNAP", "SNAP", "SNAP" },
        { "QUANTIZE", "QUANTIZZA", "CUANTIZAR", "QUANTIFIER", "QUANTISIEREN", "QUANTIZAR" },
        { "On: notes stick to the grid. Off: move them freely (Alt while dragging does the opposite)", "Acceso: le note si agganciano alla griglia. Spento: le sposti liberamente (Alt mentre trascini fa il contrario)", "Activado: las notas se pegan a la rejilla. Desactivado: las mueves libremente (Alt al arrastrar hace lo contrario)", "Activé : les notes collent à la grille. Désactivé : déplacement libre (Alt pendant le glisser fait l'inverse)", "An: Noten rasten im Raster ein. Aus: frei verschieben (Alt beim Ziehen macht das Gegenteil)", "Ligado: as notas encaixam na grade. Desligado: move livremente (Alt ao arrastar faz o contrário)" },
        { "Moves the selected note (or every note, when none is selected) onto the grid", "Porta sulla griglia la nota scelta (o tutte le note, se nessuna è scelta)", "Lleva a la rejilla la nota elegida (o todas, si no hay ninguna elegida)", "Place la note choisie (ou toutes les notes, si aucune n'est choisie) sur la grille", "Setzt die gewählte Note (oder alle Noten, wenn keine gewählt ist) ins Raster", "Coloca na grade a nota escolhida (ou todas, se nenhuma estiver escolhida)" },
        { "DRAW", "MATITA", "LÁPIZ", "CRAYON", "STIFT", "LÁPIS" },
        { "MOVE", "SPOSTA", "MOVER", "DÉPLACER", "BEWEGEN", "MOVER" },
        { "ERASE", "GOMMA", "BORRAR", "GOMME", "RADIERER", "APAGAR" },
        { "FOLLOW", "SEGUI", "SEGUIR", "SUIVRE", "FOLGEN", "SEGUIR" },
        { "ALL", "TUTTO", "TODO", "TOUT", "ALLES", "TUDO" },
        { "Pencil: click to add a note (drag to make it longer); drag a note to move it", "Matita: clic per aggiungere una nota (trascina per allungarla); trascina una nota per spostarla", "Lápiz: clic para añadir una nota (arrastra para alargarla); arrastra una nota para moverla", "Crayon : cliquez pour ajouter une note (glissez pour l'allonger) ; glissez une note pour la déplacer", "Stift: Klick fügt eine Note hinzu (ziehen verlängert sie); eine Note ziehen verschiebt sie", "Lápis: clique para adicionar uma nota (arraste para alongar); arraste uma nota para movê-la" },
        { "Arrow: drag a note to move it, its right edge to change its length", "Freccia: trascina una nota per spostarla, il suo bordo destro per cambiarne la durata", "Flecha: arrastra una nota para moverla, su borde derecho para cambiar la duración", "Flèche : glissez une note pour la déplacer, son bord droit pour changer sa durée", "Pfeil: Note ziehen verschiebt sie, ihr rechter Rand ändert die Länge", "Seta: arraste uma nota para movê-la, a borda direita para mudar a duração" },
        { "Eraser: click a note, or drag over several, to delete them", "Gomma: clicca una nota, o trascina sopra più note, per cancellarle", "Goma: haz clic en una nota, o arrastra sobre varias, para borrarlas", "Gomme : cliquez sur une note, ou glissez sur plusieurs, pour les effacer", "Radierer: Note anklicken oder über mehrere ziehen, um sie zu löschen", "Borracha: clique numa nota, ou arraste sobre várias, para apagá-las" },
        { "The pencil draws piano notes (orange)", "La matita disegna note di piano (arancioni)", "El lápiz dibuja notas de piano (naranjas)", "Le crayon dessine des notes de piano (orange)", "Der Stift zeichnet Klaviernoten (orange)", "O lápis desenha notas de piano (laranja)" },
        { "The pencil draws bass notes (pink, exported on the bass track)", "La matita disegna note di basso (rosa, esportate sulla traccia del basso)", "El lápiz dibuja notas de bajo (rosas, exportadas en la pista del bajo)", "Le crayon dessine des notes de basse (roses, exportées sur la piste basse)", "Der Stift zeichnet Bassnoten (rosa, auf der Bass-Spur exportiert)", "O lápis desenha notas de baixo (rosa, exportadas na faixa do baixo)" },
        { "The view follows the playhead while the loop plays", "La vista segue la testina mentre il loop suona", "La vista sigue el cursor mientras suena el loop", "La vue suit la tête de lecture pendant la boucle", "Die Ansicht folgt dem Abspielkopf, während der Loop läuft", "A vista segue o cursor enquanto o loop toca" },
        { "Show all the notes", "Mostra tutte le note", "Muestra todas las notas", "Afficher toutes les notes", "Alle Noten zeigen", "Mostrar todas as notas" },
        { "Show only the piano: the bass is greyed out and can't be touched", "Mostra solo il piano: il basso è grigio e non si può toccare", "Muestra solo el piano: el bajo queda en gris y no se puede tocar", "Afficher seulement le piano : la basse est grisée et intouchable", "Nur Klavier zeigen: der Bass ist grau und gesperrt", "Mostrar só o piano: o baixo fica cinza e não pode ser tocado" },
        { "Show only the bass: the piano is greyed out and can't be touched", "Mostra solo il basso: il piano è grigio e non si può toccare", "Muestra solo el bajo: el piano queda en gris y no se puede tocar", "Afficher seulement la basse : le piano est grisé et intouchable", "Nur Bass zeigen: das Klavier ist grau und gesperrt", "Mostrar só o baixo: o piano fica cinza e não pode ser tocado" },
        { "Show", "Mostra", "Mostrar", "Afficher", "Zeigen", "Mostrar" },
        { "Draws", "Disegna", "Dibuja", "Dessine", "Zeichnet", "Desenha" },
        { "Play / stop the preview (space bar)", "Avvia / ferma l'ascolto (barra spaziatrice)", "Reproducir / parar la escucha (barra espaciadora)", "Lecture / arrêt de l'écoute (barre d'espace)", "Vorhören starten / stoppen (Leertaste)", "Tocar / parar a escuta (barra de espaço)" },
        { "Hear", "Ascolta", "Escuchar", "Écouter", "Hören", "Ouvir" },
        { "notes selected", "note selezionate", "notas seleccionadas", "notes sélectionnées", "Noten ausgewählt", "notas selecionadas" },
        { "Full screen", "Schermo intero", "Pantalla completa", "Plein écran", "Vollbild", "Tela cheia" },
        { "Exit full screen", "Esci da schermo intero", "Salir de pantalla completa", "Quitter le plein écran", "Vollbild verlassen", "Sair da tela cheia" },
        { "Select: drag a rectangle with MOVE (or Shift + drag), Ctrl / Shift + click to add, Ctrl + A for all; then drag, arrows, Delete, QUANTIZE and the velocity lane work on all of them.", "Selezione: trascina un rettangolo con SPOSTA (o Shift + trascina), Ctrl / Shift + clic per aggiungere, Ctrl + A per tutte; poi trascina, frecce, Canc, QUANTIZZA e la corsia della dinamica lavorano su tutte.", "Selección: arrastra un rectángulo con MOVER (o Shift + arrastrar), Ctrl / Shift + clic para añadir, Ctrl + A para todas; luego arrastrar, flechas, Supr, CUANTIZAR y el carril de velocity actúan sobre todas.", "Sélection : tracez un rectangle avec DÉPLACER (ou Maj + glisser), Ctrl / Maj + clic pour ajouter, Ctrl + A pour tout ; ensuite glisser, flèches, Suppr, QUANTIFIER et la bande de vélocité agissent sur toutes.", "Auswahl: mit BEWEGEN ein Rechteck ziehen (oder Umschalt + ziehen), Strg / Umschalt + Klick zum Hinzufügen, Strg + A für alle; dann wirken Ziehen, Pfeile, Entf, QUANTISIEREN und die Velocity-Spur auf alle.", "Seleção: arraste um retângulo com MOVER (ou Shift + arrastar), Ctrl / Shift + clique para adicionar, Ctrl + A para todas; depois arrastar, setas, Delete, QUANTIZAR e a faixa de velocity agem em todas." },
        { "TRANSPORT & TEMPO", "TRASPORTO E TEMPO", "TRANSPORTE Y TEMPO", "TRANSPORT ET TEMPO", "TRANSPORT & TEMPO", "TRANSPORTE E ANDAMENTO" },
        { "Transport", "Trasporto", "Transporte", "Transport", "Transport", "Transporte" },
        { "Tools", "Strumenti", "Herramientas", "Outils", "Werkzeuge", "Ferramentas" },
        { "Zoom", "Zoom", "Zoom", "Zoom", "Zoom", "Zoom" },
        { "Follow", "Segui", "Seguir", "Suivre", "Folgen", "Seguir" },
        { "Back to the start", "Torna all'inizio", "Volver al inicio", "Retour au début", "Zurück zum Anfang", "Voltar ao início" },
        { "Play / pause the groove without starting the DAW (space bar)", "Avvia / metti in pausa il groove senza avviare la DAW (barra spaziatrice)", "Reproduce / pausa el groove sin arrancar la DAW (barra espaciadora)", "Lecture / pause du groove sans lancer la DAW (barre d'espace)", "Groove abspielen / pausieren, ohne die DAW zu starten (Leertaste)", "Tocar / pausar o groove sem iniciar a DAW (barra de espaço)" },
        { "Stop (the next play starts from the top)", "Stop (il prossimo play riparte dall'inizio)", "Stop (la próxima reproducción empieza desde el inicio)", "Stop (la prochaine lecture repart du début)", "Stopp (das nächste Abspielen beginnt am Anfang)", "Parar (o próximo play começa do início)" },
        { "Hear the piano (lit) or mute it: exported clips keep everything", "Senti il piano (acceso) o lo zittisci: le clip esportate tengono tutto", "Escucha el piano (encendido) o siléncialo: los clips exportados lo conservan todo", "Écouter le piano (allumé) ou le couper : les clips exportés gardent tout", "Klavier hören (an) oder stummschalten: exportierte Clips behalten alles", "Ouvir o piano (aceso) ou silenciá-lo: os clipes exportados mantêm tudo" },
        { "Hear the bass (lit) or switch it off", "Senti il basso (acceso) o lo spegni", "Escucha el bajo (encendido) o apágalo", "Écouter la basse (allumée) ou la couper", "Bass hören (an) oder ausschalten", "Ouvir o baixo (aceso) ou desligá-lo" },
        { "Show or hide the piano notes (hidden notes are greyed out and can't be touched)", "Mostra o nascondi le note del piano (quelle nascoste sono grigie e non si toccano)", "Muestra u oculta las notas del piano (las ocultas quedan en gris y no se tocan)", "Afficher ou masquer les notes du piano (masquées : grisées et intouchables)", "Klaviernoten zeigen oder ausblenden (ausgeblendete sind grau und gesperrt)", "Mostrar ou ocultar as notas do piano (as ocultas ficam cinza e não se tocam)" },
        { "Show or hide the bass notes (hidden notes are greyed out and can't be touched)", "Mostra o nascondi le note del basso (quelle nascoste sono grigie e non si toccano)", "Muestra u oculta las notas del bajo (las ocultas quedan en gris y no se tocan)", "Afficher ou masquer les notes de la basse (masquées : grisées et intouchables)", "Bassnoten zeigen oder ausblenden (ausgeblendete sind grau und gesperrt)", "Mostrar ou ocultar as notas do baixo (as ocultas ficam cinza e não se tocam)" },
        { "View", "Vista", "Vista", "Vue", "Ansicht", "Vista" },
        { "New rhythm", "Nuovo ritmo", "Nuevo ritmo", "Nouveau rythme", "Neuer Rhythmus", "Novo ritmo" },
        { "RANDOM", "CASUALE", "ALEATORIO", "ALÉATOIRE", "ZUFALL", "ALEATÓRIO" },
        { "Lush 11ths (m11, maj9#11)", "11e ricche (m11, maj9#11)", "11as ricas (m11, maj9#11)", "11es riches (m11, maj9#11)", "Volle 11er (m11, maj9#11)", "11as ricas (m11, maj9#11)" },
        { "Soulful 13ths", "13e soulful", "13as soulful", "13es soulful", "Soulful-13er", "13as soulful" },
        { "Funky 7#9", "7#9 funky", "7#9 funky", "7#9 funky", "Funky 7#9", "7#9 funky" },
        { "Upper triad (house stab)", "Triade superiore (stab house)", "Tríada superior (stab house)", "Triade supérieure (stab house)", "Obere Triade (House-Stab)", "Tríade superior (stab house)" },
        { "Spread 10ths (2 hands)", "Decime aperte (2 mani)", "Décimas abiertas (2 manos)", "Dixièmes ouvertes (2 mains)", "Gespreizte Dezimen (2 Hände)", "Décimas abertas (2 mãos)" },
        { "Open 5ths (deep pad)", "Quinte aperte (pad deep)", "Quintas abiertas (pad deep)", "Quintes ouvertes (pad deep)", "Offene Quinten (Deep-Pad)", "Quintas abertas (pad deep)" },
        { "Piano stabs (own rhythm)", "Stab di piano (ritmo suo)", "Stabs de piano (ritmo propio)", "Stabs de piano (rythme propre)", "Piano-Stabs (eigener Rhythmus)", "Stabs de piano (ritmo próprio)" },
        { "New velocity", "Nuova velocity", "Nueva velocidad", "Nouvelle vélocité", "Neuer Anschlag", "Nova velocidade" },
        { "New random velocities for the chords (unlocked parts): the rhythm stays", "Nuove velocity casuali per gli accordi (parti non bloccate): il ritmo resta", "Nuevas velocidades aleatorias para los acordes (partes no bloqueadas): el ritmo se queda", "Nouvelles vélocités aléatoires pour les accords (parties non verrouillées) : le rythme reste", "Neue Zufalls-Anschläge für die Akkorde (nicht gesperrte Teile): der Rhythmus bleibt", "Novas velocidades aleatórias para os acordes (partes não bloqueadas): o ritmo fica" },
        { "New random velocities for the bass (unlocked parts): the rhythm stays", "Nuove velocity casuali per il basso (parti non bloccate): il ritmo resta", "Nuevas velocidades aleatorias para el bajo (partes no bloqueadas): el ritmo se queda", "Nouvelles vélocités aléatoires pour la basse (parties non verrouillées) : le rythme reste", "Neue Zufalls-Anschläge für den Bass (nicht gesperrte Teile): der Rhythmus bleibt", "Novas velocidades aleatórias para o baixo (partes não bloqueadas): o ritmo fica" },
        { "On the velocity page: new random velocities for chords and bass (unlocked parts)", "Sulla pagina velocity: nuove velocity casuali per accordi e basso (parti non bloccate)", "En la página de velocidad: nuevas velocidades aleatorias para acordes y bajo (partes no bloqueadas)", "Sur la page vélocité : nouvelles vélocités aléatoires pour accords et basse (parties non verrouillées)", "Auf der Anschlag-Seite: neue Zufalls-Anschläge für Akkorde und Bass (nicht gesperrte Teile)", "Na página de velocidade: novas velocidades aleatórias para acordes e baixo (partes não bloqueadas)" },
        { "Whose velocities play here (A-D): e.g. A A A A gives every half bar the accents of the first", "Di quale parte sono le velocity che suonano qui (A-D): es. A A A A dà a ogni mezza battuta gli accenti della prima", "De qué parte son las velocidades que suenan aquí (A-D): p. ej. A A A A da a cada media barra los acentos de la primera", "De quelle partie viennent les vélocités jouées ici (A-D) : ex. A A A A donne à chaque demi-mesure les accents de la première", "Wessen Anschläge hier spielen (A-D): z. B. A A A A gibt jedem halben Takt die Akzente des ersten", "De qual parte são as velocidades que tocam aqui (A-D): ex. A A A A dá a cada meio compasso os acentos do primeiro" },
        { "New random velocities for this part (the rhythm stays)", "Nuove velocity casuali per questa parte (il ritmo resta)", "Nuevas velocidades aleatorias para esta parte (el ritmo se queda)", "Nouvelles vélocités aléatoires pour cette partie (le rythme reste)", "Neue Zufalls-Anschläge für diesen Teil (der Rhythmus bleibt)", "Novas velocidades aleatórias para esta parte (o ritmo fica)" },
        { "Keep these velocities when you press the random buttons", "Tieni queste velocity quando premi i pulsanti random", "Mantén estas velocidades al pulsar los botones aleatorios", "Garder ces vélocités quand vous appuyez sur les boutons aléatoires", "Diese Anschläge bei den Zufallsknöpfen behalten", "Manter estas velocidades ao apertar os botões aleatórios" },
        { "Pump 0%", "Pompa 0%", "Bombeo 0%", "Pompage 0%", "Pumpen 0%", "Pump 0%" },
        { "The signal arriving at the sidechain input: if it stays grey, nothing is routed to the plug-in", "Il segnale che arriva all'ingresso sidechain: se resta grigio, non arriva niente al plug-in", "La señal que llega a la entrada sidechain: si se queda gris, no llega nada al plug-in", "Le signal qui arrive à l'entrée sidechain : s'il reste gris, rien n'est envoyé au plug-in", "Das Signal am Sidechain-Eingang: bleibt es grau, kommt nichts am Plug-in an", "O sinal que chega à entrada sidechain: se ficar cinza, nada chega ao plug-in" },
        { "Sequencer", "Sequencer", "Secuenciador", "Séquenceur", "Sequencer", "Sequenciador" },
        { "Off: the groove doesn't play with the DAW, e.g. when the dragged MIDI clip plays on this track (no doubled notes). Incoming MIDI still plays the guide sound; the play button still previews.", "Spento: il groove non suona con la DAW, ad es. quando la clip MIDI trascinata suona su questa traccia (niente note doppie). Il MIDI in ingresso suona comunque il suono guida; il pulsante play fa ancora l'anteprima.", "Apagado: el groove no suena con la DAW, p. ej. cuando el clip MIDI arrastrado suena en esta pista (sin notas dobles). El MIDI entrante sigue sonando con el sonido guía; el botón play sigue haciendo la vista previa.", "Éteint : le groove ne joue pas avec la DAW, ex. quand le clip MIDI glissé joue sur cette piste (pas de notes doublées). Le MIDI entrant joue toujours le son guide ; le bouton lecture fait toujours l'aperçu.", "Aus: der Groove spielt nicht mit der DAW, z. B. wenn der gezogene MIDI-Clip auf dieser Spur spielt (keine doppelten Noten). Eingehendes MIDI spielt weiter den Guide-Sound; Play spielt weiter die Vorschau.", "Desligado: o groove não toca com a DAW, ex. quando o clipe MIDI arrastado toca nesta faixa (sem notas dobradas). O MIDI de entrada continua tocando o som guia; o botão play continua fazendo a prévia." },
        { "Hold: the note before keeps ringing (no new note). Its velocity is the one of that note.", "Tenuta: la nota prima continua a suonare (nessuna nota nuova). La sua velocity è quella di quella nota.", "Sostenido: la nota anterior sigue sonando (ninguna nota nueva). Su velocidad es la de esa nota.", "Tenue : la note précédente continue de sonner (pas de nouvelle note). Sa vélocité est celle de cette note.", "Halten: die Note davor klingt weiter (keine neue Note). Ihr Anschlag ist der dieser Note.", "Sustentada: a nota anterior continua soando (nenhuma nota nova). A velocidade é a dessa nota." },
        { "Launchpad / Push as a hardware grid (Mini MK3, X, Pro MK3, Push 2 / 3)", "Launchpad / Push come griglia hardware (Mini MK3, X, Pro MK3, Push 2 / 3)", "Launchpad / Push como cuadrícula física (Mini MK3, X, Pro MK3, Push 2 / 3)", "Launchpad / Push comme grille matérielle (Mini MK3, X, Pro MK3, Push 2 / 3)", "Launchpad / Push als Hardware-Raster (Mini MK3, X, Pro MK3, Push 2 / 3)", "Launchpad / Push como grade física (Mini MK3, X, Pro MK3, Push 2 / 3)" },
        { "The pads show and edit the groove, the round buttons play, stop and roll new rhythms. Set the Launchpad's control surface to None in your DAW.", "I pad mostrano e modificano il groove, i pulsanti tondi suonano, fermano e creano nuovi ritmi. Nella DAW imposta la control surface del Launchpad su Nessuna.", "Los pads muestran y editan el groove, los botones redondos reproducen, paran y crean ritmos nuevos. En tu DAW pon la superficie de control del Launchpad en Ninguna.", "Les pads affichent et modifient le groove, les boutons ronds jouent, arrêtent et créent de nouveaux rythmes. Dans votre DAW, réglez la surface de contrôle du Launchpad sur Aucune.", "Die Pads zeigen und bearbeiten den Groove, die runden Tasten spielen, stoppen und würfeln neue Rhythmen. Stelle in deiner DAW die Control Surface des Launchpad auf Keine.", "Os pads mostram e editam o groove, os botões redondos tocam, param e criam ritmos novos. Na sua DAW, ponha a superfície de controle do Launchpad em Nenhuma." },
        { "Not in use", "Non attivo", "No activo", "Inactif", "Nicht aktiv", "Inativo" },
        { "Looking for a Launchpad or a Push... (connect it by USB)", "Cerco un Launchpad o un Push... (collegalo via USB)", "Buscando un Launchpad o un Push... (conéctalo por USB)", "Recherche d'un Launchpad ou d'un Push... (branchez-le en USB)", "Suche ein Launchpad oder Push... (per USB anschließen)", "Procurando um Launchpad ou Push... (conecte por USB)" },
        { "Push: if the pads stay dark, press User on Push (Push 3: set the pads to Poly Aftertouch in Control Mode first)", "Push: se i pad restano spenti, premi User sul Push (Push 3: prima imposta i pad su Poly Aftertouch in Control Mode)", "Push: si los pads siguen apagados, pulsa User en el Push (Push 3: antes pon los pads en Poly Aftertouch en Control Mode)", "Push : si les pads restent éteints, appuyez sur User sur le Push (Push 3 : réglez d'abord les pads sur Poly Aftertouch en Control Mode)", "Push: bleiben die Pads dunkel, drücke User am Push (Push 3: stelle die Pads vorher in Control Mode auf Poly Aftertouch)", "Push: se os pads ficarem apagados, aperte User no Push (Push 3: antes ponha os pads em Poly Aftertouch no Control Mode)" },
        { "Connected:", "Collegato:", "Conectado:", "Connecté :", "Verbunden:", "Conectado:" },
        { "The Launchpad is in use by another program: in your DAW's MIDI settings set its control surface to None and switch off its ports (Push: switch off Track and Remote on its User Port)", "Il Launchpad è usato da un altro programma: nelle impostazioni MIDI della DAW imposta la sua control surface su Nessuna e spegni le sue porte (Push: spegni Track e Remote sulla sua User Port)", "El Launchpad lo usa otro programa: en los ajustes MIDI de tu DAW pon su superficie de control en Ninguna y apaga sus puertos (Push: apaga Track y Remote en su User Port)", "Le Launchpad est utilisé par un autre programme : dans les réglages MIDI de votre DAW, réglez sa surface de contrôle sur Aucune et désactivez ses ports (Push : désactivez Track et Remote sur son User Port)", "Das Launchpad wird von einem anderen Programm benutzt: stelle in den MIDI-Einstellungen deiner DAW seine Control Surface auf Keine und schalte seine Ports aus (Push: Track und Remote am User Port ausschalten)", "O Launchpad está em uso por outro programa: nas configurações MIDI da sua DAW ponha a superfície de controle em Nenhuma e desligue as portas (Push: desligue Track e Remote na User Port)" },
        { "Used by another Fase Groove Piano in this project", "Usato da un altro Fase Groove Piano in questo progetto", "Lo usa otro Fase Groove Piano en este proyecto", "Utilisé par un autre Fase Groove Piano dans ce projet", "Wird von einem anderen Fase Groove Piano in diesem Projekt benutzt", "Usado por outro Fase Groove Piano neste projeto" },
        { "TOTAL", "TUTTO", "TODO", "TOUT", "ALLES", "TUDO" },
        { "Total random: style, key, progression, groove, harmony, bass, sound and feel all at once (undo with the arrow, RESET to start over)", "Random totale: stile, tonalità, progressione, groove, armonia, basso, suono e feel tutti insieme (annulla con la freccia, RESET per ripartire da zero)", "Aleatorio total: estilo, tonalidad, progresión, groove, armonía, bajo, sonido y feel a la vez (deshaz con la flecha, RESET para empezar de cero)", "Aléatoire total : style, tonalité, progression, groove, harmonie, basse, son et feel d'un coup (annulez avec la flèche, RESET pour repartir de zéro)", "Totaler Zufall: Stil, Tonart, Progression, Groove, Harmonie, Bass, Sound und Feel auf einmal (Rückgängig mit dem Pfeil, RESET für den Neuanfang)", "Aleatório total: estilo, tom, progressão, groove, harmonia, baixo, som e feel de uma vez (desfaça com a seta, RESET para recomeçar)" },
        { "Piano off  -  click PIANO to turn it back on", "Piano spento  -  clicca PIANO per riaccenderlo", "Piano apagado  -  pulsa PIANO para activarlo", "Piano coupé  -  cliquez sur PIANO pour le rallumer", "Piano aus  -  PIANO klicken zum Einschalten", "Piano desligado  -  clique em PIANO para ligar" },
        { "ADVANCED", "AVANZATE", "AVANZADO", "AVANCÉ", "ERWEITERT", "AVANÇADO" },
        { "SIMPLE", "SEMPLICE", "SIMPLE", "SIMPLE", "EINFACH", "SIMPLES" },
        { "Show every control of the panels", "Mostra tutti i comandi dei pannelli", "Muestra todos los controles de los paneles", "Affiche toutes les commandes des panneaux", "Alle Regler der Bereiche zeigen", "Mostra todos os controles dos painéis" },
        { "Show only the main controls", "Mostra solo i comandi principali", "Muestra solo los controles principales", "N'affiche que les commandes principales", "Nur die wichtigsten Regler zeigen", "Mostra só os controles principais" },
        { "BROWSE...", "SFOGLIA...", "EXPLORAR...", "PARCOURIR...", "DURCHSUCHEN...", "EXPLORAR..." },
        { "Evolve over 8 bars", "Evoluzione in 8 battute", "Evolución en 8 compases", "Évolution sur 8 mesures", "Entwicklung über 8 Takte", "Evolução em 8 compassos" },
        { "Evolve over 16 bars", "Evoluzione in 16 battute", "Evolución en 16 compases", "Évolution sur 16 mesures", "Entwicklung über 16 Takte", "Evolução em 16 compassos" },
        { "Chord range", "Range accordi", "Rango de acordes", "Tessiture des accords", "Akkordumfang", "Extensão dos acordes" },
        { "all", "tutto", "todo", "tout", "alles", "tudo" },
        { "The chords' range: no chord note goes below the left thumb or above the right one (notes outside move by an octave)", "Il range degli accordi: nessuna nota degli accordi va sotto la maniglia sinistra o sopra quella destra (le note fuori si spostano di un'ottava)", "El rango de los acordes: ninguna nota baja de la manija izquierda ni sube de la derecha (las notas fuera se mueven una octava)", "La tessiture des accords : aucune note sous la poignée gauche ni au-dessus de la droite (les notes hors limites bougent d'une octave)", "Der Akkordumfang: keine Akkordnote unter dem linken oder über dem rechten Regler (Noten außerhalb rücken um eine Oktave)", "A extensão dos acordes: nenhuma nota abaixo da alça esquerda nem acima da direita (as notas fora se movem uma oitava)" },
        { "Found in this MIDI file: chords and a rhythm. What do you want to take?", "In questo file MIDI ci sono accordi e un ritmo. Cosa vuoi prendere?", "En este archivo MIDI hay acordes y un ritmo. ¿Qué quieres tomar?", "Ce fichier MIDI contient des accords et un rythme. Que voulez-vous prendre ?", "In dieser MIDI-Datei sind Akkorde und ein Rhythmus. Was möchtest du übernehmen?", "Neste arquivo MIDI há acordes e um ritmo. O que você quer pegar?" },
        { "Chords + rhythm", "Accordi + ritmo", "Acordes + ritmo", "Accords + rythme", "Akkorde + Rhythmus", "Acordes + ritmo" },
        { "Chords only", "Solo accordi", "Solo acordes", "Accords seulement", "Nur Akkorde", "Só acordes" },
        { "Rhythm only", "Solo ritmo", "Solo ritmo", "Rythme seulement", "Nur Rhythmus", "Só ritmo" },
        { "The rhythm of the MIDI file is now your groove (User pattern): edit it in the grid. The chords stay yours.", "Il ritmo del file MIDI ora è il tuo groove (Pattern utente): modificalo nella griglia. Gli accordi restano i tuoi.", "El ritmo del archivo MIDI ahora es tu groove (Patrón de usuario): edítalo en la cuadrícula. Los acordes siguen siendo los tuyos.", "Le rythme du fichier MIDI est maintenant votre groove (Motif utilisateur) : modifiez-le dans la grille. Les accords restent les vôtres.", "Der Rhythmus der MIDI-Datei ist jetzt dein Groove (User-Pattern): bearbeite ihn im Raster. Die Akkorde bleiben deine.", "O ritmo do arquivo MIDI agora é o seu groove (Padrão do usuário): edite-o na grade. Os acordes continuam os seus." },
        { "Rhythm: taken (User pattern)", "Ritmo: preso (Pattern utente)", "Ritmo: tomado (Patrón de usuario)", "Rythme : pris (Motif utilisateur)", "Rhythmus: übernommen (User-Pattern)", "Ritmo: pego (Padrão do usuário)" },
        { "Straighter", "Più dritto", "Más recto", "Plus droit", "Gerader", "Mais reto" },
        { "Live keys: the lowest octave of your keyboard controls the groove (MIDI notes 24-35)", "Tasti live: l'ottava più bassa della tastiera comanda il groove (note MIDI 24-35)", "Teclas en vivo: la octava más grave del teclado controla el groove (notas MIDI 24-35)", "Touches live : l'octave la plus grave du clavier pilote le groove (notes MIDI 24-35)", "Live-Tasten: die tiefste Oktave deiner Tastatur steuert den Groove (MIDI-Noten 24-35)", "Teclas ao vivo: a oitava mais grave do teclado controla o groove (notas MIDI 24-35)" },
        { "White keys: C intro, D main, E break, F build-up, G drop, A outro. Black keys: C# fill in the next bar, D# new riff, F# hear piano, G# hear bass, A# sequencer on/off. B: play / pause.", "Tasti bianchi: Do intro, Re main, Mi break, Fa build-up, Sol drop, La outro. Tasti neri: Do# fill nella battuta dopo, Re# nuovo riff, Fa# ascolta piano, Sol# ascolta basso, La# sequencer on/off. Si: play / pausa.", "Teclas blancas: Do intro, Re main, Mi break, Fa build-up, Sol drop, La outro. Teclas negras: Do# fill en el compás siguiente, Re# nuevo riff, Fa# oír piano, Sol# oír bajo, La# secuenciador on/off. Si: play / pausa.", "Touches blanches : Do intro, Ré main, Mi break, Fa build-up, Sol drop, La outro. Touches noires : Do# fill à la mesure suivante, Ré# nouveau riff, Fa# écouter piano, Sol# écouter basse, La# séquenceur on/off. Si : lecture / pause.", "Weiße Tasten: C Intro, D Main, E Break, F Build-up, G Drop, A Outro. Schwarze Tasten: C# Fill im nächsten Takt, D# neuer Riff, F# Piano hören, G# Bass hören, A# Sequencer an/aus. H: Play / Pause.", "Teclas brancas: Dó intro, Ré main, Mi break, Fá build-up, Sol drop, Lá outro. Teclas pretas: Dó# fill no compasso seguinte, Ré# novo riff, Fá# ouvir piano, Sol# ouvir baixo, Lá# sequenciador on/off. Si: play / pausa." },
        { "Drag only the section picked on the timeline above (click a section to pick it)", "Trascina solo la sezione scelta sulla linea del tempo in alto (clicca una sezione per sceglierla)", "Arrastra solo la sección elegida en la línea de tiempo de arriba (haz clic en una sección para elegirla)", "Glissez seulement la section choisie sur la ligne du temps en haut (cliquez sur une section pour la choisir)", "Nur den oben auf der Zeitleiste gewählten Abschnitt ziehen (klicke einen Abschnitt, um ihn zu wählen)", "Arraste só a seção escolhida na linha do tempo acima (clique numa seção para escolhê-la)" },
        { "No section", "Nessuna sezione", "Ninguna sección", "Aucune section", "Kein Abschnitt", "Nenhuma seção" },
        { "Bars of each section (- leaves it out), its own variation of the groove and the fill that leads out of it. Drag the whole song, or click a section on the timeline and drag just that one.", "Battute di ogni sezione (- la toglie), la sua variazione del groove e il fill che la chiude. Trascina il brano intero, oppure clicca una sezione sulla linea del tempo e trascina solo quella.", "Compases de cada sección (- la quita), su variación del groove y el fill que la cierra. Arrastra la canción entera, o haz clic en una sección de la línea de tiempo y arrastra solo esa.", "Mesures de chaque section (- l'enlève), sa variation du groove et le fill qui la termine. Glissez le morceau entier, ou cliquez une section sur la ligne du temps et glissez seulement celle-là.", "Takte jedes Abschnitts (- lässt ihn weg), seine Groove-Variation und der Fill, der ihn abschließt. Ziehe den ganzen Song, oder klicke einen Abschnitt auf der Zeitleiste und ziehe nur diesen.", "Compassos de cada seção (- a remove), sua variação do groove e o fill que a fecha. Arraste a música inteira, ou clique numa seção da linha do tempo e arraste só essa." },
        { "(MIDI clock)", "(MIDI clock)", "(MIDI clock)", "(MIDI clock)", "(MIDI-Clock)", "(MIDI clock)" },
        { "No DAW: send it MIDI clock", "Niente DAW: manda il MIDI clock", "Sin DAW: envía MIDI clock", "Pas de DAW : envoyez le MIDI clock", "Keine DAW: MIDI-Clock senden", "Sem DAW: envie MIDI clock" },
        { "The standalone app can't read your DAW's tempo. In your DAW send MIDI clock to the port the app listens to (enable it in the app's Options > Audio/MIDI settings), or turn SYNC DAW off and set the BPM.", "L'app standalone non può leggere il tempo della DAW. Nella DAW manda il MIDI clock alla porta che l'app ascolta (attivala nelle Opzioni > Impostazioni audio/MIDI dell'app), oppure spegni SYNC DAW e imposta i BPM.", "La app standalone no puede leer el tempo de tu DAW. En la DAW envía MIDI clock al puerto que escucha la app (actívalo en Opciones > Ajustes de audio/MIDI de la app), o apaga SYNC DAW y fija los BPM.", "L'app autonome ne peut pas lire le tempo de votre DAW. Dans la DAW, envoyez le MIDI clock au port écouté par l'app (activez-le dans Options > Réglages audio/MIDI de l'app), ou coupez SYNC DAW et réglez le BPM.", "Die Standalone-App kann das Tempo deiner DAW nicht lesen. Sende in der DAW MIDI-Clock an den Port, den die App hört (in den Optionen > Audio/MIDI-Einstellungen der App aktivieren), oder schalte SYNC DAW aus und stelle das BPM ein.", "O app standalone não lê o tempo da sua DAW. Na DAW envie MIDI clock para a porta que o app escuta (ative em Opções > Configurações de áudio/MIDI do app), ou desligue SYNC DAW e defina o BPM." },
        { "DRAG THIS SECTION", "TRASCINA LA SEZIONE", "ARRASTRA LA SECCIÓN", "GLISSER LA SECTION", "ABSCHNITT ZIEHEN", "ARRASTE A SEÇÃO" },
        { "DRAG THE WHOLE SONG", "TRASCINA IL BRANO", "ARRASTRA LA CANCIÓN", "GLISSER LE MORCEAU", "GANZEN SONG ZIEHEN", "ARRASTE A MÚSICA" },
        { "Everything", "Tutto", "Todo", "Tout", "Alles", "Tudo" },
        { "Piano only", "Solo piano", "Solo piano", "Piano seul", "Nur Piano", "Só piano" },
        { "TRACKS", "TRACCE", "PISTAS", "PISTES", "SPUREN", "FAIXAS" },
        { "Instrument", "Strumento", "Instrumento", "Instrument", "Instrument", "Instrumento" },
        { "Edit", "Modifica", "Editar", "Éditer", "Bearbeiten", "Editar" },
        { "Which notes go into the clip: everything, piano only, bass only, strings only or separate tracks", "Quali note vanno nella clip: tutto, solo piano, solo basso, solo archi o tracce separate", "Qué notas van en el clip: todo, solo piano, solo bajo, solo cuerdas o pistas separadas", "Quelles notes vont dans le clip : tout, piano seul, basse seule, cordes seules ou pistes séparées", "Welche Noten in den Clip kommen: alles, nur Piano, nur Bass, nur Streicher oder getrennte Spuren", "Quais notas vão no clipe: tudo, só piano, só baixo, só cordas ou faixas separadas" },
        { "Edit the piano (yellow): only its notes can be touched, the others are see-through", "Modifica il piano (giallo): si toccano solo le sue note, le altre sono trasparenti", "Edita el piano (amarillo): solo se tocan sus notas, las demás son transparentes", "Éditer le piano (jaune) : seules ses notes se touchent, les autres sont transparentes", "Piano bearbeiten (gelb): nur seine Noten sind bearbeitbar, die anderen durchsichtig", "Editar o piano (amarelo): só as notas dele se tocam, as outras ficam transparentes" },
        { "Edit the bass (blue): only its notes can be touched, the others are see-through", "Modifica il basso (blu): si toccano solo le sue note, le altre sono trasparenti", "Edita el bajo (azul): solo se tocan sus notas, las demás son transparentes", "Éditer la basse (bleu) : seules ses notes se touchent, les autres sont transparentes", "Bass bearbeiten (blau): nur seine Noten sind bearbeitbar, die anderen durchsichtig", "Editar o baixo (azul): só as notas dele se tocam, as outras ficam transparentes" },
        { "Edit the strings (red): only their notes can be touched, the others are see-through", "Modifica gli archi (rosso): si toccano solo le loro note, le altre sono trasparenti", "Edita las cuerdas (rojo): solo se tocan sus notas, las demás son transparentes", "Éditer les cordes (rouge) : seules leurs notes se touchent, les autres sont transparentes", "Streicher bearbeiten (rot): nur ihre Noten sind bearbeitbar, die anderen durchsichtig", "Editar as cordas (vermelho): só as notas delas se tocam, as outras ficam transparentes" },
        { "The piano's own controls: voicing, register, strum, top line, movement, range", "I controlli del solo piano: voicing, registro, arpeggiato, linea in alto, movimento, range", "Los controles del piano: voicing, registro, arpegio, línea superior, movimiento, rango", "Les réglages du piano : voicing, registre, arpège, ligne du haut, mouvement, plage", "Die Regler des Pianos: Voicing, Lage, Arpeggio, Oberstimme, Bewegung, Umfang", "Os controles do piano: voicing, registro, arpejo, linha superior, movimento, extensão" },
        { "The bass: what the left hand plays under the chords", "Il basso: cosa suona la mano sinistra sotto gli accordi", "El bajo: lo que toca la mano izquierda bajo los acordes", "La basse : ce que joue la main gauche sous les accords", "Der Bass: was die linke Hand unter den Akkorden spielt", "O baixo: o que a mão esquerda toca sob os acordes" },
        { "The strings: a pad or stabs that follow the chords, on MIDI channel 3", "Gli archi: un pad o degli stab che seguono gli accordi, sul canale MIDI 3", "Las cuerdas: un pad o stabs que siguen los acordes, en el canal MIDI 3", "Les cordes : une nappe ou des stabs qui suivent les accords, sur le canal MIDI 3", "Die Streicher: ein Pad oder Stabs, die den Akkorden folgen, auf MIDI-Kanal 3", "As cordas: um pad ou stabs que seguem os acordes, no canal MIDI 3" },
        { "What the left hand plays under the chords. MIDI channel 1, or channel 2 with MIDI out set to Bass on channel 2.", "Cosa suona la mano sinistra sotto gli accordi. Canale MIDI 1, oppure canale 2 con Uscita MIDI su Basso sul canale 2.", "Lo que toca la mano izquierda bajo los acordes. Canal MIDI 1, o canal 2 con Salida MIDI en Bajo en el canal 2.", "Ce que joue la main gauche sous les accords. Canal MIDI 1, ou canal 2 avec Sortie MIDI sur Basse sur le canal 2.", "Was die linke Hand unter den Akkorden spielt. MIDI-Kanal 1, oder Kanal 2 mit MIDI-Ausgang auf Bass auf Kanal 2.", "O que a mão esquerda toca sob os acordes. Canal MIDI 1, ou canal 2 com Saída MIDI em Baixo no canal 2." },
        { "Strings that follow the chords, for body and volume: a pad held on every chord, or stabs with the piano. Always on MIDI channel 3.", "Archi che seguono gli accordi, per dare corpo e volume: un pad tenuto su ogni accordo, o stab insieme al piano. Sempre sul canale MIDI 3.", "Cuerdas que siguen los acordes, para dar cuerpo y volumen: un pad sostenido en cada acorde, o stabs con el piano. Siempre en el canal MIDI 3.", "Des cordes qui suivent les accords, pour le corps et le volume : une nappe tenue sur chaque accord, ou des stabs avec le piano. Toujours sur le canal MIDI 3.", "Streicher, die den Akkorden folgen, für Fülle und Volumen: ein Pad auf jedem Akkord oder Stabs mit dem Piano. Immer auf MIDI-Kanal 3.", "Cordas que seguem os acordes, para dar corpo e volume: um pad sustentado em cada acorde, ou stabs com o piano. Sempre no canal MIDI 3." },
        { "Strings", "Archi", "Cuerdas", "Cordes", "Streicher", "Cordas" },
        { "STRINGS", "ARCHI", "CUERDAS", "CORDES", "STREICHER", "CORDAS" },
        { "Pad", "Pad", "Pad", "Nappe", "Pad", "Pad" },
        { "High pad", "Pad alto", "Pad agudo", "Nappe aiguë", "Hohes Pad", "Pad agudo" },
        { "Wide pad", "Pad largo", "Pad amplio", "Nappe large", "Breites Pad", "Pad amplo" },
        { "Stabs with the piano", "Stab col piano", "Stabs con el piano", "Stabs avec le piano", "Stabs mit dem Piano", "Stabs com o piano" },
        { "Strings only", "Solo archi", "Solo cuerdas", "Cordes seules", "Nur Streicher", "Só cordas" },
        { "Strings on (lit) or off: they hold the chords for body, on MIDI channel 3", "Archi accesi (illuminato) o spenti: tengono gli accordi per dare corpo, sul canale MIDI 3", "Cuerdas encendidas (iluminado) o apagadas: sostienen los acordes para dar cuerpo, en el canal MIDI 3", "Cordes allumées (éclairé) ou éteintes : elles tiennent les accords pour le corps, sur le canal MIDI 3", "Streicher an (leuchtet) oder aus: sie halten die Akkorde für mehr Fülle, auf MIDI-Kanal 3", "Cordas ligadas (aceso) ou desligadas: sustentam os acordes para dar corpo, no canal MIDI 3" },
        { "The strings: a pad that holds each chord, higher, wider, or stabs with the piano. MIDI channel 3", "Gli archi: un pad che tiene ogni accordo, più alto, più largo, o stab insieme al piano. Canale MIDI 3", "Las cuerdas: un pad que sostiene cada acorde, más agudo, más amplio, o stabs con el piano. Canal MIDI 3", "Les cordes : une nappe qui tient chaque accord, plus aiguë, plus large, ou des stabs avec le piano. Canal MIDI 3", "Die Streicher: ein Pad, das jeden Akkord hält, höher, breiter oder Stabs mit dem Piano. MIDI-Kanal 3", "As cordas: um pad que sustenta cada acorde, mais agudo, mais amplo, ou stabs com o piano. Canal MIDI 3" },
        { "Which notes go into the clip: everything, chords only, bass only, separate tracks or strings only", "Quali note vanno nella clip: tutto, solo accordi, solo basso, tracce separate o solo archi", "Qué notas van en el clip: todo, solo acordes, solo bajo, pistas separadas o solo cuerdas", "Quelles notes vont dans le clip : tout, accords seuls, basse seule, pistes séparées ou cordes seules", "Welche Noten in den Clip kommen: alles, nur Akkorde, nur Bass, getrennte Spuren oder nur Streicher", "Quais notas vão no clipe: tudo, só acordes, só baixo, faixas separadas ou só cordas" },
        { "VELOCITY", "VELOCITY", "VELOCIDAD", "VÉLOCITÉ", "ANSCHLAG", "VELOCIDADE" },
        { "The velocity of every hit, as faders: drag to set it, double-click for automatic", "La velocity di ogni colpo, come fader: trascina per regolarla, doppio clic per tornare automatica", "La velocidad de cada golpe, como faders: arrastra para ajustarla, doble clic para automática", "La vélocité de chaque coup, en faders : glissez pour la régler, double-clic pour l'automatique", "Die Anschlagstärke jedes Schlags als Fader: ziehen zum Einstellen, Doppelklick für automatisch", "A velocidade de cada toque, como faders: arraste para ajustar, duplo clique para automática" },
        { "Drag a fader up or down (or across several) to set the velocity. Double-click: automatic again. Right-click: reset", "Trascina un fader su o giù (anche su più fader) per regolare la velocity. Doppio clic: di nuovo automatica. Clic destro: azzera", "Arrastra un fader arriba o abajo (o por varios) para ajustar la velocidad. Doble clic: automática otra vez. Clic derecho: restablecer", "Glissez un fader vers le haut ou le bas (ou sur plusieurs) pour régler la vélocité. Double-clic : automatique. Clic droit : réinitialiser", "Fader nach oben oder unten ziehen (auch über mehrere), um die Anschlagstärke einzustellen. Doppelklick: wieder automatisch. Rechtsklick: zurücksetzen", "Arraste um fader para cima ou para baixo (ou por vários) para ajustar a velocidade. Duplo clique: automática de novo. Clique direito: redefinir" },
        { "A bass line from the library plays its own levels: write the bass in the grid to set them here", "Una linea di basso della libreria ha i suoi livelli: scrivi il basso nella griglia per regolarli qui", "Una línea de bajo de la biblioteca tiene sus propios niveles: escribe el bajo en la cuadrícula para ajustarlos aquí", "Une ligne de basse de la bibliothèque a ses propres niveaux : écrivez la basse dans la grille pour les régler ici", "Eine Basslinie aus der Bibliothek hat eigene Pegel: schreibe den Bass im Raster, um sie hier einzustellen", "Uma linha de baixo da biblioteca tem seus próprios níveis: escreva o baixo na grade para ajustá-los aqui" },
        { "Bass: all automatic", "Basso: tutto automatico", "Bajo: todo automático", "Basse : tout automatique", "Bass: alles automatisch", "Baixo: tudo automático" },
        { "Chords: all automatic", "Accordi: tutto automatico", "Acordes: todo automático", "Accords : tout automatique", "Akkorde: alles automatisch", "Acordes: tudo automático" },
        { "All automatic", "Tutto automatico", "Todo automático", "Tout automatique", "Alles automatisch", "Tudo automático" },
        { "Release", "Rilascio", "Liberación", "Relâchement", "Release", "Liberação" },
        { "Sidechain", "Sidechain", "Sidechain", "Sidechain", "Sidechain", "Sidechain" },
        { "Rate", "Ritmo", "Ritmo", "Cadence", "Rate", "Ritmo" },
        { "Tempo (internal)", "Tempo (interno)", "Tempo (interno)", "Tempo (interne)", "Tempo (intern)", "Tempo (interno)" },
        { "Sidechain input (external)", "Ingresso sidechain (esterno)", "Entrada sidechain (externa)", "Entrée sidechain (externe)", "Sidechain-Eingang (extern)", "Entrada sidechain (externa)" },
        { "Sidechain depth: ducks the guide sound like a kick sidechain", "Profondità del sidechain: abbassa il suono guida come un sidechain sulla cassa", "Profundidad del sidechain: baja el sonido guía como un sidechain con el bombo", "Profondeur du sidechain : baisse le son guide comme un sidechain sur le kick", "Sidechain-Tiefe: duckt den Guide-Sound wie ein Kick-Sidechain", "Profundidade do sidechain: abaixa o som guia como um sidechain no bumbo" },
        { "How fast the sound comes back after each duck", "Quanto velocemente il suono torna su dopo ogni abbassamento", "Lo rápido que vuelve el sonido tras cada bajada", "La vitesse à laquelle le son revient après chaque baisse", "Wie schnell der Sound nach jedem Ducking zurückkommt", "Quão rápido o som volta depois de cada abaixamento" },
        { "Tempo: the duck follows the beat. Sidechain input: it follows the track you send to the plug-in's sidechain input (e.g. the kick)", "Tempo: l'abbassamento segue il battito. Ingresso sidechain: segue la traccia che mandi all'ingresso sidechain del plug-in (es. la cassa)", "Tempo: la bajada sigue el pulso. Entrada sidechain: sigue la pista que envías a la entrada sidechain del plug-in (p. ej. el bombo)", "Tempo : la baisse suit le temps. Entrée sidechain : elle suit la piste envoyée à l'entrée sidechain du plug-in (ex. le kick)", "Tempo: das Ducking folgt dem Beat. Sidechain-Eingang: es folgt der Spur, die du an den Sidechain-Eingang des Plug-ins schickst (z. B. die Kick)", "Tempo: o abaixamento segue a batida. Entrada sidechain: segue a faixa que você envia à entrada sidechain do plug-in (ex. o bumbo)" },
        { "How often the tempo sidechain ducks: every beat, 8th, half bar or 16th", "Ogni quanto si abbassa il sidechain a tempo: ogni battito, ottavo, mezza battuta o sedicesimo", "Cada cuánto baja el sidechain a tempo: cada pulso, corchea, media barra o semicorchea", "La fréquence du sidechain au tempo : chaque temps, croche, demi-mesure ou double croche", "Wie oft der Tempo-Sidechain duckt: jeder Schlag, Achtel, halber Takt oder Sechzehntel", "Com que frequência o sidechain a tempo abaixa: cada batida, colcheia, meio compasso ou semicolcheia" },
    };
    // clang-format on

    std::atomic<int> currentLanguage { 0 };

    const std::unordered_map<std::string, const Entry*>& index()
    {
        static const auto map = []
        {
            std::unordered_map<std::string, const Entry*> m;
            for (const auto& e : table)
                m[e.en] = &e;
            return m;
        }();
        return map;
    }

    std::unique_ptr<juce::PropertiesFile> openSettings()
    {
        juce::PropertiesFile::Options o;
        o.applicationName = "Fase Groove Piano";
        o.folderName = "Fase Groove Piano";
        o.filenameSuffix = ".settings";
        o.osxLibrarySubFolder = "Application Support";
        return std::make_unique<juce::PropertiesFile> (o);
    }
}

const char* languageName (int language) noexcept
{
    static const char* const names[numLanguages] { "English", "Italiano", "Español", "Français", "Deutsch", "Português" };
    return names[juce::jlimit (0, numLanguages - 1, language)];
}

Language current() noexcept
{
    return (Language) currentLanguage.load();
}

namespace
{
    /** JUCE's own texts (the Standalone window's Options menu and audio settings use TRANS). */
    void applyToJuce()
    {
        static const char* const juceTexts[] {
            "Audio/MIDI Settings...", "Audio/MIDI Settings", "Save current state...", "Load a saved state...",
            "Reset to default state", "Save current state", "Load a saved state", "Error whilst saving",
            "Error whilst loading", "Couldn't write to the specified file!", "Couldn't read from the specified file!",
            "Audio device type:", "Device:", "Output:", "Input:", "Sample rate:", "Audio buffer size:",
            "Active MIDI inputs:", "MIDI Output:", "Active output channels:", "Active input channels:", "Test",
            "Plays a test tone", "Control Panel", "Reset Device", "Show advanced settings...",
            "No MIDI inputs available", "none", "OK"
        };

        if (currentLanguage.load() == 0)
        {
            juce::LocalisedStrings::setCurrentMappings (nullptr);
            return;
        }

        juce::String text ("language: Fase Groove Piano\n");
        for (auto* key : juceTexts)
            text << "\"" << juce::String (key).replace ("\"", "\\\"") << "\" = \""
                 << tr (key).replace ("\"", "\\\"") << "\"\n";

        juce::LocalisedStrings::setCurrentMappings (new juce::LocalisedStrings (text, false));
    }
}

bool tipsEnabled()
{
    if (auto settings = openSettings())
        return settings->getBoolValue ("tips", true);
    return true;
}

void setTipsEnabled (bool enabled)
{
    if (auto settings = openSettings())
    {
        settings->setValue ("tips", enabled);
        settings->saveIfNeeded();
    }
}

juce::String setting (const char* key, const juce::String& fallback)
{
    if (auto settings = openSettings())
        return settings->getValue (key, fallback);
    return fallback;
}

void setSetting (const char* key, const juce::var& value)
{
    if (auto settings = openSettings())
    {
        settings->setValue (key, value);
        settings->saveIfNeeded();
    }
}

void setCurrent (Language language)
{
    currentLanguage = juce::jlimit (0, numLanguages - 1, (int) language);
    applyToJuce();

    if (auto settings = openSettings())
    {
        settings->setValue ("language", currentLanguage.load());
        settings->saveIfNeeded();
    }
}

void loadSaved()
{
    if (auto settings = openSettings(); settings != nullptr && settings->containsKey ("language"))
    {
        currentLanguage = juce::jlimit (0, numLanguages - 1, settings->getIntValue ("language"));
        applyToJuce();
        return;
    }

    // First run: follow the computer's language.
    const auto system = juce::SystemStats::getUserLanguage().toLowerCase();
    static const char* const codes[numLanguages] { "en", "it", "es", "fr", "de", "pt" };
    for (int i = 0; i < numLanguages; ++i)
        if (system.startsWith (codes[i]))
            currentLanguage = i;
    applyToJuce();
}

juce::String upper (const juce::String& text)
{
    juce::String out;
    for (auto c : text)
    {
        if (c >= 0xe0 && c <= 0xfe && c != 0xf7)
            c -= 0x20;   // Latin-1: à -> À, é -> É, ñ -> Ñ ...
        else
            c = juce::CharacterFunctions::toUpperCase (c);
        out += juce::String::charToString (c);
    }
    return out;
}

juce::String tr (const juce::String& english)
{
    const int lang = currentLanguage.load();
    if (lang == 0)
        return english;

    const auto& map = index();
    const auto it = map.find (english.toStdString());
    if (it == map.end())
        return english;

    const Entry& e = *it->second;
    const char* text = lang == 1 ? e.it : lang == 2 ? e.es : lang == 3 ? e.fr : lang == 4 ? e.de : e.pt;
    return juce::String::fromUTF8 (text);
}
}
