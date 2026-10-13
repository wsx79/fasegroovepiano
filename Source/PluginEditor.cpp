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
        label.setText (Localisation::upper (text), juce::dontSendNotification);
        label.setFont (font (11.0f, true));
        label.setColour (juce::Label::textColourId, Palette::dim);
        label.setJustificationType (juce::Justification::centredLeft);
        label.setInterceptsMouseClicks (false, false);
    }

    /** Translates a menu item; "4 bars" style items also work as number + word. */
    juce::String translateItem (const juce::String& item)
    {
        const auto translated = tr (item);
        if (translated != item || ! item.containsChar (' '))
            return translated;

        const auto number = item.upToFirstOccurrenceOf (" ", false, false);
        if (! number.containsOnly ("0123456789/"))
            return translated;

        return number + " " + tr (item.fromFirstOccurrenceOf (" ", false, false));
    }

    void fillChoiceItems (juce::ComboBox& box, juce::RangedAudioParameter* param)
    {
        if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (param))
        {
            // Long lists open in columns instead of one endless menu.
            const int n = c->choices.size();
            const int columns = n > 16 ? (n + 13) / 14 : 1;
            const int perColumn = (n + columns - 1) / columns;

            for (int i = 0; i < n; ++i)
            {
                box.addItem (c->choices[i], i + 1);
                if (columns > 1 && (i + 1) % perColumn == 0 && i < n - 1)
                    box.getRootMenu()->addColumnBreak();
            }
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
Knob::Knob (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& captionText)
    : caption (captionText), attachment (state, paramId, slider)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 16);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, Palette::text);
    addAndMakeVisible (slider);

    styleCaption (label, tr (caption));
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void Knob::refreshText()
{
    label.setText (Localisation::upper (tr (caption)), juce::dontSendNotification);
}

void Knob::resized()
{
    auto r = getLocalBounds();
    label.setBounds (r.removeFromTop (16));
    slider.setBounds (r);
}

Choice::Choice (juce::AudioProcessorValueTreeState& state, const juce::String& paramId, const juce::String& captionText)
    : caption (captionText), parameter (state.getParameter (paramId))
{
    fillChoiceItems (box, parameter);
    for (int i = 0; i < box.getNumItems(); ++i)
        englishItems.add (box.getItemText (i));
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (state, paramId, box);
    addAndMakeVisible (box);

    styleCaption (label, caption);
    addAndMakeVisible (label);
    refreshText();
}

void Choice::refreshText()
{
    label.setText (Localisation::upper (tr (caption)), juce::dontSendNotification);

    if (! groups.empty())
    {
        rebuildGroups();
        return;
    }

    // getSelectedId() returns 0 once the shown text differs from the item, so read it first.
    const int selected = box.getSelectedId();
    for (int i = 0; i < box.getNumItems(); ++i)
    {
        const int id = box.getItemId (i);
        if (juce::isPositiveAndBelow (id - 1, englishItems.size()))
            box.changeItemText (id, translateItem (englishItems[id - 1]));
    }

    box.setSelectedId (selected, juce::dontSendNotification);
}

void Choice::setGroups (std::vector<Group> newGroups)
{
    groups = std::move (newGroups);
    attachment.reset();

    groupedAttachment = std::make_unique<juce::ParameterAttachment> (*parameter, [this] (float value)
    {
        box.setSelectedId (juce::roundToInt (value) + 1, juce::dontSendNotification);
    });

    box.onChange = [this]
    {
        if (groupedAttachment != nullptr && box.getSelectedId() > 0)
            groupedAttachment->setValueAsCompleteGesture ((float) (box.getSelectedId() - 1));
    };

    rebuildGroups();
    groupedAttachment->sendInitialUpdate();
}

void Choice::rebuildGroups()
{
    const int selected = juce::roundToInt (parameter->convertFrom0to1 (parameter->getValue())) + 1;
    box.clear (juce::dontSendNotification);

    for (const auto& g : groups)
    {
        box.addSectionHeading (tr (g.heading));
        for (int index : g.items)
            if (juce::isPositiveAndBelow (index, englishItems.size()))
                box.addItem (translateItem (englishItems[index]), index + 1);
        if (g.columnBreakAfter)
            box.getRootMenu()->addColumnBreak();
    }

    box.setSelectedId (selected, juce::dontSendNotification);
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

    for (auto* arrow : { &invertUp, &invertDown })
        addAndMakeVisible (arrow);
    invertUp.onClick   = [this] { if (onInvert != nullptr) onInvert (1); };
    invertDown.onClick = [this] { if (onInvert != nullptr) onInvert (-1); };

    degree.onChange = [this]
    {
        if (degree.userOpened && onDegreePicked != nullptr)
            onDegreePicked();
        degree.userOpened = false;
    };
    addAndMakeVisible (degree);
}

void ChordSlot::setInfo (const juce::String& chordName, const juce::String& roman, bool enabled, bool playing,
                         bool isFree, int degreeValue, float harmonyScore)
{
    // A free chord has no scale degree: the menu shows "Free" until a degree is picked.
    if (isFree && (! showingFree || degree.getText() != tr ("Free")))
        degree.setText (tr ("Free"), juce::dontSendNotification);
    else if (! isFree && showingFree)
        degree.setSelectedId (degreeValue + 1, juce::dontSendNotification);
    showingFree = isFree;

    if (chordName == name && roman == numeral && enabled == isEnabledSlot && playing == isPlaying
        && std::abs (harmonyScore - harmony) < 0.005f)
        return;

    harmony = harmonyScore;
    setTooltip (! enabled ? juce::String()
                : harmony >= 0.72f ? tr ("Sounds great with the chords next to it")
                : harmony >= 0.58f ? tr ("Sounds good with the chords next to it")
                                   : tr ("Bold: it clashes with the chords next to it"));

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

    // Harmony bar: vivid = sounds great with its neighbours, dull = clashes.
    if (isEnabledSlot)
    {
        g.setColour (CircleOfFifths::harmonyColour (harmony));
        g.fillRoundedRectangle (r.reduced (10.0f, 0.0f).removeFromTop (3.5f).translated (0.0f, 1.0f), 1.75f);
    }

    auto text = getLocalBounds().reduced (4, 6).withTrimmedBottom (34);
    g.setColour (Palette::dim.withMultipliedAlpha (alpha));
    g.setFont (font (11.0f, true));
    g.drawText (juce::String (index + 1) + "  " + numeral + (inversion != 0 ? "  " + juce::String (inversion > 0 ? "+" : "") + juce::String (inversion) : juce::String()),
                text.removeFromTop (14), juce::Justification::centred);

    g.setColour (Palette::text.withMultipliedAlpha (alpha));
    g.setFont (font (name.length() > 7 ? 14.0f : 17.0f, true));
    g.drawFittedText (name, text, juce::Justification::centred, 1, 0.7f);
}

void ChordSlot::resized()
{
    degree.setBounds (getLocalBounds().reduced (6, 6).removeFromBottom (26));

    // Inversion arrows in the top corners.
    auto top = getLocalBounds().reduced (5, 5).removeFromTop (12);
    invertDown.setBounds (top.removeFromLeft (12));
    invertUp.setBounds (top.removeFromRight (12));
}

void ChordSlot::setInversion (int newInversion)
{
    if (newInversion == inversion)
        return;
    inversion = newInversion;
    repaint();
}

//==============================================================================
void PatternView::setState (const sgk::GroovePattern& newPattern, int step, bool bassOn)
{
    if (step == currentStep && bassOn == bassEnabled
        && newPattern.rightHand == pattern.rightHand && newPattern.leftHand == pattern.leftHand)
        return;

    pattern = newPattern;
    currentStep = step;
    bassEnabled = bassOn;
    repaint();
}

bool PatternView::cellAt (juce::Point<float> p, bool& bassRow, int& step) const
{
    const int len = pattern.length();
    const float rowH = (getHeight() - headerHeight) / 2.0f;
    if (len == 0 || p.x < labelWidth || p.y < headerHeight)
        return false;

    step = juce::jlimit (0, len - 1, (int) ((p.x - labelWidth) / ((getWidth() - labelWidth) / (float) len)));
    bassRow = p.y >= headerHeight + rowH;
    return true;
}

void PatternView::mouseDown (const juce::MouseEvent& e)
{
    bool bassRow = false;
    int step = 0;
    if (cellAt (e.position, bassRow, step) && onCellClicked != nullptr)
        onCellClicked (bassRow, step, e);
}

void PatternView::mouseMove (const juce::MouseEvent& e)
{
    bool bassRow = false;
    int step = -1;
    if (! cellAt (e.position, bassRow, step))
        step = -1;

    if (step != hoverStep || bassRow != hoverBass)
    {
        hoverStep = step;
        hoverBass = bassRow;
        repaint();
    }
}

void PatternView::mouseExit (const juce::MouseEvent&)
{
    hoverStep = -1;
    repaint();
}

