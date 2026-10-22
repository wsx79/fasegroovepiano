// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// Rhythm + progression sequencer. Turns a musical-time window (in quarter
// notes, "ppq") into note on/off events. Pure C++ (no JUCE).

#include "Generators.h"
#include "Theory.h"

#include <array>
#include <memory>
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

    /** Chromatic presets: one free chord code per slot (freeChordCode / freeDominantCode);
        chords that belong to the scale are loaded as degrees, the others as free chords. */
    std::vector<int> chords {};
};

/** Turns a preset into slot values: for each chord, the scale degree (>= 0) when the chord
    belongs to the preset's scale in that key, otherwise noFreeChord and its free code. */
struct PresetSlot { int degree; int freeCode; };
std::vector<PresetSlot> presetSlots (const ProgressionPreset&, int extensionForMatching = 2);

/** Preset menu headings, in display order. */
const std::vector<const char*>& progressionCategories();

/** A chord outside the scale ("free" chord from the circle of fifths), or none.
    The root is stored in semitones above the key, so free chords follow key changes. */
constexpr int noFreeChord = -1;
constexpr int freeChordCode (int semitonesAboveKey, bool minor) noexcept
{
    return ((semitonesAboveKey % 12) + 12) % 12 + (minor ? 12 : 0);
}

/** A free dominant chord (7 / 9 / 13), e.g. the V7 of a minor key or a bVII7 backdoor. */
constexpr int freeDominantCode (int semitonesAboveKey) noexcept
{
    return ((semitonesAboveKey % 12) + 12) % 12 + 24;
}
constexpr int numFreeChordCodes = 36;

const std::vector<ProgressionPreset>& progressionPresets();

//==============================================================================
enum class BassMode { off = 0, root, octaves, rootFifth, walking, syncopated, withChords };

/** What the last beat of a fill bar plays. */
enum class FillStyle { pentatonicUp = 0, pentatonicDown, chordRoll,
                       gospelRun, chromaticWalk, pushStabs, chordChops, gospelThirds, glissando, bluesLick, mix };
constexpr int numFillStyles = 11;

/** First 16th (in the bar) a fill style takes over: most use the last beat, the gospel run two. */
int fillStartStep (FillStyle style) noexcept;
/** The fill a bar plays. "Mix" picks from that genre's moves; house keeps the original list. */
FillStyle effectiveFillStyle (FillStyle style, long long bar, int genre = -1) noexcept;
const char* fillStyleName (int style) noexcept;
/** The first 7 bass modes follow the groove's left-hand rhythm; after them come the bass lines
    of the library (each one its own rhythm and notes). */
constexpr int numBassPatternModes = 7;
constexpr int numClassicBassLines = 12;   // the first lines of the library (modes 7..18)
constexpr int numExtraBassLines = 12;     // appended later, after the "own rhythm" mode
constexpr int numBassLines = numClassicBassLines + numExtraBassLines;
/** Appended after the first bass lines (saved projects keep their mode): the piano-stab bass on the
    bass row's own rhythm (what RND PIANO switches to, so the bass doesn't change with the piano). */
constexpr int stabsOwnRhythmMode = numBassPatternModes + numClassicBassLines;
constexpr int numBassModes = stabsOwnRhythmMode + 1 + numExtraBassLines;
/** The library line a bass mode plays (0..numBassLines-1), or -1 for the modes that follow the groove. */
constexpr int bassLineIndex (int mode) noexcept
{
    return mode >= numBassPatternModes && mode < stabsOwnRhythmMode ? mode - numBassPatternModes
         : mode > stabsOwnRhythmMode && mode < numBassModes ? numClassicBassLines + mode - stabsOwnRhythmMode - 1 : -1;
}
/** The bass mode that plays a library line. */
constexpr int bassModeForLine (int line) noexcept
{
    return line < numClassicBassLines ? numBassPatternModes + line : stabsOwnRhythmMode + 1 + line - numClassicBassLines;
}
constexpr bool isBassLine (BassMode m) noexcept { return bassLineIndex ((int) m) >= 0; }

/** A bass line over two bars, one symbol per 16th, relative to the chord playing:
    R root, O octave up, 5 fifth, L fifth below, 3 the chord's 3rd, 7 its 7th, 6 sixth,
    b minor 3rd (blue note), G dead note (short, soft), A chromatic approach into the next chord's
    root, S a whole step below it, = hold, . rest. */
struct BassLine { const char* name; const char* genre; std::string line; };
const std::vector<BassLine>& bassLines();

