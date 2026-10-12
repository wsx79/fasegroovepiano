// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginEditor.h"

namespace
{
    juce::Font font (float height, bool bold = false)
    {
        return juce::Font (juce::FontOptions (height).withStyle (bold ? "Bold" : "Regular"));
    }

    void styleCaption (juce::Label& label, const juce::String& text)
    {
        label.setText (text.toUpperCase(), juce::dontSendNotification);
        label.setFont (font (11.0f, true));
        label.setColour (juce::Label::textColourId, Palette::dim);
        label.setJustificationType (juce::Justification::centredLeft);
        label.setInterceptsMouseClicks (false, false);
    }

    void fillChoiceItems (juce::ComboBox& box, juce::RangedAudioParameter* param)
    {
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (param))
        {
            box.addItemList (c->choices, 1);
        }
        else if (auto* i = dynamic_cast<juce::AudioParameterInt*> (param))
        {
            for (int v = i->getRange().getStart(); v <= i->getRange().getEnd(); ++v)
                box.addItem (juce::String (v) + (v == 1 ? " chord" : " chords"), v - i->getRange().getStart() + 1);
        }
    }
}

//==============================================================================
GrooveLookAndFeel::GrooveLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Palette::background);
    setColour (juce::ComboBox::backgroundColourId, Palette::background);
    setColour (juce::ComboBox::outlineColourId, Palette::panelEdge);
    setColour (juce::ComboBox::textColourId, Palette::text);
    setColour (juce::ComboBox::arrowColourId, Palette::accent);
    setColour (juce::ComboBox::focusedOutlineColourId, Palette::accent);
    setColour (juce::PopupMenu::backgroundColourId, Palette::panel);
    setColour (juce::PopupMenu::textColourId, Palette::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::accent.withAlpha (0.85f));
    setColour (juce::PopupMenu::highlightedTextColourId, Palette::background);
    setColour (juce::Slider::textBoxTextColourId, Palette::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, Palette::text);
    setColour (juce::ToggleButton::textColourId, Palette::text);
}

void GrooveLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                                          float startAngle, float endAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float lineW = juce::jmax (2.5f, radius * 0.16f);
    const float arcR = radius - lineW * 0.5f;

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (Palette::panelEdge);
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setGradientFill (juce::ColourGradient (Palette::accent2, bounds.getBottomLeft(), Palette::accent, bounds.getTopRight(), false));
    g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float knobR = arcR - lineW * 1.3f;
    g.setColour (Palette::panel.brighter (0.25f));
    g.fillEllipse (centre.x - knobR, centre.y - knobR, knobR * 2.0f, knobR * 2.0f);

    const auto tip = centre.getPointOnCircumference (knobR * 0.75f, angle);
    g.setColour (Palette::text);
    g.drawLine ({ centre.getPointOnCircumference (knobR * 0.25f, angle), tip }, 2.2f);
}

void GrooveLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat();
    auto pill = r.removeFromLeft (40.0f).withSizeKeepingCentre (36.0f, 18.0f);
    const bool on = b.getToggleState();

    g.setColour (on ? Palette::accent : Palette::panelEdge);
    g.fillRoundedRectangle (pill, 9.0f);

    const float d = 14.0f;
    const float cx = on ? pill.getRight() - d - 2.0f : pill.getX() + 2.0f;
    g.setColour (highlighted ? Palette::text : Palette::text.withAlpha (0.9f));
    g.fillEllipse (cx, pill.getCentreY() - d * 0.5f, d, d);

    g.setColour (Palette::text);
    g.setFont (font (14.0f));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (6.0f), juce::Justification::centredLeft);
}

//==============================================================================
Knob::Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& caption)
    : attachment (state, paramId, slider)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, Palette::text);
    addAndMakeVisible (slider);

    styleCaption (label, caption);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (16));
    slider.setBounds (r);
}

Choice::Choice (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& caption)
{
    fillChoiceItems (box, state.getParameter (paramId));
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, paramId, box);
    addAndMakeVisible (box);

    styleCaption (label, caption);
    addAndMakeVisible (label);
}

void Choice::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (16));
    box.setBounds (r.removeFromTop (28));
}

