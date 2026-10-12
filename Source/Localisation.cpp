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

void setCurrent (Language language)
{
    currentLanguage = juce::jlimit (0, numLanguages - 1, (int) language);

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
        return;
    }

    // First run: follow the computer's language.
    const auto system = juce::SystemStats::getUserLanguage().toLowerCase();
    static const char* const codes[numLanguages] { "en", "it", "es", "fr", "de", "pt" };
    for (int i = 0; i < numLanguages; ++i)
        if (system.startsWith (codes[i]))
            currentLanguage = i;
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
