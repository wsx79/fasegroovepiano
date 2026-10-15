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
    inline constexpr const char* riffSeed    = "riffSeed";
    inline constexpr const char* riffDensity = "riffDensity";
    inline constexpr const char* sound       = "sound";
    inline constexpr const char* tone        = "tone";
    inline constexpr const char* reverb      = "reverb";
    inline constexpr const char* volume      = "volume";

    inline juce::String slot (int index)    { return "slot" + juce::String (index + 1); }
    inline juce::String segment (int index) { return "segment" + juce::String (index + 1); }
    inline juce::String inversion (int index) { return "inversion" + juce::String (index + 1); }
}

class FaseGroovePianoProcessor : public juce::AudioProcessor
{
public:
    FaseGroovePianoProcessor();
    ~FaseGroovePianoProcessor() override = default;

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

    /** Quick bass on/off that remembers the last bass mode. */
    void toggleBass();
    bool isBassOn() const;

    // Editor preferences, saved with the project.
    float editorScale = 1.0f;
    bool shrinkWhileDragging = true;

    /** Replaces the progression with a random soulful-house one of the current length. */
    void rollNewProgression();

    /** Puts every setting back to its default (the interface language is not a setting
        of the plug-in, so it stays). */
    void resetToDefaults();

    /** Ready-made genre setups: progression, harmony, groove, feel and sound in one click
        (the key is kept). */
    static juce::StringArray styleNames();
    void applyStyle (int index);
    /** Loads progressionPresets()[index]: scale, length, degrees and any borrowed chords. */
    void applyProgressionPreset (int index);

    /** Reads a MIDI file (all tracks but drums) and estimates key, chords and chord length. */
    static sgk::HarmonyAnalysis analyzeMidiFile (const juce::File&);
    /** Sets key, scale, progression and chord length from an analysis. */
    void applyAnalysis (const sgk::HarmonyAnalysis&);
    /** Index in progressionPresets() that a style loads, or -1. */
    static int stylePreset (int index);

    //==============================================================================
    // LEARN: play chords on a MIDI keyboard and they fill the progression, slot by slot.
    std::atomic<bool> learnMode { false };
    /** SOLO BASS: the chords stay silent (MIDI out and guide sound); the exported clips are not affected. */
    std::atomic<bool> chordsMuted { false };

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
    enum class ClipPart { all = 0, chords, bass, tracks };   // tracks: one MIDI file, piano / bass / fills on separate tracks

    /** Pass as `bars` to export the whole song: intro, main, break, build-up, drop, outro. */
    static constexpr int songArrangement = -1;

    /** Renders the current progression + groove as a standard MIDI file.
        bars == 0 means "one full loop" (progression, groove and fills all repeat). */
    juce::MidiFile createMidiClip (int bars, ClipPart part) const;

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
    juce::String clipFileName (int bars, ClipPart part) const;
    int clipLengthInBars (int bars) const;

    juce::AudioProcessorValueTreeState apvts;

    std::atomic<bool> previewOn { false };   // play the groove without the host transport
    // Transport of the preview: play resumes where pause left it; stop and rewind go back to the start.
    std::atomic<bool> previewFromStart { true }, previewRewind { false };
    void previewPlay()   { previewOn = true; }
    void previewPause()  { previewOn = false; previewFromStart = false; }
    void previewStop()   { previewOn = false; previewFromStart = true; }
    void previewToStart() { previewFromStart = true; previewRewind = true; }
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
    } editRefresher;
    float raw (const char* id) const;

    std::array<std::atomic<float>*, sgk::maxProgressionLength> slotParams {};
    std::array<std::atomic<float>*, sgk::numSegments> segmentParams {};
    std::array<std::atomic<float>*, sgk::maxProgressionLength> inversionParams {};

    void setUserPattern (const sgk::GroovePattern&);

    // The user pattern, readable from the audio thread without locks.
    std::array<std::atomic<char>, sgk::userPatternLength> userRight, userLeft;
    std::array<std::atomic<int>, sgk::userPatternLength> userVelRight, userVelLeft;   // 0 = automatic

    // Sidechain: the level of the sidechain input, per sample of the block, and its follower.
    std::vector<float> sidechainLevel;
    float sidechainEnvelope = 0.0f;
    int baseStepIndex (int displayedStep) const;
    void resetSegmentVelocities (int segment, bool chords, bool bass);
    std::array<bool, sgk::numSegments> segmentLocked {};
    std::array<std::atomic<int>, 6> songBars;                 // per song section (0 = left out)
    std::array<std::atomic<std::uint32_t>, 6> songVariation;  // 0 = the groove as it is
    std::array<std::atomic<int>, 6> songFill;                 // see setSongFill
    sgk::GroovePattern morphSource;
    std::uint32_t morphSeed = 1;
    juce::MidiFile midiFromEvents (const std::vector<sgk::NoteEvent>&, double beats, const std::vector<sgk::GrooveEngine::Marker>*,
                                   ClipPart, const juce::String& name) const;
    std::array<std::atomic<int>, sgk::maxProgressionLength> freeChords;
    void resetUserPattern();
    int lastBassMode = 1;

    sgk::GrooveEngine engine;
    std::vector<sgk::NoteEvent> events;
    juce::MidiBuffer outMidi;

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