//==============================================================================
ChordSlot::ChordSlot (juce::AudioProcessorValueTreeState& state, int i) : index (i)
{
    const auto id = ParamIDs::slot (i);
    fillChoiceItems (degree, state.getParameter (id));
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, id, degree);
    degree.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (degree);
}

void ChordSlot::setInfo (const juce::String& chordName, const juce::String& roman, bool enabled, bool playing)
{
    if (chordName == name && roman == numeral && enabled == isEnabledSlot && playing == isPlaying)
        return;

    name = chordName;
    numeral = roman;
    isEnabledSlot = enabled;
    isPlaying = playing;
    degree.setAlpha (enabled ? 1.0f : 0.4f);
    repaint();
}

void ChordSlot::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    const float alpha = isEnabledSlot ? 1.0f : 0.35f;

    if (isPlaying)
    {
        g.setGradientFill (juce::ColourGradient (Palette::accent2.withAlpha (0.35f), r.getTopLeft(),
                                                 Palette::accent.withAlpha (0.25f), r.getBottomRight(), false));
        g.fillRoundedRectangle (r, 8.0f);
    }

    g.setColour ((isPlaying ? Palette::accent : Palette::panelEdge).withMultipliedAlpha (alpha));
    g.drawRoundedRectangle (r, 8.0f, isPlaying ? 2.0f : 1.0f);

    auto text = getLocalBounds().reduced (4, 6).withTrimmedBottom (34);
    g.setColour (Palette::dim.withMultipliedAlpha (alpha));
    g.setFont (font (11.0f, true));
    g.drawText (juce::String (index + 1) + "  " + numeral, text.removeFromTop (14), juce::Justification::centred);

    g.setColour (Palette::text.withMultipliedAlpha (alpha));
    g.setFont (font (name.length() > 7 ? 14.0f : 17.0f, true));
    g.drawFittedText (name, text, juce::Justification::centred, 1, 0.7f);
}

void ChordSlot::resized()
{
    degree.setBounds (getLocalBounds().reduced (6, 6).removeFromBottom (26));
}

//==============================================================================
void PatternView::setState (const sgk::GroovePattern& newPattern, int step)
{
    if (step == currentStep && newPattern.rightHand == pattern.rightHand && newPattern.leftHand == pattern.leftHand)
        return;

    pattern = newPattern;
    currentStep = step;
    repaint();
}

void PatternView::paint (juce::Graphics& g)
{
    const auto& p = pattern;
    const int len = p.length();
    if (len == 0)
        return;

    auto area = getLocalBounds().toFloat();
    const float labelW = 44.0f;
    const float rowH = (area.getHeight() - 16.0f) / 2.0f;
    const float cellW = (area.getWidth() - labelW) / (float) len;

    g.setFont (font (11.0f, true));

    // Beat numbers
    for (int s = 0; s < len; s += 4)
    {
        g.setColour (s % 16 == 0 ? Palette::accent : Palette::dim);
        g.drawText (juce::String ((s / 4) % 4 + 1), juce::Rectangle<float> (labelW + s * cellW, 0.0f, cellW * 2.0f, 14.0f),
                    juce::Justification::centredLeft);
    }

    auto drawRow = [&] (const std::string& line, float y, const juce::String& caption, juce::Colour colour)
    {
        g.setColour (Palette::dim);
        g.drawText (caption, juce::Rectangle<float> (0.0f, y, labelW, rowH), juce::Justification::centredLeft);

        for (int s = 0; s < len; ++s)
        {
            auto cell = juce::Rectangle<float> (labelW + s * cellW, y, cellW, rowH).reduced (1.5f, 3.0f);
            const char c = line[(size_t) s];

            g.setColour (Palette::panelEdge.withAlpha (s % 4 == 0 ? 0.9f : 0.45f));
            g.fillRoundedRectangle (cell, 3.0f);

            if (c == 'X' || c == 'x' || c == 'g' || c == '>')
            {
                g.setColour (c == '>' ? Palette::accent2 : colour.withAlpha (c == 'X' ? 1.0f : c == 'g' ? 0.35f : 0.75f));
                g.fillRoundedRectangle (cell, 3.0f);
            }
            else if (c == '=')
            {
                g.setColour (colour.withAlpha (0.35f));
                g.fillRoundedRectangle (cell.withSizeKeepingCentre (cell.getWidth() + 3.0f, cell.getHeight() * 0.45f), 2.0f);
            }

            if (s == currentStep)
            {
                g.setColour (Palette::text);
                g.drawRoundedRectangle (cell.expanded (1.0f), 3.0f, 1.5f);
            }
        }
    };

    drawRow (p.rightHand, 16.0f, "CHORDS", Palette::accent);
    drawRow (p.leftHand, 16.0f + rowH, "BASS", Palette::accent2.brighter (0.2f));
}

