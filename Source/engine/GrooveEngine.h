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
    std::string category {};   // genre heading in the groove menu

    int length() const noexcept { return (int) rightHand.size(); }
};

const std::vector<GroovePattern>& groovePatterns();

/** Genre headings of the groove menu, in display order. */
const std::vector<const char*>& grooveCategories();

/** Index of the "Random riff" entry, right after the preset grooves. */
int randomPatternIndex() noexcept;

/** Index of the "User pattern" entry (editable grid), after "Random riff". */
int userPatternIndex() noexcept;

constexpr int userPatternLength = 32;   // two bars of 16ths

/** Ready-made progressions (scale degrees, 0 = I). */
struct ProgressionPreset
{
    const char* name;
    int scale;
    std::vector<int> degrees;
    const char* category = "";   // shown as a heading in the preset menu
};

/** Preset menu headings, in display order. */
const std::vector<const char*>& progressionCategories();

/** A chord outside the scale ("free" chord from the circle of fifths), or none.
    The root is stored in semitones above the key, so free chords follow key changes. */
constexpr int noFreeChord = -1;
constexpr int freeChordCode (int semitonesAboveKey, bool minor) noexcept
{
    return ((semitonesAboveKey % 12) + 12) % 12 + (minor ? 12 : 0);
}

const std::vector<ProgressionPreset>& progressionPresets();

//==============================================================================
enum class BassMode { off = 0, root, octaves, rootFifth, walking, syncopated };

/** What the last beat of a fill bar plays. */
enum class FillStyle { pentatonicUp = 0, pentatonicDown, chordRoll,
                       gospelRun, chromaticWalk, pushStabs, chordChops, gospelThirds, glissando, bluesLick, mix };
constexpr int numFillStyles = 11;

/** First 16th (in the bar) a fill style takes over: most use the last beat, the gospel run two. */
int fillStartStep (FillStyle style) noexcept;
const char* fillStyleName (int style) noexcept;
constexpr int numBassModes = 6;

/** Shape of the top note of the chords across the progression (the melody you hear). */
enum class TopLine { automatic = 0, steady, rising, falling, arch };
constexpr int numTopLines = 5;
const char* topLineName (int line) noexcept;

/** Gospel "slide": on the last 16th before a chord change, play the next chord
    (and bass note) a half step above or below, resolving onto the new chord. */
enum class ApproachStyle { off = 0, fromAbove, fromBelow };
constexpr int numApproachStyles = 3;
const char* approachStyleName (int style) noexcept;
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

    // Per slot: noFreeChord, or freeChordCode() to play that chord instead of the scale degree.
    std::array<int, maxProgressionLength> freeChords { -1, -1, -1, -1, -1, -1, -1, -1 };
    int stepsPerChord = 16;                // 8 = half bar, 16 = 1 bar ...

    int pattern = 0;
    float swing = 0.25f;                   // 0 = straight, 1 = full triplet shuffle
    float gate = 0.55f;                    // portion of a step a non-held hit lasts
    int velocity = 96;
    float humanize = 0.25f;
    int fillEveryBars = 0;                 // 0 = no fills
    FillStyle fillStyle = FillStyle::pentatonicUp;
    float strum = 0.0f;                    // 0..1: roll the chord notes like a pianist
    bool octaveTop = false;                // double the top note an octave higher
    ApproachStyle approach = ApproachStyle::off;
    TopLine topLine = TopLine::automatic;
    std::array<int, maxProgressionLength> inversions {};   // per slot, -3..3
    int variationEvery = 0;                // 0 = off, else every 4 or 8 bars the last bar varies
    int grooveBars = 2;                    // 1, 2 or 4 bars

    Section section = Section::main;
    std::uint32_t riffSeed = 1;            // used when pattern == randomPatternIndex()
    float riffDensity = 0.5f;

    std::array<int, numSegments> segmentOrder { 0, 1, 2, 3 };   // A B C D

    // Used when pattern == userPatternIndex(): the hand-edited grid.
    std::array<char, userPatternLength> userRight {};
    std::array<char, userPatternLength> userLeft {};
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

    /** The groove before segment order and section are applied: a preset,
        the random riff or the user pattern. */
    static GroovePattern basePattern (const GrooveSettings& s);

    /** The groove actually played: base pattern, segment order, then section. */
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

    /** The chord of one slot: its scale degree, or its free chord when it has one. */
    static Chord chordForSlot (const GrooveSettings& s, int slot) noexcept;

    /** Roman numeral of a slot ("IV", or "bVI" for a free chord). */
    static std::string numeralForSlot (const GrooveSettings& s, int slot);

    /** How well the chords sound together, 0..1. transition[i] scores the move from slot i to
        the next one (the last wraps to the first); slot[i] is the average of the moves into and
        out of slot i (a single chord scores by how well it fits the key). */
    struct HarmonyScores
    {
        std::array<float, maxProgressionLength> slot {}, transition {};
    };
    static HarmonyScores harmonyScores (const GrooveSettings& s,
                                        const std::array<Chord, maxProgressionLength>& chords,
                                        const std::array<VoicedChord, maxProgressionLength>& voiced) noexcept;

    /** Chord (and its voicing) for every slot of the progression, voice-led in order. */
    static void voiceProgression (const GrooveSettings& s,
                                  std::array<Chord, maxProgressionLength>& chords,
                                  std::array<VoicedChord, maxProgressionLength>& voiced) noexcept;

private:
    struct PendingOn { double ppq; int note; int velocity; double duration; bool tie = false; Part part = Part::chords; };

    const GroovePattern& patternFor (const GrooveSettings& s);

    void collectStep (long long step, const GrooveSettings& s, const GroovePattern& normalPattern, const GroovePattern& variation,
                      const std::array<Chord, maxProgressionLength>& chords,
                      const std::array<VoicedChord, maxProgressionLength>& voiced);
    void flushOffsUpTo (double ppq, bool inclusive, std::vector<NoteEvent>& out);

    std::array<double, 128> offTime {};    // < 0 when the note is not sounding
    std::array<Part, 128> partOf {};
    std::vector<PendingOn> pending;
    std::vector<int> scratch;

    // Cache of the voiced progression, rebuilt only when the harmony changes.
    struct HarmonyKey
    {
        int key, scale, extension, voicing, centre, length, topLine;
        std::array<int, maxProgressionLength> progression, freeChords, inversions;
        bool operator== (const HarmonyKey& o) const noexcept
        {
            return key == o.key && scale == o.scale && extension == o.extension && voicing == o.voicing
                && centre == o.centre && length == o.length && topLine == o.topLine && progression == o.progression
                && freeChords == o.freeChords && inversions == o.inversions;
        }
    };
    HarmonyKey cachedHarmony {};
    bool harmonyCached = false;
    std::array<Chord, maxProgressionLength> cachedChords {};
    std::array<VoicedChord, maxProgressionLength> cachedVoiced {};

    // Cache of the resolved pattern, rebuilt only when its inputs change.
    GroovePattern cachedPattern, cachedVariation;
    int cachedBars = 2;
    int cachedIndex = -1;
    std::uint32_t cachedSeed = 0;
    float cachedDensity = -1.0f;
    Section cachedSection = Section::main;
    std::array<int, numSegments> cachedOrder {};
    std::array<char, userPatternLength> cachedUserRight {}, cachedUserLeft {};
};

} // namespace sgk
