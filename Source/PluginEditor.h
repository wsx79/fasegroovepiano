// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "Branding.h"
#include "Localisation.h"
#include "CircleOfFifths.h"
#include "PluginProcessor.h"
#include "Updater.h"

#include <juce_gui_basics/juce_gui_basics.h>

// v2 "studio" theme: graphite surfaces, one warm accent (piano) and one cool accent (bass).
namespace Palette
{
    const juce::Colour background { 0xff0d0f13 };   // window
    const juce::Colour panel      { 0xff161920 };   // panels
    const juce::Colour panelEdge  { 0xff272c35 };   // borders, tracks, empty cells
    const juce::Colour control    { 0xff1d2129 };   // buttons
    const juce::Colour field      { 0xff0f1216 };   // menus, displays
    const juce::Colour accent     { 0xffffa94d };   // amber: the piano, the main actions
    const juce::Colour accent2    { 0xff4cc3f0 };   // cyan: the bass, pushes, playing
    const juce::Colour fill       { 0xffb69cff };   // violet: fills
    const juce::Colour level      { 0xff5fd68a };   // green: velocity
    const juce::Colour text       { 0xffe7e9ec };
    const juce::Colour dim        { 0xff7f8794 };
}

//==============================================================================
class GrooveLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GrooveLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;

    // One typeface everywhere (Inter, embedded), flat controls with thin borders.
    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& background, bool highlighted, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    void drawComboBox (juce::Graphics&, int width, int height, bool isDown, int bx, int by, int bw, int bh, juce::ComboBox&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    void positionComboBoxText (juce::ComboBox&, juce::Label&) override;
    void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
    juce::Font getPopupMenuFont() override;
    void drawScrollbar (juce::Graphics&, juce::ScrollBar&, int x, int y, int width, int height, bool vertical,
                        int thumbStart, int thumbSize, bool over, bool down) override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int width, int height) override;
    juce::Rectangle<int> getTooltipBounds (const juce::String& text, juce::Point<int> screenPos, juce::Rectangle<int> parentArea) override;
    juce::Font getLabelFont (juce::Label&) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool highlighted, bool down) override;

private:
    juce::Typeface::Ptr regular, semiBold;
};

//==============================================================================
/** Rotary knob with a caption. */
class Knob : public juce::Component
{
public:
    Knob (juce::AudioProcessorValueTreeState&, const juce::String& paramId, const juce::String& caption);
    void resized() override;
    void refreshText();
    void setTip (const juce::String& tip)  { slider.setTooltip (tip); }

private:
    juce::String caption;
    juce::Slider slider;
    juce::Label label;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

/** Drop-down menu with a caption above it. */
class Choice : public juce::Component
{
public:
    Choice (juce::AudioProcessorValueTreeState&, const juce::String& paramId, const juce::String& caption);
    void resized() override;

    /** Re-applies the caption and the menu items in the current language. */
    void refreshText();

    /** Shows the items under headings, in columns. Item ids stay "parameter index + 1",
        so saved projects keep their value whatever the menu order. */
    struct Group { juce::String heading; std::vector<int> items; bool columnBreakAfter = false; };
    void setGroups (std::vector<Group>);

    /** A ComboBox whose list can be replaced by a custom browser (the groove menu). */
    struct PopupBox : public juce::ComboBox
    {
        std::function<void()> onPopup;
        void showPopup() override
        {
            if (onPopup == nullptr)
                return juce::ComboBox::showPopup();

            hidePopup();   // no JUCE menu was opened: mark it closed, or the next click is ignored
            onPopup();
        }
    };
    PopupBox box;

private:
    void rebuildGroups();

    juce::String caption;
    juce::StringArray englishItems;   // by parameter index
    juce::Label label;
    juce::RangedAudioParameter* parameter = nullptr;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    std::unique_ptr<juce::ParameterAttachment> groupedAttachment;
    std::vector<Group> groups;
};

/** One chord of the progression: degree selector + resulting chord name. */
class ChordSlot : public juce::Component, public juce::SettableTooltipClient
{
public:
    ChordSlot (juce::AudioProcessorValueTreeState&, int index);

    /** Called when the user picks a degree from the menu (it replaces a free chord). */
    std::function<void()> onDegreePicked;

    /** Called with +1 / -1 when an inversion arrow is clicked. */
    std::function<void (int)> onInvert;
    void setInversion (int);
    void setDegreeTip (const juce::String& tip)  { degree.setTooltip (tip); }