//==============================================================================
AboutOverlay::AboutOverlay() : artist (Branding::artistLogo())
{
    setInterceptsMouseClicks (true, true);

    sourceButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    sourceButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    sourceButton.onClick = [] { juce::URL (Branding::sourceUrl).launchInDefaultBrowser(); };
    addAndMakeVisible (sourceButton);

    donateButton.setColour (juce::TextButton::buttonColourId, Palette::accent2);
    donateButton.setColour (juce::TextButton::textColourOffId, Palette::background);
    donateButton.onClick = [] { juce::URL (Branding::donateUrl).launchInDefaultBrowser(); };
    addChildComponent (donateButton);
    donateButton.setVisible (juce::String (Branding::donateUrl).isNotEmpty());
}

void AboutOverlay::resized()
{
    auto row = card().toNearestInt().reduced (36, 28).removeFromBottom (58).removeFromTop (30);
    const bool donate = donateButton.isVisible();
    const int w = 150, total = donate ? 2 * w + 12 : w;
    row = row.withSizeKeepingCentre (total, row.getHeight());
    sourceButton.setBounds (row.removeFromLeft (w));
    if (donate)
        donateButton.setBounds (row.removeFromRight (w));
}

void AboutOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));

    auto card = this->card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (card, 16.0f);
    g.setColour (Palette::accent.withAlpha (0.6f));
    g.drawRoundedRectangle (card, 16.0f, 1.5f);

    auto r = card.reduced (36.0f, 28.0f);
    Branding::drawFullLogo (g, r.removeFromTop (84.0f).withSizeKeepingCentre (400.0f, 84.0f));

    g.setColour (Palette::dim);
    g.setFont (font (13.0f));
    const auto dot = juce::String::fromUTF8 ("  \xc2\xb7  ");
    g.drawText ("Version " + juce::String (JucePlugin_VersionString) + dot + Branding::tagline,
                r.removeFromTop (30.0f), juce::Justification::centred);

    r.removeFromTop (14.0f);
    g.setColour (Palette::panelEdge);
    g.fillRect (r.removeFromTop (1.0f));
    r.removeFromTop (18.0f);

    auto caption = [&g] (const juce::String& text, juce::Rectangle<float> area)
    {
        g.setColour (Palette::accent);
        g.setFont (font (11.0f, true));
        g.drawText (text, area, juce::Justification::centred);
    };

    caption ("CREATED BY", r.removeFromTop (16.0f));
    r.removeFromTop (10.0f);
    if (artist.isValid())
        g.drawImageWithin (artist, (int) r.getX(), (int) r.getY(), (int) r.getWidth(), 130,
                           juce::RectanglePlacement::centred);
    r.removeFromTop (136.0f);

    g.setColour (Palette::text);
    g.setFont (font (14.0f));
    g.drawText ("DJ & Producer", r.removeFromTop (22.0f), juce::Justification::centred);

    r.removeFromTop (18.0f);
    caption ("CODE", r.removeFromTop (16.0f));
    r.removeFromTop (6.0f);
    g.setColour (Palette::text);
    g.setFont (font (16.0f, true));
    g.drawText ("Claude  (Anthropic)", r.removeFromTop (24.0f), juce::Justification::centred);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText ("AI coding assistant", r.removeFromTop (18.0f), juce::Justification::centred);

    g.setFont (font (11.0f));
    g.drawText ("Built with JUCE" + dot + "VST is a registered trademark of Steinberg Media Technologies GmbH"
                    + dot + "click to close",
                r.removeFromBottom (16.0f), juce::Justification::centred);

    r.removeFromBottom (44.0f);   // buttons
    g.setColour (Palette::text);
    g.setFont (font (12.0f, true));
    g.drawText (Branding::licence, r.removeFromBottom (18.0f), juce::Justification::centred);
}

