// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "engine/GrooveEngine.h"
#include "engine/HarmonyAnalyzer.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include <array>
#include <atomic>

namespace ParamIDs
{
    inline constexpr const char* key         = "key";
    inline constexpr const char* scale       = "scale";
    inline constexpr const char* extension   = "extension";
    inline constexpr const char* voicing     = "voicing";
    inline constexpr const char* registerPos = "register";
    inline constexpr const char* bass        = "bass";
    inline constexpr const char* pattern     = "pattern";
    inline constexpr const char* chordLength = "chordLength";
    inline constexpr const char* progLength  = "progLength";
    inline constexpr const char* swing       = "swing";
    inline constexpr const char* gate        = "gate";
    inline constexpr const char* velocity    = "velocity";
    inline constexpr const char* humanize    = "humanize";
    inline constexpr const char* fills       = "fills";
    inline constexpr const char* playMode    = "playMode";
    inline constexpr const char* bassChannel = "bassChannel";
    inline constexpr const char* section     = "section";
    inline constexpr const char* tempoSync   = "tempoSync";
    inline constexpr const char* tempo       = "tempo";
    inline constexpr const char* soundType   = "soundType";
    inline constexpr const char* fillStyle   = "fillStyle";
    inline constexpr const char* strum       = "strum";
    inline constexpr const char* octaveTop   = "octaveTop";
    inline constexpr const char* approach    = "approach";
    inline constexpr const char* topLine     = "topLine";
    inline constexpr const char* variation   = "variation";
    inline constexpr const char* grooveBars  = "grooveBars";
    inline constexpr const char* movement    = "movement";
    inline constexpr const char* strumDir    = "strumDir";
    inline constexpr const char* humanizeStyle = "humanizeStyle";
    inline constexpr const char* humanizeSeed  = "humanizeSeed";
    inline constexpr const char* chorus      = "chorus";
    inline constexpr const char* pump        = "pump";
    inline constexpr const char* pumpSource  = "pumpSource";    // 0 = tempo (internal), 1 = sidechain input
    inline constexpr const char* pumpRate    = "pumpRate";      // 1/4, 1/8, 1/2, 1/16
    inline constexpr const char* pumpRelease = "pumpRelease";
    inline constexpr const char* sequencer   = "sequencer";     // off: the groove doesn't play with the DAW
    inline constexpr const char* chordLow    = "chordLow";      // the chords' range (lowest / highest note)
    inline constexpr const char* chordHigh   = "chordHigh";
    inline constexpr const char* riffSeed    = "riffSeed";
    inline constexpr const char* riffDensity = "riffDensity";
    inline constexpr const char* sound       = "sound";
    inline constexpr const char* tone        = "tone";
    inline constexpr const char* reverb      = "reverb";
    inline constexpr const char* volume      = "volume";
    inline constexpr const char* strings     = "strings";       // the strings (StringsMode), MIDI channel 3
    inline constexpr const char* bassFeel    = "bassFeel";      // classic, sub (voice-led), sub + sidechain gaps
    inline constexpr const char* realism     = "realism";       // groove realism (timing, accents, phrase variation)
    inline constexpr const char* pedalCC     = "pedalCC";       // sustain pedal (CC64) and expression (CC11) out

    inline juce::String slot (int index)    { return "slot" + juce::String (index + 1); }
    inline juce::String segment (int index) { return "segment" + juce::String (index + 1); }
    inline juce::String inversion (int index) { return "inversion" + juce::String (index + 1); }
}

class PadController;

class FaseGroovePianoProcessor : public juce::AudioProcessor
{
public:
    FaseGroovePianoProcessor();
    ~FaseGroovePianoProcessor() override;

    /** A Launchpad as a hardware grid (see PadController); off unless switched on in OPTIONS. */
    std::unique_ptr<PadController> pads;

    /** HEAR PIANO / HEAR BASS: lit = you hear it; switching off the only part heard switches the other on. */
    void hearPart (bool bass);

