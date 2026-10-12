// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// Rhythm + progression sequencer. Turns a musical-time window (in quarter
// notes, "ppq") into note on/off events. Pure C++ (no JUCE).

#include "Generators.h"
#include "Theory.h"

#include <array>
#include <string>
#include <vector>

namespace sgk
{

constexpr int maxProgressionLength = 8;
constexpr double stepLength = 0.25;   // a 16th note, in quarter notes

//==============================================================================
/** A two-hand rhythm. Each character is one 16th step:
      'X' accented hit   'x' normal hit   'g' ghost note (very soft)
      '>' push (hit with the NEXT chord)  '=' hold previous  '.' rest
    The right-hand line plays the chord, the left-hand line plays the bass. */
struct GroovePattern
{
    std::string name;
    std::string rightHand;
    std::string leftHand;

    int length() const noexcept { return (int) rightHand.size(); }
};

const std::vector<GroovePattern>& groovePatterns();

/** Index of the "Random riff" entry, right after the preset grooves. */
int randomPatternIndex() noexcept;

/** Ready-made progressions (scale degrees, 0 = I). */
struct ProgressionPreset
{
    const char* name;
    int scale;
    std::vector<int> degrees;
};

const std::vector<ProgressionPreset>& progressionPresets();

//==============================================================================
enum class BassMode { off = 0, root, octaves, rootFifth };
constexpr int numBassModes = 4;
const char* bassModeName (int mode) noexcept;

struct GrooveSettings
{
    int keyRoot = 9;                       // A
    int scale = 1;                         // Dorian
    Extension extension = Extension::ninth;
    Voicing voicing = Voicing::close;
    int centre = 64;                       // middle of the right hand (MIDI note)
    BassMode bass = BassMode::root;

    std::array<int, maxProgressionLength> progression { 0, 3, 0, 3, 0, 3, 0, 3 };
    int progressionLength = 2;
    int stepsPerChord = 16;                // 8 = half bar, 16 = 1 bar ...

    int pattern = 0;
    float swing = 0.25f;                   // 0 = straight, 1 = full triplet shuffle
    float gate = 0.55f;                    // portion of a step a non-held hit lasts
    int velocity = 96;
    float humanize = 0.25f;
    int fillEveryBars = 0;                 // 0 = no fills

    Section section = Section::main;
    std::uint32_t riffSeed = 1;            // used when pattern == randomPatternIndex()
    float riffDensity = 0.5f;
};

enum class Part { chords = 0, bass = 1 };

struct NoteEvent
{
    double ppq;
    int note;
    int velocity;   // 0 for note-off
    Part part = Part::chords;
    bool isNoteOn() const noexcept { return velocity > 0; }
};

//==============================================================================
class GrooveEngine
{
public:
    GrooveEngine();

    /** Appends (time-ordered) events falling in [startPpq, endPpq). */
    void render (double startPpq, double endPpq, const GrooveSettings& settings,
                 std::vector<NoteEvent>& out);

    /** Releases everything currently sounding at the given time. */
    void allNotesOff (double ppq, std::vector<NoteEvent>& out);

    bool anyNoteSounding() const noexcept;

    /** The groove actually played: preset or random riff, adapted to the section. */
    static GroovePattern resolvePattern (const GrooveSettings& s);
    static int patternLength (const GrooveSettings& s) noexcept;

    /** Renders a whole song (intro, main, break...) with section markers. */
    struct Marker { double ppq; std::string name; };
    static std::vector<NoteEvent> renderArrangement (const GrooveSettings& s, const std::vector<ArrangementPart>& parts,
                                                     std::vector<Marker>* markers = nullptr);

    /** Length (in 16th steps) after which progression, groove and fills all repeat. */
    static int loopLengthInSteps (const GrooveSettings& s) noexcept;

    /** Offline render of a whole clip starting at bar 1, with every note closed. */
    static std::vector<NoteEvent> renderClip (const GrooveSettings& s, double lengthInBeats);

    /** Chord slot that is active at a given step. */
    static int slotForStep (long long step, const GrooveSettings& s) noexcept;

    /** Chord (and its voicing) for every slot of the progression, voice-led in order. */
    static void voiceProgression (const GrooveSettings& s,
                                  std::array<Chord, maxProgressionLength>& chords,
                                  std::array<VoicedChord, maxProgressionLength>& voiced) noexcept;

private:
    struct PendingOn { double ppq; int note; int velocity; double duration; bool tie = false; Part part = Part::chords; };

    const GroovePattern& patternFor (const GrooveSettings& s);

    void collectStep (long long step, const GrooveSettings& s, const GroovePattern& pattern,
                      const std::array<Chord, maxProgressionLength>& chords,
                      const std::array<VoicedChord, maxProgressionLength>& voiced);
    void flushOffsUpTo (double ppq, bool inclusive, std::vector<NoteEvent>& out);

    std::array<double, 128> offTime {};    // < 0 when the note is not sounding
    std::array<Part, 128> partOf {};
    std::vector<PendingOn> pending;
    std::vector<int> scratch;

    // Cache of the resolved pattern, rebuilt only when its inputs change.
    GroovePattern cachedPattern;
    int cachedIndex = -1;
    std::uint32_t cachedSeed = 0;
    float cachedDensity = -1.0f;
    Section cachedSection = Section::main;
};

} // namespace sgk