    /** isFree: the slot plays a free chord (from the circle), shown instead of a degree. */
    void setInfo (const juce::String& chordName, const juce::String& roman, bool enabled, bool playing,
                  bool isFree, int degreeValue, float harmonyScore);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    /** Tells apart a degree picked by the user from a change coming from the parameter. */
    struct DegreeBox : public juce::ComboBox
    {
        bool userOpened = false;
        void showPopup() override { userOpened = true; juce::ComboBox::showPopup(); }
    };

    int index;
    DegreeBox degree;
    bool showingFree = false;
    int inversion = 0;
    juce::ArrowButton invertUp { "up", 0.75f, Palette::dim }, invertDown { "down", 0.25f, Palette::dim };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    juce::String name, numeral;
    bool isEnabledSlot = true, isPlaying = false;
    float harmony = 1.0f;   // how well it sounds with the chords next to it (0..1)
};

/** Shows the two-hand rhythm of the selected groove and the playhead. */
class PatternView : public juce::Component, public juce::SettableTooltipClient
{
public:
    static constexpr float labelWidth = 44.0f;
    static constexpr float headerHeight = 16.0f;

    /** Called when a cell is clicked (right click / ctrl-click = popup menu). */
    std::function<void (bool bassRow, int step, const juce::MouseEvent&)> onCellClicked;

    void setState (const sgk::GroovePattern& pattern, int step, bool bassOn, bool pianoOn = true);
    void paint (juce::Graphics&) override;
    juce::String getTooltip() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    bool cellAt (juce::Point<float>, bool& bassRow, int& step) const;

    sgk::GroovePattern pattern;
    int currentStep = -1;
    bool bassEnabled = true, pianoEnabled = true;
    int hoverStep = -1;
    bool hoverBass = false;
};

/** The velocity page: the grid's hits as green faders, one per step. Drag a fader (or across
    several) to set its level, double-click it for the automatic level again. */
class VelocityView : public juce::Component, public juce::SettableTooltipClient
{
public:
    static constexpr float labelWidth = PatternView::labelWidth;
    static constexpr float headerHeight = PatternView::headerHeight;

    struct Row
    {
        std::string line;            // the hits, as in the grid
        std::vector<int> levels;     // per step: 0 = automatic
        bool enabled = true;
        juce::String message;        // shown instead of the faders when disabled
        bool operator== (const Row& o) const { return line == o.line && levels == o.levels && enabled == o.enabled && message == o.message; }
    };

    std::function<void (bool bassRow, int step, int level)> onLevel;   // level 0 = automatic
    std::function<void (bool chords, bool bass)> onReset;

    void setState (const Row& chords, const Row& bass, int step);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

    static bool isHit (char c) { return c == 'X' || c == 'x' || c == 'g' || c == '>' || c == 'U' || c == 'u' || c == 'D' || c == 'd'; }
    /** The level a hit gets without a hand-set one (118 = the Velocity control). */
    static int automaticLevel (char c) { return c == 'g' ? 65 : c == 'x' || c == 'u' || c == 'd' ? 106 : c == '>' ? 114 : 118; }

private:
    juce::Rectangle<float> rowArea (bool bassRow) const;
    int stepAt (float x) const;
    int levelAt (bool bassRow, float y) const;
    void setFrom (juce::Point<float>);
    int ownerOf (int row, int step) const;   // a hold belongs to the hit before it

    Row rows[2];
    int currentStep = -1, hoverStep = -1, dragStep = -1;
    bool hoverBass = false, dragBass = false, dragging = false;
};

/** The level arriving at the sidechain input, next to the Sidechain menu. */
class SidechainMeter : public juce::Component, public juce::SettableTooltipClient
{
public:
    void setState (float newLevel, bool pumpOff)
    {
        newLevel = juce::jlimit (0.0f, 1.0f, newLevel);
        if (std::abs (newLevel - level) < 0.01f && pumpOff == noDepth)
            return;
        level = newLevel;
        noDepth = pumpOff;
        repaint();
    }
    void paint (juce::Graphics&) override;

private:
    float level = 0.0f;
    bool noDepth = false;
};

/** Piano roll of what the plug-in plays: every chord and bass note of the loop on a keyboard,
    the chord names above, and the playhead. Read-only (the rhythm grid is where you edit). */
class PianoRollView : public juce::Component, public juce::SettableTooltipClient
{
public:
    struct Note
    {
        double start, length;   // in beats
        int note, velocity;
        sgk::Part part;
        bool operator== (const Note& o) const
        {
            return juce::approximatelyEqual (start, o.start) && juce::approximatelyEqual (length, o.length)
                   && note == o.note && velocity == o.velocity && part == o.part;
        }
    };
    struct ChordSpan
    {
        double start, length;
        juce::String name;
        bool operator== (const ChordSpan& o) const
        {
            return juce::approximatelyEqual (start, o.start) && juce::approximatelyEqual (length, o.length) && name == o.name;
        }
    };