    //==============================================================================
    // Live keys: the lowest octave of a MIDI keyboard (notes 24..35) as controls, when switched on
    // in OPTIONS (global, like the language). White keys pick the song section (C intro, D main,
    // E break, F build-up, G drop, A outro); C# plays a fill in the next bar, D# a new riff,
    // F# / G# hear piano / bass, A# the sequencer on / off, B play / pause.
    static constexpr int liveKeysLowest = 24;
    static bool liveKeysEnabled();
    static void setLiveKeysEnabled (bool);
    /** Runs a live key's command (message thread). */
    void runLiveKey (int key);

    juce::VST3ClientExtensions* getVST3ClientExtensions() override { return &vst3Extensions; }

    /** True while a MIDI clock drives the tempo (SYNC DAW in the standalone app). */
    std::atomic<bool> clockSync { false };

    /** Peak level arriving at the sidechain input (for the meter in the UI). */
    std::atomic<float> sidechainMeter { 0.0f };

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    /** Current parameter values as engine settings (safe from any thread). */
    sgk::GrooveSettings currentSettings() const;

    /** Sets a parameter from the UI, notifying the host. */
    void setParameterFromUI (const juce::String& id, float plainValue);

    /** Rolls a brand new random riff. Locked segments are kept: then the result
        becomes the (editable) user pattern. */
    void rollNewRiff();

    //==============================================================================
    // Rhythm grid editing (message thread). Steps are as displayed, i.e. after the
    // segment order; edits go into the user pattern, which then becomes the groove.
    char stepSymbol (bool bassRow, int displayedStep) const;
    void editStep (bool bassRow, int displayedStep, char symbol);

    /** Base segment (0..3 = A..D) shown at a displayed step. */
    int segmentAtStep (int displayedStep) const;

    /** New random content for one segment (A..D) of the groove. */
    void randomizeSegment (int segment);
    /** New random rhythm for one hand only (piano or bass); the other hand and locked parts stay. */
    void rerollHand (bool bass);
    bool isSegmentLocked (int segment) const          { return segmentLocked[(size_t) juce::jlimit (0, 3, segment)]; }
    void setSegmentLocked (int segment, bool locked)  { segmentLocked[(size_t) juce::jlimit (0, 3, segment)] = locked; }

    /** Velocity set by hand on a displayed step (1..127, 118 = the Velocity control), 0 = automatic. */
    int stepVelocity (bool bassRow, int displayedStep) const;
    /** Sets the level of a displayed step (0 = back to automatic); the groove becomes the user pattern. */
    void setStepVelocity (bool bassRow, int displayedStep, int level);
    /** All steps of one row (or both) back to the automatic level. */
    void resetStepVelocities (bool chords = true, bool bass = true);
    /** True when some step has a level set by hand (and the user groove plays). */
    bool hasStepVelocities() const;

    // The velocity page's own parts: which part's levels play in each half bar (A-D), locks,
    // and new random levels (only the levels change, never the rhythm).
    int velocityPartAt (int position) const   { return velocityOrder[(size_t) juce::jlimit (0, sgk::numSegments - 1, position)].load(); }
    void setVelocityPartAt (int position, int part);
    bool isVelocityLocked (int part) const    { return velocityLocked[(size_t) juce::jlimit (0, sgk::numSegments - 1, part)]; }
    void setVelocityLocked (int part, bool locked) { velocityLocked[(size_t) juce::jlimit (0, sgk::numSegments - 1, part)] = locked; }
    /** New random levels for the chords and/or the bass: one part (0..3) or every unlocked one (-1). */
    void randomizeVelocities (bool chords, bool bass, int part = -1);

    /** Quick bass on/off that remembers the last bass mode. */
    void toggleBass();
    bool isBassOn() const;

    /** The strings on / off, remembering the last strings mode (Pad the first time). */
    void toggleStrings();
    bool isStringsOn() const;