void PatternView::paint (juce::Graphics& g)
{
    const auto& p = pattern;
    const int len = p.length();
    if (len == 0)
        return;

    auto area = getLocalBounds().toFloat();
    const float labelW = labelWidth;
    const float rowH = (area.getHeight() - headerHeight) / 2.0f;
    const float cellW = (area.getWidth() - labelW) / (float) len;

    g.setFont (font (11.0f, true));

    // Beat numbers
    for (int s = 0; s < len; s += 4)
    {
        g.setColour (s % 16 == 0 ? Palette::accent : Palette::dim);
        g.drawText (juce::String ((s / 4) % 4 + 1), juce::Rectangle<float> (labelW + s * cellW, 0.0f, cellW * 2.0f, 14.0f),
                    juce::Justification::centredLeft);
    }

    // Segment separators (A | B | C | D)
    if (len == sgk::userPatternLength)
    {
        g.setColour (Palette::accent.withAlpha (0.35f));
        for (int k = 1; k < sgk::numSegments; ++k)
        {
            const float x = labelW + (float) (k * len / sgk::numSegments) * cellW;
            g.fillRect (x - 0.75f, headerHeight, 1.5f, area.getHeight() - headerHeight);
        }
    }

    auto drawRow = [&] (const std::string& line, float y, const juce::String& caption, juce::Colour colour, bool isBass)
    {
        g.setColour (Palette::dim);
        g.drawFittedText (caption, juce::Rectangle<float> (0.0f, y, labelW - 2.0f, rowH).toNearestInt(), juce::Justification::centredLeft, 1, 0.6f);

        if (isBass && ! bassEnabled)
        {
            g.setColour (Palette::dim.withAlpha (0.6f));
            g.drawText (tr ("Bass off  -  click BASS to turn it back on"),
                        juce::Rectangle<float> (labelW, y, area.getWidth() - labelW, rowH), juce::Justification::centred);
            return;
        }

        for (int s = 0; s < len; ++s)
        {
            auto cell = juce::Rectangle<float> (labelW + s * cellW, y, cellW, rowH).reduced (1.5f, 3.0f);
            const char c = line[(size_t) s];

            g.setColour (Palette::panelEdge.withAlpha (s % 4 == 0 ? 0.9f : 0.45f));
            g.fillRoundedRectangle (cell, 3.0f);

            const bool up = c == 'u' || c == 'U', down = c == 'd' || c == 'D';
            if (c == 'X' || c == 'x' || c == 'g' || c == '>' || up || down)
            {
                const bool accent = c == 'X' || c == 'U' || c == 'D';
                g.setColour (c == '>' ? Palette::accent2 : colour.withAlpha (accent ? 1.0f : c == 'g' ? 0.35f : 0.75f));
                g.fillRoundedRectangle (cell, 3.0f);

                // Voicing moves: a small arrow (the chord one inversion up or down).
                if (up || down)
                {
                    const auto mid = cell.getCentre();
                    const float w = juce::jmin (cell.getWidth() * 0.6f, 9.0f), h = w * 0.6f;
                    juce::Path arrow;
                    if (up) arrow.addTriangle (mid.x - w / 2, mid.y + h / 2, mid.x + w / 2, mid.y + h / 2, mid.x, mid.y - h / 2);
                    else    arrow.addTriangle (mid.x - w / 2, mid.y - h / 2, mid.x + w / 2, mid.y - h / 2, mid.x, mid.y + h / 2);
                    g.setColour (Palette::background.withAlpha (0.85f));
                    g.fillPath (arrow);
                }
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
            else if (s == hoverStep && isBass == hoverBass)
            {
                g.setColour (Palette::text.withAlpha (0.5f));
                g.drawRoundedRectangle (cell.expanded (0.5f), 3.0f, 1.0f);
            }
        }
    };

    drawRow (p.rightHand, headerHeight, tr ("CHORDS"), Palette::accent, false);
    drawRow (p.leftHand, headerHeight + rowH, tr ("BASS"), Palette::accent2.brighter (0.2f), true);
}

//==============================================================================
OptionsOverlay::OptionsOverlay() : artist (Branding::artistLogo())
{
    setInterceptsMouseClicks (true, true);

    auto styleTab = [this] (juce::TextButton& b)
    {
        b.setClickingTogglesState (false);
        b.setColour (juce::TextButton::buttonColourId, Palette::background);
        b.setColour (juce::TextButton::buttonOnColourId, Palette::accent);
        b.setColour (juce::TextButton::textColourOffId, Palette::text);
        b.setColour (juce::TextButton::textColourOnId, Palette::background);
        addAndMakeVisible (b);
    };
    styleTab (settingsTab);
    styleTab (aboutTab);
    styleTab (closeButton);
    settingsTab.onClick = [this] { selectTab (false); };
    aboutTab.onClick    = [this] { selectTab (true); };
    closeButton.onClick = [this] { setVisible (false); };

    auto setUpLink = [this] (juce::TextButton& b, const char* url, juce::Colour colour, juce::Colour text)
    {
        b.setColour (juce::TextButton::buttonColourId, colour);
        b.setColour (juce::TextButton::textColourOffId, text);
        b.onClick = [url] { juce::URL (url).launchInDefaultBrowser(); };
        addChildComponent (b);
    };
    setUpLink (sourceButton, Branding::sourceUrl, Palette::background, Palette::text);
    setUpLink (kofiButton, Branding::kofiUrl, Palette::accent, Palette::background);
    setUpLink (paypalButton, Branding::paypalUrl, Palette::accent2, Palette::background);
    setUpLink (spotifyButton, Branding::spotifyUrl, juce::Colour (0xff1db954), Palette::background);

    // Settings tab
    for (auto* l : { &languageLabel, &sizeLabel })
    {
        l->setFont (font (11.0f, true));
        l->setColour (juce::Label::textColourId, Palette::dim);
        addChildComponent (l);
    }

    for (int i = 0; i < Localisation::numLanguages; ++i)
        languageBox.addItem (juce::String::fromUTF8 (Localisation::languageName (i)), i + 1);
    languageBox.onChange = [this]
    {
        Localisation::setCurrent ((Localisation::Language) (languageBox.getSelectedItemIndex()));
        if (onLanguageChanged != nullptr)
            onLanguageChanged();
    };
    addChildComponent (languageBox);

    for (int percent : { 60, 75, 90, 100, 110, 125, 150, 175, 200 })
        sizeBox.addItem (juce::String (percent) + " %", percent);
    sizeBox.onChange = [this]
    {
        if (onScaleChosen != nullptr && sizeBox.getSelectedId() > 0)
            onScaleChosen (sizeBox.getSelectedId() / 100.0f);
    };
    addChildComponent (sizeBox);

    shrinkToggle.onClick = [this] { if (onShrinkChanged != nullptr) onShrinkChanged (shrinkToggle.getToggleState()); };
    addChildComponent (shrinkToggle);
    tipsToggle.onClick = [this] { if (onTipsChanged != nullptr) onTipsChanged (tipsToggle.getToggleState()); };
    addChildComponent (tipsToggle);
    updatesToggle.onClick = [this] { Updater::setAutoCheckEnabled (updatesToggle.getToggleState()); };
    addChildComponent (updatesToggle);
    checkUpdatesButton.onClick = [this] { if (onCheckUpdates != nullptr) onCheckUpdates(); };
    addChildComponent (checkUpdatesButton);

    refreshText();
    selectTab (false);
}

void OptionsOverlay::show (bool about, float currentScale, bool shrinkWhileDragging)
{
    languageBox.setSelectedItemIndex ((int) Localisation::current(), juce::dontSendNotification);

    int closest = 100;
    for (int i = 0; i < sizeBox.getNumItems(); ++i)
        if (std::abs (sizeBox.getItemId (i) - currentScale * 100.0f) < std::abs (closest - currentScale * 100.0f))
            closest = sizeBox.getItemId (i);
    sizeBox.setSelectedId (closest, juce::dontSendNotification);

    shrinkToggle.setToggleState (shrinkWhileDragging, juce::dontSendNotification);
    tipsToggle.setToggleState (Localisation::tipsEnabled(), juce::dontSendNotification);
    updatesToggle.setToggleState (Updater::autoCheckEnabled(), juce::dontSendNotification);
    selectTab (about);
    setVisible (true);
    toFront (false);
}

void OptionsOverlay::refreshText()
{
    settingsTab.setButtonText (tr ("SETTINGS"));
    aboutTab.setButtonText (tr ("ABOUT"));
    closeButton.setButtonText (tr ("CLOSE"));
    sourceButton.setButtonText (tr ("SOURCE CODE"));
    kofiButton.setButtonText (tr ("DONATE: Ko-fi"));
    paypalButton.setButtonText (tr ("DONATE: PayPal"));
    spotifyButton.setButtonText (tr ("SPOTIFY"));
    spotifyButton.setTooltip (tr ("Listen to my productions on Spotify"));
    languageLabel.setText (Localisation::upper (tr ("Language")), juce::dontSendNotification);
    sizeLabel.setText (Localisation::upper (tr ("Interface size")), juce::dontSendNotification);
    shrinkToggle.setButtonText (tr ("Mini window while dragging"));
    tipsToggle.setButtonText (tr ("Show tips when the mouse rests on a control"));
    updatesToggle.setButtonText (tr ("Check for updates automatically (GitHub)"));
    checkUpdatesButton.setButtonText (tr ("CHECK NOW"));
    repaint();
}

void OptionsOverlay::selectTab (bool about)
{
    showingAbout = about;
    settingsTab.setToggleState (! about, juce::dontSendNotification);
    aboutTab.setToggleState (about, juce::dontSendNotification);

    for (auto* c : std::initializer_list<juce::Component*> { &languageLabel, &languageBox, &sizeLabel, &sizeBox, &shrinkToggle, &tipsToggle,
                                                                 &updatesToggle, &checkUpdatesButton })
        c->setVisible (! about);

    auto showLink = [about] (juce::Component& b, const char* url) { b.setVisible (about && juce::String (url).isNotEmpty()); };
    showLink (sourceButton, Branding::sourceUrl);
    showLink (kofiButton, Branding::kofiUrl);
    showLink (paypalButton, Branding::paypalUrl);
    showLink (spotifyButton, Branding::spotifyUrl);

    resized();
    repaint();
}

void OptionsOverlay::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
        setVisible (false);
}