//==============================================================================
void MidiDragTile::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setGradientFill (juce::ColourGradient (Palette::accent.withAlpha (hover ? 1.0f : 0.85f), r.getTopLeft(),
                                             Palette::accent2.withAlpha (hover ? 1.0f : 0.85f), r.getBottomRight(), false));
    g.fillRoundedRectangle (r, 8.0f);

    g.setColour (Palette::background);
    g.setFont (juce::Font (juce::FontOptions (15.0f).withStyle ("Bold")));
    g.drawText ("DRAG MIDI TO DAW", r, juce::Justification::centred);
}

void MidiDragTile::mouseDrag (const juce::MouseEvent& e)
{
    if (dragStarted || e.getDistanceFromDragStart() < 4 || createFile == nullptr)
        return;

    dragStarted = true;
    const auto file = createFile();
    if (file.existsAsFile())
        juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this);
}

//==============================================================================
FaseGroovePianoEditor::FaseGroovePianoEditor (FaseGroovePianoProcessor& p)
    : AudioProcessorEditor (&p), processor (p),
      progLength (p.apvts, ParamIDs::progLength, "Length"),
      chordLength (p.apvts, ParamIDs::chordLength, "Each chord"),
      key (p.apvts, ParamIDs::key, "Key"),
      scale (p.apvts, ParamIDs::scale, "Scale / mode"),
      extension (p.apvts, ParamIDs::extension, "Extensions"),
      voicing (p.apvts, ParamIDs::voicing, "Voicing"),
      bass (p.apvts, ParamIDs::bass, "Left-hand bass"),
      registerKnob (p.apvts, ParamIDs::registerPos, "Register"),
      pattern (p.apvts, ParamIDs::pattern, "Groove"),
      section (p.apvts, ParamIDs::section, "Song section"),
      fills (p.apvts, ParamIDs::fills, "Pentatonic fills"),
      playMode (p.apvts, ParamIDs::playMode, "Play mode"),
      swing (p.apvts, ParamIDs::swing, "Swing"),
      gate (p.apvts, ParamIDs::gate, "Gate"),
      velocity (p.apvts, ParamIDs::velocity, "Velocity"),
      humanize (p.apvts, ParamIDs::humanize, "Humanize"),
      riffDensity (p.apvts, ParamIDs::riffDensity, "Density"),
      bassChannel (p.apvts, ParamIDs::bassChannel, "MIDI out"),
      soundAttachment (p.apvts, ParamIDs::sound, soundToggle),
      tone (p.apvts, ParamIDs::tone, "Tone"),
      reverb (p.apvts, ParamIDs::reverb, "Reverb"),
      volume (p.apvts, ParamIDs::volume, "Volume"),
      syncAttachment (p.apvts, ParamIDs::tempoSync, syncButton),
      tempoAttachment (p.apvts, ParamIDs::tempo, tempoSlider)
{
    setLookAndFeel (&lookAndFeel);

    styleCaption (presetLabel, "Progression presets");
    addAndMakeVisible (presetLabel);

    presetBox.setTextWhenNothingSelected ("Load a progression...");
    const auto& presets = sgk::progressionPresets();
    for (int i = 0; i < (int) presets.size(); ++i)
        presetBox.addItem (presets[(size_t) i].name + juce::String ("   (") + sgk::scaleName (presets[(size_t) i].scale) + ")", i + 1);
    presetBox.onChange = [this] { loadPreset (presetBox.getSelectedItemIndex()); };
    addAndMakeVisible (presetBox);

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        addAndMakeVisible (slots.add (new ChordSlot (p.apvts, i)));

    for (juce::Component* c : std::initializer_list<juce::Component*> {
             &progLength, &chordLength, &key, &scale, &extension, &voicing, &bass, &registerKnob, &circle,
             &diceProgression, &pattern, &section, &newRiffButton, &fills, &playMode,
             &swing, &gate, &velocity, &humanize, &riffDensity,
             &soundToggle, &previewButton, &bassChannel, &tone, &reverb, &volume, &patternView,
             &clipLengthLabel, &clipPartLabel, &clipLength, &clipPart, &dragTile, &saveButton })
        addAndMakeVisible (c);

    auto styleButton = [] (juce::TextButton& b, juce::Colour colour)
    {
        b.setColour (juce::TextButton::buttonColourId, colour);
        b.setColour (juce::TextButton::textColourOffId, Palette::background);
    };

    styleButton (newRiffButton, Palette::accent);
    newRiffButton.setTooltip ("Generate a brand new random funky riff");
    newRiffButton.onClick = [this] { processor.rollNewRiff(); };

    styleButton (diceProgression, Palette::accent2);
    diceProgression.setTooltip ("Random soulful-house progression");
    diceProgression.onClick = [this] { processor.rollNewProgression(); presetBox.setSelectedId (0, juce::dontSendNotification); };

    circle.onPick = [this] (int pc, bool minorKey)
    {
        processor.setParameterFromUI (ParamIDs::key, (float) pc);

        const int currentScale = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::scale)->load());
        if (sgk::scaleIsMinor (currentScale) != minorKey)
            processor.setParameterFromUI (ParamIDs::scale, minorKey ? 1.0f : 0.0f);   // Dorian / Major
    };

    // Tempo: "SYNC DAW" on = follow the DAW; off = set the BPM by hand.
    syncButton.setClickingTogglesState (true);
    syncButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    syncButton.setColour (juce::TextButton::buttonOnColourId, Palette::accent);
    syncButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    syncButton.setColour (juce::TextButton::textColourOnId, Palette::background);
    syncButton.setTooltip ("On: follow the DAW tempo. Off: set your own BPM.");

    tempoSlider.setSliderStyle (juce::Slider::LinearBar);
    tempoSlider.setColour (juce::Slider::trackColourId, Palette::accent2.withAlpha (0.55f));
    tempoSlider.setColour (juce::Slider::backgroundColourId, Palette::background);
    tempoSlider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    tempoSlider.setTextValueSuffix ({});
    tempoSlider.setTooltip ("Drag, or double-click to type the BPM");
    tempoSlider.setDoubleClickReturnValue (true, 124.0);

    dawTempo.setColour (juce::Label::backgroundColourId, Palette::background);
    dawTempo.setColour (juce::Label::textColourId, Palette::text);
    dawTempo.setJustificationType (juce::Justification::centred);
    dawTempo.setFont (font (15.0f, true));

    for (auto* c : std::initializer_list<juce::Component*> { &syncButton, &tempoSlider, &dawTempo, &creditsButton })
        addAndMakeVisible (c);

    creditsButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    creditsButton.setColour (juce::TextButton::textColourOffId, Palette::dim);
    creditsButton.onClick = [this] { about.setVisible (true); about.toFront (false); };
    addChildComponent (about);

    previewButton.setClickingTogglesState (true);
    previewButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    previewButton.setColour (juce::TextButton::buttonOnColourId, Palette::accent2);
    previewButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    previewButton.setColour (juce::TextButton::textColourOnId, Palette::background);
    previewButton.setTooltip ("Play the groove with the guide sound without starting the DAW");
    previewButton.onClick = [this] { processor.previewOn = previewButton.getToggleState(); };

    styleCaption (clipLengthLabel, "Clip length");
    styleCaption (clipPartLabel, "Notes");
    clipLength.addItemList ({ "Full loop", "1 bar", "2 bars", "4 bars", "8 bars", "16 bars", "32 bars",
                              "Song: intro > drop > outro" }, 1);
    clipLength.setSelectedId (1, juce::dontSendNotification);
    clipPart.addItemList ({ "Chords + bass", "Chords only", "Bass only" }, 1);
    clipPart.setSelectedId (1, juce::dontSendNotification);

    dragTile.createFile = [this] { return processor.writeMidiClip (selectedClipBars(), selectedClipPart()); };

    saveButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    saveButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    saveButton.onClick = [this] { saveClip(); };

    setSize (1180, 740);
    startTimerHz (30);
    timerCallback();
}