    /** The strings always play on their own MIDI channel. */
    static constexpr int stringsChannel = 3;

    // Editor preferences, saved with the project.
    float editorScale = 1.0f;
    bool shrinkWhileDragging = true;    // while dragging a clip the window becomes a thin strip, so the DAW is free
    bool advancedView = false;   // the panels show every control (ADVANCED) or only the main ones

    /** Replaces the progression with a random soulful-house one of the current length. */
    void rollNewProgression();

    /** Puts every setting back to its default (the interface language is not a setting
        of the plug-in, so it stays). */
    void resetToDefaults();

    /** Ready-made genre setups: progression, harmony, groove, feel and sound in one click
        (the key is kept). */
    static juce::StringArray styleNames();
    void applyStyle (int index);

    /** TOTAL RANDOM: a random style, then a random key, progression, groove and sometimes a
        different harmony, bass, sound and feel. Always inside choices that sound good; undoable. */
    void totalRandom();
    /** Loads progressionPresets()[index]: scale, length, degrees and any borrowed chords. */
    void applyProgressionPreset (int index);

    /** Reads a MIDI file (all tracks but drums) and estimates key, chords and chord length. */
    static sgk::HarmonyAnalysis analyzeMidiFile (const juce::File&);
    /** Sets key, scale, progression and chord length from an analysis. */
    void applyAnalysis (const sgk::HarmonyAnalysis&);
    /** A rhythm read from a MIDI file (HarmonyAnalysis::groove) becomes the user pattern. */
    void applyGroove (const sgk::GroovePattern&);
    /** Index in progressionPresets() that a style loads, or -1. */
    static int stylePreset (int index);

    //==============================================================================
    // LEARN: play chords on a MIDI keyboard and they fill the progression, slot by slot.
    std::atomic<bool> learnMode { false };
    /** SOLO BASS: the chords stay silent (MIDI out and guide sound); the exported clips are not affected. */
    std::atomic<bool> chordsMuted { false };
    /** HEAR BASS off: the bass is only muted (what you hear); its notes, the piano's voicing, the
        piano roll and the exports stay the same. */
    std::atomic<bool> bassMuted { false };
    bool isBassHeard() const { return isBassOn() && ! bassMuted.load(); }

    // Notes edited one by one in the big piano roll. While set, they play (and are exported)
    // instead of the generated notes in the main groove and the drop.
    // Change anything (chords, key, voicing, groove...) and the edits are carried over to the
    // newly generated notes (refreshEditedLoop, a few times a second).
    using EditedLoop = std::shared_ptr<const std::vector<sgk::LoopNote>>;
    /** The edited loop as it plays now (the edits carried over to the current settings). */
    EditedLoop editedLoop() const              { return std::atomic_load (&playingNotes); }
    double editedLoopLength() const            { return playingLength.load(); }
    bool hasEditedLoop() const                 { auto l = editedLoop(); return l != nullptr && ! l->empty(); }
    /** The notes as edited now: `origin`s point into generatedLoop (lengthInBeats) of the current settings. */
    void setEditedLoop (std::vector<sgk::LoopNote> notes, double lengthInBeats);
    void clearEditedLoop();
    /** Carries the edits over to the notes generated by the current settings, if they changed. */
    void refreshEditedLoop();
    /** The loop length the piano roll shows: long enough for groove and progression to come round (8 bars at most). */
    double naturalLoopBeats() const;
    /** The loop as it plays now (generated), ready to be edited: velocities relative to the Velocity control. */
    std::vector<sgk::LoopNote> generatedLoop (double lengthInBeats) const;
    std::atomic<int> learnedCount { 0 };       // bumps every time a chord is heard
    std::atomic<int> learnedMask { 0 };        // pitch classes of the last chord (bit per pc)
    std::atomic<int> learnedBass { 0 };        // its lowest note's pitch class
    int learnSlot = 0;                         // next slot to fill (message thread)
    void startLearning()                       { learnSlot = 0; learnMode = true; }