    /** The loop to show; repaints only when something changed. */
    void setContent (std::vector<Note> notes, std::vector<ChordSpan> chords, double lengthInBeats, bool chordsMuted);
    /** Playhead in beats from the start of the loop, or -1 when stopped. */
    void setPlayhead (double beat);
    /** The notes were edited by hand (a tag in the corner). */
    void setEdited (bool e)  { if (e != edited) { edited = e; repaint(); } }
    std::function<void()> onDoubleClick;
    void mouseDoubleClick (const juce::MouseEvent&) override  { if (onDoubleClick != nullptr) onDoubleClick(); }

    void paint (juce::Graphics&) override;

    static constexpr float keyboardWidth = 34.0f, chordLaneHeight = 20.0f;

private:
    juce::Rectangle<float> noteArea() const;
    float xOf (double beat) const;

    std::vector<Note> notes;
    std::vector<ChordSpan> chords;
    double length = 8.0, playhead = -1.0;
    bool mutedChords = false, edited = false;
    int lowNote = 36, highNote = 84;
};

/** Drag this tile into the DAW to drop the current groove as a MIDI clip. */
class MidiDragTile : public juce::Component, public juce::SettableTooltipClient
{
public:
    std::function<juce::File()> createFile;
    std::function<void()> onDragStart, onDragEnd;
    void setSubtitle (const juce::String& text)  { if (text != subtitle) { subtitle = text; repaint(); } }

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override  { hover = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override  { dragStarted = false; }
    void mouseDrag (const juce::MouseEvent&) override;

private:
    bool hover = false, dragStarted = false;
    juce::String subtitle;
};

/** OPTIONS page with two tabs: SETTINGS (language, interface size, mini window)
    and ABOUT (logo, version, credits, links). Click outside the card to close. */
class OptionsOverlay : public juce::Component
{
public:
    OptionsOverlay();

    std::function<void()> onLanguageChanged;
    std::function<void (float)> onScaleChosen;
    std::function<void (bool)> onShrinkChanged;
    std::function<void (bool)> onTipsChanged;
    std::function<void()> onCheckUpdates;

    juce::TextButton checkUpdatesButton;

    void show (bool aboutTab, float currentScale, bool shrinkWhileDragging);
    void refreshText();
    void setPadStatus (const juce::String& text, bool good);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (620.0f, 620.0f); }
    void selectTab (bool about);

    bool showingAbout = false;
    juce::Image artist;
    juce::TextButton settingsTab, aboutTab, closeButton;
    juce::TextButton sourceButton, kofiButton, paypalButton, spotifyButton;
    juce::Label languageLabel, sizeLabel;
    juce::ComboBox languageBox, sizeBox;
    juce::ToggleButton shrinkToggle, tipsToggle, updatesToggle, padToggle;
    juce::Label padStatus;
};

/** The browser for grooves and progressions: genres on the left, the items of that genre on the
    right (with a picture of the rhythm, or the chords in your key), a search box, and your saved
    grooves. Clicking an item plays it at once. */
class GrooveBrowser : public juce::Component, private juce::ListBoxModel
{
public:
    explicit GrooveBrowser (FaseGroovePianoProcessor&);

    enum class Mode { grooves, progressions };
    void show (Mode = Mode::grooves, int currentPreset = -1);
    /** Pick mode (the MORPH target): the same browser, but a click hands the groove to `picked`
        and closes, without changing what plays. */
    void pickGroove (int current, std::function<void (int pattern)> picked);
    std::function<void (int)> onProgressionChosen;
    std::function<void()> onImportMidi;   // progressions: "Import a MIDI file..."
    void refreshText();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;
    void visibilityChanged() override;

    /** Grooves in menu order (genre by genre), for the previous / next arrows. */
    static std::vector<int> menuOrder();

private:
    struct Entry
    {
        juce::String name, detail;
        int pattern = -1;          // groove index, or -1 for a saved groove file
        juce::File file;
        std::string right, left;   // for the rhythm picture
        int preset = -1;           // progression preset index
        juce::String chords;       // the progression in the current key
        juce::String key;          // name used for favourites and recents
    };