FaseGroovePianoEditor::~FaseGroovePianoEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

int FaseGroovePianoEditor::selectedClipBars() const
{
    static constexpr int bars[] { 0, 1, 2, 4, 8, 16, 32, FaseGroovePianoProcessor::songArrangement };
    return bars[juce::jlimit (0, 7, clipLength.getSelectedItemIndex())];
}

FaseGroovePianoProcessor::ClipPart FaseGroovePianoEditor::selectedClipPart() const
{
    return (FaseGroovePianoProcessor::ClipPart) juce::jlimit (0, 2, clipPart.getSelectedItemIndex());
}

void FaseGroovePianoEditor::saveClip()
{
    const int bars = selectedClipBars();
    const auto part = selectedClipPart();
    const auto folder = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);

    fileChooser = std::make_unique<juce::FileChooser> ("Save MIDI clip",
                                                       folder.getChildFile (processor.clipFileName (bars, part)), "*.mid");

    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::warnAboutOverwriting,
                              [this, bars, part] (const juce::FileChooser& chooser)
                              {
                                  auto file = chooser.getResult();
                                  if (file == juce::File())
                                      return;

                                  file = file.withFileExtension ("mid");
                                  file.deleteFile();

                                  juce::FileOutputStream stream (file);
                                  if (stream.openedOk())
                                      processor.createMidiClip (bars, part).writeTo (stream);
                              });
}