void OptionsOverlay::resized()
{
    auto inner = card().toNearestInt().reduced (24, 18);

    auto tabs = inner.removeFromTop (30);
    closeButton.setBounds (tabs.removeFromRight (100));
    settingsTab.setBounds (tabs.removeFromLeft (150));
    tabs.removeFromLeft (8);
    aboutTab.setBounds (tabs.removeFromLeft (150));

    // Settings tab
    auto settings = inner.withTrimmedTop (40).reduced (40, 0);
    languageLabel.setBounds (settings.removeFromTop (18));
    languageBox.setBounds (settings.removeFromTop (30).withWidth (260));
    settings.removeFromTop (22);
    sizeLabel.setBounds (settings.removeFromTop (18));
    sizeBox.setBounds (settings.removeFromTop (30).withWidth (260));
    settings.removeFromTop (22);
    shrinkToggle.setBounds (settings.removeFromTop (26).withWidth (360));
    settings.removeFromTop (10);
    tipsToggle.setBounds (settings.removeFromTop (26).withWidth (460));
    settings.removeFromTop (10);
    {
        auto row = settings.removeFromTop (26);
        updatesToggle.setBounds (row.removeFromLeft (380));
        checkUpdatesButton.setBounds (row.removeFromLeft (130).reduced (0, 1));
    }

    // About tab: link buttons above the footer
    auto row = inner.withTrimmedBottom (26).removeFromBottom (30);
    juce::Array<juce::Component*> buttons;
    for (auto* b : { &sourceButton, &kofiButton, &paypalButton, &spotifyButton })
        if (b->isVisible())
            buttons.add (b);

    const int w = 126, gap = 10;
    row = row.withSizeKeepingCentre (juce::jmax (1, buttons.size() * w + (buttons.size() - 1) * gap), row.getHeight());
    for (auto* b : buttons)
    {
        b->setBounds (row.removeFromLeft (w));
        row.removeFromLeft (gap);
    }
}

void OptionsOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));

    auto card = this->card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (card, 16.0f);
    g.setColour (Palette::accent.withAlpha (0.6f));
    g.drawRoundedRectangle (card, 16.0f, 1.5f);

    auto r = card.reduced (36.0f, 18.0f);
    r.removeFromTop (46.0f);   // tabs
    const auto dot = juce::String::fromUTF8 ("  \xc2\xb7  ");

    if (! showingAbout)
    {
        g.setColour (Palette::dim);
        g.setFont (font (12.0f));
        g.drawText (tr ("Settings are saved for all your projects."),
                    juce::Rectangle<float> (r.getX() + 28.0f, r.getY() + 290.0f, r.getWidth() - 28.0f, 18.0f), juce::Justification::centredLeft);
        g.setFont (font (11.0f));
        g.drawText ("Fase Groove Piano " + juce::String (JucePlugin_VersionString) + dot + tr ("Built with JUCE"),
                    r.removeFromBottom (16.0f), juce::Justification::centred);
        return;
    }

    Branding::drawFullLogo (g, r.removeFromTop (70.0f).withSizeKeepingCentre (340.0f, 70.0f));

    g.setColour (Palette::dim);
    g.setFont (font (13.0f));
    g.drawText (tr ("Version") + " " + juce::String (JucePlugin_VersionString) + dot + tr (Branding::tagline),
                r.removeFromTop (26.0f), juce::Justification::centred);

    r.removeFromTop (8.0f);
    g.setColour (Palette::panelEdge);
    g.fillRect (r.removeFromTop (1.0f));
    r.removeFromTop (12.0f);

    auto caption = [&g] (const juce::String& text, juce::Rectangle<float> area)
    {
        g.setColour (Palette::accent);
        g.setFont (font (11.0f, true));
        g.drawText (text, area, juce::Justification::centred);
    };

    caption (tr ("CREATED BY"), r.removeFromTop (16.0f));
    r.removeFromTop (6.0f);
    if (artist.isValid())
        g.drawImageWithin (artist, (int) r.getX(), (int) r.getY(), (int) r.getWidth(), 110,
                           juce::RectanglePlacement::centred);
    r.removeFromTop (114.0f);

    g.setColour (Palette::text);
    g.setFont (font (14.0f));
    g.drawText (tr ("DJ & Producer"), r.removeFromTop (20.0f), juce::Justification::centred);
    g.setColour (juce::Colour (0xff1db954));
    g.setFont (font (12.0f, true));
    g.drawText (tr ("Listen to my productions on Spotify"), r.removeFromTop (18.0f), juce::Justification::centred);

    r.removeFromTop (12.0f);
    caption (tr ("CODE"), r.removeFromTop (16.0f));
    r.removeFromTop (4.0f);
    g.setColour (Palette::text);
    g.setFont (font (16.0f, true));
    g.drawText ("Claude  (Anthropic)", r.removeFromTop (22.0f), juce::Justification::centred);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (tr ("AI coding assistant"), r.removeFromTop (16.0f), juce::Justification::centred);

    g.setFont (font (11.0f));
    g.drawText (tr ("Built with JUCE") + dot + "VST is a registered trademark of Steinberg Media Technologies GmbH",
                r.removeFromBottom (16.0f), juce::Justification::centred);

    r.removeFromBottom (48.0f);   // link buttons
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (juce::String (Branding::sourceUrl).fromFirstOccurrenceOf ("://", false, false),
                r.removeFromBottom (18.0f), juce::Justification::centred);
    g.setColour (Palette::text);
    g.setFont (font (12.0f, true));
    g.drawText (tr (Branding::licence), r.removeFromBottom (18.0f), juce::Justification::centred);
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
    g.drawFittedText (tr ("DRAG MIDI TO DAW"), r.reduced (6.0f, 2.0f).toNearestInt(), juce::Justification::centred, 2, 0.8f);
}

void MidiDragTile::mouseDrag (const juce::MouseEvent& e)
{
    if (dragStarted || e.getDistanceFromDragStart() < 4 || createFile == nullptr)
        return;

    dragStarted = true;
    const auto file = createFile();
    if (! file.existsAsFile())
        return;

    if (onDragStart != nullptr)
        onDragStart();

    juce::Component::SafePointer<MidiDragTile> safeThis (this);
    juce::DragAndDropContainer::performExternalDragDropOfFiles ({ file.getFullPathName() }, false, this, [safeThis]
    {
        if (safeThis != nullptr && safeThis->onDragEnd != nullptr)
            safeThis->onDragEnd();
    });
}

//==============================================================================
UpdateOverlay::UpdateOverlay()
{
    notes.setMultiLine (true, true);
    notes.setReadOnly (true);
    notes.setScrollbarsShown (true);
    notes.setIndents (10, 8);
    notes.setCaretVisible (false);
    notes.setFont (font (14.0f));
    notes.setColour (juce::TextEditor::backgroundColourId, Palette::background);
    notes.setColour (juce::TextEditor::textColourId, Palette::text);
    notes.setColour (juce::TextEditor::outlineColourId, Palette::accent.withAlpha (0.3f));
    addAndMakeVisible (notes);

    installButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
    installButton.setColour (juce::TextButton::textColourOffId, Palette::background);
    for (auto* b : { &installButton, &laterButton, &skipButton })
        addAndMakeVisible (b);

    installButton.onClick = [this]
    {
        Updater::install (result.file);
        setVisible (false);
    };
    laterButton.onClick = [this] { setVisible (false); };
    skipButton.onClick = [this]
    {
        Updater::skipVersion (result.version);
        setVisible (false);
    };
    refreshText();
}

void UpdateOverlay::show (const Updater::Result& r)
{
    result = r;
    notes.setText (r.notes, false);
    notes.moveCaretToTop (false);
    refreshText();
    setVisible (true);
    toFront (false);
}

void UpdateOverlay::refreshText()
{
    installButton.setButtonText (tr ("INSTALL NOW"));
    laterButton.setButtonText (tr ("LATER"));
    skipButton.setButtonText (tr ("SKIP THIS VERSION"));
    repaint();
}

void UpdateOverlay::resized()
{
    auto inner = card().toNearestInt().reduced (28, 22);
    inner.removeFromTop (92);
    auto buttons = inner.removeFromBottom (34);
    inner.removeFromBottom (44);
    notes.setBounds (inner);

    const int w = 170, gap = 12;
    buttons = buttons.withSizeKeepingCentre (3 * w + 2 * gap, buttons.getHeight());
    for (auto* b : { &installButton, &laterButton, &skipButton })
    {
        b->setBounds (buttons.removeFromLeft (w));
        buttons.removeFromLeft (gap);
    }
}

void UpdateOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));

    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 16.0f);
    g.setColour (Palette::accent.withAlpha (0.6f));
    g.drawRoundedRectangle (c, 16.0f, 1.5f);

    auto r = c.reduced (28.0f, 22.0f);
    g.setColour (Palette::accent);
    g.setFont (font (22.0f, true));
    g.drawText (tr ("New version") + " " + result.version + " " + tr ("downloaded"), r.removeFromTop (30.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (13.0f));
    g.drawText (tr ("You have") + " " + juce::String (JucePlugin_VersionString) + ". " + tr ("What's new:"),
                r.removeFromTop (26.0f), juce::Justification::centredLeft);

   #if JUCE_WINDOWS
    const char* how = "INSTALL NOW starts the installer. Close your DAW when it asks, then reopen it.";
   #elif JUCE_MAC
    const char* how = "INSTALL NOW opens the zip: copy the plug-ins to the same folders as the first time, then restart your DAW.";
   #else
    const char* how = "INSTALL NOW shows the downloaded zip: copy the plug-ins to the same folders as the first time.";
   #endif
    auto bottom = c.reduced (28.0f, 22.0f).removeFromBottom (34.0f + 44.0f).removeFromTop (44.0f);
    g.drawFittedText (tr (how), bottom.toNearestInt(), juce::Justification::centredLeft, 2);
    g.setFont (font (11.0f));
    g.drawText (result.file.getFullPathName(), r.removeFromTop (20.0f), juce::Justification::centredLeft);
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
      approach (p.apvts, ParamIDs::approach, "Approach chord"),
      topLine (p.apvts, ParamIDs::topLine, "Top line"),
      movement (p.apvts, ParamIDs::movement, "Movement"),
      registerKnob (p.apvts, ParamIDs::registerPos, "Register"),
      strumKnob (p.apvts, ParamIDs::strum, "Strum"),
      pattern (p.apvts, ParamIDs::pattern, "Groove"),
      section (p.apvts, ParamIDs::section, "Song section"),
      fills (p.apvts, ParamIDs::fills, "Fills"),
      fillStyle (p.apvts, ParamIDs::fillStyle, "Fill style"),
      playMode (p.apvts, ParamIDs::playMode, "Play mode"),
      variation (p.apvts, ParamIDs::variation, "Variation"),
      grooveBars (p.apvts, ParamIDs::grooveBars, "Groove length"),
      swing (p.apvts, ParamIDs::swing, "Swing"),
      gate (p.apvts, ParamIDs::gate, "Gate"),
      velocity (p.apvts, ParamIDs::velocity, "Velocity"),
      humanize (p.apvts, ParamIDs::humanize, "Humanize"),
      riffDensity (p.apvts, ParamIDs::riffDensity, "Density"),
      bassChannel (p.apvts, ParamIDs::bassChannel, "MIDI out"),
      soundType (p.apvts, ParamIDs::soundType, "Sound"),
      soundAttachment (p.apvts, ParamIDs::sound, soundToggle),
      tone (p.apvts, ParamIDs::tone, "Tone"),
      reverb (p.apvts, ParamIDs::reverb, "Reverb"),
      volume (p.apvts, ParamIDs::volume, "Volume"),
      chorus (p.apvts, ParamIDs::chorus, "Chorus"),
      pump (p.apvts, ParamIDs::pump, "Pump"),
      syncAttachment (p.apvts, ParamIDs::tempoSync, syncButton),
      tempoAttachment (p.apvts, ParamIDs::tempo, tempoSlider)
{
    setLookAndFeel (&lookAndFeel);
    Localisation::loadSaved();

    styleCaption (presetLabel, "Progression presets");
    addAndMakeVisible (presetLabel);

    presetBox.setTextWhenNothingSelected ("Load a progression...");
    buildPresetMenu();
    presetBox.onChange = [this] { loadPreset (presetBox.getSelectedId() - 1); };
    addAndMakeVisible (presetBox);

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        addAndMakeVisible (slots.add (new ChordSlot (p.apvts, i)));

    for (juce::Component* c : std::initializer_list<juce::Component*> {
             &progLength, &chordLength, &key, &scale, &extension, &voicing, &bass, &approach, &topLine, &movement, &variation, &grooveBars, &chorus, &pump, &registerKnob, &strumKnob,
             &octaveToggle, &circle,
             &diceProgression, &pattern, &section, &newRiffButton, &fills, &fillStyle, &playMode, &soundType,
             &swing, &gate, &velocity, &humanize, &riffDensity,
             &soundToggle, &previewButton, &bassChannel, &tone, &reverb, &volume, &patternView,
             &clipLengthLabel, &clipPartLabel, &clipLength, &clipPart, &dragTile, &saveButton })
        addAndMakeVisible (c);

    octaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::octaveTop, octaveToggle);
    octaveToggle.setTooltip ("Double the top note of every chord an octave higher (bright house piano)");

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

    circle.onMoveChord = [this] (int slot, int degree, int rootPc, bool minorChord)
    {
        if (degree >= 0)
        {
            processor.setFreeChord (slot, sgk::noFreeChord);
            processor.setParameterFromUI (ParamIDs::slot (slot), (float) degree);
        }
        else
        {
            const int key = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::key)->load());
            processor.setFreeChord (slot, sgk::freeChordCode (rootPc - key, minorChord));
        }
        presetBox.setSelectedId (0, juce::dontSendNotification);
    };

    // Free mode: drag the dots anywhere, even outside the scale.
    freeToggle.setToggleState (processor.freeCircle, juce::dontSendNotification);
    freeToggle.onClick = [this]
    {
        processor.freeCircle = freeToggle.getToggleState();
        circle.setFreeMode (processor.freeCircle);
    };
    circle.setFreeMode (processor.freeCircle);
    addAndMakeVisible (freeToggle);

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        slots[i]->onDegreePicked = [this, i] { processor.setFreeChord (i, sgk::noFreeChord); };

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

    for (auto* c : std::initializer_list<juce::Component*> { &syncButton, &tempoSlider, &dawTempo, &optionsButton })
        addAndMakeVisible (c);

    // Groove menu: grouped by genre, in columns.
    {
        std::vector<Choice::Group> groups;
        const auto& patterns = sgk::groovePatterns();
        const auto& genres = sgk::grooveCategories();
        for (size_t g = 0; g < genres.size(); ++g)
        {
            Choice::Group group { genres[g], {}, false };
            for (int i = 0; i < (int) patterns.size(); ++i)
                if (patterns[(size_t) i].category == genres[g])
                    group.items.push_back (i);

            // Columns: piano house, soulful, deep | gospel, jackin', jackin' piano | garage, afro, funky | '90s, neo soul, chill, dance stabs, generate
            group.columnBreakAfter = g == 2 || g == 5 || g == 8;
            groups.push_back (group);
        }
        groups.push_back ({ "Generate", { sgk::randomPatternIndex(), sgk::userPatternIndex() }, false });
        pattern.setGroups (std::move (groups));
    }

    for (auto* b : { &styleMenuButton, &resetButton, &optionsButton })
    {
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::textColourOffId, Palette::dim);
        addAndMakeVisible (b);
    }
    styleMenuButton.setColour (juce::TextButton::textColourOffId, Palette::accent);
    styleMenuButton.onClick = [this] { showStyleMenu(); };
    resetButton.onClick = [this] { confirmReset(); };
    optionsButton.onClick = [this] { options.show (false, uiScale, processor.shrinkWhileDragging); };

    options.onLanguageChanged = [this] { applyLanguage(); };
    options.onScaleChosen = [this] (float newScale)
    {
        setSize (juce::roundToInt (baseWidth * newScale), juce::roundToInt (baseHeight * newScale));
    };
    options.onShrinkChanged = [this] (bool shrink)
    {
        processor.shrinkWhileDragging = shrink;
        shrinkToggle.setToggleState (shrink, juce::dontSendNotification);
    };
    options.onTipsChanged = [this] (bool on)
    {
        Localisation::setTipsEnabled (on);
        applyTips();
    };
    options.onCheckUpdates = [this]
    {
        options.checkUpdatesButton.setEnabled (false);
        options.checkUpdatesButton.setButtonText (tr ("CHECKING..."));
        updateChecker.start (true);
    };
    addChildComponent (options);
    addChildComponent (updateOverlay);
    updateChecker.onResult = [this] (const Updater::Result& result, bool manual)
    {
        options.checkUpdatesButton.setEnabled (true);
        options.refreshText();
    updateOverlay.refreshText();

        if (result.status == Updater::Result::Status::ready)
        {
            options.setVisible (false);
            updateOverlay.show (result);
        }
        else if (manual)
        {
            const bool upToDate = result.status == Updater::Result::Status::upToDate;
            juce::AlertWindow::showMessageBoxAsync (upToDate ? juce::MessageBoxIconType::InfoIcon : juce::MessageBoxIconType::WarningIcon,
                                                    "Fase Groove Piano",
                                                    upToDate ? tr ("You have the latest version.") + " (" + juce::String (JucePlugin_VersionString) + ")"
                                                             : tr ("Could not check for updates. Check your internet connection and try again."));
        }
    };
    if (Updater::autoCheckEnabled() && Updater::firstAutoCheckThisSession())
        updateChecker.start (false);
    applyTips();

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

    // Mini window while dragging: frees the screen so you can see where to drop.
    dragTile.onDragStart = [this]
    {
        if (processor.shrinkWhileDragging)
        {
            juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
            juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->enterMiniMode(); });
        }
    };
    dragTile.onDragEnd = [this]
    {
        juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
        juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->exitMiniMode(); });
    };

    shrinkToggle.setToggleState (processor.shrinkWhileDragging, juce::dontSendNotification);
    shrinkToggle.setTooltip ("Shrink the plug-in window while you drag a clip, so you can see your DAW");
    shrinkToggle.onClick = [this] { processor.shrinkWhileDragging = shrinkToggle.getToggleState(); };
    addAndMakeVisible (shrinkToggle);

    // Rhythm grid: editable cells, A/B/C/D segments, quick bass on/off.
    patternView.onCellClicked = [this] (bool bassRow, int step, const juce::MouseEvent& e) { cellClicked (bassRow, step, e); };
    patternView.setTooltip ("Click a step: hit > accent > ghost > rest.  Right-click: more options");

    bassButton.setClickingTogglesState (false);
    bassButton.setTooltip ("Left-hand bass on / off");
    bassButton.onClick = [this] { processor.toggleBass(); };
    addAndMakeVisible (bassButton);

    for (int i = 0; i < sgk::numSegments; ++i)
    {
        auto* box = segmentBoxes.add (new juce::ComboBox());
        fillChoiceItems (*box, p.apvts.getParameter (ParamIDs::segment (i)));
        box->setJustificationType (juce::Justification::centred);
        box->setTooltip ("Which part (A-D) plays here: combine parts like A A B C");
        segmentAttachments.add (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (p.apvts, ParamIDs::segment (i), *box));
        addAndMakeVisible (box);

        auto* dice = segmentDice.add (new juce::TextButton ("RND"));
        dice->setColour (juce::TextButton::buttonColourId, Palette::background);
        dice->setColour (juce::TextButton::textColourOffId, Palette::accent);
        dice->setTooltip ("New random rhythm for this part");
        dice->onClick = [this, i] { processor.randomizeSegment (processor.segmentAtStep (i * sgk::userPatternLength / sgk::numSegments)); };
        addAndMakeVisible (dice);

        auto* lock = segmentLocks.add (new juce::TextButton ("LOCK"));
        lock->setClickingTogglesState (true);
        lock->setColour (juce::TextButton::buttonColourId, Palette::background);
        lock->setColour (juce::TextButton::buttonOnColourId, Palette::accent2);
        lock->setColour (juce::TextButton::textColourOffId, Palette::dim);
        lock->setColour (juce::TextButton::textColourOnId, Palette::background);
        lock->setTooltip ("Keep this part when you press NEW RIFF");
        lock->onClick = [this, i, lock]
        {
            processor.setSegmentLocked (processor.segmentAtStep (i * sgk::userPatternLength / sgk::numSegments), lock->getToggleState());
        };
        addAndMakeVisible (lock);
    }

    newRiffButton.setTooltip ("New random funky riff (locked parts are kept)");

    saveButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    saveButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    saveButton.onClick = [this] { showSaveMenu(); };

    // Presets, undo / redo, learn.
    for (auto* b : { &presetsButton, &learnButton })
    {
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::textColourOffId, Palette::dim);
        addAndMakeVisible (b);
    }
    learnButton.setClickingTogglesState (false);
    learnButton.setColour (juce::TextButton::buttonOnColourId, Palette::accent2);
    learnButton.setColour (juce::TextButton::textColourOnId, Palette::background);
    learnButton.onClick = [this]
    {
        if (processor.learnMode.load())
            processor.learnMode = false;
        else
        {
            processor.startLearning();
            lastLearnCount = processor.learnedCount.load();
        }
    };
    presetsButton.onClick = [this] { showPresetMenu(); };

    // Strum direction: click to cycle Up / Down / Alternate.
    strumDirButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    strumDirButton.setColour (juce::TextButton::textColourOffId, Palette::accent);
    strumDirButton.onClick = [this]
    {
        const int now = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::strumDir)->load());
        processor.setParameterFromUI (ParamIDs::strumDir, (float) ((now + 1) % 3));
    };
    addAndMakeVisible (strumDirButton);
    addAndMakeVisible (undoButton);
    addAndMakeVisible (redoButton);
    undoButton.onClick = [this] { undo(); };
    redoButton.onClick = [this] { redo(); };
    setWantsKeyboardFocus (true);

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        slots[i]->onInvert = [this, i] (int delta)
        {
            const auto id = ParamIDs::inversion (i);
            const int now = juce::roundToInt (processor.apvts.getRawParameterValue (id)->load());
            processor.setParameterFromUI (id, (float) juce::jlimit (-3, 3, now + delta));
        };

    applyLanguage();

    // Read the saved size first: setting the limits resizes (and re-saves) the window.
    const float savedScale = juce::jlimit (0.6f, 2.0f, processor.editorScale);
    setResizable (true, true);
    setResizeLimits ((int) (baseWidth * 0.6f), (int) (baseHeight * 0.6f), baseWidth * 2, baseHeight * 2);
    getConstrainer()->setFixedAspectRatio ((double) baseWidth / baseHeight);
    setSize (juce::roundToInt (baseWidth * savedScale), juce::roundToInt (baseHeight * savedScale));
    startTimerHz (30);
    timerCallback();
}