    // Left column: Favourites, Recent, then the genres (grooves: + My grooves, Generate).
    static constexpr int favourites = 0, recent = 1, firstCategory = 2;
    int numGenres() const;
    const std::vector<const char*>& categories() const;

    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (920.0f, 640.0f); }
    void selectGenre (int);
    void rebuildList();
    void choose (int row);
    int genreOfCurrent() const;

    // Favourites and recents are saved for all projects, by name.
    juce::StringArray loadList (const char* what) const;
    void saveList (const char* what, const juce::StringArray&) const;
    const char* favouritesKey() const { return mode == Mode::progressions ? "favProgressions" : "favGrooves"; }
    const char* recentKey() const     { return mode == Mode::progressions ? "recentProgressions" : "recentGrooves"; }
    bool isFavourite (const Entry&) const;
    void toggleFavourite (int row);

    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
    void selectedRowsChanged (int lastRowSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;
    void returnKeyPressed (int lastRowSelected) override;

    static constexpr int starWidth = 30;

    FaseGroovePianoProcessor& processor;
    juce::OwnedArray<juce::TextButton> genreButtons;
    juce::TextEditor search;
    juce::TextButton closeButton, importButton;
    juce::ListBox list;
    std::vector<Entry> entries;
    juce::StringArray favouriteNames;
    int genre = firstCategory, currentPreset = -1;
    Mode mode = Mode::grooves;
    bool quiet = false;   // selecting without playing (when the browser opens)
    std::function<void (int)> onPicked;   // pick mode
    int pickCurrent = -1;
};

/** Voicing Coach: two voicings of your progression side by side (the notes of every chord, the
    top line, range and how much the voices move), and HEAR A / HEAR B to switch while it plays. */
class VoicingCoach : public juce::Component
{
public:
    explicit VoicingCoach (FaseGroovePianoProcessor&);

    /** HEAR plays even with the DAW stopped: it starts the preview (stopped again on close). */
    void visibilityChanged() override;
    bool startedPreview = false;
    void show();
    void refreshText();
    void update();   // recompute (the progression or the register may have changed)

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (980.0f, 640.0f); }
    juce::Rectangle<float> sideArea (int side) const;
    void hear (int side);
    void paintSide (juce::Graphics&, int side, const sgk::GrooveEngine::VoicingReport&, int low, int high);

    FaseGroovePianoProcessor& processor;
    juce::ComboBox boxA, boxB;
    juce::TextButton hearA, hearB, closeButton;
    sgk::GrooveEngine::VoicingReport reportA, reportB;
};

/** The song arrangement: length of each section, and a NEW VARIATION per section (the rest stays). */
class SongOverlay : public juce::Component
{
public:
    explicit SongOverlay (FaseGroovePianoProcessor&);

    void show();
    void refreshText();
    std::function<void()> onSaveSections, onSaveSong;
    MidiDragTile songTile;   // the whole song as one clip (set up by the editor)
    /** While the song is dragged the window shrinks: only the tile is shown. */
    void setMini (bool);

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (960.0f, 640.0f); }
    juce::Rectangle<int> rowArea (int part) const;
    juce::Rectangle<float> timelineArea() const;
    void refresh();

    FaseGroovePianoProcessor& processor;
    juce::OwnedArray<juce::ComboBox> barBoxes;
    juce::OwnedArray<juce::TextButton> newButtons, originalButtons;
    juce::OwnedArray<juce::ComboBox> fillBoxes;
    juce::TextButton saveButton, saveSongButton, closeButton;
    bool mini = false;
};

/** The one button of the interface: a drawn icon, optionally a second icon and a caption.
    Off: raised and neutral; on (toggle state): filled with `onColour`. `mutedWhenOff` draws a
    slash over the icon when off (a part you don't hear / don't see). */
class IconButton : public juce::Button
{
public:
    enum class Icon { none, play, pause, stop, rewind, dice, piano, bass, pencil, arrow, eraser, magnet, quantize,
                      zoomIn, zoomOut, fit, follow, undo, redo, gear, close, fullScreen, exitFullScreen, eye, reset,
                      speaker, speakerOff };

    explicit IconButton (Icon i = Icon::none, Icon second = Icon::none) : juce::Button ({}), icon (i), icon2 (second) {}