void FaseGroovePianoEditor::loadPreset (int index)
{
    const auto& presets = sgk::progressionPresets();
    if (index < 0 || index >= (int) presets.size())
        return;

    const auto& preset = presets[(size_t) index];
    processor.setParameterFromUI (ParamIDs::scale, (float) preset.scale);
    processor.setParameterFromUI (ParamIDs::progLength, (float) preset.degrees.size());

    for (int i = 0; i < (int) preset.degrees.size(); ++i)
        processor.setParameterFromUI (ParamIDs::slot (i), (float) preset.degrees[(size_t) i]);
}

//==============================================================================
void FaseGroovePianoEditor::timerCallback()
{
    auto s = processor.currentSettings();
    const int heldRoot = processor.uiHeldRoot.load();
    if (heldRoot >= 0)
        s.keyRoot = heldRoot;

    std::array<sgk::Chord, sgk::maxProgressionLength> chords;
    std::array<sgk::VoicedChord, sgk::maxProgressionLength> voiced;
    sgk::GrooveEngine::voiceProgression (s, chords, voiced);

    const int playingSlot = processor.uiSlot.load();

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
    {
        const bool enabled = i < s.progressionLength;
        const auto chord = sgk::buildChord (s.keyRoot, s.scale, s.progression[(size_t) i], s.extension);
        slots[i]->setInfo (sgk::chordName (chord), juce::String::fromUTF8 (sgk::romanNumeral (s.progression[(size_t) i], chord).c_str()),
                           enabled, enabled && i == playingSlot);
    }

    const int shown = playingSlot >= 0 ? playingSlot : 0;
    const juce::String chordText = sgk::chordName (chords[(size_t) shown]);
    const juce::String romanText = juce::String::fromUTF8 (sgk::romanNumeral (s.progression[(size_t) shown], chords[(size_t) shown]).c_str());
    const juce::String keyText = juce::String (sgk::noteName (s.keyRoot)) + " " + sgk::scaleName (s.scale);

    if (chordText != currentChord || romanText != currentRoman || keyText != currentKey || showingHold != (heldRoot >= 0))
    {
        currentChord = chordText;
        currentRoman = romanText;
        currentKey = keyText;
        showingHold = heldRoot >= 0;
        repaint (chordDisplayArea);
    }

    patternView.setState (sgk::GrooveEngine::resolvePattern (s), processor.uiStep.load());

    std::vector<CircleOfFifths::ChordMark> marks;
    for (int i = 0; i < s.progressionLength; ++i)
        marks.push_back ({ chords[(size_t) i].rootPc, chords[(size_t) i].third == 3 });
    circle.setState (s.keyRoot, sgk::scaleIsMinor (s.scale), marks, playingSlot);
    previewButton.setToggleState (processor.previewOn.load(), juce::dontSendNotification);

    const bool synced = syncButton.getToggleState();
    tempoSlider.setVisible (! synced);
    dawTempo.setVisible (synced);
    dawTempo.setText (juce::String (processor.hostBpm.load(), 1) + " BPM  (DAW)", juce::dontSendNotification);
}