FaseGroovePianoEditor::~FaseGroovePianoEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void FaseGroovePianoEditor::showStyleMenu()
{
    juce::PopupMenu menu;
    menu.addSectionHeader (tr ("Ready-made styles (the key stays)"));
    const auto names = FaseGroovePianoProcessor::styleNames();
    for (int i = 0; i < names.size(); ++i)
        menu.addItem (i + 1, tr (names[i]));

    juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&styleMenuButton), [safe] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;

        safe->processor.applyStyle (result - 1);
        safe->presetBox.setSelectedId (FaseGroovePianoProcessor::stylePreset (result - 1) + 1, juce::dontSendNotification);
    });
}

void FaseGroovePianoEditor::confirmReset()
{
    juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::QuestionIcon)
                                      .withTitle (tr ("RESET"))
                                      .withMessage (tr ("Put every setting of the plug-in back to its default? The language stays as it is."))
                                      .withButton (tr ("RESET"))
                                      .withButton (tr ("Cancel"))
                                      .withAssociatedComponent (this),
                                  [safe] (int result)
                                  {
                                      if (safe != nullptr && result == 1)
                                      {
                                          safe->processor.resetToDefaults();
                                          safe->afterReset();
                                      }
                                  });
}

void FaseGroovePianoEditor::applyTips()
{
    // Tips appear when the mouse rests on a control; they can be switched off in OPTIONS.
    if (Localisation::tipsEnabled())
    {
        if (tooltips == nullptr)
            tooltips = std::make_unique<juce::TooltipWindow> (this, 700);
    }
    else
    {
        tooltips.reset();
    }
}

void FaseGroovePianoEditor::parentHierarchyChanged()
{
    translateHostWindow();
}

void FaseGroovePianoEditor::translateHostWindow()
{
    // The Standalone app's window (Options button, feedback warning) has a few fixed English
    // texts; its menus and the audio settings follow the language through JUCE's TRANS().
    if (processor.wrapperType != juce::AudioProcessor::wrapperType_Standalone)
        return;

    auto* top = getTopLevelComponent();
    if (top == nullptr || top == this)
        return;

    static const char* const fixedTexts[] { "Options", "Mute audio input", "Feedback Loop:", "Audio input is muted to avoid feedback loop" };

    std::function<void (juce::Component&)> visit = [&] (juce::Component& c)
    {
        if (&c == this)
            return;

        auto& props = c.getProperties();
        auto translate = [&] (const juce::String& current, auto&& apply)
        {
            if (! props.contains ("fgpEnglish"))
                for (auto* t : fixedTexts)
                    if (current == t)
                        props.set ("fgpEnglish", t);

            if (props.contains ("fgpEnglish"))
                apply (tr (props["fgpEnglish"].toString()));
        };

        if (auto* b = dynamic_cast<juce::Button*> (&c))
            translate (b->getButtonText(), [b] (const juce::String& t) { b->setButtonText (t); });
        else if (auto* l = dynamic_cast<juce::Label*> (&c))
            translate (l->getText(), [l] (const juce::String& t) { l->setText (t, juce::dontSendNotification); });

        for (auto* child : c.getChildren())
            visit (*child);
    };

    visit (*top);
}

void FaseGroovePianoEditor::afterRestore()
{
    presetBox.setSelectedId (0, juce::dontSendNotification);
    freeToggle.setToggleState (processor.freeCircle, juce::dontSendNotification);
    circle.setFreeMode (processor.freeCircle);
    shrinkToggle.setToggleState (processor.shrinkWhileDragging, juce::dontSendNotification);
    timerCallback();
    repaint();
}

void FaseGroovePianoEditor::undo()
{
    if (undoStack.isEmpty())
        return;

    redoStack.add (lastSnapshot);
    lastSnapshot = undoStack[undoStack.size() - 1];
    undoStack.remove (undoStack.size() - 1);
    processor.restoreSnapshot (lastSnapshot);
    afterRestore();
}

void FaseGroovePianoEditor::redo()
{
    if (redoStack.isEmpty())
        return;

    undoStack.add (lastSnapshot);
    lastSnapshot = redoStack[redoStack.size() - 1];
    redoStack.remove (redoStack.size() - 1);
    processor.restoreSnapshot (lastSnapshot);
    afterRestore();
}

bool FaseGroovePianoEditor::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    if (mods.isCommandDown() && (key.getKeyCode() == 'Z' || key.getKeyCode() == 'z'))
    {
        mods.isShiftDown() ? redo() : undo();
        return true;
    }
    if (mods.isCommandDown() && (key.getKeyCode() == 'Y' || key.getKeyCode() == 'y'))
    {
        redo();
        return true;
    }
    return false;
}

void FaseGroovePianoEditor::showPresetMenu()
{
    juce::PopupMenu menu, remove;
    menu.addItem (1, tr ("Save current settings as..."));
    menu.addSeparator();

    const auto files = FaseGroovePianoProcessor::presetFiles();
    if (files.isEmpty())
        menu.addItem (-1, tr ("No presets yet"), false);
    for (int i = 0; i < files.size(); ++i)
    {
        menu.addItem (100 + i, files[i].getFileNameWithoutExtension());
        remove.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    }

    menu.addSeparator();
    menu.addSubMenu (tr ("Delete"), remove, ! files.isEmpty());
    menu.addItem (2, tr ("Open the presets folder"));

    juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&presetsButton), [safe, files] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;

        if (result == 1)
            safe->savePresetAs();
        else if (result == 2)
        {
            FaseGroovePianoProcessor::presetFolder().createDirectory();
            FaseGroovePianoProcessor::presetFolder().startAsProcess();
        }
        else if (result >= 1000 && result - 1000 < files.size())
            files[result - 1000].deleteFile();
        else if (result >= 100 && result - 100 < files.size())
        {
            safe->processor.loadPreset (files[result - 100]);
            safe->afterRestore();
        }
    });
}

void FaseGroovePianoEditor::savePresetAs()
{
    auto* window = new juce::AlertWindow (tr ("Save preset"), tr ("Name:"), juce::MessageBoxIconType::NoIcon, this);
    window->addTextEditor ("name", tr ("My groove"));
    window->addButton (tr ("Save"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton (tr ("Cancel"), 0, juce::KeyPress (juce::KeyPress::escapeKey));

    juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
    juce::Component::SafePointer<juce::AlertWindow> safeWindow (window);
    window->enterModalState (true, juce::ModalCallbackFunction::create ([safe, safeWindow] (int result)
    {
        if (safe != nullptr && safeWindow != nullptr && result == 1)
            safe->processor.savePreset (safeWindow->getTextEditorContents ("name"));
    }), true);
}

void FaseGroovePianoEditor::showSaveMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, tr ("Save this clip as .mid..."));
    menu.addItem (2, tr ("Save all parts to a folder..."));

    juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&saveButton), [safe] (int result)
    {
        if (safe == nullptr)
            return;
        if (result == 1) safe->saveClip();
        if (result == 2) safe->saveAllParts();
    });
}