    void setIcon (Icon i, Icon second = Icon::none)  { if (i != icon || second != icon2) { icon = i; icon2 = second; repaint(); } }
    juce::Colour onColour = Palette::accent;
    bool mutedWhenOff = false;
    bool primary = false;   // filled with onColour even when off (the main action of a panel)
    juce::Colour iconColour;  // when set: the icon's own colour (e.g. the dice of every random button)

    /** The look shared by every "random" button: neutral, with a violet dice. */
    void makeRandomButton()   { icon = Icon::dice; iconColour = Palette::fill; primary = false; repaint(); }

    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
    static void drawIcon (juce::Graphics&, Icon, juce::Rectangle<float>, juce::Colour);
    /** A mouse cursor drawn like the icon (pencil, eraser). */
    static juce::MouseCursor cursor (Icon);

private:
    Icon icon, icon2;
};
using ToolButton = IconButton;

/** The big piano roll: every note of the loop, zoomable, to edit one by one. Click an empty
    spot to add a note, drag a note to move it, drag its right edge to make it longer or
    shorter, double-click (or right-click, or Delete) to remove it; the lane at the bottom sets
    the velocity. Edited notes replace the generated ones until BACK TO GENERATED. */
class PianoRollEditor : public juce::Component, private juce::Timer, private juce::ScrollBar::Listener
{
public:
    explicit PianoRollEditor (FaseGroovePianoProcessor&);
    ~PianoRollEditor() override;

    /** Opens it on the loop as it plays (`generatedLength`: the loop length when nothing is edited yet). */
    void show (double generatedLength);
    void refreshText();
    std::function<std::vector<PianoRollView::ChordSpan> (double length)> chordSpans;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;
    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
    bool keyPressed (const juce::KeyPress&) override;

private:
    void timerCallback() override;
    void scrollBarMoved (juce::ScrollBar*, double) override;

    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().reduced (14.0f); }
    juce::Rectangle<float> gridArea() const;       // notes (under the chord lane)
    juce::Rectangle<float> velocityArea() const;
    juce::Rectangle<float> keyboardArea() const;
    float xOf (double beat) const;
    double beatAt (float x) const;
    float yOf (int note) const;
    int noteAt (float y) const;
    float rowHeight() const;
    int hitNote (juce::Point<float>, bool& onRightEdge) const;
    double snapped (double beat) const;
    void setZoom (double pixelsPerBeat, float anchorX);
    void updateScrollBar();
    void fitRange();
    void commit();                                 // sends the notes to the processor
    void updateStatus();

    FaseGroovePianoProcessor& processor;
    std::vector<sgk::LoopNote> notes;
    std::vector<PianoRollView::ChordSpan> chords;
    double length = 8.0, pixelsPerBeat = 100.0, scroll = 0.0, snap = 0.25, playhead = -1.0;
    bool snapOn = false;   // free movement by default; SNAP sticks to the grid (Alt while dragging: the other way)
    bool snapping (const juce::MouseEvent&) const;
    void quantize();
    int lowNote = 36, highNote = 84;
    int selected = -1;
    sgk::Part drawPart = sgk::Part::chords;

    enum class Tool { draw, select, erase };
    Tool tool = Tool::draw;
    void setTool (Tool);
    void updateCursor (const juce::MouseEvent&);
    bool follow = true;                            // the view follows the playhead
    int shown = 0;                                 // 0 everything, 1 piano only, 2 bass only
    bool isShown (const sgk::LoopNote& n) const    { return shown == 0 || (shown == 2) == (n.part == sgk::Part::bass); }

    // Selection: several notes at once (rubber-band rectangle, Ctrl / Shift + click, Ctrl + A).
    std::vector<char> picked;                      // one flag per note, in the same order
    bool isPicked (int i) const                    { return juce::isPositiveAndBelow (i, (int) picked.size()) && picked[(size_t) i] != 0; }
    int numPicked() const;
    void clearPicks();
    void syncPicks()                               { picked.resize (notes.size(), 0); }
    void eraseWhere (std::function<bool (int)>);
    void sortNotes();
    juce::Rectangle<float> marquee;
    std::vector<char> marqueeBase;
    std::vector<sgk::LoopNote> dragOrigin;

    void setListening (int which);                 // 0 both, 1 piano, 2 bass (what you hear)
    int listening() const;

    enum class Drag { none, move, resize, velocity, erase, marquee };
    Drag drag = Drag::none;
    double dragStartBeat = 0.0, noteStartBeat = 0.0, noteStartLength = 0.0;
    int dragStartNote = 0, noteStartPitch = 0;
    bool changed = false;

    FaseGroovePianoProcessor::EditedLoop lastLoop;   // to notice an undo / a preset while open
    juce::ScrollBar scrollBar { false };
    // Toolbar, in groups: transport | tools | draws | grid | view | hear | zoom.
    IconButton rewindButton { IconButton::Icon::rewind }, playButton { IconButton::Icon::play }, stopButton { IconButton::Icon::stop };
    IconButton pianoPartButton { IconButton::Icon::piano }, bassPartButton { IconButton::Icon::bass };
    IconButton snapToggle { IconButton::Icon::magnet }, quantizeButton { IconButton::Icon::quantize };
    juce::TextButton snapButton;
    IconButton showPianoButton { IconButton::Icon::eye, IconButton::Icon::piano }, showBassButton { IconButton::Icon::eye, IconButton::Icon::bass };
    IconButton hearPianoButton { IconButton::Icon::speaker, IconButton::Icon::piano }, hearBassButton { IconButton::Icon::speaker, IconButton::Icon::bass };
    IconButton zoomOut { IconButton::Icon::zoomOut }, zoomIn { IconButton::Icon::zoomIn }, fitButton { IconButton::Icon::fit },
               followButton { IconButton::Icon::follow };
    IconButton generatedButton { IconButton::Icon::reset }, closeButton { IconButton::Icon::close };
    juce::TextButton helpButton;
    std::vector<std::pair<juce::Rectangle<int>, juce::String>> groupCaptions;   // set in resized()
    IconButton drawTool { IconButton::Icon::pencil }, selectTool { IconButton::Icon::arrow }, eraseTool { IconButton::Icon::eraser };
    juce::String status;
};