    /** Recognises a chord from its pitch classes: root and whether it is minor. */
    static bool recogniseChord (int pitchClassMask, int lowestPc, int& rootPc, bool& minor);

    /** Puts the chord into the next slot: as a scale degree when it belongs to the key,
        otherwise as a free chord. Returns the slot used, or -1. */
    int learnChord (int pitchClassMask, int lowestPc);

    //==============================================================================
    // User presets: files in the user's application data folder.
    /** Documents/Fase Groove Piano: presets, saved grooves and MIDI clips, easy to find and back up. */
    static juce::File userFolder();
    static juce::File presetFolder();
    static juce::File grooveFolder();
    static juce::File clipFolder();
    static juce::Array<juce::File> grooveFiles();
    /** Saves the rhythm grid (both hands, in the A B C D order you hear) as a .fgg file. */
    bool saveGroove (const juce::String& name);
    bool loadGroove (const juce::File&);
    /** The groove saved or loaded last (its name): SAVE offers it again, saved with the project. */
    juce::String lastGrooveName;

    // LINK: genre and swing shared with Fase Groove Drums (and other Fase Groove instances).
    bool isLinked() const { return linkOn.load(); }
    void setLinked (bool on);
    static juce::File linkFile();
    /** A linked Fase Groove Guitar set to LEAD: the piano only fills in a chord now and then
        (a few answers in four bars, placed by compingSeed()), the bass as usual. */
    bool isGuitarLeading() const noexcept { return guitarLeads.load(); }
    /** KEY (with LINK on): the same key and scale in every linked plug-in that has KEY on. */
    void setKeyLinked (bool on);
    bool isKeyLinked() const noexcept { return keyLinked.load(); }
    static bool compingKeeps (double ppq, std::uint32_t seed) noexcept;
    /** One window of the four-bar comping cycle: where (beats in the cycle) and what plays there,
        the piano's own groove (figure -1) or an answer of its own (a figure). */
    struct CompingWindow { double start = 0.0; int figure = -1; };
    static int compingWindows (std::uint32_t seed, std::array<CompingWindow, 8>& out) noexcept;
    struct CompingHit { double offset, length; float level; };
    struct CompingFigure { int count; std::array<CompingHit, 3> hits; };
    static constexpr int numCompingFigures = 8;
    static const CompingFigure& compingFigure (int figure) noexcept;
    /** The answers' note-ons and note-offs that fall in [from, to), on the chords of the moment. */
    static void compingAnswers (double from, double to, std::uint32_t seed, const sgk::GrooveSettings&,
                                std::vector<sgk::NoteEvent>& out);
    /** The same on a progression already voiced (the audio thread voices it only when it changes). */
    static void compingAnswers (double from, double to, std::uint32_t seed, const sgk::GrooveSettings&,
                                const std::array<sgk::VoicedChord, sgk::maxProgressionLength>& voiced,
                                std::vector<sgk::NoteEvent>& out);
    /** One hit of the answers in the four-bar cycle (beats), ended before the next one starts. */
    struct CompingSpan { double on = 0.0, off = 0.0; float level = 1.0f; int id = 0; };
    static int compingHits (std::uint32_t seed, std::array<CompingSpan, 24>& out) noexcept;
    /** The comping's seed: the groove and its riff seed (NEW RHYTHM gives other answers). */
    std::uint32_t compingSeed() const;
    /** The events as the piano plays them under a leading guitar: the chord and fill notes
        outside compingKeeps() left out (each with its note-off), the bass and strings as they are. */
    static std::vector<sgk::NoteEvent> comping (const std::vector<sgk::NoteEvent>& events, std::uint32_t seed,
                                               const sgk::GrooveSettings&, double length);
    /** The groove as the grid shows it then: four bars, the right hand only on the comping beats. */
    static sgk::GroovePattern compingPattern (const sgk::GroovePattern&, std::uint32_t seed);
    /** Writes the genre and swing for the others, or takes theirs (message thread, a few times a second). */
    void updateLink();
    /** The genre shared with LINK: a preset's own, else the last preset's. */
    int currentGenre() const;
    static juce::Array<juce::File> presetFiles();
    bool savePreset (const juce::String& name);
    bool loadPreset (const juce::File&);