void FaseGroovePianoEditor::saveAllParts()
{
    fileChooser = std::make_unique<juce::FileChooser> (tr ("Choose a folder"),
                                                       juce::File::getSpecialLocation (juce::File::userDocumentsDirectory));

    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                              [this] (const juce::FileChooser& chooser)
                              {
                                  const auto folder = chooser.getResult();
                                  if (! folder.isDirectory())
                                      return;

                                  // The loop (in the clip length you chose) and the whole song, each as
                                  // full, chords only and bass only: ready to drop on separate tracks.
                                  juce::Array<int> lengths { selectedClipBars() };
                                  if (selectedClipBars() != FaseGroovePianoProcessor::songArrangement)
                                      lengths.add (FaseGroovePianoProcessor::songArrangement);

                                  for (int bars : lengths)
                                      for (auto part : { FaseGroovePianoProcessor::ClipPart::all, FaseGroovePianoProcessor::ClipPart::chords,
                                                         FaseGroovePianoProcessor::ClipPart::bass })
                                      {
                                          auto file = folder.getChildFile (processor.clipFileName (bars, part));
                                          file.deleteFile();
                                          juce::FileOutputStream stream (file);
                                          if (stream.openedOk())
                                              processor.createMidiClip (bars, part).writeTo (stream);
                                      }

                                  folder.revealToUser();
                              });
}

void FaseGroovePianoEditor::afterReset()
{
    presetBox.setSelectedId (0, juce::dontSendNotification);
    shrinkToggle.setToggleState (processor.shrinkWhileDragging, juce::dontSendNotification);
    freeToggle.setToggleState (processor.freeCircle, juce::dontSendNotification);
    circle.setFreeMode (processor.freeCircle);
    for (auto* lock : segmentLocks)
        lock->setToggleState (false, juce::dontSendNotification);
    setSize (baseWidth, baseHeight);
    timerCallback();
    repaint();
}

void FaseGroovePianoEditor::buildPresetMenu()
{
    // Grouped by style, one heading per group, in columns (no endless list).
    const int selected = presetBox.getSelectedId();
    presetBox.clear (juce::dontSendNotification);

    const auto& presets = sgk::progressionPresets();
    const auto& categories = sgk::progressionCategories();
    for (size_t c = 0; c < categories.size(); ++c)
    {
        // Columns: soulful + circle | deep + modal | jackin' | chromatic & borrowed
        if (c == 2 || c == 4 || c == 5)
            presetBox.getRootMenu()->addColumnBreak();

        presetBox.addSectionHeading (tr (categories[c]));
        for (int i = 0; i < (int) presets.size(); ++i)
            if (juce::String (presets[(size_t) i].category) == categories[c])
                presetBox.addItem (presets[(size_t) i].name + juce::String ("   (") + tr (sgk::scaleName (presets[(size_t) i].scale)) + ")", i + 1);
    }

    presetBox.setSelectedId (selected, juce::dontSendNotification);
}

void FaseGroovePianoEditor::applyLanguage()
{
    for (auto* k : { &registerKnob, &strumKnob, &swing, &gate, &velocity, &humanize, &riffDensity, &tone, &reverb, &volume, &chorus, &pump })
        k->refreshText();

    for (auto* c : { &progLength, &chordLength, &key, &scale, &extension, &voicing, &bass, &approach, &topLine, &movement, &variation, &grooveBars, &pattern, &section,
                     &fills, &fillStyle, &playMode, &bassChannel, &soundType })
        c->refreshText();

    presetLabel.setText (Localisation::upper (tr ("Progression presets")), juce::dontSendNotification);
    presetBox.setTextWhenNothingSelected (tr ("Load a progression..."));
    clipLengthLabel.setText (Localisation::upper (tr ("Clip length")), juce::dontSendNotification);
    clipPartLabel.setText (Localisation::upper (tr ("Notes")), juce::dontSendNotification);

    static const char* const lengths[] { "Full loop", "1 bar", "2 bars", "4 bars", "8 bars", "16 bars", "32 bars",
                                         "Song: intro > drop > outro" };
    const int selectedLength = clipLength.getSelectedId();
    for (int i = 0; i < clipLength.getNumItems(); ++i)
        clipLength.changeItemText (i + 1, tr (lengths[i]));
    clipLength.setSelectedId (selectedLength, juce::dontSendNotification);

    static const char* const parts[] { "Chords + bass", "Chords only", "Bass only" };
    const int selectedPart = clipPart.getSelectedId();
    for (int i = 0; i < clipPart.getNumItems(); ++i)
        clipPart.changeItemText (i + 1, tr (parts[i]));
    clipPart.setSelectedId (selectedPart, juce::dontSendNotification);

    diceProgression.setButtonText (tr ("DICE"));
    newRiffButton.setButtonText (tr ("NEW RIFF"));
    previewButton.setButtonText (tr ("PREVIEW"));
    syncButton.setButtonText (tr ("SYNC DAW"));
    optionsButton.setButtonText (tr ("OPTIONS"));
    styleMenuButton.setButtonText (tr ("STYLE"));
    styleMenuButton.setTooltip (tr ("Ready-made setups by genre: progression, voicing, groove, feel and sound in one click (the key stays)"));
    resetButton.setButtonText (tr ("RESET"));
    presetsButton.setButtonText (tr ("PRESETS"));
    presetsButton.setTooltip (tr ("Save your own setups and load them in any project"));
    learnButton.setButtonText (tr ("LEARN"));
    learnButton.setTooltip (tr ("Learn: play chords on your MIDI keyboard, they fill the progression slot by slot (press again to stop)"));
    undoButton.setTooltip (tr ("Undo (Ctrl+Z)"));
    redoButton.setTooltip (tr ("Redo (Ctrl+Shift+Z)"));
    movement.box.setTooltip (tr ("Voicing movement: some hits play the chord an inversion up or down so the riff moves instead of repeating the same chord"));
    topLine.box.setTooltip (tr ("The melody the top notes of the chords draw across the progression"));
    variation.box.setTooltip (tr ("Every 4 or 8 bars the last bar plays a variation of the groove, so the loop never feels static"));
    grooveBars.box.setTooltip (tr ("1 bar, 2 bars, or 4 bars (the groove followed by a variation of it)"));
    resetButton.setTooltip (tr ("Put every setting back to its default (the language stays)"));
    soundToggle.setButtonText (tr ("Guide sound"));
    octaveToggle.setButtonText (tr ("Top +8va"));
    saveButton.setButtonText (tr ("Save .mid..."));
    shrinkToggle.setButtonText (tr ("Mini window while dragging"));

    octaveToggle.setTooltip (tr ("Double the top note of every chord an octave higher (bright house piano)"));
    freeToggle.setButtonText (tr ("Free chords"));
    freeToggle.setTooltip (tr ("Free chords: drag the dots anywhere on the circle, even outside the scale"));
    buildPresetMenu();
    approach.box.setTooltip (tr ("Gospel slide: on the last 16th before a chord change, play the next chord a half step above or below"));
    newRiffButton.setTooltip (tr ("New random funky riff (locked parts are kept)"));
    diceProgression.setTooltip (tr ("Random soulful-house progression"));
    syncButton.setTooltip (tr ("On: follow the DAW tempo. Off: set your own BPM."));
    tempoSlider.setTooltip (tr ("Drag, or double-click to type the BPM"));
    previewButton.setTooltip (tr ("Play the groove with the guide sound without starting the DAW"));
    shrinkToggle.setTooltip (tr ("Shrink the plug-in window while you drag a clip, so you can see your DAW"));
    patternView.setTooltip (tr ("Click a step: hit > accent > ghost > rest.  Right-click: more options"));
    bassButton.setTooltip (tr ("Left-hand bass on / off"));

    for (int i = 0; i < sgk::numSegments; ++i)
    {
        segmentBoxes[i]->setTooltip (tr ("Which part (A-D) plays here: combine parts like A A B C"));
        segmentDice[i]->setButtonText (tr ("RND"));
        segmentDice[i]->setTooltip (tr ("New random rhythm for this part"));
        segmentLocks[i]->setButtonText (tr ("LOCK"));
        segmentLocks[i]->setTooltip (tr ("Keep this part when you press NEW RIFF"));
    }

    options.refreshText();
    translateHostWindow();

    // Tips for every control (shown when the mouse rests on it).
    progLength.box.setTooltip (tr ("How many chords the progression has (1 to 8)"));
    chordLength.box.setTooltip (tr ("How long each chord lasts, from half a bar to 4 bars"));
    key.box.setTooltip (tr ("The key of the song"));
    scale.box.setTooltip (tr ("Scale or mode: Dorian is the deep house classic"));
    extension.box.setTooltip (tr ("How rich the chords are: triads, 7ths, 9ths, 11ths/13ths, 6/9, sus..."));
    voicing.box.setTooltip (tr ("How the chord notes are spread on the keyboard"));
    bass.box.setTooltip (tr ("What the left hand plays"));
    registerKnob.setTip (tr ("Moves the right hand up or down the keyboard"));
    strumKnob.setTip (tr ("Rolls the chord notes like a pianist instead of hitting them all together"));
    strumDirButton.setTooltip (tr ("Direction of the roll: up, down, or alternating (up on the beat, down off it). Click to change"));
    pattern.box.setTooltip (tr ("The rhythm of both hands, grouped by genre"));
    section.box.setTooltip (tr ("Reshapes the groove for intro, main, break, build-up, drop and outro"));
    fills.box.setTooltip (tr ("How often a fill leads into the next chord"));
    fillStyle.box.setTooltip (tr ("What the fill plays"));
    playMode.box.setTooltip (tr ("Play with the DAW, or while you hold a key (the key you hold sets the key)"));
    swing.setTip (tr ("Delays the off-beat 16ths for a shuffled, human feel"));
    gate.setTip (tr ("How long the chords ring: from short stabs to long chords"));
    velocity.setTip (tr ("How hard the notes are played"));
    humanize.setTip (tr ("Small timing and dynamics variations, like a real player"));
    riffDensity.setTip (tr ("Random riffs only: from sparse stabs to busy chops"));
    soundType.box.setTooltip (tr ("The guide sound you hear (the MIDI stays the same)"));
    bassChannel.box.setTooltip (tr ("Bass on the same MIDI channel, or on channel 2 to play it with another instrument"));
    tone.setTip (tr ("Brightness of the guide sound"));
    reverb.setTip (tr ("Reverb on the guide sound"));
    volume.setTip (tr ("Volume of the guide sound"));
    chorus.setTip (tr ("Widens the guide sound"));
    pump.setTip (tr ("Ducks the guide sound on every beat, like a kick sidechain"));
    soundToggle.setTooltip (tr ("Turns the built-in guide sound on or off (the MIDI keeps playing)"));
    presetBox.setTooltip (tr ("Ready-made progressions, grouped by style (they also set the scale)"));
    clipLength.setTooltip (tr ("How long the exported MIDI clip is"));
    clipPart.setTooltip (tr ("Which notes go into the clip: chords and bass, chords only or bass only"));
    dragTile.setTooltip (tr ("Drag this onto a track in your DAW to drop the groove as a MIDI clip"));
    saveButton.setTooltip (tr ("Save the clip, or every part at once, as .mid files"));
    optionsButton.setTooltip (tr ("Language, interface size, tips and credits"));
    for (auto* slot : slots)
        slot->setDegreeTip (tr ("Scale degree of this chord; the small arrows change its inversion"));
    currentKey = {};   // re-translates the key / scale line on the next timer tick
    timerCallback();
    repaint();
    for (auto* c : getChildren())
        c->repaint();
}