/** Groove morphing: from the groove you have towards another one, step by step, with a slider. */
class MorphOverlay : public juce::Component
{
public:
    explicit MorphOverlay (FaseGroovePianoProcessor&);

    void show();
    /** Opens the groove browser to choose the target (set by the editor). */
    std::function<void (int current, std::function<void (int)> picked)> onChooseTarget;
    void refreshText();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<float> card() const;
    void apply();

    FaseGroovePianoProcessor& processor;
    juce::TextButton targetButton;
    juce::ArrowButton targetPrev { "previous target", 0.5f, Palette::dim }, targetNext { "next target", 0.0f, Palette::dim };
    int targetPattern = -1;
    void setTarget (int pattern);
    juce::Slider amount;
    juce::TextButton keepButton, cancelButton;
    juce::String before, fromName;
};

/** Chord Variation Lab: alternatives of your progression (same length, so the rhythm stays) to
    hear one after the other while the loop plays; KEEP the one you like or CANCEL. */
class ChordLab : public juce::Component
{
public:
    explicit ChordLab (FaseGroovePianoProcessor&);

    /** HEAR plays even with the DAW stopped: it starts the preview (stopped again on close). */
    void visibilityChanged() override;
    bool startedPreview = false;
    void show();
    void refreshText();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (820.0f, 430.0f).withY (110.0f); }
    juce::Rectangle<int> rowArea (int row) const;
    juce::String namesOf (const std::vector<int>& codes) const;
    void hear (int row);

    FaseGroovePianoProcessor& processor;
    std::vector<std::pair<juce::String, std::vector<int>>> rows;   // Original first
    juce::OwnedArray<juce::TextButton> hearButtons;
    juce::TextButton keepButton, cancelButton;
    juce::String before;
    int playing = 0;
};

/** Musical DNA: one click = one step towards an intention (more soulful, funkier...). */
class DnaOverlay : public juce::Component
{
public:
    explicit DnaOverlay (FaseGroovePianoProcessor&);

    void show();
    void refreshText();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    juce::Rectangle<float> card() const;

    FaseGroovePianoProcessor& processor;
    juce::OwnedArray<juce::TextButton> intentions;
    juce::ToggleButton keepRhythm, keepHarmony;
    juce::TextButton closeButton;
    int steps = 0;
};

/** "A new version is ready": what's new (the changelog) and the user confirms the install. */
class UpdateOverlay : public juce::Component
{
public:
    UpdateOverlay();

    void show (const Updater::Result&);
    void refreshText();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (640.0f, 560.0f); }

    Updater::Result result;
    juce::TextEditor notes;
    juce::TextButton installButton, laterButton, skipButton;
};