/** The rhythm of a bass line as grid symbols (x on the hits, g dead notes), for the display. */
std::string bassLineRhythm (int lineIndex, int length);

/** Shape of the top note of the chords across the progression (the melody you hear). */
enum class TopLine { automatic = 0, steady, rising, falling, arch, pedal };
constexpr int numTopLines = 6;
const char* topLineName (int line) noexcept;

/** Gospel "slide": on the last 16th before a chord change, play the next chord
    (and bass note) a half step above or below, resolving onto the new chord. */
enum class ApproachStyle { off = 0, fromAbove, fromBelow };
constexpr int numApproachStyles = 3;
const char* approachStyleName (int style) noexcept;
const char* bassModeName (int mode) noexcept;

/** The strings: a third part under / around the piano for body. Pads hold each chord until the
    next one (voice-led, close to the piano's middle), the high pad sits an octave up, the wide
    pad adds the root and fifth an octave above the bass, stabs double the piano's chord hits. */
enum class StringsMode { off = 0, pad, highPad, widePad, stabs,
                         // appended (saved projects keep their mode):
                         offbeatStabs, answerStabs, pulse8ths, halfBarPad, discoOctaves, riseIntoChords };
constexpr int numStringsModes = 11;
const char* stringsModeName (int mode) noexcept;

/** Humanize per voice: Even, Pianist (top note sings, a hair early), Laid-back bass, Tight bass. */
constexpr int numHumanizeStyles = 4;
const char* humanizeStyleName (int) noexcept;

enum class Part { chords = 0, bass = 1, fill = 2, strings = 3 };   // fill: the right hand during a fill (exported on its own track)

/** A note of a loop edited by hand in the piano roll (times in beats from the loop start). */
struct LoopNote
{
    double start = 0.0, length = 0.25;
    int note = 60, velocity = 100;
    Part part = Part::chords;
    int origin = -1;   // the generated note it comes from (index), -1 = added by hand
};

/** Carries the edits made by hand on a loop over to the loop generated now (other chords,
    voicing, key, groove...): every generated note of `base` that was deleted, moved, lengthened
    or played softer in `edited` is found again in `now` (same part, same moment, same place in
    the chord) and changed the same way; notes added by hand follow the harmony; new generated
    notes stay. The result's `origin`s point into `now`. */
/** The rhythm the "Piano stabs" bass plays for a groove (a root under every chord stab, plus the
    bass's own hits that don't crowd a stab), written as a left-hand line. */
std::string stabBassLine (const GroovePattern&);

std::vector<LoopNote> carryEdits (const std::vector<LoopNote>& base, const std::vector<LoopNote>& edited, double baseLength,
                                  const std::vector<LoopNote>& now, double nowLength);

struct GrooveSettings
{
    int keyRoot = 9;                       // A
    int scale = 1;                         // Dorian
    Extension extension = Extension::ninth;
    Voicing voicing = Voicing::close;
    int centre = 64;                       // middle of the right hand (MIDI note)
    BassMode bass = BassMode::root;
    StringsMode strings = StringsMode::off;

    std::array<int, maxProgressionLength> progression { 0, 3, 0, 3, 0, 3, 0, 3 };
    int progressionLength = 2;

    // Per slot: noFreeChord, or freeChordCode() to play that chord instead of the scale degree.
    std::array<int, maxProgressionLength> freeChords { -1, -1, -1, -1, -1, -1, -1, -1 };
    int stepsPerChord = 16;                // 8 = half bar, 16 = 1 bar ...

    int pattern = 0;
    float swing = 0.25f;                   // 0 = straight, 1 = full triplet shuffle
    float bassGate = -1.0f;                 // negative: legacy shared gate; explicit family presets may separate it
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
    int evolveBars = 0;                    // 0 = off, else the groove grows over 8 or 16 bars (see GrooveEngine)
    int rangeLow = 24, rangeHigh = 108;    // the chords' range: their notes are kept between these
    int grooveBars = 2;                    // 1, 2 or 4 bars
    int movement = 1;                      // voicing movement: 0 off, 1 subtle, 2 lively
    int strumDirection = 0;                // 0 up, 1 down, 2 alternate (up on the beat, down off it)
    double bpm = 120.0;                    // used to make the strum a real time (ms), not a note value

    Section section = Section::main;
    std::uint32_t riffSeed = 1;            // used when pattern == randomPatternIndex()
    float riffDensity = 0.5f;
    int styleGenre = -1;                   // genre of a random or user groove; a library preset uses its own category