void FaseGroovePianoEditor::mouseUp (const juce::MouseEvent& e)
{
    // Clicking the logo opens the credits.
    const auto pos = e.getEventRelativeTo (this).position / uiScale;
    if (! miniMode && headerArea.toFloat().withWidth (300.0f).withHeight (56.0f).contains (pos))
        options.show (true, uiScale, processor.shrinkWhileDragging);
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

void FaseGroovePianoEditor::cellClicked (bool bassRow, int step, const juce::MouseEvent& e)
{
    const char current = processor.stepSymbol (bassRow, step);

    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        menu.addSectionHeader (tr (bassRow ? "Bass" : "Chords") + " - " + tr ("step") + " " + juce::String (step + 1));
        menu.addItem (1, tr ("Hit"), true, current == 'x');
        menu.addItem (2, tr ("Accent"), true, current == 'X');
        menu.addItem (3, tr ("Ghost note"), true, current == 'g');
        menu.addItem (4, tr ("Push (anticipate the next chord)"), true, current == '>');
        menu.addItem (5, tr ("Hold (tie from the previous note)"), true, current == '=');
        menu.addItem (6, tr ("Rest"), true, current == '.');
        if (! bassRow)
        {
            menu.addSeparator();
            menu.addItem (7, tr ("Hit, voicing up"), true, current == 'u' || current == 'U');
            menu.addItem (8, tr ("Hit, voicing down"), true, current == 'd' || current == 'D');
        }

        juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&patternView),
                            [safe, bassRow, step] (int result)
                            {
                                static constexpr char symbols[] { 0, 'x', 'X', 'g', '>', '=', '.', 'u', 'd' };
                                if (safe != nullptr && result > 0 && result < 9)
                                    safe->processor.editStep (bassRow, step, symbols[result]);
                            });
        return;
    }

    // Click cycles: rest -> hit -> accent -> ghost -> rest.
    const char next = current == 'x' ? 'X' : current == 'X' ? 'g' : current == 'g' ? '.' : 'x';
    processor.editStep (bassRow, step, next);
}

void FaseGroovePianoEditor::enterMiniMode()
{
    if (miniMode)
        return;

    miniMode = true;
    fullSize = { getWidth(), getHeight() };
    miniStartedAt = juce::Time::getMillisecondCounter();

    hiddenForMini.clear();
    for (auto* c : getChildren())
    {
        if (c == &dragTile || ! c->isVisible()
            || dynamic_cast<juce::ResizableCornerComponent*> (c) != nullptr
            || dynamic_cast<juce::TooltipWindow*> (c) != nullptr)
            continue;

        hiddenForMini.push_back (c);
        c->setVisible (false);
    }

    getConstrainer()->setFixedAspectRatio (0.0);
    setResizeLimits (miniWidth / 2, miniHeight / 2, baseWidth * 2, baseHeight * 2);
    setSize (juce::roundToInt (miniWidth * uiScale), juce::roundToInt (miniHeight * uiScale));
}

void FaseGroovePianoEditor::exitMiniMode()
{
    if (! miniMode)
        return;

    miniMode = false;
    for (auto& c : hiddenForMini)
        if (c != nullptr)
            c->setVisible (true);
    hiddenForMini.clear();

    setResizeLimits ((int) (baseWidth * 0.6f), (int) (baseHeight * 0.6f), baseWidth * 2, baseHeight * 2);
    getConstrainer()->setFixedAspectRatio ((double) baseWidth / baseHeight);
    setSize (fullSize.x, fullSize.y);
}

void FaseGroovePianoEditor::loadPreset (int index)
{
    processor.applyProgressionPreset (index);
}

//==============================================================================
void FaseGroovePianoEditor::timerCallback()
{
    if (miniMode)
    {
        // Safety net: leave mini mode once the drag is over, even if the host never told us.
        if (juce::Time::getMillisecondCounter() - miniStartedAt > 800
            && ! juce::ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown())
            exitMiniMode();
        return;
    }

    auto s = processor.currentSettings();
    const int heldRoot = processor.uiHeldRoot.load();
    if (heldRoot >= 0)
        s.keyRoot = heldRoot;

    std::array<sgk::Chord, sgk::maxProgressionLength> chords;
    std::array<sgk::VoicedChord, sgk::maxProgressionLength> voiced;
    sgk::GrooveEngine::voiceProgression (s, chords, voiced);

    // LEARN: chords heard on the MIDI keyboard fill the progression.
    learnButton.setToggleState (processor.learnMode.load(), juce::dontSendNotification);
    if (const int heard = processor.learnedCount.load(); heard != lastLearnCount)
    {
        lastLearnCount = heard;
        if (processor.learnMode.load() && processor.learnChord (processor.learnedMask.load(), processor.learnedBass.load()) >= 0)
            presetBox.setSelectedId (0, juce::dontSendNotification);
    }

    // Undo history: a snapshot every half second; a change pushes the previous one.
    if (++snapshotTicks >= 15)
    {
        snapshotTicks = 0;
        const auto now = processor.snapshot();
        if (lastSnapshot.isEmpty())
            lastSnapshot = now;
        else if (now != lastSnapshot)
        {
            undoStack.add (lastSnapshot);
            if (undoStack.size() > 60)
                undoStack.remove (0);
            redoStack.clear();
            lastSnapshot = now;
        }
    }
    undoButton.setEnabled (! undoStack.isEmpty());
    redoButton.setEnabled (! redoStack.isEmpty());
    undoButton.setAlpha (undoStack.isEmpty() ? 0.3f : 1.0f);
    redoButton.setAlpha (redoStack.isEmpty() ? 0.3f : 1.0f);

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        slots[i]->setInversion (s.inversions[(size_t) i]);

    {
        static const char* const directions[] { "Up", "Down", "Alternate" };
        const auto text = Localisation::upper (tr (directions[juce::jlimit (0, 2, s.strumDirection)]));
        if (strumDirButton.getButtonText() != text)
            strumDirButton.setButtonText (text);
    }

    const int playingSlot = processor.uiSlot.load();
    const auto harmony = sgk::GrooveEngine::harmonyScores (s, chords, voiced);

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
    {
        const bool enabled = i < s.progressionLength;
        const auto chord = sgk::GrooveEngine::chordForSlot (s, i);
        slots[i]->setInfo (sgk::chordName (chord), juce::String::fromUTF8 (sgk::GrooveEngine::numeralForSlot (s, i).c_str()),
                           enabled, enabled && i == playingSlot, s.freeChords[(size_t) i] >= 0, s.progression[(size_t) i],
                           enabled ? harmony.slot[(size_t) i] : 1.0f);
    }

    const int shown = playingSlot >= 0 ? playingSlot : 0;
    const juce::String chordText = sgk::chordName (chords[(size_t) shown]);
    const juce::String romanText = juce::String::fromUTF8 (sgk::GrooveEngine::numeralForSlot (s, shown).c_str());
    const juce::String keyText = juce::String (sgk::noteName (s.keyRoot)) + " " + tr (sgk::scaleName (s.scale));

    if (chordText != currentChord || romanText != currentRoman || keyText != currentKey || showingHold != (heldRoot >= 0))
    {
        currentChord = chordText;
        currentRoman = romanText;
        currentKey = keyText;
        showingHold = heldRoot >= 0;
        repaint (chordDisplayArea.toFloat().transformedBy (juce::AffineTransform::scale (uiScale)).getSmallestIntegerContainer().expanded (2));
    }

    const bool bassOn = processor.isBassOn();
    patternView.setState (sgk::GrooveEngine::resolvePattern (s), processor.uiStep.load(), bassOn);

    bassButton.setButtonText (tr (bassOn ? "BASS: ON" : "BASS: OFF"));
    bassButton.setColour (juce::TextButton::buttonColourId, bassOn ? Palette::accent2 : Palette::background);
    bassButton.setColour (juce::TextButton::textColourOffId, bassOn ? Palette::background : Palette::dim);

    for (int i = 0; i < sgk::numSegments; ++i)
        segmentLocks[i]->setToggleState (processor.isSegmentLocked (processor.segmentAtStep (i * sgk::userPatternLength / sgk::numSegments)),
                                         juce::dontSendNotification);

    std::vector<CircleOfFifths::ChordMark> marks;
    std::vector<float> moves;
    for (int i = 0; i < s.progressionLength; ++i)
    {
        marks.push_back ({ chords[(size_t) i].rootPc, chords[(size_t) i].third == 3, {}, harmony.slot[(size_t) i] });
        moves.push_back (harmony.transition[(size_t) i]);
    }
    std::vector<CircleOfFifths::ChordMark> diatonic;
    for (int d = 0; d < 7; ++d)
    {
        const auto chord = sgk::buildChord (s.keyRoot, s.scale, d, s.extension);
        diatonic.push_back ({ chord.rootPc, chord.third == 3, juce::String::fromUTF8 (sgk::romanNumeral (d, chord).c_str()) });
    }
    circle.setState (s.keyRoot, sgk::scaleIsMinor (s.scale), tr (sgk::scaleName (s.scale)), marks, moves, playingSlot, diatonic);
    previewButton.setToggleState (processor.previewOn.load(), juce::dontSendNotification);

    const bool synced = syncButton.getToggleState();
    tempoSlider.setVisible (! synced);
    dawTempo.setVisible (synced);
    dawTempo.setText (juce::String (processor.hostBpm.load(), 1) + " BPM  " + tr ("(DAW)"), juce::dontSendNotification);
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
    g.addTransform (juce::AffineTransform::scale (uiScale));

    if (miniMode)
    {
        g.setColour (Palette::dim);
        g.setFont (font (11.0f));
        g.drawText (tr ("Drop the clip on a track in your DAW"), juce::Rectangle<int> (0, miniHeight - 22, miniWidth, 18),
                    juce::Justification::centred);
        return;
    }

    // Header: logo + credit line
    auto header = headerArea.toFloat();
    Branding::drawFullLogo (g, header.removeFromTop (56.0f).withWidth (300.0f));
    g.setColour (Palette::dim);
    g.setFont (font (11.5f));
    g.drawText (tr ("by") + " " + Branding::artist, header.withTrimmedLeft (66.0f), juce::Justification::centredLeft);

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
    g.drawText ((showingHold ? tr ("HOLD: ") : juce::String()) + currentKey, cd, juce::Justification::centredRight);

    drawPanel (g, tempoArea, tr ("TEMPO"));
    drawPanel (g, progressionPanel, tr ("PROGRESSION"));
    drawPanel (g, harmonyPanel, tr ("HARMONY & SCALES"));
    drawPanel (g, circlePanel, tr ("CIRCLE OF FIFTHS"));
    drawPanel (g, groovePanel, tr ("GROOVE"));
    drawPanel (g, soundPanel, tr ("GUIDE SOUND & MIDI OUT"));
    drawPanel (g, gridPanel, tr ("RHYTHM GRID"));
    drawPanel (g, clipPanel, tr ("MIDI CLIP"));

    auto hint = clipPanel.reduced (14, 8).removeFromBottom (16);
    g.setColour (Palette::dim);
    g.setFont (font (11.0f));
    g.drawText (tr ("Drop it on any instrument track (your own piano / Rhodes)"), hint, juce::Justification::centredLeft);
}

