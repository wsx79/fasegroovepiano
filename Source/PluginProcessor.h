// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "engine/GrooveEngine.h"

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
    inline constexpr const char* riffSeed    = "riffSeed";
    inline constexpr const char* riffDensity = "riffDensity";
    inline constexpr const char* sound       = "sound";
    inline constexpr const char* tone        = "tone";
    inline constexpr const char* reverb      = "reverb";
    inline constexpr const char* volume      = "volume";

    inline juce::String slot (int index)    { return "slot" + juce::String (index + 1); }
    inline juce::String segment (int index) { return "segment" + juce::String (index + 1); }
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
    bool isSegmentLocked (int segment) const          { return segmentLocked[(size_t) juce::jlimit (0, 3, segment)]; }
    void setSegmentLocked (int segment, bool locked)  { segmentLocked[(size_t) juce::jlimit (0, 3, segment)] = locked; }

    /** Quick bass on/off that remembers the last bass mode. */
    void toggleBass();
    bool isBassOn() const;

    // Editor preferences, saved with the project.
    float editorScale = 1.0f;
    bool shrinkWhileDragging = true;

    /** Replaces the progression with a random soulful-house one of the current length. */
    void rollNewProgression();

    // Free chords (any chord, even outside the scale), set from the circle of fifths.
    void setFreeChord (int slot, int code);          // code: sgk::noFreeChord or sgk::freeChordCode()
    int freeChord (int slot) const                   { return freeChords[(size_t) juce::jlimit (0, sgk::maxProgressionLength - 1, slot)].load(); }
    void clearFreeChords();
    bool freeCircle = false;                         // circle of fifths accepts any chord (saved with the project)

    //==============================================================================
    enum class ClipPart { all = 0, chords, bass };

    /** Pass as `bars` to export the whole song: intro, main, break, build-up, drop, outro. */
    static constexpr int songArrangement = -1;

    /** Renders the current progression + groove as a standard MIDI file.
        bars == 0 means "one full loop" (progression, groove and fills all repeat). */
    juce::MidiFile createMidiClip (int bars, ClipPart part) const;

    /** Writes the clip into Documents/Fase Groove Piano and returns the file. */
    juce::File writeMidiClip (int bars, ClipPart part) const;
    juce::String clipFileName (int bars, ClipPart part) const;
    int clipLengthInBars (int bars) const;

    juce::AudioProcessorValueTreeState apvts;

    std::atomic<bool> previewOn { false };   // play the groove without the host transport
    std::atomic<double> lastBpm { 120.0 };   // tempo in use (DAW or manual)
    std::atomic<double> hostBpm { 120.0 };   // what the DAW reports

    // Read by the editor for display.
    std::atomic<int> uiStep { -1 };      // current 16th within the pattern, -1 = stopped
    std::atomic<int> uiSlot { -1 };      // current chord slot, -1 = stopped
    std::atomic<int> uiHeldRoot { -1 };  // root picked by a held key (hold mode)

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    float raw (const char* id) const;

    std::array<std::atomic<float>*, sgk::maxProgressionLength> slotParams {};
    std::array<std::atomic<float>*, sgk::numSegments> segmentParams {};

    void setUserPattern (const sgk::GroovePattern&);

    // The user pattern, readable from the audio thread without locks.
    std::array<std::atomic<char>, sgk::userPatternLength> userRight, userLeft;
    std::array<bool, sgk::numSegments> segmentLocked {};
    std::array<std::atomic<int>, sgk::maxProgressionLength> freeChords;
    int lastBassMode = 1;

    sgk::GrooveEngine engine;
    std::vector<sgk::NoteEvent> events;
    juce::MidiBuffer outMidi;

    juce::Synthesiser synth;
    juce::Reverb reverb;
    juce::SmoothedValue<float> outputGain;

    // Hold-key mode
    std::array<int, 16> heldStack {};
    int heldCount = 0;

    double internalPpq = 0.0;
    double expectedNextPpq = 0.0;
    bool wasRunning = false;
    bool soundWasOn = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaseGroovePianoProcessor)
};
