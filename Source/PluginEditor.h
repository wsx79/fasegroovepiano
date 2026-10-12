// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include "Branding.h"
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

private:
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

    juce::ComboBox box;

private:
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};

/** One chord of the progression: degree selector + resulting chord name. */
class ChordSlot : public juce::Component
{
public:
    ChordSlot (juce::AudioProcessorValueTreeState&, int index);

    void setInfo (const juce::String& chordName, const juce::String& roman, bool enabled, bool playing);
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    int index;
    juce::ComboBox degree;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    juce::String name, numeral;
    bool isEnabledSlot = true, isPlaying = false;
};

/** Shows the two-hand rhythm of the selected groove and the playhead. */
class PatternView : public juce::Component
{
public:
    void setState (const sgk::GroovePattern& pattern, int step);
    void paint (juce::Graphics&) override;

private:
    sgk::GroovePattern pattern;
    int currentStep = -1;
};

/** Drag this tile into the DAW to drop the current groove as a MIDI clip. */
class MidiDragTile : public juce::Component
{
public:
    std::function<juce::File()> createFile;

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override { hover = true; repaint(); }
    void mouseExit (const juce::MouseEvent&) override  { hover = false; repaint(); }
    void mouseDown (const juce::MouseEvent&) override  { dragStarted = false; }
    void mouseDrag (const juce::MouseEvent&) override;

private:
    bool hover = false, dragStarted = false;
};

/** Credits page: app logo, version, artist and code credits. Click to close. */
class AboutOverlay : public juce::Component
{
public:
    AboutOverlay();
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent&) override { setVisible (false); }

private:
    juce::Rectangle<float> card() const { return getLocalBounds().toFloat().withSizeKeepingCentre (620.0f, 560.0f); }

    juce::Image artist;
    juce::TextButton sourceButton { "SOURCE CODE" }, donateButton { "DONATE (Ko-fi)" };
};

//==============================================================================
class FaseGroovePianoEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit FaseGroovePianoEditor (FaseGroovePianoProcessor&);
    ~FaseGroovePianoEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void loadPreset (int index);
    void drawPanel (juce::Graphics&, juce::Rectangle<int> area, const juce::String& title);

    FaseGroovePianoProcessor& processor;
    GrooveLookAndFeel lookAndFeel;
    juce::TooltipWindow tooltips { this, 600 };

    juce::TextButton creditsButton { "CREDITS" };
    AboutOverlay about;

    // Progression
    juce::Label presetLabel;
    juce::ComboBox presetBox;
    juce::TextButton diceProgression { "DICE" };
    Choice progLength, chordLength;
    juce::OwnedArray<ChordSlot> slots;

    // Harmony
    Choice key, scale, extension, voicing, bass;
    Knob registerKnob;
    CircleOfFifths circle;

    // Groove
    Choice pattern, section, fills, playMode;
    juce::TextButton newRiffButton { "NEW RIFF" };
    Knob swing, gate, velocity, humanize, riffDensity;

    // Sound & MIDI
    juce::ToggleButton soundToggle { "Guide sound" };
    juce::TextButton previewButton { "PREVIEW" };
    Choice bassChannel;
    juce::AudioProcessorValueTreeState::ButtonAttachment soundAttachment;
    Knob tone, reverb, volume;

    PatternView patternView;

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