    std::array<int, numSegments> segmentOrder { 0, 1, 2, 3 };   // A B C D
    std::uint32_t sectionVariation = 0;    // a song section regenerated on its own (0: the groove as it is)
    int humanizeStyle = 0;                 // how Humanize is shared between the voices (see humanizeStyleName)
    std::uint32_t humanizeSeed = 0;        // NEW FEEL: another, repeatable humanization
    long long forcedFillBar = -1;          // a song section's own fill, on its last bar...
    int forcedFillStyle = -1;              // ...in this style (-2: no fill there)

    // Notes edited one by one in the piano roll: when set they replace the generated notes in the
    // main groove and the drop (the other song sections keep being generated).
    std::shared_ptr<const std::vector<LoopNote>> editedLoop;
    double editedLoopLength = 0.0;         // in beats
    bool playsEditedLoop() const noexcept
    {
        return editedLoop != nullptr && ! editedLoop->empty() && editedLoopLength > 0.0
               && (section == Section::main || section == Section::drop);
    }

    // Used when pattern == userPatternIndex(): the hand-edited grid.
    std::array<char, userPatternLength> userRight {};
    std::array<char, userPatternLength> userLeft {};
    // Velocity set by hand for each step of the user pattern (0 = automatic, from the symbol).
    std::array<unsigned char, userPatternLength> userVelRight {};
    std::array<unsigned char, userPatternLength> userVelLeft {};
    // Which part's levels play in each half bar (the velocity page's own A-D order, over the
    // displayed groove): 0 1 2 3 = each part keeps its own levels.
    std::array<int, numSegments> velocityOrder { 0, 1, 2, 3 };

    // Bass feel: 0 classic (the root in the octave of C2), 1 sub (the root voice-led in the low
    // octave, the nearest to the chord before), 2 sub with sidechain gaps (no note rings over a beat).
    int bassFeel = 0;
    // Groove realism: humanize both ways with a slow drift, accents by place in the bar, the loop
    // varied over a phrase (another take now and then, a few hits left out, a top note that answers).
    bool realism = false;
};

constexpr int numBassFeels = 3;
const char* bassFeelName (int feel) noexcept;


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

    /** The same as a loop: a note still ringing at the end (a push, a held chord) ends there and
        its rest sounds again from the start, as it does when the loop goes round. */
    static std::vector<NoteEvent> renderLoopClip (const GrooveSettings& s, double lengthInBeats);

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

    /** Voicing Coach: how a voicing behaves over the whole progression. */
    struct VoicingReport
    {
        int lowest = 0, highest = 0;      // the range used (both hands)
        double averageMove = 0.0;         // semitones each voice moves from chord to chord (loop included)
        int biggestTopLeap = 0;           // the largest jump of the top note (the melody you hear)
        int muddy = 0;                    // close intervals (under a 4th) below E3: they sound muddy
        int clashes = 0;                  // semitones between neighbouring voices
        std::vector<NoteSet> chordNotes;  // per chord, both hands, sorted
        std::vector<std::string> names;
    };
    static VoicingReport voicingReport (const GrooveSettings& s);

    /** How much of Humanize each voice gets (timing, velocity), and the pianist's singing top note. */
    struct HumanizeVoices { double chordTime, chordVel, topLead, topAccent, bassTime, bassVel, bassLate; };
    static HumanizeVoices humanizeVoices (int style) noexcept;

    /** Chord Variation Lab: the progression as free-chord codes (relative to the key), and
        alternatives of it with the same number of chords (so the rhythm stays): more soulful,
        gospel, deep, chromatic. */
    struct ProgressionVariant { const char* name; std::vector<int> codes; };
    static std::vector<int> progressionCodes (const GrooveSettings& s);
    static std::vector<ProgressionVariant> progressionVariants (const GrooveSettings& s);

    /** Chord (and its voicing) for every slot of the progression, voice-led in order. */
    /** The extension a song section plays: lighter in the intro / outro, fuller in the drop. */
    static Extension sectionExtension (const GrooveSettings& s) noexcept;

    static void voiceProgression (const GrooveSettings& s,
                                  std::array<Chord, maxProgressionLength>& chords,
                                  std::array<VoicedChord, maxProgressionLength>& voiced) noexcept;

    /** The strings' notes for every slot (empty when they are off), voice-led from chord to chord. */
    static void voiceStrings (const GrooveSettings& s, const std::array<Chord, maxProgressionLength>& chords,
                              std::array<NoteSet, maxProgressionLength>& out) noexcept;

    /** Everything the voiced progression depends on: a change of any of these voices it again. */
    struct HarmonyKey
    {
        int key, scale, extension, voicing, centre, length, topLine, section, rangeLow, rangeHigh, bassOn, bassFeel;
        std::array<int, maxProgressionLength> progression, freeChords, inversions;
        bool operator== (const HarmonyKey& o) const noexcept
        {
            return key == o.key && scale == o.scale && extension == o.extension && voicing == o.voicing
                && centre == o.centre && length == o.length && topLine == o.topLine && section == o.section
                && rangeLow == o.rangeLow && rangeHigh == o.rangeHigh && bassOn == o.bassOn && bassFeel == o.bassFeel
                && progression == o.progression && freeChords == o.freeChords && inversions == o.inversions;
        }
    };
    static HarmonyKey harmonyKey (const GrooveSettings& s) noexcept;

    /** The bass root of every slot: C2..B2 (classic), or in the low octave voice-led from chord
        to chord (sub feels: the nearest to the root before, the loop point included). */
    static void bassRoots (const GrooveSettings& s, const std::array<Chord, maxProgressionLength>& chords,
                           std::array<int, maxProgressionLength>& out) noexcept;