void FaseGroovePianoEditor::resized()
{
    if (miniMode)
    {
        uiScale = juce::jmax (0.3f, getHeight() / (float) miniHeight);
        layoutMini();
    }
    else
    {
        uiScale = juce::jmax (0.3f, getWidth() / (float) baseWidth);
        processor.editorScale = uiScale;
        layoutFull();
    }

    // Everything is laid out at base size and scaled as a whole.
    for (auto* c : getChildren())
        if (dynamic_cast<juce::ResizableCornerComponent*> (c) == nullptr && dynamic_cast<juce::TooltipWindow*> (c) == nullptr)
            c->setTransform (juce::AffineTransform::scale (uiScale));
}

void FaseGroovePianoEditor::layoutMini()
{
    dragTile.setBounds (juce::Rectangle<int> (0, 0, miniWidth, miniHeight).reduced (10).withTrimmedBottom (14));
}

void FaseGroovePianoEditor::layoutFull()
{
    auto r = juce::Rectangle<int> (0, 0, baseWidth, baseHeight).reduced (20, 14);

    // Header
    auto top = r.removeFromTop (76);
    chordDisplayArea = top.removeFromRight (380).reduced (0, 4);
    top.removeFromRight (10);
    tempoArea = top.removeFromRight (330).reduced (0, 4);
    headerArea = top;
    optionsButton.setBounds (headerArea.getRight() - 78, headerArea.getBottom() - 22, 74, 20);
    resetButton.setBounds (optionsButton.getBounds().translated (-62, 0).withWidth (58));
    styleMenuButton.setBounds (resetButton.getBounds().translated (-70, 0).withWidth (66));
    {
        auto topRow = juce::Rectangle<int> (headerArea.getRight() - 118, headerArea.getY() + 4, 114, 22);
        redoButton.setBounds (topRow.removeFromRight (20).reduced (2));
        topRow.removeFromRight (4);
        undoButton.setBounds (topRow.removeFromRight (20).reduced (2));
        topRow.removeFromRight (6);
        presetsButton.setBounds (topRow);
    }
    options.setBounds (0, 0, baseWidth, baseHeight);
    updateOverlay.setBounds (0, 0, baseWidth, baseHeight);
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
        auto leftCol = inner.removeFromLeft (276);
        presetLabel.setBounds (leftCol.removeFromTop (16));
        auto presetRow = leftCol.removeFromTop (28);
        learnButton.setBounds (presetRow.removeFromRight (58));
        presetRow.removeFromRight (4);
        diceProgression.setBounds (presetRow.removeFromRight (54));
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
    auto mid = r.removeFromTop (340);
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
        auto leftCol = last.removeFromLeft (124);
        bass.setBounds (leftCol.removeFromTop (44));
        leftCol.removeFromTop (8);
        octaveToggle.setBounds (leftCol.removeFromTop (24));
        leftCol.removeFromTop (10);
        approach.setBounds (leftCol.removeFromTop (44));
        leftCol.removeFromTop (6);
        topLine.setBounds (leftCol.removeFromTop (44));
        last.removeFromLeft (6);
        movement.setBounds (last.removeFromBottom (44));
        last.removeFromBottom (6);
        const int kw = last.getWidth() / 2;
        const int kh = juce::jmin (kw + 34, last.getHeight());
        registerKnob.setBounds (last.removeFromLeft (kw).withSizeKeepingCentre (kw, kh));
        strumKnob.setBounds (last.withSizeKeepingCentre (kw, kh));
        strumDirButton.setBounds (strumKnob.getBounds().getCentreX() - 34, strumKnob.getBottom() + 2, 68, 18);
    }

    {
        auto inner = circlePanel.reduced (10, 8).withTrimmedTop (22);
        freeToggle.setBounds (inner.removeFromBottom (24).withTrimmedLeft (4));
        circle.setBounds (inner);
    }

    {
        auto inner = groovePanel.reduced (14, 8);
        inner.removeFromTop (22);
        auto grooveRow = row (inner, 44);
        newRiffButton.setBounds (grooveRow.removeFromRight (84).withTrimmedTop (16));
        pattern.setBounds (grooveRow.withTrimmedRight (8));
        split (row (inner, 44), section, fills, inner.getWidth() / 2 - 5);
        split (row (inner, 44), fillStyle, playMode, inner.getWidth() / 2 - 5);
        split (row (inner, 44), variation, grooveBars, inner.getWidth() / 2 - 5);
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
        soundType.setBounds (row (inner, 44));
        bassChannel.setBounds (row (inner, 44));
        const int w = inner.getWidth() / 3;
        auto top = inner.removeFromTop (inner.getHeight() / 2);
        for (auto* k : { &tone, &reverb, &volume })
            k->setBounds (top.removeFromLeft (w));
        auto bottom = inner.withSizeKeepingCentre (w * 2, inner.getHeight());
        for (auto* k : { &chorus, &pump })
            k->setBounds (bottom.removeFromLeft (w));
    }

    r.removeFromTop (10);
    clipPanel = r.removeFromRight (300);
    r.removeFromRight (10);
    gridPanel = r;
    {
        auto inner = gridPanel.reduced (14, 8);
        bassButton.setBounds (inner.removeFromTop (22).removeFromRight (92).withTrimmedBottom (2));

        auto strip = inner.removeFromTop (24);
        strip.removeFromLeft ((int) PatternView::labelWidth);
        const int segW = strip.getWidth() / sgk::numSegments;
        for (int i = 0; i < sgk::numSegments; ++i)
        {
            auto cell = strip.removeFromLeft (segW).reduced (4, 1);
            segmentBoxes[i]->setBounds (cell.removeFromLeft (54));
            cell.removeFromLeft (6);
            segmentLocks[i]->setBounds (cell.removeFromRight (52));
            cell.removeFromRight (6);
            segmentDice[i]->setBounds (cell.removeFromRight (46));
        }

        inner.removeFromTop (2);
        patternView.setBounds (inner);
    }

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
        inner.removeFromTop (4);
        shrinkToggle.setBounds (inner.removeFromTop (22));
    }
}
