// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "Branding.h"
#include "Localisation.h"
#include "CircleOfFifths.h"
#include "PluginProcessor.h"

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

    juce::ComboBox box;

private:
    juce::String caption;
    juce::StringArray englishItems;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

/** One chord of the progression: degree selector + resulting chord name. */
class ChordSlot : public juce::Component
{
public:
    ChordSlot (juce::AudioProcessorValueTreeState&, int index);

    /** Called when the user picks a degree from the menu (it replaces a free chord). */
    std::function<void()> onDegreePicked;

    /** isFree: the slot plays a free chord (from the circle), shown instead of a degree. */
    void setInfo (const juce::String& chordName, const juce::String& roman, bool enabled, bool playing,
                  bool isFree, int degreeValue);
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
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    juce::String name, numeral;
    bool isEnabledSlot = true, isPlaying = false;
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
class MidiDragTile : public juce::Component
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
    juce::ToggleButton shrinkToggle;
};

//==============================================================================
class FaseGroovePianoEditor : public juce::AudioProcessorEditor, private juce::Timer
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
    juce::TooltipWindow tooltips { this, 600 };

    juce::TextButton optionsButton { "OPTIONS" };
    OptionsOverlay options;

    void applyLanguage();
    void buildPresetMenu();
    void mouseUp (const juce::MouseEvent&) override;

    // Progression
    juce::Label presetLabel;
    juce::ComboBox presetBox;
    juce::TextButton diceProgression { "DICE" };
    Choice progLength, chordLength;
    juce::OwnedArray<ChordSlot> slots;

    // Harmony
    Choice key, scale, extension, voicing, bass, approach;
    Knob registerKnob, strumKnob;
    juce::ToggleButton octaveToggle { "Top +8va" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> octaveAttachment;
    CircleOfFifths circle;
    juce::ToggleButton freeToggle { "Free" };

    // Groove
    Choice pattern, section, fills, fillStyle, playMode;
    juce::TextButton newRiffButton { "NEW RIFF" };
    Knob swing, gate, velocity, humanize, riffDensity;

    // Sound & MIDI
    juce::ToggleButton soundToggle { "Guide sound" };
    juce::TextButton previewButton { "PREVIEW" };
    Choice bassChannel, soundType;
    juce::AudioProcessorValueTreeState::ButtonAttachment soundAttachment;
    Knob tone, reverb, volume;

    PatternView patternView;
    juce::TextButton bassButton { "BASS" };
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