//==============================================================================
class FaseGroovePianoEditor : public juce::AudioProcessorEditor, public juce::Timer, public juce::FileDragAndDropTarget
{
public:
    explicit FaseGroovePianoEditor (FaseGroovePianoProcessor&);
    ~FaseGroovePianoEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Mini window shown while a clip is being dragged to the DAW. */
    void enterMiniMode (juce::Component* keep = nullptr);   // keep: the component being dragged from (default: the clip tile)
    void exitMiniMode();

private:
    void timerCallback() override;
    void loadPreset (int index);
    void layoutFull();

    // Simple view: the panels show their main controls; ADVANCED (in each panel's header) shows all.
    std::array<juce::TextButton, 3> advancedButtons;   // harmony, groove, sound
    void applyViewMode();
    std::vector<juce::Component*> advancedOnly() ;
    void layoutMini();
    void cellClicked (bool bassRow, int step, const juce::MouseEvent&);
    void drawPanel (juce::Graphics&, juce::Rectangle<int> area, const juce::String& title);

    FaseGroovePianoProcessor& processor;
    GrooveLookAndFeel lookAndFeel;
    std::unique_ptr<juce::TooltipWindow> tooltips;
    void applyTips();
    void translateHostWindow();
    void parentHierarchyChanged() override;

    IconButton optionsButton { IconButton::Icon::gear };
    juce::TextButton styleMenuButton { "STYLE" }, resetButton { "RESET" };
    juce::TextButton presetsButton { "PRESETS" }, learnButton { "LEARN" };
    IconButton totalRandomButton { IconButton::Icon::dice };
    IconButton undoButton { IconButton::Icon::undo }, redoButton { IconButton::Icon::redo };
    void showPresetMenu();
    void savePresetAs();
    void showGrooveMenu();
    void saveGrooveAs();

    // MIDI harmony analyzer: drop a .mid on the window, or "Import a MIDI file..." in the browser.
    bool isInterestedInFileDrag (const juce::StringArray&) override;
    void filesDropped (const juce::StringArray&, int, int) override;
    void chooseMidiToImport();
    void importMidi (const juce::File&);
    void showSaveMenu();
    void saveAllParts();
    void undo();
    void redo();
    void afterRestore();
    bool keyPressed (const juce::KeyPress&) override;
    juce::StringArray undoStack, redoStack;
    juce::String lastSnapshot;
    int snapshotTicks = 0, lastLearnCount = 0;
    void showStyleMenu();
    void confirmReset();
    void afterReset();
    OptionsOverlay options;
    GrooveBrowser grooveBrowser { processor };
    VoicingCoach voicingCoach { processor };
    SongOverlay songOverlay { processor };
    MorphOverlay morphOverlay { processor };
    DnaOverlay dnaOverlay { processor };
    ChordLab chordLab { processor };
    juce::TextButton songButton { "SONG" }, morphButton { "MORPH" }, dnaButton { "DNA" }, variantsButton { "VARIANTS" };
    juce::TextButton feelStyleButton, newFeelButton;
    juce::TextButton coachButton { "A/B" };
    juce::ArrowButton groovePrev { "previous groove", 0.5f, Palette::dim }, grooveNext { "next groove", 0.0f, Palette::dim };
    void stepGroove (int direction);
    UpdateOverlay updateOverlay;
    Updater::Checker updateChecker;

    void applyLanguage();
    void buildPresetMenu();
    void mouseUp (const juce::MouseEvent&) override;

    // Progression
    juce::Label presetLabel;
    Choice::PopupBox presetBox;        // holds the chosen preset (its list opens the browser)
    juce::TextButton presetBrowseButton;   // what you see: the preset's name, or BROWSE
    IconButton diceProgression { IconButton::Icon::dice };
    Choice progLength, chordLength;
    juce::OwnedArray<ChordSlot> slots;