void FaseGroovePianoEditor::drawPanel (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& title)
{
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (area.toFloat(), 10.0f);
    g.setColour (Palette::panelEdge);
    g.drawRoundedRectangle (area.toFloat().reduced (0.5f), 10.0f, 1.0f);

    if (title.isNotEmpty())
    {
        g.setColour (Palette::accent);
        g.setFont (font (12.0f, true));
        g.drawText (title, area.reduced (14, 8).removeFromTop (16), juce::Justification::centredLeft);
    }
}

void FaseGroovePianoEditor::paint (juce::Graphics& g)
{
    g.fillAll (Palette::background);

    // Header: logo + credit line
    auto header = headerArea.toFloat();
    Branding::drawFullLogo (g, header.removeFromTop (56.0f).withWidth (330.0f));
    g.setColour (Palette::dim);
    g.setFont (font (11.5f));
    g.drawText (juce::String ("by ") + Branding::artist, header.withTrimmedLeft (66.0f), juce::Justification::centredLeft);

    // Current chord
    drawPanel (g, chordDisplayArea, {});
    auto cd = chordDisplayArea.reduced (16, 8);
    auto left = cd.removeFromLeft (cd.getWidth() / 2);
    g.setColour (Palette::text);
    g.setFont (font (34.0f, true));
    g.drawFittedText (currentChord, left, juce::Justification::centredLeft, 1, 0.6f);

    g.setColour (Palette::accent);
    g.setFont (font (18.0f, true));
    g.drawText (currentRoman, cd.removeFromTop (cd.getHeight() / 2), juce::Justification::bottomRight);
    g.setColour (showingHold ? Palette::accent2 : Palette::dim);
    g.setFont (font (13.0f));
    g.drawText ((showingHold ? juce::String ("HOLD: ") : juce::String()) + currentKey, cd, juce::Justification::centredRight);

    drawPanel (g, tempoArea, "TEMPO");
    drawPanel (g, progressionPanel, "PROGRESSION");
    drawPanel (g, harmonyPanel, "HARMONY & SCALES");
    drawPanel (g, circlePanel, "CIRCLE OF FIFTHS");
    drawPanel (g, groovePanel, "GROOVE");
    drawPanel (g, soundPanel, "GUIDE SOUND & MIDI OUT");
    drawPanel (g, gridPanel, "RHYTHM GRID");
    drawPanel (g, clipPanel, "MIDI CLIP");

    auto hint = clipPanel.reduced (14, 8).removeFromBottom (16);
    g.setColour (Palette::dim);
    g.setFont (font (11.0f));
    g.drawText ("Drop it on any instrument track (your own piano / Rhodes)", hint, juce::Justification::centredLeft);
}