    // Undo / redo: snapshots of everything but the window size.
    juce::String snapshot();
    void restoreSnapshot (const juce::String&);

    // Free chords (any chord, even outside the scale), set from the circle of fifths.
    void setFreeChord (int slot, int code);          // code: sgk::noFreeChord or sgk::freeChordCode()
    int freeChord (int slot) const                   { return freeChords[(size_t) juce::jlimit (0, sgk::maxProgressionLength - 1, slot)].load(); }
    void clearFreeChords();
    bool freeCircle = false;                         // circle of fifths accepts any chord (saved with the project)

    //==============================================================================
    enum class ClipPart { all = 0, chords, bass, tracks, strings };   // tracks: one MIDI file, piano / bass / fills / strings on separate tracks
    static juce::String instrumentName (ClipPart);   // "Piano", "Bass", "Strings": the first word of a clip's name

    /** Pass as `bars` to export the whole song: intro, main, break, build-up, drop, outro. */
    static constexpr int songArrangement = -1;

    /** Renders the current progression + groove as a standard MIDI file.
        bars == 0 means "one full loop" (progression, groove and fills all repeat). */
    juce::MidiFile createMidiClip (int bars, ClipPart part) const;
    /** Writes a .mid whole or not at all: a temporary file first, the one already there replaced
        only once the new one is complete (a full disk or no permission keeps the old clip). */
    static bool writeMidiFileSafely (const juce::MidiFile&, const juce::File& target);

    // Song arrangement: Intro, Main, Break, Build-up, Drop, Outro (the order of sgk::defaultArrangement).
    static constexpr int numSongParts = 6;
    std::vector<sgk::ArrangementPart> songArrangementParts() const;
    void setSongBars (int part, int bars);
    void setSongFill (int part, int fill);   // -1 as the Groove panel, -2 none, else a fill style (last bar of the section)
    void regenerateSongPart (int part);   // this section plays a new variation of the groove
    void resetSongPart (int part);        // back to the groove as it is
    void resetSong();
    /** Writes every section of the song as its own .mid file; returns how many were written. */
    int saveSongSections (const juce::File& folder, ClipPart) const;

    // Groove morphing: from the current groove (kept as it is when you start) towards another one.
    /** Musical DNA: one step towards an intention. The grid (with its feel) and the harmony can each
        be kept; locked parts always stay. */
    void applyIntention (int intention, bool keepRhythm, bool keepHarmony);

    /** Chord Variation Lab: sets the progression to these chords (codes relative to the key, as
        sgk::GrooveEngine::progressionCodes), keeping key, scale, length and rhythm. */
    void applyProgressionCodes (const std::vector<int>& codes);
    /** NEW FEEL: another humanization (repeatable: it is saved with the project). */
    void newFeel();

    void beginMorph();
    void setMorph (int targetPattern, float amount);

    /** Writes the clip into Documents/Fase Groove Piano and returns the file. */
    juce::File writeMidiClip (int bars, ClipPart part) const;
    /** One section of the song (songArrangementParts()[partIndex]) as a clip, named after it:
        "Piano Intro - ...", "Piano Main groove - ..." (empty file if that section is left out). */
    juce::File writeSongSectionClip (int partIndex, ClipPart part) const;
    juce::String clipFileName (int bars, ClipPart part) const;
    int clipLengthInBars (int bars) const;

    juce::AudioProcessorValueTreeState apvts;

