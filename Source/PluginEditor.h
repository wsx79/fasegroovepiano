// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "Branding.h"
#include "Localisation.h"
#include "CircleOfFifths.h"
#include "PluginProcessor.h"
#include "Updater.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace Palette
{
    const juce::Colour background { 0xff17111c };
    const juce::Colour panel      { 0xff231a2b };
    const juce::Colour panelEdge  { 0xff3b2c46 };
    const juce::Colour accent     { 0xfff2a65a };   // warm orange
    const juce::Colour accent2    { 0xffe85d75 };   // soulful pink
    const juce::Colour text       { 0xfff4e9dc };
    const juce::Colour dim        { 0xff9c8ca8 };
}

//==============================================================================
class GrooveLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GrooveLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
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
        void showPopup() override { if (onPopup != nullptr) onPopup(); else juce::ComboBox::showPopup(); }
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
    juce::ArrowButton invertUp { "up", 0.75f, Palette::accent }, invertDown { "down", 0.25f, Palette::accent };
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

    void setState (const sgk::GroovePattern& pattern, int step, bool bassOn);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    bool cellAt (juce::Point<float>, bool& bassRow, int& step) const;

    sgk::GroovePattern pattern;
    int currentStep = -1;
    bool bassEnabled = true;
    int hoverStep = -1;
    bool hoverBass = false;
};

/** Drag this tile into the DAW to drop the current groove as a MIDI clip. */
class MidiDragTile : public juce::Component, public juce::SettableTooltipClient
{
public:
    std::function<juce::File()> createFile;
    std::function<void()> onDragStart, onDragEnd;

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override  { hover = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override  { dragStarted = false; }
    void mouseDrag (const juce::MouseEvent&) override;

private:
    bool hover = false, dragStarted = false;
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
    juce::ToggleButton shrinkToggle, tipsToggle, updatesToggle;
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
    std::function<void (int)> onProgressionChosen;
    void refreshText();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override;
    bool keyPressed (const juce::KeyPress&) override;

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
    };
    int numGenres() const;

    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (920.0f, 640.0f); }
    void selectGenre (int);
    void rebuildList();
    void choose (int row);
    int genreOfCurrent() const;

    int getNumRows() override;
    void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool selected) override;
    void selectedRowsChanged (int lastRowSelected) override;
    void listBoxItemDoubleClicked (int row, const juce::MouseEvent&) override;
    void returnKeyPressed (int lastRowSelected) override;

    FaseGroovePianoProcessor& processor;
    juce::OwnedArray<juce::TextButton> genreButtons;
    juce::TextEditor search;
    juce::TextButton closeButton;
    juce::ListBox list;
    std::vector<Entry> entries;
    int genre = 0, currentPreset = -1;
    Mode mode = Mode::grooves;
    bool quiet = false;   // selecting without playing (when the browser opens)
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
class FaseGroovePianoEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit FaseGroovePianoEditor (FaseGroovePianoProcessor&);
    ~FaseGroovePianoEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Mini window shown while a clip is being dragged to the DAW. */
    void enterMiniMode();
    void exitMiniMode();

private:
    void timerCallback() override;
    void loadPreset (int index);
    void layoutFull();
    void layoutMini();
    void cellClicked (bool bassRow, int step, const juce::MouseEvent&);
    void drawPanel (juce::Graphics&, juce::Rectangle<int> area, const juce::String& title);

    FaseGroovePianoProcessor& processor;
    GrooveLookAndFeel lookAndFeel;
    std::unique_ptr<juce::TooltipWindow> tooltips;
    void applyTips();
    void translateHostWindow();
    void parentHierarchyChanged() override;

    juce::TextButton optionsButton { "OPTIONS" };
    juce::TextButton styleMenuButton { "STYLE" }, resetButton { "RESET" };
    juce::TextButton presetsButton { "PRESETS" }, learnButton { "LEARN" };
    juce::ArrowButton undoButton { "undo", 0.5f, Palette::accent }, redoButton { "redo", 0.0f, Palette::accent };
    void showPresetMenu();
    void savePresetAs();
    void showGrooveMenu();
    void saveGrooveAs();
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
    juce::ArrowButton groovePrev { "previous groove", 0.5f, Palette::accent }, grooveNext { "next groove", 0.0f, Palette::accent };
    void stepGroove (int direction);
    UpdateOverlay updateOverlay;
    Updater::Checker updateChecker;

    void applyLanguage();
    void buildPresetMenu();
    void mouseUp (const juce::MouseEvent&) override;

    // Progression
    juce::Label presetLabel;
    Choice::PopupBox presetBox;
    juce::TextButton diceProgression { "DICE" };
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
    juce::TextButton newRiffButton { "NEW RIFF" };
    Knob swing, gate, velocity, humanize, riffDensity;

    // Sound & MIDI
    juce::ToggleButton soundToggle { "Guide sound" };
    juce::TextButton previewButton { "PREVIEW" };
    Choice bassChannel, soundType;
    juce::AudioProcessorValueTreeState::ButtonAttachment soundAttachment;
    Knob tone, reverb, volume, chorus, pump;

    PatternView patternView;
    juce::TextButton bassButton { "SOLO PIANO" }, soloBassButton { "SOLO BASS" };
    juce::TextButton rndPianoButton { "RND PIANO" }, rndBassButton { "RND BASS" }, myGroovesButton { "MY GROOVES" };
    juce::OwnedArray<juce::ComboBox> segmentBoxes;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::ComboBoxAttachment> segmentAttachments;
    juce::OwnedArray<juce::TextButton> segmentDice, segmentLocks;

    // Scaling (the whole UI is laid out at base size, then scaled) and mini mode.
    static constexpr int baseWidth = 1180, baseHeight = 780;
    static constexpr int miniWidth = 320, miniHeight = 96;
    float uiScale = 1.0f;
    bool miniMode = false;
    juce::Point<int> fullSize;
    std::vector<juce::Component::SafePointer<juce::Component>> hiddenForMini;
    juce::uint32 miniStartedAt = 0;

    // Tempo
    juce::TextButton syncButton { "SYNC DAW" };
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
                         soundPanel, gridPanel, clipPanel;

    juce::String currentChord { "-" }, currentRoman, currentKey;
    bool showingHold = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FaseGroovePianoEditor)
};