void FaseGroovePianoEditor::resized()
{
    auto r = getLocalBounds().reduced (20, 14);

    // Header
    auto top = r.removeFromTop (76);
    chordDisplayArea = top.removeFromRight (380).reduced (0, 4);
    top.removeFromRight (10);
    tempoArea = top.removeFromRight (330).reduced (0, 4);
    headerArea = top;
    creditsButton.setBounds (headerArea.getRight() - 74, headerArea.getBottom() - 22, 70, 20);
    about.setBounds (getLocalBounds());
    {
        auto inner = tempoArea.reduced (12, 8).withTrimmedLeft (64);
        auto controls = inner.withSizeKeepingCentre (inner.getWidth(), 32);
        syncButton.setBounds (controls.removeFromLeft (96));
        controls.removeFromLeft (8);
        tempoSlider.setBounds (controls);
        dawTempo.setBounds (controls);
    }
    r.removeFromTop (10);

    // Progression
    progressionPanel = r.removeFromTop (128);
    {
        auto inner = progressionPanel.reduced (14, 8);
        inner.removeFromTop (20);
        auto leftCol = inner.removeFromLeft (240);
        presetLabel.setBounds (leftCol.removeFromTop (16));
        auto presetRow = leftCol.removeFromTop (28);
        diceProgression.setBounds (presetRow.removeFromRight (52));
        presetBox.setBounds (presetRow.withTrimmedRight (6));
        leftCol.removeFromTop (4);
        progLength.setBounds (leftCol.removeFromLeft (114));
        chordLength.setBounds (leftCol.withTrimmedLeft (12));

        inner.removeFromLeft (14);
        const int w = inner.getWidth() / sgk::maxProgressionLength;
        for (auto* slot : slots)
            slot->setBounds (inner.removeFromLeft (w).reduced (3, 0));
    }
    r.removeFromTop (10);

    // Middle panels
    auto mid = r.removeFromTop (300);
    harmonyPanel = mid.removeFromLeft (300);
    mid.removeFromLeft (10);
    circlePanel = mid.removeFromLeft (240);
    mid.removeFromLeft (10);
    soundPanel = mid.removeFromRight (250);
    mid.removeFromRight (10);
    groovePanel = mid;

    auto row = [] (juce::Rectangle<int>& area, int h) { auto out = area.removeFromTop (h); area.removeFromTop (8); return out; };
    auto split = [] (juce::Rectangle<int> area, juce::Component& a, juce::Component& b, int leftWidth)
    {
        a.setBounds (area.removeFromLeft (leftWidth));
        b.setBounds (area.withTrimmedLeft (10));
    };

    {
        auto inner = harmonyPanel.reduced (14, 8);
        inner.removeFromTop (22);
        split (row (inner, 44), key, scale, 80);
        split (row (inner, 44), extension, voicing, 125);
        auto last = inner;
        bass.setBounds (last.removeFromLeft (150).removeFromTop (44));
        registerKnob.setBounds (last.withTrimmedLeft (10).withSizeKeepingCentre (110, juce::jmin (120, last.getHeight())));
    }

    circle.setBounds (circlePanel.reduced (10, 8).withTrimmedTop (22));

    {
        auto inner = groovePanel.reduced (14, 8);
        inner.removeFromTop (22);
        auto grooveRow = row (inner, 44);
        newRiffButton.setBounds (grooveRow.removeFromRight (84).withTrimmedTop (16));
        pattern.setBounds (grooveRow.withTrimmedRight (8));
        split (row (inner, 44), section, fills, inner.getWidth() / 2 - 5);
        const int w = inner.getWidth() / 5;
        auto knobs = inner.withSizeKeepingCentre (inner.getWidth(), juce::jmin (inner.getHeight(), w + 40));
        for (auto* k : { &swing, &gate, &velocity, &humanize, &riffDensity })
            k->setBounds (knobs.removeFromLeft (w));
    }

    {
        auto inner = soundPanel.reduced (14, 8);
        inner.removeFromTop (22);
        auto toggleRow = row (inner, 28);
        previewButton.setBounds (toggleRow.removeFromRight (96));
        soundToggle.setBounds (toggleRow);
        bassChannel.setBounds (row (inner, 44));
        playMode.setBounds (row (inner, 44));
        const int w = inner.getWidth() / 3;
        for (auto* k : { &tone, &reverb, &volume })
            k->setBounds (inner.removeFromLeft (w));
    }

    r.removeFromTop (10);
    clipPanel = r.removeFromRight (300);
    r.removeFromRight (10);
    gridPanel = r;
    patternView.setBounds (gridPanel.reduced (14, 8).withTrimmedTop (22));

    {
        auto inner = clipPanel.reduced (14, 8);
        inner.removeFromTop (22);
        inner.removeFromBottom (18);
        auto labels = inner.removeFromTop (16);
        clipLengthLabel.setBounds (labels.removeFromLeft (labels.getWidth() / 2));
        clipPartLabel.setBounds (labels.withTrimmedLeft (8));
        auto boxes = inner.removeFromTop (28);
        clipLength.setBounds (boxes.removeFromLeft (boxes.getWidth() / 2 - 4));
        clipPart.setBounds (boxes.withTrimmedLeft (8));
        inner.removeFromTop (10);
        auto buttons = inner.removeFromTop (juce::jmin (44, inner.getHeight()));
        saveButton.setBounds (buttons.removeFromRight (96));
        dragTile.setBounds (buttons.withTrimmedRight (8));
    }
}