    std::atomic<bool> previewOn { false };   // play the groove without the host transport
    // Transport of the preview: play resumes where pause left it; stop and rewind go back to the start.
    std::atomic<bool> previewFromStart { true }, previewRewind { false };
    // (With LINK on, play / pause / stop / back to the start are shared with the linked plug-ins.)
    void previewPlay();
    void previewPause();
    void previewStop();
    void previewToStart();
    void previewToggle() { previewOn.load() ? previewPause() : previewPlay(); }
    std::atomic<double> lastBpm { 120.0 };   // tempo in use (DAW or manual)
    /** The tempo exported files use: the manual BPM as set now, or the DAW's (as last reported). */
    double exportBpm() const;
    std::atomic<double> hostBpm { 120.0 };   // what the DAW reports

    // Read by the editor for display.
    std::atomic<int> uiStep { -1 };      // current 16th within the pattern, -1 = stopped
    std::atomic<int> uiSlot { -1 };      // current chord slot, -1 = stopped
    std::atomic<double> uiPpq { -1.0 };  // playing position in beats, -1 = stopped (for the piano roll)
    std::atomic<int> uiHeldRoot { -1 };  // root picked by a held key (hold mode)

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    EditedLoop editedNotes, editedBase, playingNotes;   // the edits, the notes they were made on, what plays
    std::atomic<double> editedLength { 0.0 }, playingLength { 0.0 };
    std::vector<sgk::LoopNote> lastGenerated;           // message thread: to notice a change
    double lastGeneratedLength = 0.0;
    struct Refresher : juce::Timer
    {
        std::function<void()> callback;
        void timerCallback() override { if (callback) callback(); }
    } editRefresher, linkRefresher;
    std::atomic<bool> linkOn { false };
    std::atomic<bool> guitarLeads { false };
    std::atomic<bool> keyLinked { false };
    std::vector<sgk::NoteEvent> answerEvents;   // the comping answers of one block
    std::array<bool, 128> answerSounding {};
    // Piano notes the comping left out (their note-on never went out, so neither does their note-off,
    // which would cut an answer of the same pitch).
    std::array<bool, 256> compingDropped {};
    // The answers' voiced progression, voiced again only when the harmony changes.
    sgk::GrooveEngine::HarmonyKey compingHarmony {};
    bool compingVoiced = false;
    std::array<sgk::Chord, sgk::maxProgressionLength> compingChords {};
    std::array<sgk::VoicedChord, sgk::maxProgressionLength> compingVoicing {};
    bool pedalDown = false;   // CC64 sent down (lifted on a stop or a jump)
    int leadChecks = 0;
    void readGuitarLead();
    // What the linked plug-ins share: genre, swing, tempo, DAW sync and the play button's clock
    // (anchor: the system time, in high-resolution ms, at which that clock was at beat 0).
    struct LinkState
    {
        int genre = -2;   // -1: no genre (a random or user groove)
        float swing = -1.0f, bpm = -1.0f;
        bool sync = false, playing = false, paused = false;
        double anchor = -1.0;
        int key = -1, scale = -1;   // shared only with KEY on (-1: not shared)
        bool sameAs (const LinkState& o) const noexcept
        {
            return genre == o.genre && key == o.key && scale == o.scale && std::abs (swing - o.swing) < 0.001f && std::abs (bpm - o.bpm) < 0.01f && sync == o.sync
                && playing == o.playing && paused == o.paused && std::abs (anchor - o.anchor) < 1.0;
        }
    };
    LinkState linkShared;                      // what the link file says (or what we wrote)
    juce::String linkSeenText;
    std::atomic<double> linkAnchor { -1.0 };   // the shared clock while playing, -1 = none
    LinkState localLinkState() const;
    mutable std::atomic<int> lastGenre { 0 };   // the genre of the last preset (a random or edited groove keeps it)
    float raw (const char* id) const;