    // Harmony
    Choice key, scale, extension, voicing, bass, approach, topLine, movement;
    Knob registerKnob, strumKnob;
    juce::TextButton strumDirButton;
    juce::ToggleButton octaveToggle { "Top +8va" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> octaveAttachment;
    CircleOfFifths circle;
    juce::ToggleButton freeToggle { "Free" };

    // Groove
    Choice pattern, section, fills, fillStyle, playMode, variation, grooveBars;
    IconButton newRiffButton { IconButton::Icon::dice };
    Knob swing, gate, velocity, humanize, riffDensity;

    // Sound & MIDI
    juce::ToggleButton soundToggle { "Guide sound" };
    juce::TextButton previewButton { "PREVIEW" };
    IconButton rewindButton { IconButton::Icon::rewind }, playButton { IconButton::Icon::play }, stopButton { IconButton::Icon::stop };
    Choice bassChannel, soundType;
    juce::AudioProcessorValueTreeState::ButtonAttachment soundAttachment;
    Knob tone, reverb, volume, chorus, pump, pumpRelease;
    Choice pumpSource, pumpRate;
    SidechainMeter sidechainMeter;

    PatternView patternView;
    VelocityView velocityView;
    juce::TextButton velocityTab;
    bool velocityShown = false;
    void showVelocity (bool);
    void updateRandomTips();
    void updateVelocityView (const sgk::GrooveSettings&);
    PianoRollView pianoRoll;
    PianoRollEditor rollEditor { processor };
    juce::TextButton rollTab { "PIANO ROLL" }, gridTab { "EDIT GRID" }, expandRollButton, generatedButton;
    std::vector<PianoRollView::ChordSpan> chordSpansFor (const sgk::GrooveSettings&, double length) const;
    void showRoll (bool);
    bool rollShown = true;
    double rollLength = 8.0;
    void updatePianoRoll (const sgk::GrooveSettings&);
    // What you hear: the piano and / or the bass (lit = you hear it).
    IconButton hearPianoButton { IconButton::Icon::speaker }, hearBassButton { IconButton::Icon::speaker };
    void hearPart (bool bass);
    std::vector<std::pair<juce::Rectangle<int>, juce::String>> gridCaptions;   // captions over the groups of the grid header
    IconButton rndPianoButton { IconButton::Icon::dice }, rndBassButton { IconButton::Icon::dice };
    juce::TextButton myGroovesButton { "MY GROOVES" };
    juce::OwnedArray<juce::ComboBox> segmentBoxes;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> segmentAttachments;
    juce::OwnedArray<IconButton> segmentDice;
    juce::OwnedArray<juce::TextButton> segmentLocks;
    // The same strip on the velocity page: the levels' own A-D order, random levels, locks.
    juce::OwnedArray<juce::ComboBox> velocityBoxes;
    juce::OwnedArray<IconButton> velocityDice;
    juce::OwnedArray<juce::TextButton> velocityLocks;
    bool onVelocityPage() const { return velocityShown && ! rollShown; }

    // Scaling (the whole UI is laid out at base size, then scaled) and mini mode.
    static constexpr int baseWidth = 1180, baseHeight = 860;
    static constexpr int miniWidth = 320, miniHeight = 96;
    float uiScale = 1.0f;
    juce::Point<float> uiOffset;
    juce::AffineTransform uiTransform() const { return juce::AffineTransform::scale (uiScale).translated (uiOffset); }
    IconButton fullScreenButton { IconButton::Icon::fullScreen };
    bool isFullScreen() const;
    void toggleFullScreen();
    juce::Rectangle<int> sizeBeforeFullScreen;
    bool miniMode = false;
    juce::Point<int> fullSize;
    juce::Component* miniKeep = nullptr;
    std::vector<juce::Component::SafePointer<juce::Component>> hiddenForMini;
    juce::uint32 miniStartedAt = 0;

    // Tempo
    juce::TextButton syncButton { "SYNC DAW" };
    juce::ToggleButton sequencerToggle { "Sequencer" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sequencerAttachment;
    juce::Slider tempoSlider;
    juce::Label dawTempo;
    juce::AudioProcessorValueTreeState::ButtonAttachment syncAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment tempoAttachment;

    // MIDI clip export
    juce::Label clipLengthLabel, clipPartLabel;
    juce::ComboBox clipLength, clipPart;
    MidiDragTile dragTile;
    juce::TextButton saveButton { "Save .mid..." };
    juce::ToggleButton shrinkToggle { "Mini window while dragging" };
    std::unique_ptr<juce::FileChooser> fileChooser;

    int selectedClipBars() const;
    FaseGroovePianoProcessor::ClipPart selectedClipPart() const;
    void saveClip();

    juce::Rectangle<int> headerArea, tempoArea, chordDisplayArea, progressionPanel, harmonyPanel, circlePanel, groovePanel,
                         soundPanel, gridPanel, clipPanel, feelHeader;

    juce::String currentChord { "-" }, currentRoman, currentKey;
    bool showingHold = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaseGroovePianoEditor)
};