private:
    struct PendingOn { double ppq; int note; int velocity; double duration; bool tie = false; Part part = Part::chords; std::size_t order = 0; };

    const GroovePattern& patternFor (const GrooveSettings& s);

    static void clearLowRegister (VoicedChord&, const Chord&, bool bassOn) noexcept;
    static void keepInRange (VoicedChord&, int low, int high) noexcept;
    void collectStep (long long step, const GrooveSettings& s, const GroovePattern& normalPattern, const GroovePattern& variation,
                      const std::array<Chord, maxProgressionLength>& chords,
                      const std::array<VoicedChord, maxProgressionLength>& voiced);
    void collectBassLine (long long step, const GrooveSettings& s, const std::array<Chord, maxProgressionLength>& chords,
                          const std::string& line);
    void collectStrings (long long step, const GrooveSettings& s, double startPpq, double endPpq);
    /** The notes the strings hold on a chord (the pad, or the top note in two octaves for disco). */
    NoteSet heldStrings (int slot, const GrooveSettings& s) const noexcept;
    static int stringsVelocity (const GrooveSettings& s) noexcept;
    void flushOffsUpTo (double ppq, bool inclusive, std::vector<NoteEvent>& out);

    // Per note (0..127), and again for the strings (128..255): they have their own channel, so a
    // string note can hold under the same piano note. Bass and piano share channel 1 by default, so
    // they share a note: struck together, it belongs to the bass (the bass line never loses a note).
    static constexpr int numTracked = 256;
    static int trackIndex (int note, Part part) noexcept { return note + (part == Part::strings ? 128 : 0); }
    std::array<double, numTracked> offTime {};    // < 0 when the note is not sounding
    std::array<Part, numTracked> partOf {};
    double stringsUntil = -1.0;                   // when the strings' current pad ends
    std::vector<PendingOn> pending;
    std::vector<int> scratch;
    std::string stabLine;   // reused: no allocation on the audio thread once it has grown

    // Cache of the voiced progression, rebuilt only when the harmony changes.
    HarmonyKey cachedHarmony {};
    bool harmonyCached = false;
    std::array<Chord, maxProgressionLength> cachedChords {};
    std::array<VoicedChord, maxProgressionLength> cachedVoiced {};
    std::array<NoteSet, maxProgressionLength> cachedStrings {};
    std::array<int, maxProgressionLength> cachedBassRoots {};
    int cachedStringsMode = -1;

    // Cache of the resolved pattern, rebuilt only when its inputs change.
    GroovePattern cachedPattern, cachedVariation;
    std::array<GroovePattern, 3> cachedEvolution;   // the groove's evolution: + ghosts, + variation, + fill
    std::array<GroovePattern, 2> cachedTakes;       // realism: two other takes of the groove over a phrase
    int cachedBass = -1;
    bool cachedRealism = false;
    int cachedBars = 2, cachedMovement = 1;
    int cachedIndex = -1;
    std::uint32_t cachedSeed = 0;
    float cachedDensity = -1.0f;
    Section cachedSection = Section::main;
    std::uint32_t cachedSectionVariation = 0;
    std::array<int, numSegments> cachedOrder {};
    std::array<char, userPatternLength> cachedUserRight {}, cachedUserLeft {};
};

} // namespace sgk