    std::array<std::atomic<float>*, sgk::maxProgressionLength> slotParams {};
    std::array<std::atomic<float>*, sgk::numSegments> segmentParams {};
    std::array<std::atomic<float>*, sgk::maxProgressionLength> inversionParams {};

    void setUserPattern (const sgk::GroovePattern&);

    // The user pattern, readable from the audio thread without locks.
    std::array<std::atomic<char>, sgk::userPatternLength> userRight, userLeft;
    std::array<std::atomic<int>, sgk::userPatternLength> userVelRight, userVelLeft;   // 0 = automatic

    // Sidechain: the level of the sidechain input, per sample of the block, and its follower.
    std::vector<float> sidechainLevel, lastOutput;

    // MIDI clock (the standalone app has no DAW: a DAW can send it its clock): tempo, start / stop
    // and song position, used when the host gives no position.
    juce::int64 samplesProcessed = 0, lastClockSample = -1;
    double clockInterval = 0.0, clockPpq = 0.0;   // samples per tick (smoothed), beats since start
    bool clockRunning = false;

    // Live keys pressed on the audio thread, run on the message thread (lock-free).
    juce::AbstractFifo liveKeyFifo { 32 };
    std::array<int, 32> liveKeyBuffer {};
    std::atomic<bool> liveFillRequested { false };
    long long liveFillBar = -1;
    Refresher liveKeyRunner;
    float sidechainEnvelope = 0.0f;
    static BusesProperties makeBuses();

    struct Vst3Extensions : juce::VST3ClientExtensions
    {
        bool getPluginHasMainInput() const override { return false; }   // the input is a sidechain
    } vst3Extensions;
    int baseStepIndex (int displayedStep) const;
    int levelIndex (int displayedStep) const;
    bool makeUserPattern();
    std::array<std::atomic<int>, sgk::numSegments> velocityOrder;
    std::array<bool, sgk::numSegments> velocityLocked {};
    void resetSegmentVelocities (int segment, bool chords, bool bass);
    std::array<bool, sgk::numSegments> segmentLocked {};
    std::array<std::atomic<int>, 6> songBars;                 // per song section (0 = left out)
    std::array<std::atomic<std::uint32_t>, 6> songVariation;  // 0 = the groove as it is
    std::array<std::atomic<int>, 6> songFill;                 // see setSongFill
    sgk::GroovePattern morphSource;
    std::uint32_t morphSeed = 1;
    juce::MidiFile midiFromEvents (const std::vector<sgk::NoteEvent>&, double beats, const std::vector<sgk::GrooveEngine::Marker>*,
                                   ClipPart, const juce::String& name, const juce::String& songPart = {}) const;
    juce::MidiFile midiFromEventsAsIs (const std::vector<sgk::NoteEvent>&, double beats, const std::vector<sgk::GrooveEngine::Marker>*,
                                       ClipPart, const juce::String& name, const juce::String& songPart) const;
    std::array<std::atomic<int>, sgk::maxProgressionLength> freeChords;
    void resetUserPattern();
    int lastBassMode = 1;
    int lastStringsMode = 1;

    sgk::GrooveEngine engine;
    std::vector<sgk::NoteEvent> events;
    juce::MidiBuffer outMidi;
    std::array<int, 128> bassNoteChannels {};

    juce::Synthesiser synth;
    juce::Reverb reverb;
    juce::dsp::Chorus<float> chorusFx;
    juce::SmoothedValue<float> outputGain;

    // Hold-key mode
    std::array<int, 16> heldStack {};
    int heldCount = 0;

    // LEARN: notes currently down
    std::array<bool, 128> learnDown {};
    int learnPeakMask = 0, learnPeakCount = 0, learnPeakLowest = 0;   // the fullest chord of this press

    double internalPpq = 0.0;
    double expectedNextPpq = 0.0;
    bool wasRunning = false;
    bool soundWasOn = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaseGroovePianoProcessor)
};
