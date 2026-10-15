// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PluginEditor.h"

#include <BinaryData.h>

#include <map>
#include <numeric>

namespace
{
    juce::Font font (float height, bool bold = false)
    {
        return juce::Font (juce::FontOptions (height).withStyle (bold ? "Bold" : "Regular"));
    }

    void styleCaption (juce::Label& label, const juce::String& text)
    {
        label.setText (Localisation::upper (text), juce::dontSendNotification);
        auto f = font (10.0f, true);
        f.setExtraKerningFactor (0.08f);
        label.setFont (f);
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
    regular = juce::Typeface::createSystemTypefaceFor (BinaryData::InterRegular_otf, BinaryData::InterRegular_otfSize);
    semiBold = juce::Typeface::createSystemTypefaceFor (BinaryData::InterSemiBold_otf, BinaryData::InterSemiBold_otfSize);

    setColour (juce::ResizableWindow::backgroundColourId, Palette::background);
    setColour (juce::TextButton::buttonColourId, Palette::control);
    setColour (juce::TextEditor::backgroundColourId, Palette::field);
    setColour (juce::TextEditor::outlineColourId, Palette::panelEdge);
    setColour (juce::TextEditor::focusedOutlineColourId, Palette::accent.withAlpha (0.6f));
    setColour (juce::ListBox::backgroundColourId, Palette::field);
    setColour (juce::ScrollBar::thumbColourId, Palette::panelEdge.brighter (0.3f));
    setColour (juce::ComboBox::backgroundColourId, Palette::field);
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

juce::Typeface::Ptr GrooveLookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    // The default sans-serif becomes Inter (bold styles: Inter SemiBold), the same on every system.
    if (f.getTypefaceName() == juce::Font::getDefaultSansSerifFontName() && regular != nullptr && semiBold != nullptr)
        return f.isBold() || f.getTypefaceStyle().containsIgnoreCase ("bold") ? semiBold : regular;
    return juce::LookAndFeel_V4::getTypefaceForFont (f);
}

void GrooveLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour& background, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const float corner = 4.0f;
    const bool flatLeft = b.isConnectedOnLeft(), flatRight = b.isConnectedOnRight();

    // The near-black "background" colour many buttons ask for becomes the raised control colour.
    auto fillColour = background == Palette::background ? Palette::control : background;
    if (! b.isEnabled())
        fillColour = fillColour.withMultipliedAlpha (0.5f);
    else if (down)
        fillColour = fillColour.darker (0.15f);
    else if (highlighted)
        fillColour = fillColour.brighter (0.08f);

    juce::Path shape;
    shape.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), corner, corner,
                               ! flatLeft, ! flatRight, ! flatLeft, ! flatRight);
    g.setColour (fillColour);
    g.fillPath (shape);

    const bool coloured = fillColour.getPerceivedBrightness() > 0.35f;
    g.setColour (coloured ? fillColour.darker (0.25f) : (highlighted ? Palette::panelEdge.brighter (0.35f) : Palette::panelEdge));
    g.strokePath (shape, juce::PathStrokeType (1.0f));
}

juce::Font GrooveLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return juce::Font (juce::FontOptions (juce::jlimit (10.0f, 13.5f, buttonHeight * 0.5f)).withStyle ("Bold"));
}

void GrooveLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (0.5f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (box.hasKeyboardFocus (true) || box.isMouseOver (true) ? Palette::panelEdge.brighter (0.4f) : Palette::panelEdge);
    g.drawRoundedRectangle (r, 4.0f, 1.0f);

    // A small chevron.
    const float cx = (float) width - 12.0f, cy = height * 0.5f;
    juce::Path chevron;
    chevron.startNewSubPath (cx - 3.5f, cy - 1.8f);
    chevron.lineTo (cx, cy + 1.8f);
    chevron.lineTo (cx + 3.5f, cy - 1.8f);
    g.setColour (box.isEnabled() ? Palette::dim : Palette::dim.withAlpha (0.4f));
    g.strokePath (chevron, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font GrooveLookAndFeel::getComboBoxFont (juce::ComboBox& box)
{
    return juce::Font (juce::FontOptions (juce::jlimit (11.0f, 14.0f, box.getHeight() * 0.48f)));
}

void GrooveLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (2, 1, box.getWidth() - 22, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

void GrooveLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (Palette::panel);
    g.setColour (Palette::panelEdge);
    g.drawRect (0, 0, width, height, 1);
}

juce::Font GrooveLookAndFeel::getPopupMenuFont()
{
    return juce::Font (juce::FontOptions (14.0f));
}

void GrooveLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar&, int x, int y, int width, int height, bool vertical,
                                       int thumbStart, int thumbSize, bool over, bool down)
{
    auto track = juce::Rectangle<int> (x, y, width, height).toFloat();
    g.setColour (Palette::field);
    g.fillRoundedRectangle (track, 3.0f);
    auto thumb = vertical ? juce::Rectangle<float> ((float) x, (float) thumbStart, (float) width, (float) thumbSize)
                          : juce::Rectangle<float> ((float) thumbStart, (float) y, (float) thumbSize, (float) height);
    g.setColour (down ? Palette::accent : over ? Palette::panelEdge.brighter (0.6f) : Palette::panelEdge.brighter (0.3f));
    g.fillRoundedRectangle (thumb.reduced (1.5f), 3.0f);
}

void GrooveLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int width, int height)
{
    auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);
    g.setColour (Palette::field);
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (r.reduced (0.5f), 4.0f, 1.0f);
    g.setColour (Palette::text);
    g.setFont (juce::Font (juce::FontOptions (13.0f)));
    g.drawFittedText (text, r.reduced (10.0f, 6.0f).toNearestInt(), juce::Justification::centredLeft, 8, 1.0f);
}

juce::Rectangle<int> GrooveLookAndFeel::getTooltipBounds (const juce::String& text, juce::Point<int> screenPos, juce::Rectangle<int> parentArea)
{
    const juce::Font f (juce::FontOptions (13.0f));
    const int w = juce::jmin (420, juce::roundToInt (juce::GlyphArrangement::getStringWidth (f, text)) + 22);
    const int lines = juce::jmax (1, juce::roundToInt (juce::GlyphArrangement::getStringWidth (f, text) / 398.0f + 0.5f));
    const int h = juce::roundToInt (lines * f.getHeight() * 1.2f) + 12;
    return juce::Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                                 screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6) : screenPos.y + 6, w, h)
        .constrainedWithin (parentArea);
}

void GrooveLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    // Secondary buttons speak in plain text; the accent is kept for filled (primary) buttons.
    const bool on = b.getToggleState();
    auto colour = b.findColour (on ? juce::TextButton::textColourOnId : juce::TextButton::textColourOffId);
    const auto fillColour = b.findColour (on ? juce::TextButton::buttonOnColourId : juce::TextButton::buttonColourId);
    const bool filled = fillColour != Palette::background && fillColour != Palette::control && fillColour.getPerceivedBrightness() > 0.35f;
    if (! filled && colour == Palette::accent)
        colour = Palette::text.withAlpha (0.9f);
    g.setColour (colour.withMultipliedAlpha (b.isEnabled() ? 1.0f : 0.5f));
    g.setFont (getTextButtonFont (b, b.getHeight()));
    g.drawFittedText (b.getButtonText(), b.getLocalBounds().reduced (4, 2), juce::Justification::centred, 1, 0.75f);
}

juce::Font GrooveLookAndFeel::getLabelFont (juce::Label& label)
{
    return label.getFont();
}

void GrooveLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                                          float startAngle, float endAngle, juce::Slider&)
{
    // A studio knob: thin track, accent value arc, a dark body with a soft top light and a pointer.
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float lineW = juce::jlimit (2.0f, 3.5f, radius * 0.11f);
    const float arcR = radius - lineW * 0.5f;

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (Palette::panelEdge);
    g.strokePath (track, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);
    g.setColour (Palette::accent);
    g.strokePath (value, juce::PathStrokeType (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    const float knobR = arcR - lineW - 3.0f;
    const auto body = juce::Rectangle<float> (knobR * 2.0f, knobR * 2.0f).withCentre (centre);
    g.setColour (juce::Colours::black.withAlpha (0.35f));
    g.fillEllipse (body.translated (0.0f, 1.5f));
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2c313b), centre.x, body.getY(),
                                             juce::Colour (0xff181b21), centre.x, body.getBottom(), false));
    g.fillEllipse (body);
    g.setColour (juce::Colour (0xff3a404b));
    g.drawEllipse (body.reduced (0.5f), 1.0f);

    g.setColour (Palette::text);
    g.drawLine ({ centre.getPointOnCircumference (knobR * 0.35f, angle), centre.getPointOnCircumference (knobR * 0.85f, angle) }, 2.0f);
}

void GrooveLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool highlighted, bool)
{
    auto r = b.getLocalBounds().toFloat();
    auto pill = r.removeFromLeft (34.0f).withSizeKeepingCentre (30.0f, 16.0f);
    const bool on = b.getToggleState();

    g.setColour (on ? Palette::accent : Palette::field);
    g.fillRoundedRectangle (pill, 8.0f);
    g.setColour (on ? Palette::accent.darker (0.2f) : (highlighted ? Palette::panelEdge.brighter (0.4f) : Palette::panelEdge));
    g.drawRoundedRectangle (pill.reduced (0.5f), 8.0f, 1.0f);

    const float d = 12.0f;
    const float cx = on ? pill.getRight() - d - 2.0f : pill.getX() + 2.0f;
    g.setColour (on ? Palette::background : Palette::dim);
    g.fillEllipse (cx, pill.getCentreY() - d * 0.5f, d, d);

    g.setColour (Palette::text);
    g.setFont (font (13.0f));
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

    g.setColour (Palette::field.withMultipliedAlpha (alpha));
    g.fillRoundedRectangle (r, 6.0f);
    if (isPlaying)
    {
        g.setColour (Palette::accent.withAlpha (0.07f));
        g.fillRoundedRectangle (r, 6.0f);
    }
    g.setColour ((isPlaying ? Palette::accent : Palette::panelEdge).withMultipliedAlpha (alpha));
    g.drawRoundedRectangle (r, 6.0f, isPlaying ? 1.5f : 1.0f);

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

juce::String PatternView::getTooltip()
{
    // On a hold, say what it is: it looks small, but it is the note before ringing on.
    const auto& line = hoverBass ? pattern.leftHand : pattern.rightHand;
    if (hoverStep >= 0 && hoverStep < (int) line.size() && line[(size_t) hoverStep] == '=')
        return tr ("Hold: the note before keeps ringing (no new note). Its velocity is the one of that note.");
    return SettableTooltipClient::getTooltip();
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
                // Hold: the note before rings on. Drawn as its tail, joined to it (not a new note).
                auto tail = cell.withSizeKeepingCentre (cell.getWidth() + 3.0f, juce::jmax (3.0f, cell.getHeight() * 0.22f)).withTrimmedLeft (-3.0f);
                g.setColour (colour.withAlpha (0.6f));
                g.fillRect (tail);
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
void SidechainMeter::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    if (noDepth)
    {
        // Sidechain chosen but no depth: say which knob does it.
        g.setColour (Palette::accent);
        g.setFont (font (10.0f, true));
        g.drawText (tr ("Pump 0%"), r, juce::Justification::centredRight);
        return;
    }

    const auto bar = r.withSizeKeepingCentre (r.getWidth(), 5.0f);
    g.setColour (Palette::panelEdge);
    g.fillRoundedRectangle (bar, 2.0f);
    const float db = juce::Decibels::gainToDecibels (level, -60.0f);
    const float t = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
    g.setColour (t > 0.0f ? Palette::level : Palette::dim);
    g.fillRoundedRectangle (bar.withWidth (juce::jmax (2.0f, bar.getWidth() * t)), 2.0f);
}

//==============================================================================
void VelocityView::setState (const Row& chords, const Row& bass, int step)
{
    if (step == currentStep && chords == rows[0] && bass == rows[1])
        return;
    rows[0] = chords;
    rows[1] = bass;
    currentStep = step;
    repaint();
}

juce::Rectangle<float> VelocityView::rowArea (bool bassRow) const
{
    const float rowH = (getHeight() - headerHeight) / 2.0f;
    return { labelWidth, headerHeight + (bassRow ? rowH : 0.0f), getWidth() - labelWidth, rowH };
}

int VelocityView::stepAt (float x) const
{
    const int len = (int) rows[0].line.size();
    if (len == 0 || x < labelWidth)
        return -1;
    return juce::jlimit (0, len - 1, (int) ((x - labelWidth) / ((getWidth() - labelWidth) / (float) len)));
}

int VelocityView::levelAt (bool bassRow, float y) const
{
    // The fader track inside a cell: bottom = 1, top = 127.
    const auto area = rowArea (bassRow).reduced (0.0f, 6.0f);
    const float t = 1.0f - (y - area.getY()) / juce::jmax (1.0f, area.getHeight());
    return juce::jlimit (1, 127, juce::roundToInt (1.0f + 126.0f * t));
}

int VelocityView::ownerOf (int row, int step) const
{
    const auto& line = rows[row].line;
    while (step > 0 && step < (int) line.size() && line[(size_t) step] == '=')
        --step;
    return step;
}

void VelocityView::setFrom (juce::Point<float> p)
{
    const int step = ownerOf (dragBass ? 1 : 0, stepAt (p.x));
    const auto& row = rows[dragBass ? 1 : 0];
    if (step < 0 || step >= (int) row.line.size() || ! row.enabled || ! isHit (row.line[(size_t) step]))
        return;

    const int level = levelAt (dragBass, p.y);
    if (step < (int) row.levels.size() && row.levels[(size_t) step] == level)
        return;

    // Shown at once; the processor gets it too and the next refresh confirms it.
    if (step < (int) rows[dragBass ? 1 : 0].levels.size())
        rows[dragBass ? 1 : 0].levels[(size_t) step] = level;
    dragStep = step;
    if (onLevel != nullptr)
        onLevel (dragBass, step, level);
    repaint();
}

void VelocityView::mouseDown (const juce::MouseEvent& e)
{
    if (e.position.y < headerHeight || e.position.x < labelWidth)
        return;
    dragBass = e.position.y >= rowArea (true).getY();

    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        menu.addSectionHeader (tr ("Velocity"));
        menu.addItem (1, dragBass ? tr ("Bass: all automatic") : tr ("Chords: all automatic"));
        menu.addItem (2, tr ("All automatic"));
        const bool bassRow = dragBass;
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this).withMousePosition(),
                            [this, bassRow] (int r)
                            {
                                if (r > 0 && onReset != nullptr)
                                    onReset (r == 2 || ! bassRow, r == 2 || bassRow);
                            });
        return;
    }

    dragging = true;
    setFrom (e.position);
}

void VelocityView::mouseDrag (const juce::MouseEvent& e)
{
    if (dragging)
        setFrom (e.position);
}

void VelocityView::mouseUp (const juce::MouseEvent&)
{
    dragging = false;
    dragStep = -1;
    repaint();
}

void VelocityView::mouseDoubleClick (const juce::MouseEvent& e)
{
    const bool bassRow = e.position.y >= rowArea (true).getY();
    const int step = ownerOf (bassRow ? 1 : 0, stepAt (e.position.x));
    auto& row = rows[bassRow ? 1 : 0];
    if (step < 0 || ! row.enabled || step >= (int) row.line.size() || ! isHit (row.line[(size_t) step]))
        return;
    if (step < (int) row.levels.size())
        row.levels[(size_t) step] = 0;
    if (onLevel != nullptr)
        onLevel (bassRow, step, 0);
    repaint();
}

void VelocityView::mouseMove (const juce::MouseEvent& e)
{
    const int step = e.position.y < headerHeight ? -1 : stepAt (e.position.x);
    const bool bassRow = e.position.y >= rowArea (true).getY();
    if (step != hoverStep || bassRow != hoverBass)
    {
        hoverStep = step;
        hoverBass = bassRow;
        repaint();
    }
}

void VelocityView::mouseExit (const juce::MouseEvent&)
{
    hoverStep = -1;
    repaint();
}

void VelocityView::paint (juce::Graphics& g)
{
    const int len = (int) rows[0].line.size();
    if (len == 0)
        return;

    const auto area = getLocalBounds().toFloat();
    const float cellW = (area.getWidth() - labelWidth) / (float) len;

    g.setFont (font (11.0f, true));
    for (int s = 0; s < len; s += 4)
    {
        g.setColour (s % 16 == 0 ? Palette::level : Palette::dim);
        g.drawText (juce::String ((s / 4) % 4 + 1), juce::Rectangle<float> (labelWidth + s * cellW, 0.0f, cellW * 2.0f, 14.0f),
                    juce::Justification::centredLeft);
    }

    if (len == sgk::userPatternLength)
    {
        g.setColour (Palette::level.withAlpha (0.3f));
        for (int k = 1; k < sgk::numSegments; ++k)
            g.fillRect (labelWidth + (float) (k * len / sgk::numSegments) * cellW - 0.75f, headerHeight, 1.5f, area.getHeight() - headerHeight);
    }

    for (int r = 0; r < 2; ++r)
    {
        const auto& row = rows[r];
        const auto rowBounds = rowArea (r == 1);
        g.setColour (Palette::dim);
        g.setFont (font (11.0f, true));
        g.drawFittedText (r == 0 ? tr ("CHORDS") : tr ("BASS"),
                          juce::Rectangle<float> (0.0f, rowBounds.getY(), labelWidth - 2.0f, rowBounds.getHeight()).toNearestInt(),
                          juce::Justification::centredLeft, 1, 0.6f);

        if (! row.enabled)
        {
            g.setColour (Palette::dim.withAlpha (0.6f));
            g.drawFittedText (row.message, rowBounds.toNearestInt().reduced (8, 0), juce::Justification::centred, 2);
            continue;
        }

        // Guide lines: the automatic levels of a hit and of a ghost note.
        const auto track = rowBounds.reduced (0.0f, 6.0f);
        auto yFor = [&track] (int level) { return track.getBottom() - track.getHeight() * (float) (level - 1) / 126.0f; };
        g.setColour (Palette::panelEdge.withAlpha (0.6f));
        for (int level : { 118, 65 })
            g.fillRect (track.getX(), yFor (level) - 0.5f, track.getWidth(), 1.0f);

        for (int s = 0; s < len && s < (int) row.line.size(); ++s)
        {
            const char c = row.line[(size_t) s];
            auto cell = juce::Rectangle<float> (labelWidth + s * cellW, track.getY(), cellW, track.getHeight()).reduced (juce::jmin (3.0f, cellW * 0.18f), 0.0f);

            g.setColour (Palette::panelEdge.withAlpha (s % 4 == 0 ? 0.55f : 0.3f));
            g.fillRoundedRectangle (cell, 2.5f);

            if (! isHit (c))
            {
                // A hold rings at the level of its note: a faint continuation of that fader.
                if (const int owner = ownerOf (r, s); c == '=' && owner != s && isHit (row.line[(size_t) owner]))
                {
                    const int set = owner < (int) row.levels.size() ? row.levels[(size_t) owner] : 0;
                    const float top = yFor (set > 0 ? set : automaticLevel (row.line[(size_t) owner]));
                    const auto tail = cell.withTop (top).withTrimmedLeft (-juce::jmin (3.0f, cellW * 0.18f) - 1.0f);
                    g.setColour (Palette::level.withAlpha (0.14f));
                    g.fillRect (tail);
                    g.setColour (Palette::level.withAlpha (0.5f));
                    g.fillRect (tail.withHeight (2.0f));
                }
                continue;
            }

            const int set = s < (int) row.levels.size() ? row.levels[(size_t) s] : 0;
            const int level = set > 0 ? set : automaticLevel (c);
            auto bar = cell.withTop (yFor (level));
            const auto colour = r == 0 ? Palette::level : Palette::level.interpolatedWith (Palette::accent2, 0.35f);
            g.setColour (colour.withAlpha (set > 0 ? 0.95f : 0.4f));
            g.fillRoundedRectangle (bar, 2.5f);
            g.setColour (colour.brighter (0.4f).withAlpha (set > 0 ? 1.0f : 0.6f));
            g.fillRoundedRectangle (bar.removeFromTop (3.0f), 1.5f);   // the fader cap

            const bool active = (s == hoverStep && hoverBass == (r == 1)) || (dragging && s == dragStep && dragBass == (r == 1));
            if (active && cellW > 14.0f)
            {
                g.setColour (Palette::text);
                g.setFont (font (10.0f, true));
                g.drawText (juce::String (level), juce::Rectangle<float> (cell.getX() - 6.0f, juce::jmax (rowBounds.getY(), yFor (level) - 14.0f), cell.getWidth() + 12.0f, 12.0f),
                            juce::Justification::centred);
            }
            if (s == currentStep)
            {
                g.setColour (Palette::text);
                g.drawRoundedRectangle (cell.expanded (1.0f), 2.5f, 1.5f);
            }
        }
    }
}

//==============================================================================
void PianoRollView::setContent (std::vector<Note> newNotes, std::vector<ChordSpan> newChords, double lengthInBeats, bool chordsMuted)
{
    if (newNotes == notes && newChords == chords && juce::approximatelyEqual (lengthInBeats, length) && chordsMuted == mutedChords)
        return;

    notes = std::move (newNotes);
    chords = std::move (newChords);
    length = juce::jmax (1.0, lengthInBeats);
    mutedChords = chordsMuted;

    // The keyboard follows the notes, with a little room around them (two octaves at least).
    int low = 127, high = 0;
    for (const auto& n : notes)
    {
        low = juce::jmin (low, n.note);
        high = juce::jmax (high, n.note);
    }
    if (low > high) { low = 48; high = 72; }
    low -= 2; high += 2;
    while (high - low < 24) { --low; ++high; }
    lowNote = juce::jmax (0, low);
    highNote = juce::jmin (127, high);
    repaint();
}

void PianoRollView::setPlayhead (double beat)
{
    if (std::abs (beat - playhead) < 1.0e-4)
        return;
    playhead = beat;
    repaint();
}

juce::Rectangle<float> PianoRollView::noteArea() const
{
    return getLocalBounds().toFloat().withTrimmedLeft (keyboardWidth).withTrimmedTop (chordLaneHeight);
}

float PianoRollView::xOf (double beat) const
{
    const auto area = noteArea();
    return area.getX() + (float) (beat / length) * area.getWidth();
}

void PianoRollView::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    const auto area = noteArea();
    const int rows = highNote - lowNote + 1;
    const float rowH = area.getHeight() / (float) rows;
    auto yOf = [&] (int note) { return area.getBottom() - (float) (note - lowNote + 1) * rowH; };
    auto isBlack = [] (int note) { const int pc = note % 12; return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10; };

    g.setColour (Palette::background);
    g.fillRoundedRectangle (bounds, 6.0f);

    // Rows: the black keys a little darker, a line under every C.
    for (int note = lowNote; note <= highNote; ++note)
    {
        const auto row = juce::Rectangle<float> (area.getX(), yOf (note), area.getWidth(), rowH);
        if (isBlack (note))
        {
            g.setColour (juce::Colours::black.withAlpha (0.22f));
            g.fillRect (row);
        }
        if (note % 12 == 0)
        {
            g.setColour (Palette::panelEdge.withAlpha (0.8f));
            g.fillRect (row.getX(), row.getBottom() - 0.5f, row.getWidth(), 1.0f);
        }
    }

    // Keyboard on the left.
    for (int note = lowNote; note <= highNote; ++note)
    {
        const auto key = juce::Rectangle<float> (bounds.getX(), yOf (note), keyboardWidth - 2.0f, rowH);
        g.setColour (isBlack (note) ? Palette::panel.darker (0.4f) : Palette::text.withAlpha (0.82f));
        g.fillRect (key.reduced (0.0f, 0.25f));
        if (note % 12 == 0 && rowH * 3.0f >= 9.0f)
        {
            g.setColour (Palette::background);
            g.setFont (font (juce::jmin (9.0f, rowH * 2.2f), true));
            g.drawText ("C" + juce::String (note / 12 - 1), key.withHeight (rowH * 2.0f).translated (0.0f, -rowH).withTrimmedRight (2.0f),
                        juce::Justification::centredRight);
        }
    }

    // Beat and bar lines.
    const int sixteenths = (int) std::lround (length * 4.0);
    for (int s = 0; s <= sixteenths; ++s)
    {
        const float x = xOf (s / 4.0);
        const bool bar = s % 16 == 0, beat = s % 4 == 0;
        g.setColour (bar ? Palette::accent.withAlpha (0.35f) : beat ? Palette::panelEdge.withAlpha (0.9f) : Palette::panelEdge.withAlpha (0.3f));
        g.fillRect (x - (bar ? 0.75f : 0.5f), area.getY(), bar ? 1.5f : 1.0f, area.getHeight());
    }

    // Chord lane: one block per chord, with its name.
    for (size_t i = 0; i < chords.size(); ++i)
    {
        const auto& c = chords[i];
        auto block = juce::Rectangle<float> (xOf (c.start), bounds.getY(), xOf (c.start + c.length) - xOf (c.start), chordLaneHeight).reduced (1.0f, 2.0f);
        const bool now = playhead >= c.start && playhead < c.start + c.length;
        g.setColour (now ? Palette::accent : (i % 2 == 0 ? Palette::panelEdge : Palette::panelEdge.darker (0.25f)));
        g.fillRoundedRectangle (block, 4.0f);
        g.setColour (now ? Palette::background : Palette::text);
        g.setFont (font (11.5f, true));
        g.drawFittedText (c.name, block.reduced (5.0f, 0.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.6f);
    }

    // Notes: piano (orange), bass (pink), fills (blue); brighter while they sound.
    const juce::Colour fillColour (Palette::fill);
    for (const auto& n : notes)
    {
        if (n.note < lowNote || n.note > highNote)
            continue;
        const float x0 = xOf (n.start), x1 = xOf (juce::jmin (n.start + n.length, length));
        auto r = juce::Rectangle<float> (x0, yOf (n.note), juce::jmax (2.0f, x1 - x0), rowH).reduced (0.5f, rowH > 5.0f ? 0.75f : 0.25f);
        juce::Colour c = n.part == sgk::Part::bass ? Palette::accent2 : n.part == sgk::Part::fill ? fillColour : Palette::accent;
        float alpha = 0.55f + 0.45f * juce::jlimit (0.0f, 1.0f, (n.velocity - 40) / 87.0f);
        if (mutedChords && n.part != sgk::Part::bass)
            alpha *= 0.3f;
        const bool sounding = playhead >= n.start && playhead < n.start + n.length;
        g.setColour (sounding ? c.brighter (0.5f) : c.withAlpha (alpha));
        g.fillRoundedRectangle (r, juce::jmin (2.5f, rowH * 0.4f));
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.drawRoundedRectangle (r, juce::jmin (2.5f, rowH * 0.4f), 0.6f);
    }

    if (edited)
    {
        auto tag = juce::Rectangle<float> (bounds.getRight() - 128.0f, area.getY() + 4.0f, 122.0f, 16.0f);
        g.setColour (Palette::accent2);
        g.fillRoundedRectangle (tag, 4.0f);
        g.setColour (Palette::background);
        g.setFont (font (10.0f, true));
        g.drawText (Localisation::upper (tr ("Edited notes")), tag, juce::Justification::centred);
    }

    // Playhead.
    if (playhead >= 0.0)
    {
        const float x = xOf (playhead);
        g.setColour (Palette::text.withAlpha (0.9f));
        g.fillRect (x - 0.75f, bounds.getY(), 1.5f, bounds.getHeight());
    }

    if (notes.empty())
    {
        g.setColour (Palette::dim);
        g.setFont (font (12.0f));
        g.drawText (tr ("No notes: the piano and the bass are both silent"), area, juce::Justification::centred);
    }
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
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (card, 10.0f, 1.0f);

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
    g.setColour (hover ? Palette::accent.brighter (0.1f) : Palette::accent);
    g.fillRoundedRectangle (r, 8.0f);
    g.setColour (Palette::background.withAlpha (0.25f));
    g.drawRoundedRectangle (r.reduced (4.0f), 6.0f, 1.0f);

    // A small "MIDI clip" picture: four notes on a strip.
    auto content = r.reduced (12.0f, 8.0f);
    if (content.getHeight() > 54.0f)
    {
        auto icon = content.removeFromTop (content.getHeight() * 0.38f).withSizeKeepingCentre (56.0f, juce::jmin (26.0f, content.getHeight() * 0.38f));
        g.setColour (Palette::background.withAlpha (0.85f));
        g.fillRoundedRectangle (icon, 4.0f);
        g.setColour (Palette::accent);
        const float w = icon.getWidth() / 6.0f, h = icon.getHeight() / 5.0f;
        const int rows[] { 3, 1, 2, 1 };
        for (int k = 0; k < 4; ++k)
            g.fillRoundedRectangle (icon.getX() + w * (0.6f + 1.2f * k), icon.getY() + h * (float) rows[k], w, h, 1.5f);
    }

    g.setColour (Palette::background);
    g.setFont (juce::Font (juce::FontOptions (15.0f).withStyle ("Bold")));
    auto title = subtitle.isEmpty() ? content : content.removeFromTop (content.getHeight() * 0.55f);
    g.drawFittedText (tr ("DRAG MIDI TO DAW"), title.toNearestInt(), subtitle.isEmpty() ? juce::Justification::centred : juce::Justification::centredBottom, 1, 0.8f);
    if (subtitle.isNotEmpty())
    {
        g.setColour (Palette::background.withAlpha (0.75f));
        g.setFont (juce::Font (juce::FontOptions (11.5f)));
        g.drawFittedText (subtitle, content.toNearestInt().withTrimmedTop (3), juce::Justification::centredTop, 1, 0.8f);
    }
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
GrooveBrowser::GrooveBrowser (FaseGroovePianoProcessor& p) : processor (p)
{
    setWantsKeyboardFocus (true);

    const int buttons = firstCategory + juce::jmax ((int) sgk::grooveCategories().size() + 2,   // + My grooves, Generate
                                                    (int) sgk::progressionCategories().size());
    for (int i = 0; i < buttons; ++i)
    {
        auto* b = genreButtons.add (new juce::TextButton());
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::buttonOnColourId, Palette::accent);
        b->setColour (juce::TextButton::textColourOffId, i < firstCategory ? Palette::accent : Palette::text);
        b->setColour (juce::TextButton::textColourOnId, Palette::background);
        b->onClick = [this, i]
        {
            search.clear();
            selectGenre (i);
        };
        addAndMakeVisible (b);
    }

    search.setFont (font (14.0f));
    search.setColour (juce::TextEditor::backgroundColourId, Palette::background);
    search.setColour (juce::TextEditor::textColourId, Palette::text);
    search.setColour (juce::TextEditor::outlineColourId, Palette::panelEdge);
    search.setColour (juce::TextEditor::focusedOutlineColourId, Palette::accent);
    search.setIndents (8, 6);
    search.onTextChange = [this] { rebuildList(); };
    search.onEscapeKey = [this] { setVisible (false); };
    addAndMakeVisible (search);

    closeButton.onClick = [this] { setVisible (false); };
    addAndMakeVisible (closeButton);

    importButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    importButton.setColour (juce::TextButton::textColourOffId, Palette::accent2.brighter (0.3f));
    importButton.onClick = [this]
    {
        setVisible (false);
        if (onImportMidi != nullptr)
            onImportMidi();
    };
    addChildComponent (importButton);

    list.setModel (this);
    list.setRowHeight (34);
    list.setColour (juce::ListBox::backgroundColourId, Palette::background);
    list.setColour (juce::ListBox::outlineColourId, Palette::panelEdge);
    list.setOutlineThickness (1);
    addAndMakeVisible (list);

    refreshText();
}

std::vector<int> GrooveBrowser::menuOrder()
{
    std::vector<int> order;
    const auto& patterns = sgk::groovePatterns();
    for (const auto* genre : sgk::grooveCategories())
        for (int i = 0; i < (int) patterns.size(); ++i)
            if (patterns[(size_t) i].category == genre)
                order.push_back (i);
    return order;
}

const std::vector<const char*>& GrooveBrowser::categories() const
{
    return mode == Mode::progressions ? sgk::progressionCategories() : sgk::grooveCategories();
}

int GrooveBrowser::numGenres() const
{
    return firstCategory + (int) categories().size() + (mode == Mode::grooves ? 2 : 0);
}

juce::StringArray GrooveBrowser::loadList (const char* what) const
{
    return juce::StringArray::fromTokens (Localisation::setting (what), "|", {});
}

void GrooveBrowser::saveList (const char* what, const juce::StringArray& names) const
{
    Localisation::setSetting (what, names.joinIntoString ("|"));
}

bool GrooveBrowser::isFavourite (const Entry& e) const
{
    return e.key.isNotEmpty() && favouriteNames.contains (e.key);
}

void GrooveBrowser::toggleFavourite (int row)
{
    if (! juce::isPositiveAndBelow (row, (int) entries.size()) || entries[(size_t) row].key.isEmpty())
        return;

    const auto& key = entries[(size_t) row].key;
    if (favouriteNames.contains (key)) favouriteNames.removeString (key);
    else                               favouriteNames.add (key);
    saveList (favouritesKey(), favouriteNames);

    if (genre == favourites && search.isEmpty())
        rebuildList();
    else
        list.repaintRow (row);
}

void GrooveBrowser::refreshText()
{
    search.setTextToShowWhenEmpty (tr (mode == Mode::progressions ? "Search a progression..." : "Search a groove..."), Palette::dim);
    closeButton.setButtonText (tr ("CLOSE"));
    importButton.setButtonText (tr ("Import a MIDI file..."));
    importButton.setVisible (mode == Mode::progressions);
    favouriteNames = loadList (favouritesKey());

    const auto& cats = categories();
    for (int i = 0; i < genreButtons.size(); ++i)
    {
        auto* b = genreButtons[i];
        b->setVisible (i < numGenres());
        if (i >= numGenres())
            continue;

        juce::String name;
        int count = -1;
        if (i == favourites)
        {
            name = juce::String::fromUTF8 ("\xe2\x98\x85 ") + tr ("Favourites");
            count = favouriteNames.size();
        }
        else if (i == recent)
        {
            name = tr ("Recent");
        }
        else if (i - firstCategory < (int) cats.size())
        {
            const auto* category = cats[(size_t) (i - firstCategory)];
            name = tr (category);
            count = 0;
            if (mode == Mode::progressions)
                for (const auto& p : sgk::progressionPresets()) count += juce::String (p.category) == category ? 1 : 0;
            else
                for (const auto& p : sgk::groovePatterns()) count += p.category == category ? 1 : 0;
        }
        else
        {
            name = i - firstCategory == (int) cats.size() ? tr ("MY GROOVES") : tr ("Generate");
        }
        b->setButtonText (count >= 0 ? name + "  (" + juce::String (count) + ")" : name);
    }
    repaint();
}

int GrooveBrowser::genreOfCurrent() const
{
    const auto& cats = categories();
    if (mode == Mode::progressions)
    {
        const auto& presets = sgk::progressionPresets();
        if (juce::isPositiveAndBelow (currentPreset, (int) presets.size()))
            for (size_t c = 0; c < cats.size(); ++c)
                if (juce::String (presets[(size_t) currentPreset].category) == cats[c])
                    return firstCategory + (int) c;
        return firstCategory;
    }

    const int current = pickCurrent >= 0 ? pickCurrent : juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::pattern)->load());
    const auto& patterns = sgk::groovePatterns();
    if (current >= (int) patterns.size())
        return firstCategory + (int) cats.size() + 1;   // Generate
    for (size_t g = 0; g < cats.size(); ++g)
        if (patterns[(size_t) current].category == cats[g])
            return firstCategory + (int) g;
    return firstCategory;
}

void GrooveBrowser::show (Mode newMode, int preset)
{
    onPicked = nullptr;
    pickCurrent = -1;
    mode = newMode;
    currentPreset = preset;
    search.clear();
    refreshText();
    resized();
    selectGenre (genreOfCurrent());
    setVisible (true);
    toFront (true);
    list.grabKeyboardFocus();
}

void GrooveBrowser::pickGroove (int current, std::function<void (int)> picked)
{
    show (Mode::grooves);
    pickCurrent = current;
    onPicked = std::move (picked);
    selectGenre (genreOfCurrent());
    repaint();
}

void GrooveBrowser::visibilityChanged()
{
    if (! isVisible())
    {
        onPicked = nullptr;
        pickCurrent = -1;
    }
}

void GrooveBrowser::selectGenre (int g)
{
    genre = juce::jlimit (0, numGenres() - 1, g);
    rebuildList();
}

void GrooveBrowser::rebuildList()
{
    entries.clear();
    const auto query = search.getText().trim();
    const auto& cats = categories();
    const auto recentNames = loadList (recentKey());

    // Which names to list for Favourites / Recent (in that order).
    const juce::StringArray* byName = query.isEmpty() && genre == favourites ? &favouriteNames
                                    : query.isEmpty() && genre == recent ? &recentNames : nullptr;
    const auto* category = query.isEmpty() && genre >= firstCategory && genre - firstCategory < (int) cats.size()
                               ? cats[(size_t) (genre - firstCategory)] : nullptr;

    if (mode == Mode::progressions)
    {
        const auto& presets = sgk::progressionPresets();
        const auto settings = processor.currentSettings();
        auto makeEntry = [&] (int i)
        {
            const auto& p = presets[(size_t) i];
            const juce::String name (p.name);

            // The chords in the key you are using.
            auto s = settings;
            s.scale = p.scale;
            const auto slots = sgk::presetSlots (p);
            s.progressionLength = (int) slots.size();
            for (int k = 0; k < (int) slots.size() && k < sgk::maxProgressionLength; ++k)
            {
                s.progression[(size_t) k] = slots[(size_t) k].degree;
                s.freeChords[(size_t) k] = slots[(size_t) k].freeCode;
            }
            juce::StringArray names;
            for (int k = 0; k < s.progressionLength; ++k)
                names.add (sgk::chordName (sgk::GrooveEngine::chordForSlot (s, k)));

            Entry e;
            e.name = name.upToFirstOccurrenceOf ("  ", false, false);
            e.detail = name.fromFirstOccurrenceOf ("  ", false, false).trim() + "   " + tr (sgk::scaleName (p.scale));
            e.preset = i;
            e.chords = names.joinIntoString ("  ");
            e.key = name;
            entries.push_back (e);
        };

        if (byName != nullptr)
        {
            for (const auto& wanted : *byName)
                for (int i = 0; i < (int) presets.size(); ++i)
                    if (juce::String (presets[(size_t) i].name) == wanted)
                        makeEntry (i);
        }
        else
        {
            for (int i = 0; i < (int) presets.size(); ++i)
            {
                const auto& p = presets[(size_t) i];
                const bool match = query.isNotEmpty() ? juce::String (p.name).containsIgnoreCase (query) || tr (p.category).containsIgnoreCase (query)
                                                      : category != nullptr && juce::String (p.category) == category;
                if (match)
                    makeEntry (i);
            }
        }
    }
    else
    {
        const auto& patterns = sgk::groovePatterns();
        auto addPattern = [this, &patterns] (int i)
        {
            const auto& p = patterns[(size_t) i];
            Entry e;
            e.name = p.name;
            e.detail = tr (p.category);
            e.pattern = i;
            e.right = p.rightHand;
            e.left = p.leftHand;
            e.key = p.name;
            entries.push_back (e);
        };

        if (byName != nullptr)
        {
            for (const auto& wanted : *byName)
                for (int i = 0; i < (int) patterns.size(); ++i)
                    if (juce::String (patterns[(size_t) i].name) == wanted)
                        addPattern (i);
        }
        else if (query.isNotEmpty())
        {
            for (int i : menuOrder())
                if (juce::String (patterns[(size_t) i].name).containsIgnoreCase (query)
                    || tr (patterns[(size_t) i].category).containsIgnoreCase (query))
                    addPattern (i);
        }
        else if (category != nullptr)
        {
            for (int i = 0; i < (int) patterns.size(); ++i)
                if (patterns[(size_t) i].category == category)
                    addPattern (i);
        }
        else if (genre - firstCategory == (int) cats.size())
        {
            for (const auto& f : FaseGroovePianoProcessor::grooveFiles())
            {
                Entry e;
                e.name = f.getFileNameWithoutExtension();
                e.detail = tr ("Saved groove");
                e.file = f;
                if (auto xml = juce::XmlDocument::parse (f))
                {
                    e.right = xml->getStringAttribute ("chords").toStdString();
                    e.left = xml->getStringAttribute ("bass").toStdString();
                }
                entries.push_back (e);
            }
        }
        else
        {
            Entry random, user;
            random.name = tr ("* Random riff *");  random.detail = tr ("NEW RIFF makes a new one"); random.pattern = sgk::randomPatternIndex();
            user.name = tr ("* User pattern *");   user.detail = tr ("Your edited grid");           user.pattern = sgk::userPatternIndex();
            entries.push_back (random);
            entries.push_back (user);
        }
    }

    for (int i = 0; i < genreButtons.size(); ++i)
        genreButtons[i]->setToggleState (i == genre && query.isEmpty(), juce::dontSendNotification);
    genreButtons[favourites]->setButtonText (juce::String::fromUTF8 ("\xe2\x98\x85 ") + tr ("Favourites")
                                             + "  (" + juce::String (favouriteNames.size()) + ")");

    list.updateContent();
    list.repaint();

    // Show the groove (or progression) that is playing now.
    const int current = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::pattern)->load());
    quiet = true;
    list.deselectAllRows();
    for (int r = 0; r < (int) entries.size(); ++r)
        if (mode == Mode::progressions ? (entries[(size_t) r].preset == currentPreset && currentPreset >= 0)
                                       : (entries[(size_t) r].pattern == current && entries[(size_t) r].pattern >= 0))
        {
            list.selectRow (r);
            break;
        }
    quiet = false;
    repaint();
}

void GrooveBrowser::choose (int row)
{
    if (quiet || ! juce::isPositiveAndBelow (row, (int) entries.size()))
        return;

    const auto e = entries[(size_t) row];
    if (onPicked != nullptr)
    {
        // Pick mode: only the built-in grooves can be a morph target.
        if (e.pattern >= 0 && ! e.file.existsAsFile())
        {
            auto picked = onPicked;
            setVisible (false);
            picked (e.pattern);
        }
        return;
    }
    if (e.preset >= 0)
    {
        processor.applyProgressionPreset (e.preset);
        currentPreset = e.preset;
        if (onProgressionChosen != nullptr)
            onProgressionChosen (e.preset);
    }
    else if (e.file.existsAsFile())
        processor.loadGroove (e.file);
    else if (e.pattern >= 0)
        processor.setParameterFromUI (ParamIDs::pattern, (float) e.pattern);

    // Recent: newest first, twelve at most.
    if (e.key.isNotEmpty())
    {
        auto names = loadList (recentKey());
        names.removeString (e.key);
        names.insert (0, e.key);
        while (names.size() > 12)
            names.remove (names.size() - 1);
        saveList (recentKey(), names);
    }
}

int GrooveBrowser::getNumRows()
{
    return (int) entries.size();
}

void GrooveBrowser::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool selected)
{
    if (! juce::isPositiveAndBelow (row, (int) entries.size()))
        return;

    const auto& e = entries[(size_t) row];
    auto r = juce::Rectangle<int> (0, 0, width, height).reduced (4, 2).toFloat();
    if (selected)
    {
        g.setColour (Palette::accent.withAlpha (0.22f));
        g.fillRoundedRectangle (r, 5.0f);
        g.setColour (Palette::accent);
        g.drawRoundedRectangle (r, 5.0f, 1.2f);
    }

    // The star: click it to add or remove a favourite.
    auto star = r.removeFromLeft ((float) starWidth - 4.0f);
    if (e.key.isNotEmpty())
    {
        const bool fav = isFavourite (e);
        g.setColour (fav ? Palette::accent : Palette::dim.withAlpha (0.5f));
        g.setFont (font (17.0f, true));
        g.drawText (juce::String::fromUTF8 (fav ? "\xe2\x98\x85" : "\xe2\x98\x86"), star, juce::Justification::centred);
    }

    auto text = r.reduced (6.0f, 0.0f);
    auto picture = text.removeFromRight (juce::jmin (330.0f, text.getWidth() * 0.5f));
    g.setColour (selected ? Palette::accent : Palette::text);
    g.setFont (font (15.0f, selected));
    g.drawText (e.name, text.removeFromTop (text.getHeight() * 0.62f), juce::Justification::bottomLeft);
    g.setColour (Palette::dim);
    g.setFont (font (11.0f));
    g.drawText (e.detail, text, juce::Justification::topLeft);

    if (e.chords.isNotEmpty())
    {
        g.setColour (selected ? Palette::text : Palette::accent);
        g.setFont (font (14.0f, true));
        g.drawText (e.chords, picture, juce::Justification::centredRight);
        return;
    }

    // The rhythm at a glance: chords on top, bass below, one cell per 16th.
    auto drawLine = [&g] (const std::string& line, juce::Rectangle<float> area, juce::Colour colour)
    {
        if (line.empty())
            return;
        const float w = area.getWidth() / (float) line.size();
        for (size_t i = 0; i < line.size(); ++i)
        {
            const char c = line[i];
            auto cell = juce::Rectangle<float> (area.getX() + w * (float) i, area.getY(), w, area.getHeight()).reduced (0.8f, 0.0f);
            if (c == '=')
                g.setColour (colour.withAlpha (0.35f));
            else if (c == '.')
                g.setColour (Palette::panelEdge.withAlpha (i % 4 == 0 ? 0.9f : 0.45f));
            else
                g.setColour (colour.withAlpha (c == 'g' ? 0.4f : (c == 'X' || c == 'U' || c == 'D') ? 1.0f : 0.8f));
            g.fillRect (cell);
        }
    };
    picture = picture.reduced (0.0f, 7.0f);
    drawLine (e.right, picture.removeFromTop (picture.getHeight() * 0.5f).reduced (0.0f, 1.0f), Palette::accent);
    drawLine (e.left, picture.reduced (0.0f, 1.0f), Palette::accent2.brighter (0.2f));
}

void GrooveBrowser::selectedRowsChanged (int lastRowSelected)
{
    // Clicks are handled in listBoxItemClicked (a click on the star must not play the item);
    // here only the arrow keys.
    if (! juce::ModifierKeys::currentModifiers.isAnyMouseButtonDown())
        choose (lastRowSelected);
}

void GrooveBrowser::listBoxItemClicked (int row, const juce::MouseEvent& e)
{
    if (e.x < starWidth)
        toggleFavourite (row);
    else
        choose (row);
}

void GrooveBrowser::listBoxItemDoubleClicked (int row, const juce::MouseEvent& e)
{
    if (e.x < starWidth)
        return;
    choose (row);
    setVisible (false);
}

void GrooveBrowser::returnKeyPressed (int)
{
    setVisible (false);
}

bool GrooveBrowser::keyPressed (const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        setVisible (false);
        return true;
    }
    if (key == juce::KeyPress::spaceKey)
    {
        processor.previewToggle();   // play / pause the preview
        return true;
    }
    return false;
}

void GrooveBrowser::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
        setVisible (false);
}

void GrooveBrowser::resized()
{
    auto inner = card().toNearestInt().reduced (24, 18);
    auto top = inner.removeFromTop (44);
    closeButton.setBounds (top.removeFromRight (100).withSizeKeepingCentre (100, 30));
    top.removeFromRight (10);
    search.setBounds (top.removeFromRight (280).withSizeKeepingCentre (280, 30));
    inner.removeFromTop (10);

    auto left = inner.removeFromLeft (230);
    inner.removeFromLeft (14);
    if (mode == Mode::progressions)
    {
        importButton.setBounds (left.removeFromBottom (34).reduced (0, 2));
        left.removeFromBottom (8);
    }
    const int visible = juce::jmax (1, numGenres());
    const int h = juce::jmin (34, left.getHeight() / visible);
    for (int i = 0; i < genreButtons.size(); ++i)
    {
        genreButtons[i]->setBounds (left.removeFromTop (h).reduced (0, 2));
        if (i == recent)
            left.removeFromTop (6);   // a little gap after Favourites / Recent
    }
    list.setBounds (inner);
}

void GrooveBrowser::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));

    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto top = c.reduced (24.0f, 18.0f).removeFromTop (44.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    const bool progressions = mode == Mode::progressions;
    const bool picking = onPicked != nullptr;
    g.drawText (Localisation::upper (tr (progressions ? "Progressions" : picking ? "Morph towards..." : "Groove")), top.removeFromTop (26.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (tr (progressions ? "Click a progression to hear it, double-click or Enter to close. The star keeps it in your favourites."
                    : picking ? "Click the groove to morph towards: genres on the left, or type its name."
                              : "Click a groove to hear it, double-click or Enter to close. The star keeps it in your favourites."),
                top, juce::Justification::centredLeft);
}

//==============================================================================
VoicingCoach::VoicingCoach (FaseGroovePianoProcessor& p) : processor (p)
{
    for (auto* box : { &boxA, &boxB })
    {
        for (int i = 0; i < sgk::numVoicings; ++i)
            box->addItem (sgk::voicingName (i), i + 1);
        box->onChange = [this] { update(); };
        addAndMakeVisible (box);
    }

    for (auto* b : { &hearA, &hearB })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::buttonOnColourId, Palette::accent);
        b->setColour (juce::TextButton::textColourOffId, Palette::text);
        b->setColour (juce::TextButton::textColourOnId, Palette::background);
        addAndMakeVisible (b);
    }
    hearA.onClick = [this] { hear (0); };
    hearB.onClick = [this] { hear (1); };

    closeButton.onClick = [this] { setVisible (false); };
    addAndMakeVisible (closeButton);
    refreshText();
}

void VoicingCoach::refreshText()
{
    for (auto* box : { &boxA, &boxB })
    {
        const int selected = box->getSelectedId();
        for (int i = 0; i < sgk::numVoicings; ++i)
            box->changeItemText (i + 1, tr (sgk::voicingName (i)));
        box->setSelectedId (selected, juce::dontSendNotification);
    }
    hearA.setButtonText (tr ("HEAR A"));
    hearB.setButtonText (tr ("HEAR B"));
    closeButton.setButtonText (tr ("CLOSE"));
    repaint();
}

void VoicingCoach::show()
{
    // A = the voicing you use now, B = a different one worth comparing.
    const int current = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::voicing)->load());
    const int other = current == (int) sgk::Voicing::rootless ? (int) sgk::Voicing::drop2 : (int) sgk::Voicing::rootless;
    boxA.setSelectedId (current + 1, juce::dontSendNotification);
    boxB.setSelectedId (other + 1, juce::dontSendNotification);
    refreshText();
    update();
    setVisible (true);
    toFront (true);
}

void VoicingCoach::update()
{
    auto s = processor.currentSettings();
    s.voicing = (sgk::Voicing) juce::jlimit (0, sgk::numVoicings - 1, boxA.getSelectedId() - 1);
    reportA = sgk::GrooveEngine::voicingReport (s);
    s.voicing = (sgk::Voicing) juce::jlimit (0, sgk::numVoicings - 1, boxB.getSelectedId() - 1);
    reportB = sgk::GrooveEngine::voicingReport (s);

    const int current = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::voicing)->load());
    hearA.setToggleState (current == boxA.getSelectedId() - 1, juce::dontSendNotification);
    hearB.setToggleState (current == boxB.getSelectedId() - 1 && ! hearA.getToggleState(), juce::dontSendNotification);
    repaint();
}

namespace
{
    /** HEAR in a window: when nothing plays, start the preview so you can hear it. */
    void startPreviewIfSilent (FaseGroovePianoProcessor& processor, bool& started)
    {
        if (processor.uiStep.load() < 0 && ! processor.previewOn.load())
        {
            processor.previewOn = true;
            started = true;
        }
    }
}

void VoicingCoach::visibilityChanged()
{
    if (! isVisible() && startedPreview)
    {
        processor.previewStop();
        startedPreview = false;
    }
}

void ChordLab::visibilityChanged()
{
    if (! isVisible() && startedPreview)
    {
        processor.previewStop();
        startedPreview = false;
    }
}

void VoicingCoach::hear (int side)
{
    const int voicing = (side == 0 ? boxA : boxB).getSelectedId() - 1;
    if (voicing >= 0)
        processor.setParameterFromUI (ParamIDs::voicing, (float) voicing);   // only the voicing: the chords stay
    startPreviewIfSilent (processor, startedPreview);
    update();
}

void VoicingCoach::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
        setVisible (false);
}

juce::Rectangle<float> VoicingCoach::sideArea (int side) const
{
    auto inner = card().reduced (24.0f, 18.0f);
    inner.removeFromTop (54.0f);    // title
    inner.removeFromBottom (44.0f); // verdict
    const float w = (inner.getWidth() - 20.0f) / 2.0f;
    return side == 0 ? inner.removeFromLeft (w) : inner.removeFromRight (w);
}

void VoicingCoach::resized()
{
    auto inner = card().toNearestInt().reduced (24, 18);
    closeButton.setBounds (inner.removeFromTop (44).removeFromRight (100).withSizeKeepingCentre (100, 30));

    for (int side = 0; side < 2; ++side)
    {
        auto top = sideArea (side).toNearestInt().removeFromTop (32);
        auto& box = side == 0 ? boxA : boxB;
        auto& button = side == 0 ? hearA : hearB;
        top.removeFromLeft (28);   // the "A" / "B" letter
        button.setBounds (top.removeFromRight (110).reduced (0, 1));
        top.removeFromRight (8);
        box.setBounds (top.reduced (0, 1));
    }
}

void VoicingCoach::paintSide (juce::Graphics& g, int side, const sgk::GrooveEngine::VoicingReport& r, int low, int high)
{
    auto area = sideArea (side);
    const auto colour = side == 0 ? Palette::accent : Palette::accent2.brighter (0.25f);

    g.setColour (colour);
    g.setFont (font (22.0f, true));
    g.drawText (side == 0 ? "A" : "B", area.removeFromTop (32.0f).removeFromLeft (24.0f), juce::Justification::centredLeft);
    area.removeFromTop (8.0f);

    auto stats = area.removeFromBottom (78.0f);
    auto names = area.removeFromBottom (22.0f);
    auto roll = area.reduced (0.0f, 4.0f);

    g.setColour (Palette::background);
    g.fillRoundedRectangle (roll, 8.0f);

    // Pitch grid: a line on every C, labelled.
    auto yOf = [&] (int note) { return roll.getBottom() - 8.0f - (roll.getHeight() - 16.0f) * (float) (note - low) / (float) juce::jmax (1, high - low); };
    g.setFont (font (10.0f));
    for (int n = low; n <= high; ++n)
        if (n % 12 == 0)
        {
            g.setColour (Palette::panelEdge);
            g.drawHorizontalLine (juce::roundToInt (yOf (n)), roll.getX() + 26.0f, roll.getRight() - 6.0f);
            g.setColour (Palette::dim);
            g.drawText ("C" + juce::String (n / 12 - 1), juce::Rectangle<float> (roll.getX() + 3.0f, yOf (n) - 7.0f, 24.0f, 14.0f), juce::Justification::centredLeft);
        }

    const int count = (int) r.chordNotes.size();
    if (count == 0)
        return;
    const float colW = (roll.getWidth() - 34.0f) / (float) count;
    juce::Path topLine;
    for (int i = 0; i < count; ++i)
    {
        const float x = roll.getX() + 30.0f + colW * ((float) i + 0.5f);
        const auto& notes = r.chordNotes[(size_t) i];
        for (int k = 0; k < notes.size; ++k)
        {
            const bool top = k == notes.size - 1;
            const float y = yOf (notes[k]);
            g.setColour (top ? colour : colour.withAlpha (0.6f));
            g.fillRoundedRectangle (juce::Rectangle<float> (x - colW * 0.28f, y - 3.5f, colW * 0.56f, 7.0f), 3.0f);
            if (k > 0 && notes[k] - notes[k - 1] == 1)   // a semitone rub between neighbours
            {
                g.setColour (Palette::accent2);
                g.drawRoundedRectangle (juce::Rectangle<float> (x - colW * 0.32f, y - 4.5f, colW * 0.64f, yOf (notes[k - 1]) - y + 9.0f), 3.0f, 1.2f);
            }
        }
        if (notes.size > 0)
        {
            const juce::Point<float> p (x, yOf (notes.highest()));
            if (i == 0) topLine.startNewSubPath (p); else topLine.lineTo (p);
        }
        g.setColour (Palette::text);
        g.setFont (font (12.0f, true));
        g.drawText (r.names[(size_t) i], juce::Rectangle<float> (x - colW * 0.5f, names.getY(), colW, names.getHeight()), juce::Justification::centred);
    }
    g.setColour (colour.withAlpha (0.8f));
    g.strokePath (topLine, juce::PathStrokeType (1.6f));

    auto noteName = [] (int n) { return juce::String (sgk::noteName (n)) + juce::String (n / 12 - 1); };
    g.setColour (Palette::text);
    g.setFont (font (13.0f));
    auto line = [&] (const juce::String& text) { g.drawText (text, stats.removeFromTop (19.0f), juce::Justification::centredLeft); };
    line (tr ("Range") + ": " + noteName (r.lowest) + " - " + noteName (r.highest) + "  (" + juce::String (r.highest - r.lowest) + " " + tr ("semitones") + ")");
    line (tr ("Voices move") + ": " + juce::String (r.averageMove, 1) + " " + tr ("semitones per chord change"));
    line (tr ("Biggest top-note leap") + ": " + juce::String (r.biggestTopLeap));
    g.setColour (r.muddy > 0 || r.clashes > 0 ? Palette::accent2 : Palette::dim);
    line (tr ("Muddy low notes") + ": " + juce::String (r.muddy) + "   " + tr ("Semitone rubs") + ": " + juce::String (r.clashes));
}

void VoicingCoach::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.78f));

    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto inner = c.reduced (24.0f, 18.0f);
    auto title = inner.removeFromTop (44.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    g.drawText (Localisation::upper (tr ("Voicing Coach")), title.removeFromTop (26.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (tr ("Two voicings of your progression: same chords, different notes. Press HEAR A or HEAR B while the loop plays."),
                title, juce::Justification::centredLeft);

    int low = juce::jmin (reportA.lowest, reportB.lowest) - 2, high = juce::jmax (reportA.highest, reportB.highest) + 2;
    if (high - low < 24) { low -= (24 - (high - low)) / 2; high = low + 24; }
    paintSide (g, 0, reportA, low, high);
    paintSide (g, 1, reportB, low, high);

    // The verdict, in words.
    juce::StringArray verdict;
    const double diff = reportA.averageMove - reportB.averageMove;
    if (std::abs (diff) >= 0.4)
        verdict.add ((diff > 0 ? juce::String ("B") : juce::String ("A")) + " " + tr ("is smoother: its voices move less."));
    else
        verdict.add (tr ("Both lead the voices about the same: choose by ear."));
    if (reportA.muddy != reportB.muddy)
        verdict.add ((reportA.muddy > reportB.muddy ? juce::String ("A") : juce::String ("B")) + " " + tr ("is muddier in the low register: raise the Register."));
    const int spanA = reportA.highest - reportA.lowest, spanB = reportB.highest - reportB.lowest;
    if (std::abs (spanA - spanB) >= 7)
        verdict.add ((spanA < spanB ? juce::String ("A") : juce::String ("B")) + " " + tr ("is more compact (leaves room for vocals and bass)."));
    g.setColour (Palette::text);
    g.setFont (font (14.0f, true));
    g.drawFittedText (verdict.joinIntoString ("   "), inner.removeFromBottom (36.0f).toNearestInt(), juce::Justification::centredLeft, 2);
}

//==============================================================================
namespace
{
    constexpr int songBarChoices[] { 0, 2, 4, 8, 16, 32 };
}

SongOverlay::SongOverlay (FaseGroovePianoProcessor& p) : processor (p)
{
    for (int i = 0; i < FaseGroovePianoProcessor::numSongParts; ++i)
    {
        auto* box = barBoxes.add (new juce::ComboBox());
        for (int bars : songBarChoices)
            box->addItem (bars == 0 ? "-" : juce::String (bars), bars + 1);
        box->onChange = [this, i, box]
        {
            processor.setSongBars (i, box->getSelectedId() - 1);
            refresh();
        };
        addAndMakeVisible (box);

        auto* fresh = newButtons.add (new juce::TextButton());
        fresh->setColour (juce::TextButton::buttonColourId, Palette::background);
        fresh->setColour (juce::TextButton::textColourOffId, Palette::accent);
        fresh->onClick = [this, i] { processor.regenerateSongPart (i); refresh(); };
        addAndMakeVisible (fresh);

        auto* fill = fillBoxes.add (new juce::ComboBox());
        fill->onChange = [this, i, fill]
        {
            const int id = fill->getSelectedId();
            processor.setSongFill (i, id == 1 ? -1 : id == 2 ? -2 : id - 3);
        };
        addAndMakeVisible (fill);

        auto* original = originalButtons.add (new juce::TextButton());
        original->setColour (juce::TextButton::buttonColourId, Palette::background);
        original->setColour (juce::TextButton::textColourOffId, Palette::dim);
        original->onClick = [this, i] { processor.resetSongPart (i); refresh(); };
        addAndMakeVisible (original);
    }

    saveButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
    saveButton.setColour (juce::TextButton::textColourOffId, Palette::background);
    saveButton.onClick = [this] { if (onSaveSections != nullptr) onSaveSections(); };
    addAndMakeVisible (saveButton);
    saveSongButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    saveSongButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    saveSongButton.onClick = [this] { if (onSaveSong != nullptr) onSaveSong(); };
    addAndMakeVisible (saveSongButton);
    addAndMakeVisible (songTile);
    closeButton.onClick = [this] { setVisible (false); };
    addAndMakeVisible (closeButton);
    refreshText();
}

void SongOverlay::refreshText()
{
    for (auto* b : newButtons)      b->setButtonText (tr ("NEW VARIATION"));
    for (auto* b : originalButtons) b->setButtonText (tr ("ORIGINAL"));
    for (auto* box : fillBoxes)
    {
        const int selected = box->getSelectedId();
        box->clear (juce::dontSendNotification);
        box->addItem (tr ("Fill: as set"), 1);
        box->addItem (tr ("No fill"), 2);
        for (int f = 0; f < sgk::numFillStyles; ++f)
            box->addItem (tr ("Fill") + ": " + tr (sgk::fillStyleName (f)), f + 3);
        box->setSelectedId (selected > 0 ? selected : 1, juce::dontSendNotification);
    }
    saveButton.setButtonText (tr ("ONE .MID PER SECTION..."));
    saveSongButton.setButtonText (tr ("SAVE SONG .MID..."));
    songTile.setTooltip (tr ("Drag the whole song (every section, with markers) onto a track in your DAW"));
    closeButton.setButtonText (tr ("CLOSE"));
    repaint();
}

void SongOverlay::show()
{
    refreshText();
    refresh();
    setVisible (true);
    toFront (true);
}

void SongOverlay::refresh()
{
    const auto parts = processor.songArrangementParts();
    for (int i = 0; i < (int) parts.size() && i < barBoxes.size(); ++i)
    {
        int id = 1;
        for (int bars : songBarChoices)
            if (bars <= parts[(size_t) i].bars) id = bars + 1;
        barBoxes[i]->setSelectedId (id, juce::dontSendNotification);
        // Intro, outro and breakdown are pads / no bass: a variation matters little there, but it is allowed.
        originalButtons[i]->setEnabled (parts[(size_t) i].variation != 0);
        const int fill = parts[(size_t) i].endFill;
        fillBoxes[i]->setSelectedId (fill == -1 ? 1 : fill == -2 ? 2 : fill + 3, juce::dontSendNotification);
        newButtons[i]->setEnabled (parts[(size_t) i].bars > 0);
    }
    repaint();
}

void SongOverlay::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
        setVisible (false);
}

juce::Rectangle<float> SongOverlay::timelineArea() const
{
    return card().reduced (28.0f, 20.0f).withTrimmedTop (64.0f).withHeight (46.0f);
}

juce::Rectangle<int> SongOverlay::rowArea (int part) const
{
    auto inner = card().toNearestInt().reduced (28, 20);
    inner.removeFromTop (64 + 46 + 14 + 18);   // title, timeline, gap, column names
    return inner.removeFromTop (46 * (part + 1)).removeFromBottom (46).reduced (0, 4);
}

void SongOverlay::setMini (bool shouldBeMini)
{
    mini = shouldBeMini;
    for (auto* c : getChildren())
        if (c != &songTile)
            c->setVisible (! mini);
    resized();
    repaint();
}

void SongOverlay::resized()
{
    if (mini)
    {
        songTile.setBounds (getLocalBounds().reduced (10).withTrimmedBottom (14));
        return;
    }

    for (int i = 0; i < barBoxes.size(); ++i)
    {
        auto row = rowArea (i);
        row.removeFromLeft (190);   // section name
        barBoxes[i]->setBounds (row.removeFromLeft (76));
        row.removeFromLeft (14);
        newButtons[i]->setBounds (row.removeFromLeft (150));
        row.removeFromLeft (6);
        originalButtons[i]->setBounds (row.removeFromLeft (100));
        row.removeFromLeft (14);
        fillBoxes[i]->setBounds (row);
    }
    // Bottom: the export of the whole song.
    auto bottom = card().toNearestInt().reduced (28, 20).removeFromBottom (52);
    closeButton.setBounds (bottom.removeFromRight (90).withSizeKeepingCentre (90, 32));
    bottom.removeFromRight (12);
    saveButton.setBounds (bottom.removeFromRight (200).withSizeKeepingCentre (200, 32));
    bottom.removeFromRight (8);
    saveSongButton.setBounds (bottom.removeFromRight (170).withSizeKeepingCentre (170, 32));
    bottom.removeFromRight (8);
    songTile.setBounds (bottom.removeFromRight (230));
}

void SongOverlay::paint (juce::Graphics& g)
{
    if (mini)
    {
        g.fillAll (Palette::background);
        g.setColour (Palette::dim);
        g.setFont (font (11.0f));
        g.drawText (tr ("Drop the clip on a track in your DAW"), getLocalBounds().removeFromBottom (22), juce::Justification::centred);
        return;
    }

    g.fillAll (juce::Colours::black.withAlpha (0.78f));
    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto inner = c.reduced (28.0f, 20.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    g.drawText (Localisation::upper (tr ("Song arrangement")), inner.removeFromTop (28.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawFittedText (tr ("Bars of each section (- leaves it out), its own variation of the groove and the fill that leads out of it. Then drag or save the whole song."),
                      inner.removeFromTop (30.0f).toNearestInt(), juce::Justification::topLeft, 2);

    const auto parts = processor.songArrangementParts();
    int total = 0;
    for (const auto& part : parts)
        total += juce::jmax (0, part.bars);

    // The timeline: each section as long as its bars.
    {
        auto line = timelineArea();
        g.setColour (Palette::background);
        g.fillRoundedRectangle (line, 8.0f);
        const juce::Colour colours[] { juce::Colour (0xff6f8fd6), Palette::accent, juce::Colour (0xff9b7fd6), juce::Colour (0xffd6b25f),
                                       Palette::accent2, juce::Colour (0xff6fc8a8) };
        float x = line.getX() + 3.0f;
        const float width = line.getWidth() - 6.0f;
        for (const auto& part : parts)
        {
            if (part.bars <= 0 || total == 0)
                continue;
            const float w = width * (float) part.bars / (float) total;
            auto block = juce::Rectangle<float> (x, line.getY() + 3.0f, w, line.getHeight() - 6.0f).reduced (1.5f, 0.0f);
            const auto colour = colours[(size_t) juce::jlimit (0, 5, (int) part.section)];
            g.setColour (colour.withAlpha (part.variation != 0 ? 1.0f : 0.8f));
            g.fillRoundedRectangle (block, 6.0f);
            g.setColour (Palette::background);
            g.setFont (font (12.0f, true));
            auto text = block.reduced (6.0f, 3.0f);
            g.drawFittedText (tr (sgk::sectionName ((int) part.section)), text.removeFromTop (text.getHeight() * 0.55f).toNearestInt(), juce::Justification::centredLeft, 1, 0.6f);
            g.setFont (font (10.5f));
            g.drawFittedText (juce::String (part.bars) + " " + tr ("bars"), text.toNearestInt(), juce::Justification::centredLeft, 1, 0.6f);
            x += w;
        }
    }

    // Column names.
    {
        auto header = rowArea (0).toFloat().translated (0.0f, -22.0f).withHeight (16.0f);
        g.setColour (Palette::dim);
        g.setFont (font (10.5f, true));
        auto col = [&] (float w, const juce::String& text) { g.drawText (Localisation::upper (text), header.removeFromLeft (w), juce::Justification::centredLeft); };
        col (190.0f, tr ("Section"));
        col (90.0f, tr ("Bars"));
        col (270.0f, tr ("Groove"));
        col (300.0f, tr ("Fill out"));
    }

    for (int i = 0; i < (int) parts.size(); ++i)
    {
        const auto row = rowArea (i).toFloat();
        g.setColour (Palette::background.withAlpha (i % 2 == 0 ? 1.0f : 0.6f));
        g.fillRoundedRectangle (row.expanded (8.0f, 2.0f), 6.0f);
        auto label = row.withWidth (180.0f);
        g.setColour (parts[(size_t) i].bars > 0 ? Palette::text : Palette::dim);
        g.setFont (font (14.0f, true));
        g.drawText (tr (sgk::sectionName ((int) parts[(size_t) i].section)), label.removeFromTop (label.getHeight() * 0.56f), juce::Justification::bottomLeft);
        g.setFont (font (10.5f));
        g.setColour (parts[(size_t) i].variation != 0 ? Palette::accent : Palette::dim);
        g.drawText (parts[(size_t) i].bars <= 0 ? tr ("left out")
                    : parts[(size_t) i].variation != 0 ? tr ("own variation") : tr ("the groove as it is"),
                    label, juce::Justification::topLeft);
    }

    // Length of the whole song.
    const double bpm = processor.exportBpm();
    const int seconds = bpm > 0.0 ? juce::roundToInt (total * 4 * 60.0 / bpm) : 0;
    g.setColour (Palette::text);
    g.setFont (font (14.0f, true));
    auto info = c.reduced (28.0f, 20.0f).removeFromBottom (52.0f).withWidth (220.0f);
    g.drawText (tr ("Total") + ": " + juce::String (total) + " " + tr ("bars"), info.removeFromTop (26.0f), juce::Justification::bottomLeft);
    g.setColour (Palette::dim);
    g.setFont (font (11.5f));
    g.drawText (juce::String (seconds / 60) + ":" + juce::String (seconds % 60).paddedLeft ('0', 2) + "  @ " + juce::String (bpm, 1) + " BPM",
                info, juce::Justification::topLeft);
}

//==============================================================================
namespace
{
    /** Icons are drawn in a unit square, then fitted to the area. */
    juce::Path unitIcon (IconButton::Icon icon, float& strokeWidth, bool& filled)
    {
        using I = IconButton::Icon;
        juce::Path p;
        strokeWidth = 0.09f;
        filled = false;
        switch (icon)
        {
            case I::play:   p.addTriangle (0.28f, 0.18f, 0.28f, 0.82f, 0.82f, 0.5f); filled = true; break;
            case I::pause:  p.addRoundedRectangle (0.24f, 0.2f, 0.17f, 0.6f, 0.03f); p.addRoundedRectangle (0.59f, 0.2f, 0.17f, 0.6f, 0.03f); filled = true; break;
            case I::stop:   p.addRoundedRectangle (0.24f, 0.24f, 0.52f, 0.52f, 0.05f); filled = true; break;
            case I::rewind: p.addRectangle (0.2f, 0.22f, 0.11f, 0.56f); p.addTriangle (0.8f, 0.22f, 0.8f, 0.78f, 0.33f, 0.5f); filled = true; break;
            case I::dice:
                p.addRoundedRectangle (0.14f, 0.14f, 0.72f, 0.72f, 0.14f);
                for (auto pip : { juce::Point<float> (0.33f, 0.33f), { 0.67f, 0.33f }, { 0.5f, 0.5f }, { 0.33f, 0.67f }, { 0.67f, 0.67f } })
                    p.addEllipse (pip.x - 0.065f, pip.y - 0.065f, 0.13f, 0.13f);
                strokeWidth = 0.08f;
                break;
            case I::piano:
                p.addRoundedRectangle (0.1f, 0.2f, 0.8f, 0.6f, 0.06f);
                for (float x : { 0.3f, 0.5f, 0.7f })
                {
                    p.startNewSubPath (x, 0.5f);
                    p.lineTo (x, 0.8f);
                }
                for (float x : { 0.3f, 0.5f, 0.7f })
                    p.addRectangle (x - 0.055f, 0.2f, 0.11f, 0.32f);
                strokeWidth = 0.07f;
                break;
            case I::bass:
                // The bass clef.
                p.addEllipse (0.16f, 0.3f, 0.16f, 0.16f);
                p.startNewSubPath (0.2f, 0.36f);
                p.cubicTo (0.22f, 0.12f, 0.66f, 0.12f, 0.66f, 0.4f);
                p.cubicTo (0.66f, 0.62f, 0.46f, 0.78f, 0.2f, 0.88f);
                p.addEllipse (0.76f, 0.26f, 0.1f, 0.1f);
                p.addEllipse (0.76f, 0.5f, 0.1f, 0.1f);
                strokeWidth = 0.08f;
                break;
            case I::pencil:
                p.startNewSubPath (0.78f, 0.12f); p.lineTo (0.88f, 0.22f); p.lineTo (0.32f, 0.78f); p.lineTo (0.14f, 0.86f);
                p.lineTo (0.22f, 0.68f); p.closeSubPath(); filled = true; break;
            case I::arrow:
                p.startNewSubPath (0.26f, 0.1f); p.lineTo (0.26f, 0.8f); p.lineTo (0.42f, 0.64f); p.lineTo (0.55f, 0.92f);
                p.lineTo (0.67f, 0.86f); p.lineTo (0.55f, 0.6f); p.lineTo (0.76f, 0.6f); p.closeSubPath(); filled = true; break;
            case I::eraser:
            {
                juce::Path block;
                block.addRoundedRectangle (-0.36f, -0.16f, 0.72f, 0.32f, 0.05f);
                block.applyTransform (juce::AffineTransform::rotation (-0.75f).translated (0.5f, 0.5f));
                p = block;
                p.startNewSubPath (0.18f, 0.88f); p.lineTo (0.86f, 0.88f);
                strokeWidth = 0.08f;
                break;
            }
            case I::magnet:
                p.startNewSubPath (0.24f, 0.18f); p.lineTo (0.24f, 0.5f);
                p.addCentredArc (0.5f, 0.5f, 0.26f, 0.26f, 0.0f, -juce::MathConstants<float>::halfPi, juce::MathConstants<float>::halfPi, false);
                p.lineTo (0.76f, 0.18f);
                p.startNewSubPath (0.14f, 0.3f); p.lineTo (0.34f, 0.3f);
                p.startNewSubPath (0.66f, 0.3f); p.lineTo (0.86f, 0.3f);
                strokeWidth = 0.12f;
                break;
            case I::quantize:
                for (float x : { 0.2f, 0.5f, 0.8f })
                {
                    p.startNewSubPath (x, 0.12f);
                    p.lineTo (x, 0.88f);
                }
                p.addRectangle (0.2f, 0.32f, 0.26f, 0.14f);
                p.addRectangle (0.5f, 0.56f, 0.26f, 0.14f);
                strokeWidth = 0.06f;
                break;
            case I::zoomIn:
            case I::zoomOut:
                p.addEllipse (0.14f, 0.14f, 0.52f, 0.52f);
                p.startNewSubPath (0.6f, 0.6f); p.lineTo (0.86f, 0.86f);
                p.startNewSubPath (0.28f, 0.4f); p.lineTo (0.52f, 0.4f);
                if (icon == I::zoomIn) { p.startNewSubPath (0.4f, 0.28f); p.lineTo (0.4f, 0.52f); }
                break;
            case I::fit:
                p.startNewSubPath (0.12f, 0.22f); p.lineTo (0.12f, 0.78f);
                p.startNewSubPath (0.88f, 0.22f); p.lineTo (0.88f, 0.78f);
                p.startNewSubPath (0.24f, 0.5f); p.lineTo (0.76f, 0.5f);
                p.startNewSubPath (0.36f, 0.38f); p.lineTo (0.24f, 0.5f); p.lineTo (0.36f, 0.62f);
                p.startNewSubPath (0.64f, 0.38f); p.lineTo (0.76f, 0.5f); p.lineTo (0.64f, 0.62f);
                break;
            case I::follow:
                p.startNewSubPath (0.3f, 0.12f); p.lineTo (0.3f, 0.88f);
                p.startNewSubPath (0.46f, 0.5f); p.lineTo (0.86f, 0.5f);
                p.startNewSubPath (0.72f, 0.36f); p.lineTo (0.86f, 0.5f); p.lineTo (0.72f, 0.64f);
                break;
            case I::undo:
            case I::redo:
            {
                juce::Path a;
                a.addCentredArc (0.5f, 0.56f, 0.28f, 0.24f, 0.0f, -2.2f, 1.6f, true);
                a.startNewSubPath (0.12f, 0.24f); a.lineTo (0.24f, 0.42f); a.lineTo (0.42f, 0.3f);
                if (icon == I::redo)
                    a.applyTransform (juce::AffineTransform::scale (-1.0f, 1.0f).translated (1.0f, 0.0f));
                p = a;
                break;
            }
            case I::gear:
            {
                // A gear outline (8 teeth) with a hole in the middle.
                const int teeth = 8;
                const float outer = 0.46f, inner = 0.34f;
                for (int k = 0; k < teeth * 4; ++k)
                {
                    const float a = (k / (float) (teeth * 4)) * juce::MathConstants<float>::twoPi - juce::MathConstants<float>::halfPi;
                    const float r = (k % 4 == 0 || k % 4 == 1) ? outer : inner;
                    const juce::Point<float> pt (0.5f + r * std::cos (a), 0.5f + r * std::sin (a));
                    if (k == 0) p.startNewSubPath (pt); else p.lineTo (pt);
                }
                p.closeSubPath();
                p.addEllipse (0.36f, 0.36f, 0.28f, 0.28f);
                p.setUsingNonZeroWinding (false);
                filled = true;
                break;
            }
            case I::close:
                p.startNewSubPath (0.24f, 0.24f); p.lineTo (0.76f, 0.76f);
                p.startNewSubPath (0.76f, 0.24f); p.lineTo (0.24f, 0.76f);
                strokeWidth = 0.1f;
                break;
            case I::fullScreen:
            case I::exitFullScreen:
            {
                const bool in = icon == I::exitFullScreen;
                for (int c = 0; c < 4; ++c)
                {
                    const float x = c % 2 == 0 ? 0.16f : 0.84f, y = c < 2 ? 0.16f : 0.84f;
                    const float dx = c % 2 == 0 ? 0.24f : -0.24f, dy = c < 2 ? 0.24f : -0.24f;
                    if (! in) { p.startNewSubPath (x + dx, y); p.lineTo (x, y); p.lineTo (x, y + dy); }
                    else      { p.startNewSubPath (x + dx, y); p.lineTo (x + dx, y + dy); p.lineTo (x, y + dy); }
                }
                break;
            }
            case I::eye:
                p.startNewSubPath (0.08f, 0.5f);
                p.quadraticTo (0.5f, 0.1f, 0.92f, 0.5f);
                p.quadraticTo (0.5f, 0.9f, 0.08f, 0.5f);
                p.closeSubPath();
                p.addEllipse (0.38f, 0.38f, 0.24f, 0.24f);
                strokeWidth = 0.07f;
                break;
            case I::reset:
                p.addCentredArc (0.5f, 0.5f, 0.32f, 0.32f, 0.0f, 0.6f, juce::MathConstants<float>::twoPi - 0.2f, true);
                p.startNewSubPath (0.5f, 0.06f); p.lineTo (0.5f, 0.24f); p.lineTo (0.66f, 0.16f);
                break;
            case I::speaker:
            case I::speakerOff:
                // A loudspeaker; sound waves when on, a cross when muted.
                p.startNewSubPath (0.1f, 0.38f); p.lineTo (0.26f, 0.38f); p.lineTo (0.48f, 0.18f);
                p.lineTo (0.48f, 0.82f); p.lineTo (0.26f, 0.62f); p.lineTo (0.1f, 0.62f); p.closeSubPath();
                if (icon == I::speaker)
                {
                    p.addCentredArc (0.5f, 0.5f, 0.16f, 0.16f, 0.0f, 0.75f, 2.4f, true);
                    p.addCentredArc (0.5f, 0.5f, 0.32f, 0.32f, 0.0f, 0.7f, 2.45f, true);
                }
                else
                {
                    p.startNewSubPath (0.62f, 0.36f); p.lineTo (0.88f, 0.64f);
                    p.startNewSubPath (0.88f, 0.36f); p.lineTo (0.62f, 0.64f);
                }
                strokeWidth = 0.08f;
                break;
            case I::none:
            default: break;
        }
        return p;
    }
}

void IconButton::drawIcon (juce::Graphics& g, Icon icon, juce::Rectangle<float> r, juce::Colour colour)
{
    if (icon == Icon::none)
        return;
    float stroke = 0.09f;
    bool filled = false;
    auto p = unitIcon (icon, stroke, filled);
    const float s = juce::jmin (r.getWidth(), r.getHeight());
    const auto box = r.withSizeKeepingCentre (s, s);
    p.applyTransform (juce::AffineTransform::scale (s).translated (box.getX(), box.getY()));
    g.setColour (colour);
    if (filled)
        g.fillPath (p);
    else
        g.strokePath (p, juce::PathStrokeType (juce::jmax (1.2f, stroke * s), juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::MouseCursor IconButton::cursor (Icon icon)
{
    if (icon != Icon::pencil && icon != Icon::eraser)
        return juce::MouseCursor::NormalCursor;

    juce::Image image (juce::Image::ARGB, 24, 24, true);
    {
        juce::Graphics g (image);
        drawIcon (g, icon, { 0.0f, 0.0f, 24.0f, 24.0f }, juce::Colours::black);
        drawIcon (g, icon, { 2.0f, 2.0f, 20.0f, 20.0f }, juce::Colours::white);
    }
    return icon == Icon::pencil ? juce::MouseCursor (image, 4, 20) : juce::MouseCursor (image, 12, 12);
}

void IconButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    auto r = getLocalBounds().toFloat().reduced (0.5f);
    const bool on = getToggleState();
    const bool filled = on || primary;
    const bool flatLeft = isConnectedOnLeft(), flatRight = isConnectedOnRight();

    auto fillColour = filled ? onColour : Palette::control;
    if (down)             fillColour = fillColour.darker (0.15f);
    else if (highlighted) fillColour = fillColour.brighter (0.08f);
    if (! isEnabled())    fillColour = fillColour.withMultipliedAlpha (0.5f);

    juce::Path shape;
    shape.addRoundedRectangle (r.getX(), r.getY(), r.getWidth(), r.getHeight(), 4.0f, 4.0f, ! flatLeft, ! flatRight, ! flatLeft, ! flatRight);
    g.setColour (fillColour);
    g.fillPath (shape);
    g.setColour (filled ? fillColour.darker (0.3f) : (highlighted ? Palette::panelEdge.brighter (0.35f) : Palette::panelEdge));
    g.strokePath (shape, juce::PathStrokeType (1.0f));

    const auto ink = filled ? Palette::background : (mutedWhenOff ? Palette::dim : Palette::text.withAlpha (0.92f));
    const auto iconInk = ! filled && ! iconColour.isTransparent() ? iconColour : ink;
    const float iconSize = juce::jmin (r.getHeight() * 0.56f, 18.0f);
    const auto text = getButtonText();

    auto content = r.reduced (6.0f, 0.0f);
    if (text.isEmpty())
    {
        // Icons only, centred.
        const float total = iconSize * (icon2 != Icon::none ? 2.15f : 1.0f);
        auto area = content.withSizeKeepingCentre (total, iconSize);
        drawIcon (g, icon, area.removeFromLeft (iconSize), iconInk);
        if (icon2 != Icon::none)
            drawIcon (g, icon2, area.removeFromRight (iconSize), ink);
    }
    else
    {
        juce::Font f (juce::FontOptions (juce::jlimit (10.0f, 13.0f, r.getHeight() * 0.46f)).withStyle ("Bold"));
        const float textW = juce::GlyphArrangement::getStringWidth (f, text);
        const float iconsW = icon == Icon::none ? 0.0f : iconSize * (icon2 != Icon::none ? 2.15f : 1.0f) + 6.0f;
        auto area = content.withSizeKeepingCentre (juce::jmin (content.getWidth(), iconsW + textW), content.getHeight());
        if (icon != Icon::none)
        {
            auto icons = area.removeFromLeft (iconsW - 6.0f).withSizeKeepingCentre (iconsW - 6.0f, iconSize);
            drawIcon (g, icon, icons.removeFromLeft (iconSize), iconInk);
            if (icon2 != Icon::none)
                drawIcon (g, icon2, icons.removeFromRight (iconSize), ink);
            area.removeFromLeft (6.0f);
        }
        g.setColour (ink);
        g.setFont (f);
        g.drawFittedText (text, area.toNearestInt(), juce::Justification::centredLeft, 1, 0.7f);
    }

    // Muted / hidden: a slash across the icon.
    if (mutedWhenOff && ! on)
    {
        const auto iconArea = text.isEmpty() ? content.withSizeKeepingCentre (iconSize, iconSize)
                                             : content.withWidth (iconSize).withSizeKeepingCentre (iconSize, iconSize)
                                                   .translated (juce::jmax (0.0f, (content.getWidth() - (iconSize + 6.0f + juce::GlyphArrangement::getStringWidth (juce::Font (juce::FontOptions (juce::jlimit (10.0f, 13.0f, r.getHeight() * 0.46f)).withStyle ("Bold")), text))) * 0.5f), 0.0f);
        g.setColour (Palette::dim.brighter (0.3f));
        g.drawLine ({ iconArea.getBottomLeft().translated (-1.0f, 1.0f), iconArea.getTopRight().translated (1.0f, -1.0f) }, 1.8f);
    }
}

//==============================================================================
PianoRollEditor::PianoRollEditor (FaseGroovePianoProcessor& p) : processor (p)
{
    setWantsKeyboardFocus (true);
    for (juce::Button* b : std::initializer_list<juce::Button*> {
             &rewindButton, &playButton, &stopButton, &drawTool, &selectTool, &eraseTool, &pianoPartButton, &bassPartButton,
             &snapToggle, &snapButton, &quantizeButton, &showPianoButton, &showBassButton, &hearPianoButton, &hearBassButton,
             &zoomOut, &zoomIn, &fitButton, &followButton, &generatedButton, &closeButton, &helpButton })
    {
        b->setWantsKeyboardFocus (false);
        addAndMakeVisible (b);
    }

    // Segments: buttons of a group touch each other.
    auto segment = [] (std::initializer_list<juce::Button*> buttons)
    {
        const int n = (int) buttons.size();
        int i = 0;
        for (auto* b : buttons)
        {
            b->setConnectedEdges ((i > 0 ? juce::Button::ConnectedOnLeft : 0) | (i < n - 1 ? juce::Button::ConnectedOnRight : 0));
            ++i;
        }
    };
    segment ({ &rewindButton, &playButton, &stopButton });
    segment ({ &drawTool, &selectTool, &eraseTool });
    segment ({ &pianoPartButton, &bassPartButton });
    segment ({ &snapToggle, &snapButton, &quantizeButton });
    segment ({ &showPianoButton, &showBassButton });
    segment ({ &hearPianoButton, &hearBassButton });
    segment ({ &zoomOut, &zoomIn, &fitButton });

    for (auto* b : { &bassPartButton, &hearBassButton, &showBassButton })
        b->onColour = Palette::accent2;
    playButton.onColour = Palette::accent2;
    for (auto* b : { &showPianoButton, &showBassButton })
        b->mutedWhenOff = true;
    for (auto* b : { &showPianoButton, &showBassButton })
        b->onColour = Palette::panelEdge.brighter (0.5f);

    rewindButton.onClick = [this] { processor.previewToStart(); };
    playButton.onClick = [this] { processor.previewToggle(); };
    stopButton.onClick = [this] { processor.previewStop(); };

    zoomOut.onClick = [this] { setZoom (pixelsPerBeat / 1.5, gridArea().getCentreX()); };
    zoomIn.onClick  = [this] { setZoom (pixelsPerBeat * 1.5, gridArea().getCentreX()); };
    fitButton.onClick = [this] { setZoom (gridArea().getWidth() / length, gridArea().getX()); };
    snapToggle.onClick = [this] { snapOn = ! snapOn; refreshText(); };
    quantizeButton.onClick = [this] { quantize(); };
    snapButton.onClick = [this]
    {
        snap = snap > 0.2 ? 0.125 : snap > 0.1 ? 0.5 : 0.25;
        refreshText();
    };
    // Which part the pencil draws: the colours are the notes' own.
    pianoPartButton.onClick = [this] { drawPart = sgk::Part::chords; setTool (Tool::draw); refreshText(); };
    bassPartButton.onClick  = [this] { drawPart = sgk::Part::bass;   setTool (Tool::draw); refreshText(); };
    followButton.onClick = [this] { follow = ! follow; refreshText(); };

    // What the roll shows (eye): the piano, the bass or both; a hidden part is greyed out and can't be touched.
    auto toggleView = [this] (bool bass)
    {
        const bool pianoVisible = shown != 2, bassVisible = shown != 1;
        if (bass) shown = bassVisible ? (pianoVisible ? 1 : 2) : 0;
        else      shown = pianoVisible ? (bassVisible ? 2 : 1) : 0;
        syncPicks();
        for (int i = 0; i < (int) notes.size(); ++i)
            if (! isShown (notes[(size_t) i]))
                picked[(size_t) i] = 0;
        if (shown != 0)
            drawPart = shown == 2 ? sgk::Part::bass : sgk::Part::chords;   // the pencil draws what you see
        refreshText();
    };
    showPianoButton.onClick = [toggleView] { toggleView (false); };
    showBassButton.onClick  = [toggleView] { toggleView (true); };

    // What you hear (lit = you hear it), as under the main piano roll.
    hearPianoButton.onClick = [this]
    {
        const int now = listening();   // 0 both, 1 piano, 2 bass
        setListening (now == 0 ? 2 : now == 1 ? 2 : 0);
    };
    hearBassButton.onClick = [this]
    {
        const int now = listening();
        setListening (now == 0 ? 1 : now == 2 ? 1 : 0);
    };
    helpButton.setButtonText ("?");

    drawTool.onClick   = [this] { setTool (Tool::draw); };
    selectTool.onClick = [this] { setTool (Tool::select); };
    eraseTool.onClick  = [this] { setTool (Tool::erase); };
    setTool (Tool::draw);
    generatedButton.onClick = [this]
    {
        processor.clearEditedLoop();
        lastLoop = nullptr;
        notes = processor.generatedLoop (length);
        selected = -1;
        clearPicks();
        changed = false;
        updateStatus();
        repaint();
    };
    closeButton.onClick = [this] { setVisible (false); };

    scrollBar.addListener (this);
    scrollBar.setAutoHide (false);
    addAndMakeVisible (scrollBar);
    refreshText();
}

PianoRollEditor::~PianoRollEditor()
{
    scrollBar.removeListener (this);
}

void PianoRollEditor::refreshText()
{
    snapButton.setButtonText (snap > 0.4 ? "1/8" : snap > 0.2 ? "1/16" : "1/32");
    snapButton.setTooltip (tr ("Grid") + ": 1/8, 1/16, 1/32");
    snapToggle.setToggleState (snapOn, juce::dontSendNotification);
    snapToggle.setTooltip (tr ("On: notes stick to the grid. Off: move them freely (Alt while dragging does the opposite)"));
    quantizeButton.setTooltip (tr ("Moves the selected note (or every note, when none is selected) onto the grid"));

    pianoPartButton.setToggleState (drawPart != sgk::Part::bass, juce::dontSendNotification);
    bassPartButton.setToggleState (drawPart == sgk::Part::bass, juce::dontSendNotification);
    pianoPartButton.setTooltip (tr ("The pencil draws piano notes (orange)"));
    bassPartButton.setTooltip (tr ("The pencil draws bass notes (pink, exported on the bass track)"));

    showPianoButton.setToggleState (shown != 2, juce::dontSendNotification);
    showBassButton.setToggleState (shown != 1, juce::dontSendNotification);
    showPianoButton.setTooltip (tr ("Show or hide the piano notes (hidden notes are greyed out and can't be touched)"));
    showBassButton.setTooltip (tr ("Show or hide the bass notes (hidden notes are greyed out and can't be touched)"));

    const int now = listening();
    hearPianoButton.setToggleState (now != 2, juce::dontSendNotification);
    hearBassButton.setToggleState (now != 1, juce::dontSendNotification);
    hearPianoButton.setIcon (now != 2 ? IconButton::Icon::speaker : IconButton::Icon::speakerOff, IconButton::Icon::piano);
    hearBassButton.setIcon (now != 1 ? IconButton::Icon::speaker : IconButton::Icon::speakerOff, IconButton::Icon::bass);
    hearPianoButton.setTooltip (tr ("Hear the piano (lit) or mute it: exported clips keep everything"));
    hearBassButton.setTooltip (tr ("Hear the bass (lit) or switch it off"));

    followButton.setToggleState (follow, juce::dontSendNotification);
    followButton.setTooltip (tr ("The view follows the playhead while the loop plays"));
    drawTool.setTooltip (tr ("Pencil: click to add a note (drag to make it longer); drag a note to move it"));
    selectTool.setTooltip (tr ("Arrow: drag a note to move it, its right edge to change its length"));
    eraseTool.setTooltip (tr ("Eraser: click a note, or drag over several, to delete them"));
    rewindButton.setTooltip (tr ("Back to the start"));
    playButton.setTooltip (tr ("Play / pause the groove without starting the DAW (space bar)"));
    stopButton.setTooltip (tr ("Stop (the next play starts from the top)"));
    zoomOut.setTooltip (tr ("Zoom out (Ctrl + mouse wheel)"));
    zoomIn.setTooltip (tr ("Zoom in (Ctrl + mouse wheel)"));
    fitButton.setTooltip (tr ("FIT"));
    generatedButton.setButtonText (tr ("BACK TO GENERATED"));
    generatedButton.setTooltip (tr ("Forget your edits: the plug-in plays the generated notes again"));
    closeButton.setTooltip (tr ("CLOSE"));
    helpButton.setTooltip (tr ("Click: add a note  |  drag: move (freely, or on the grid with SNAP)  |  drag the right edge: length  |  double-click or right-click: delete  |  arrows: move the selected note (Shift: octave)  |  bottom lane: velocity")
                           + "\n" + tr ("Select: drag a rectangle with MOVE (or Shift + drag), Ctrl / Shift + click to add, Ctrl + A for all; then drag, arrows, Delete, QUANTIZE and the velocity lane work on all of them."));
    resized();
    repaint();
}

void PianoRollEditor::show (double generatedLength)
{
    if (processor.hasEditedLoop())
    {
        notes = *processor.editedLoop();
        length = processor.editedLoopLength();
    }
    else
    {
        length = juce::jmax (4.0, generatedLength);
        notes = processor.generatedLoop (length);
    }
    selected = -1;
    clearPicks();
    changed = false;
    lastLoop = processor.editedLoop();
    lowNote = 127; highNote = 0;
    fitRange();
    if (chordSpans != nullptr)
        chords = chordSpans (length);
    setVisible (true);
    toFront (true);
    resized();
    setZoom (gridArea().getWidth() / length, gridArea().getX());
    grabKeyboardFocus();
    startTimerHz (30);
    updateStatus();
}

void PianoRollEditor::fitRange()
{
    // The keyboard covers the notes with some room, at least three octaves; it only grows while you edit.
    int low = 127, high = 0;
    for (const auto& n : notes)
    {
        low = juce::jmin (low, n.note);
        high = juce::jmax (high, n.note);
    }
    if (low > high) { low = 40; high = 76; }
    low = juce::jmin (lowNote, low - 6);
    high = juce::jmax (highNote, high + 6);
    while (high - low < 36) { --low; ++high; }
    lowNote = juce::jlimit (0, 120, low);
    highNote = juce::jlimit (lowNote + 7, 127, high);
}

juce::Rectangle<float> PianoRollEditor::keyboardArea() const
{
    auto r = card().reduced (16.0f, 12.0f).withTrimmedTop (92.0f + PianoRollView::chordLaneHeight).withTrimmedBottom (110.0f);
    return r.withWidth (44.0f);
}

juce::Rectangle<float> PianoRollEditor::gridArea() const
{
    auto r = card().reduced (16.0f, 12.0f).withTrimmedTop (92.0f + PianoRollView::chordLaneHeight).withTrimmedBottom (110.0f);
    return r.withTrimmedLeft (44.0f);
}

juce::Rectangle<float> PianoRollEditor::velocityArea() const
{
    const auto g = gridArea();
    return { g.getX(), g.getBottom() + 6.0f, g.getWidth(), 62.0f };
}

float PianoRollEditor::rowHeight() const
{
    return gridArea().getHeight() / (float) (highNote - lowNote + 1);
}

float PianoRollEditor::xOf (double beat) const
{
    return gridArea().getX() + (float) (beat * pixelsPerBeat - scroll);
}

double PianoRollEditor::beatAt (float x) const
{
    return (x - gridArea().getX() + scroll) / pixelsPerBeat;
}

float PianoRollEditor::yOf (int note) const
{
    return gridArea().getBottom() - (float) (note - lowNote + 1) * rowHeight();
}

int PianoRollEditor::noteAt (float y) const
{
    return juce::jlimit (lowNote, highNote, lowNote + (int) std::floor ((gridArea().getBottom() - y) / rowHeight()));
}

void PianoRollEditor::setTool (Tool t)
{
    tool = t;
    drawTool.setToggleState (t == Tool::draw, juce::dontSendNotification);
    selectTool.setToggleState (t == Tool::select, juce::dontSendNotification);
    eraseTool.setToggleState (t == Tool::erase, juce::dontSendNotification);
    setMouseCursor (t == Tool::draw ? IconButton::cursor (IconButton::Icon::pencil)
                  : t == Tool::erase ? IconButton::cursor (IconButton::Icon::eraser) : juce::MouseCursor::NormalCursor);
}

void PianoRollEditor::updateCursor (const juce::MouseEvent& e)
{
    if (velocityArea().contains (e.position))
        return setMouseCursor (juce::MouseCursor::UpDownResizeCursor);
    if (! gridArea().contains (e.position))
        return setMouseCursor (juce::MouseCursor::NormalCursor);
    if (tool == Tool::erase)
        return setMouseCursor (IconButton::cursor (IconButton::Icon::eraser));

    bool edge = false;
    const int hit = hitNote (e.position, edge);
    if (hit >= 0)
        setMouseCursor (edge ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::DraggingHandCursor);
    else
        setMouseCursor (tool == Tool::draw ? IconButton::cursor (IconButton::Icon::pencil) : juce::MouseCursor::NormalCursor);
}

bool PianoRollEditor::snapping (const juce::MouseEvent& e) const
{
    return snapOn != e.mods.isAltDown();
}

int PianoRollEditor::numPicked() const
{
    return (int) std::count_if (picked.begin(), picked.end(), [] (char c) { return c != 0; });
}

void PianoRollEditor::clearPicks()
{
    picked.assign (notes.size(), 0);
}

void PianoRollEditor::eraseWhere (std::function<bool (int)> shouldErase)
{
    syncPicks();
    std::vector<sgk::LoopNote> keptNotes;
    std::vector<char> keptPicks;
    for (int i = 0; i < (int) notes.size(); ++i)
        if (! shouldErase (i))
        {
            keptNotes.push_back (notes[(size_t) i]);
            keptPicks.push_back (picked[(size_t) i]);
        }
    notes = std::move (keptNotes);
    picked = std::move (keptPicks);
    selected = -1;
}

void PianoRollEditor::sortNotes()
{
    // Same order as the processor keeps (time, pitch, part), with the selection following its notes.
    syncPicks();
    const auto primary = juce::isPositiveAndBelow (selected, (int) notes.size()) ? selected : -1;
    std::vector<int> order ((size_t) notes.size());
    std::iota (order.begin(), order.end(), 0);
    std::stable_sort (order.begin(), order.end(), [this] (int a, int b)
    {
        const auto& x = notes[(size_t) a];
        const auto& y = notes[(size_t) b];
        return std::tie (x.start, x.note, x.part) < std::tie (y.start, y.note, y.part);
    });
    std::vector<sgk::LoopNote> sortedNotes;
    std::vector<char> sortedPicks;
    for (int k = 0; k < (int) order.size(); ++k)
    {
        sortedNotes.push_back (notes[(size_t) order[(size_t) k]]);
        sortedPicks.push_back (picked[(size_t) order[(size_t) k]]);
        if (order[(size_t) k] == primary)
            selected = k;
    }
    notes = std::move (sortedNotes);
    picked = std::move (sortedPicks);
}

int PianoRollEditor::listening() const
{
    return ! processor.isBassOn() ? 1 : processor.chordsMuted.load() ? 2 : 0;
}

void PianoRollEditor::setListening (int which)
{
    // The same as BOTH / PIANO / BASS under the main piano roll.
    processor.chordsMuted = which == 2;
    if ((which == 1) == processor.isBassOn())
        processor.toggleBass();
    refreshText();
}

void PianoRollEditor::quantize()
{
    auto onGrid = [this] (double beat) { return juce::jlimit (0.0, length - snap, std::round (beat / snap) * snap); };
    syncPicks();
    const bool some = numPicked() > 0;
    for (int i = 0; i < (int) notes.size(); ++i)
        if (some ? isPicked (i) : isShown (notes[(size_t) i]))
            notes[(size_t) i].start = onGrid (notes[(size_t) i].start);
    sortNotes();
    commit();
}

double PianoRollEditor::snapped (double beat) const
{
    return juce::jlimit (0.0, length - snap, std::floor (beat / snap + 1.0e-6) * snap);
}

void PianoRollEditor::setZoom (double ppb, float anchorX)
{
    const double minimum = gridArea().getWidth() / length;   // the whole loop fits
    const double anchorBeat = beatAt (anchorX);
    pixelsPerBeat = juce::jlimit (minimum, 600.0, ppb);
    scroll = anchorBeat * pixelsPerBeat - (anchorX - gridArea().getX());
    updateScrollBar();
    repaint();
}

void PianoRollEditor::updateScrollBar()
{
    const double total = length * pixelsPerBeat, visible = gridArea().getWidth();
    scroll = juce::jlimit (0.0, juce::jmax (0.0, total - visible), scroll);
    scrollBar.setRangeLimits (0.0, total);
    scrollBar.setCurrentRange (scroll, visible, juce::dontSendNotification);
}

void PianoRollEditor::scrollBarMoved (juce::ScrollBar*, double newStart)
{
    scroll = newStart;
    repaint();
}

void PianoRollEditor::resized()
{
    auto head = card().reduced (16.0f, 12.0f).toNearestInt();

    // Title row: the title on the left, BACK TO GENERATED and close on the right.
    auto top = head.removeFromTop (28);
    closeButton.setBounds (top.removeFromRight (28));
    top.removeFromRight (8);
    generatedButton.setBounds (top.removeFromRight (196));

    // Toolbar: groups of buttons, each with a small caption above it.
    head.removeFromTop (6);
    auto captions = head.removeFromTop (14);
    auto bar = head.removeFromTop (28);
    groupCaptions.clear();
    auto group = [&] (const juce::String& caption, std::initializer_list<std::pair<juce::Component*, int>> items)
    {
        const int x = bar.getX();
        for (const auto& [c, w] : items)
            c->setBounds (bar.removeFromLeft (w));
        groupCaptions.push_back ({ juce::Rectangle<int> (x, captions.getY(), bar.getX() - x, captions.getHeight()), caption });
        bar.removeFromLeft (20);
    };
    const int sq = 32;
    group (tr ("Transport"), { { &rewindButton, sq }, { &playButton, sq + 6 }, { &stopButton, sq } });
    group (tr ("Tools"),     { { &drawTool, sq }, { &selectTool, sq }, { &eraseTool, sq } });
    group (tr ("Draws"),     { { &pianoPartButton, sq }, { &bassPartButton, sq } });
    group (tr ("Grid"),      { { &snapToggle, sq }, { &snapButton, 50 }, { &quantizeButton, sq } });
    group (tr ("Show"),      { { &showPianoButton, 50 }, { &showBassButton, 50 } });
    group (tr ("Hear"),      { { &hearPianoButton, 50 }, { &hearBassButton, 50 } });
    group (tr ("Zoom"),      { { &zoomOut, sq }, { &zoomIn, sq }, { &fitButton, sq } });
    group (tr ("Follow"),    { { &followButton, sq } });

    helpButton.setBounds (card().reduced (16.0f, 0.0f).toNearestInt().withTop ((int) card().getBottom() - 30).withHeight (22).removeFromRight (26));

    const auto g = gridArea();
    scrollBar.setBounds (juce::Rectangle<float> (g.getX(), velocityArea().getBottom() + 4.0f, g.getWidth(), 12.0f).toNearestInt());
    updateScrollBar();
}

int PianoRollEditor::hitNote (juce::Point<float> p, bool& onRightEdge) const
{
    onRightEdge = false;
    for (int i = (int) notes.size(); --i >= 0;)
    {
        const auto& n = notes[(size_t) i];
        if (! isShown (n))
            continue;
        const auto r = juce::Rectangle<float> (xOf (n.start), yOf (n.note), juce::jmax (4.0f, (float) (n.length * pixelsPerBeat)), rowHeight());
        if (r.contains (p))
        {
            onRightEdge = p.x > r.getRight() - juce::jmin (7.0f, r.getWidth() * 0.4f);
            return i;
        }
    }
    return -1;
}

void PianoRollEditor::commit()
{
    changed = true;
    processor.setEditedLoop (notes, length);
    lastLoop = processor.editedLoop();
    // Keep the selection on the same note after the processor sorted them.
    updateStatus();
    repaint();
}

void PianoRollEditor::updateStatus()
{
    auto text = processor.hasEditedLoop() ? tr ("Edited notes") : tr ("Generated notes (edit one to start)");
    if (const int count = numPicked(); count > 1)
        text << "   |   " << count << " " << tr ("notes selected");
    else if (juce::isPositiveAndBelow (selected, (int) notes.size()))
    {
        const auto& n = notes[(size_t) selected];
        text << "   |   " << sgk::noteName (n.note) << (n.note / 12 - 1) << "  " << tr ("Velocity") << " " << n.velocity
             << "  " << tr (n.part == sgk::Part::bass ? "BASS" : "PIANO");
    }
    status = text;
    repaint();
}

void PianoRollEditor::mouseDown (const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    syncPicks();
    const auto p = e.position;
    drag = Drag::none;

    if (! card().contains (p))
    {
        setVisible (false);
        return;
    }

    if (velocityArea().contains (p))
    {
        drag = Drag::velocity;
        mouseDrag (e);
        return;
    }

    if (! gridArea().contains (p))
        return;

    bool edge = false;
    const int hit = hitNote (p, edge);
    const bool adding = e.mods.isCtrlDown() || e.mods.isCommandDown() || e.mods.isShiftDown();

    if (tool == Tool::erase)
    {
        drag = Drag::erase;
        if (hit >= 0)
        {
            eraseWhere ([hit] (int i) { return i == hit; });
            commit();
        }
        return;
    }

    if (hit >= 0 && e.mods.isPopupMenu())
    {
        // Right-click: delete it (or the whole selection, if it is part of it).
        const bool group = isPicked (hit);
        eraseWhere ([this, hit, group] (int i) { return group ? isPicked (i) : i == hit; });
        commit();
        return;
    }

    if (hit < 0 && (tool == Tool::select || e.mods.isShiftDown()))
    {
        // Rubber band: drag a rectangle to select the notes in it (Ctrl / Shift: add to the selection).
        drag = Drag::marquee;
        if (! adding)
            clearPicks();
        marqueeBase = picked;
        marquee = { p, p };
        selected = -1;
        updateStatus();
        return;
    }

    if (hit >= 0)
    {
        if (adding)
            picked[(size_t) hit] = isPicked (hit) ? 0 : 1;   // Ctrl / Shift + click: add or remove it
        else if (! isPicked (hit))
        {
            clearPicks();
            picked[(size_t) hit] = 1;
        }
        selected = hit;
        drag = edge ? Drag::resize : Drag::move;
        if (adding && ! isPicked (hit))
            drag = Drag::none;
    }
    else
    {
        // A new note, as long as the grid step (drag to make it longer).
        sgk::LoopNote n;
        n.start = snapping (e) ? snapped (beatAt (p.x)) : juce::jlimit (0.0, length - 0.01, beatAt (p.x));
        n.length = snap;
        n.note = noteAt (p.y);
        n.velocity = 100;
        n.part = drawPart;
        notes.push_back (n);
        clearPicks();
        picked.back() = 1;
        selected = (int) notes.size() - 1;
        drag = Drag::resize;
        commit();
    }

    dragOrigin = notes;
    dragStartBeat = beatAt (p.x);
    dragStartNote = noteAt (p.y);
    updateStatus();
    repaint();
}

void PianoRollEditor::mouseDrag (const juce::MouseEvent& e)
{
    const auto p = e.position;
    syncPicks();

    if (drag == Drag::erase)
    {
        bool edge = false;
        if (const int hit = hitNote (p, edge); hit >= 0)
        {
            eraseWhere ([hit] (int i) { return i == hit; });
            commit();
        }
        return;
    }

    if (drag == Drag::marquee)
    {
        marquee = juce::Rectangle<float> (e.mouseDownPosition, p);
        picked = marqueeBase;
        for (int i = 0; i < (int) notes.size(); ++i)
        {
            const auto& n = notes[(size_t) i];
            const auto r = juce::Rectangle<float> (xOf (n.start), yOf (n.note), juce::jmax (3.0f, (float) (n.length * pixelsPerBeat)), rowHeight());
            if (isShown (n) && r.intersects (marquee))
                picked[(size_t) i] = 1;
        }
        updateStatus();
        repaint();
        return;
    }

    if (drag == Drag::velocity)
    {
        // The note nearest to the mouse takes the height as velocity: with a selection, all the
        // selected notes; otherwise the notes struck with it (Shift: only that one).
        const auto lane = velocityArea();
        const int velocity = juce::jlimit (1, 127, juce::roundToInt ((lane.getBottom() - p.y) / lane.getHeight() * 127.0f));
        int best = -1;
        float bestDistance = 8.0f;
        for (int i = 0; i < (int) notes.size(); ++i)
        {
            if (! isShown (notes[(size_t) i]))
                continue;
            const float d = std::abs (xOf (notes[(size_t) i].start) - p.x) - (isPicked (i) ? 3.0f : 0.0f);
            if (d < bestDistance) { bestDistance = d; best = i; }
        }
        if (best >= 0)
        {
            const double start = notes[(size_t) best].start;
            const auto part = notes[(size_t) best].part;
            const bool group = isPicked (best) && numPicked() > 1;
            for (int i = 0; i < (int) notes.size(); ++i)
            {
                auto& n = notes[(size_t) i];
                const bool together = group ? isPicked (i)
                                            : (i == best || (! e.mods.isShiftDown() && isShown (n) && n.part == part && std::abs (n.start - start) < 1.0e-4));
                if (together)
                    n.velocity = velocity;
            }
            selected = best;
            commit();
        }
        return;
    }

    if (drag != Drag::move && drag != Drag::resize)
        return;
    if (dragOrigin.size() != notes.size())
        return;

    if (drag == Drag::move)
    {
        // Free: the notes follow the mouse. SNAP: they move by whole grid steps, keeping their feel.
        const double delta = beatAt (p.x) - dragStartBeat;
        double moved = snapping (e) ? std::round (delta / snap) * snap : delta;
        int pitch = noteAt (p.y) - dragStartNote;

        // The whole selection stays inside the loop and the keyboard.
        for (int i = 0; i < (int) notes.size(); ++i)
            if (isPicked (i))
            {
                const auto& o = dragOrigin[(size_t) i];
                moved = juce::jlimit (-o.start, length - 0.01 - o.start, moved);
                pitch = juce::jlimit (lowNote - o.note, highNote - o.note, pitch);
            }
        for (int i = 0; i < (int) notes.size(); ++i)
            if (isPicked (i))
            {
                notes[(size_t) i].start = dragOrigin[(size_t) i].start + moved;
                notes[(size_t) i].note = dragOrigin[(size_t) i].note + pitch;
            }
    }
    else
    {
        // Lengths: the note you hold follows the mouse, the other selected ones change by as much.
        if (! juce::isPositiveAndBelow (selected, (int) notes.size()))
            return;
        const auto& held = dragOrigin[(size_t) selected];
        const double end = snapping (e) ? std::round (beatAt (p.x) / snap) * snap : beatAt (p.x);
        const double shortest = snapping (e) ? snap * 0.5 : 0.03;
        const double change = juce::jmax (shortest, end - held.start) - held.length;
        for (int i = 0; i < (int) notes.size(); ++i)
            if (isPicked (i) || i == selected)
            {
                const auto& o = dragOrigin[(size_t) i];
                notes[(size_t) i].length = juce::jlimit (shortest, juce::jmax (shortest, length - o.start), o.length + change);
            }
    }
    repaint();
    updateStatus();
}

void PianoRollEditor::mouseUp (const juce::MouseEvent&)
{
    if (drag == Drag::move || drag == Drag::resize)
    {
        sortNotes();
        fitRange();
        commit();
    }
    if (drag == Drag::marquee)
    {
        marquee = {};
        repaint();
    }
    drag = Drag::none;
}

void PianoRollEditor::mouseMove (const juce::MouseEvent& e)
{
    updateCursor (e);
}

void PianoRollEditor::mouseDoubleClick (const juce::MouseEvent& e)
{
    bool edge = false;
    const int hit = gridArea().contains (e.position) ? hitNote (e.position, edge) : -1;
    if (hit >= 0)
    {
        eraseWhere ([hit] (int i) { return i == hit; });
        commit();
    }
}

void PianoRollEditor::mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (e.mods.isCommandDown() || e.mods.isCtrlDown())
        setZoom (pixelsPerBeat * (wheel.deltaY > 0 ? 1.2 : 1.0 / 1.2), e.position.x);
    else
    {
        scroll -= (std::abs (wheel.deltaX) > 1.0e-6f ? wheel.deltaX : wheel.deltaY) * 300.0;
        updateScrollBar();
        repaint();
    }
}

bool PianoRollEditor::keyPressed (const juce::KeyPress& key)
{
    syncPicks();
    const auto mods = key.getModifiers();
    if (key == juce::KeyPress::escapeKey)
    {
        // First Esc clears the selection, the second one closes.
        if (numPicked() > 0) { clearPicks(); selected = -1; updateStatus(); }
        else                 setVisible (false);
        return true;
    }
    if (key == juce::KeyPress::spaceKey)
    {
        processor.previewToggle();   // play / pause the preview
        return true;
    }
    if (mods.isCommandDown() && (key.getKeyCode() == 'A' || key.getKeyCode() == 'a'))
    {
        for (int i = 0; i < (int) notes.size(); ++i)
            picked[(size_t) i] = isShown (notes[(size_t) i]) ? 1 : 0;
        updateStatus();
        return true;
    }
    if (numPicked() == 0)
        return false;

    if (key == juce::KeyPress::deleteKey || key == juce::KeyPress::backspaceKey)
        eraseWhere ([this] (int i) { return isPicked (i); });
    else if (key.getKeyCode() == juce::KeyPress::upKey || key.getKeyCode() == juce::KeyPress::downKey)
    {
        const int step = (key.getKeyCode() == juce::KeyPress::upKey ? 1 : -1) * (mods.isShiftDown() ? 12 : 1);
        for (int i = 0; i < (int) notes.size(); ++i)
            if (isPicked (i))
                notes[(size_t) i].note = juce::jlimit (0, 127, notes[(size_t) i].note + step);
    }
    else if (key.getKeyCode() == juce::KeyPress::rightKey || key.getKeyCode() == juce::KeyPress::leftKey)
    {
        const double step = key.getKeyCode() == juce::KeyPress::rightKey ? snap : -snap;
        for (int i = 0; i < (int) notes.size(); ++i)
            if (isPicked (i))
                notes[(size_t) i].start = juce::jlimit (0.0, length - 0.01, notes[(size_t) i].start + step);
        sortNotes();
    }
    else
        return false;

    fitRange();
    commit();
    return true;
}

void PianoRollEditor::timerCallback()
{
    if (! isVisible())
    {
        stopTimer();
        return;
    }
    {
        const bool previewing = processor.previewOn.load();
        playButton.setIcon (previewing ? IconButton::Icon::pause : IconButton::Icon::play);
        playButton.setToggleState (previewing, juce::dontSendNotification);
        const int now = listening();
        hearPianoButton.setToggleState (now != 2, juce::dontSendNotification);
        hearBassButton.setToggleState (now != 1, juce::dontSendNotification);
        hearPianoButton.setIcon (now != 2 ? IconButton::Icon::speaker : IconButton::Icon::speakerOff, IconButton::Icon::piano);
        hearBassButton.setIcon (now != 1 ? IconButton::Icon::speaker : IconButton::Icon::speakerOff, IconButton::Icon::bass);
    }

    // Undo, a preset or BACK TO GENERATED from outside: show what plays now.
    if (drag == Drag::none && processor.editedLoop() != lastLoop)
    {
        lastLoop = processor.editedLoop();
        if (lastLoop != nullptr)
        {
            notes = *lastLoop;
            length = processor.editedLoopLength();
        }
        else
            notes = processor.generatedLoop (length);
        selected = -1;
        clearPicks();
        updateScrollBar();
        updateStatus();
    }

    const double ppq = processor.uiPpq.load();
    const double now = ppq < 0.0 ? -1.0 : std::fmod (ppq, length);
    if (std::abs (now - playhead) > 1.0e-4)
    {
        playhead = now;

        // FOLLOW: when the playhead leaves the view, turn the page (not while you drag a note).
        if (follow && now >= 0.0 && drag == Drag::none)
        {
            const float x = xOf (now);
            const auto grid = gridArea();
            if (x < grid.getX() || x > grid.getRight() - 4.0f)
            {
                scroll = now * pixelsPerBeat - 8.0;
                updateScrollBar();
                repaint();
            }
        }
        repaint (gridArea().withTrimmedTop (-PianoRollView::chordLaneHeight).toNearestInt().expanded (2));
    }
}

void PianoRollEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.8f));
    const auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto head = c.reduced (16.0f, 12.0f);
    g.setColour (Palette::accent);
    g.setFont (font (18.0f, true));
    auto titleRow = head.removeFromTop (28.0f);
    g.drawText (Localisation::upper (tr ("Piano roll")), titleRow.removeFromLeft (150.0f), juce::Justification::centredLeft);

    // Group captions over the toolbar, and a hairline under it.
    {
        auto f = font (9.5f, true);
        f.setExtraKerningFactor (0.1f);
        g.setFont (f);
        g.setColour (Palette::dim);
        for (const auto& [area, caption] : groupCaptions)
            g.drawText (Localisation::upper (caption), area.withWidth (juce::jmax (area.getWidth(), 110)), juce::Justification::centredLeft);
        if (! groupCaptions.empty())
        {
            g.setColour (Palette::panelEdge);
            g.fillRect (c.getX() + 16.0f, (float) zoomIn.getBottom() + 8.0f, c.getWidth() - 32.0f, 1.0f);
        }
    }

    const auto grid = gridArea();
    const auto keys = keyboardArea();
    const float rowH = rowHeight();
    auto isBlack = [] (int note) { const int pc = note % 12; return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10; };

    g.setColour (Palette::background);
    g.fillRect (grid);

    {
        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (grid.withTrimmedTop (-PianoRollView::chordLaneHeight).toNearestInt());

        for (int note = lowNote; note <= highNote; ++note)
        {
            const auto row = juce::Rectangle<float> (grid.getX(), yOf (note), grid.getWidth(), rowH);
            if (isBlack (note))
            {
                g.setColour (juce::Colours::black.withAlpha (0.25f));
                g.fillRect (row);
            }
            if (note % 12 == 0)
            {
                g.setColour (Palette::panelEdge);
                g.fillRect (row.getX(), row.getBottom() - 0.5f, row.getWidth(), 1.0f);
            }
        }

        // Grid lines: bars, beats, and the snap step when zoomed in enough.
        const double step = pixelsPerBeat * snap >= 8.0 ? snap : 0.25;
        for (int i = 0; i * step <= length + 1.0e-6; ++i)
        {
            const double beat = i * step;
            const float x = xOf (beat);
            if (x < grid.getX() - 1.0f || x > grid.getRight() + 1.0f)
                continue;
            const bool bar = std::fmod (beat, 4.0) < 1.0e-6, isBeat = std::fmod (beat, 1.0) < 1.0e-6;
            g.setColour (bar ? Palette::accent.withAlpha (0.4f) : isBeat ? Palette::panelEdge : Palette::panelEdge.withAlpha (0.35f));
            g.fillRect (x - (bar ? 0.75f : 0.5f), grid.getY(), bar ? 1.5f : 1.0f, grid.getHeight());
            if (bar)
            {
                g.setColour (Palette::dim);
                g.setFont (font (10.0f, true));
                g.drawText (juce::String ((int) (beat / 4.0) + 1), juce::Rectangle<float> (x + 3.0f, grid.getY() + 2.0f, 30.0f, 12.0f), juce::Justification::centredLeft);
            }
        }

        // Chord lane.
        for (size_t i = 0; i < chords.size(); ++i)
        {
            const auto& ch = chords[i];
            auto block = juce::Rectangle<float> (xOf (ch.start), grid.getY() - PianoRollView::chordLaneHeight,
                                                 (float) (ch.length * pixelsPerBeat), PianoRollView::chordLaneHeight).reduced (1.0f, 2.0f);
            const bool now = playhead >= ch.start && playhead < ch.start + ch.length;
            g.setColour (now ? Palette::accent : (i % 2 == 0 ? Palette::panelEdge : Palette::panelEdge.darker (0.25f)));
            g.fillRoundedRectangle (block, 4.0f);
            g.setColour (now ? Palette::background : Palette::text);
            g.setFont (font (12.0f, true));
            g.drawFittedText (ch.name, block.reduced (6.0f, 0.0f).toNearestInt(), juce::Justification::centredLeft, 1, 0.6f);
        }

        // Notes.
        const juce::Colour fillColour (Palette::fill);
        for (int i = 0; i < (int) notes.size(); ++i)
        {
            const auto& n = notes[(size_t) i];
            if (n.note < lowNote || n.note > highNote)
                continue;
            auto r = juce::Rectangle<float> (xOf (n.start), yOf (n.note), juce::jmax (3.0f, (float) (n.length * pixelsPerBeat)), rowH).reduced (0.5f, 1.0f);
            const auto colour = n.part == sgk::Part::bass ? Palette::accent2 : n.part == sgk::Part::fill ? fillColour : Palette::accent;
            const bool sounding = playhead >= n.start && playhead < n.start + n.length;
            if (! isShown (n))
            {
                g.setColour (Palette::dim.withAlpha (0.18f));
                g.fillRoundedRectangle (r, 3.0f);
                continue;
            }
            g.setColour ((sounding ? colour.brighter (0.4f) : colour).withAlpha (0.5f + 0.5f * (float) n.velocity / 127.0f));
            g.fillRoundedRectangle (r, 3.0f);
            g.setColour (isPicked (i) ? Palette::text : juce::Colours::black.withAlpha (0.4f));
            g.drawRoundedRectangle (r, 3.0f, isPicked (i) ? 2.0f : 0.8f);
            if (r.getWidth() > 34.0f && rowH >= 10.0f)
            {
                g.setColour (Palette::background);
                g.setFont (font (juce::jmin (10.0f, rowH - 1.0f), true));
                g.drawText (juce::String (sgk::noteName (n.note)) + juce::String (n.note / 12 - 1), r.reduced (3.0f, 0.0f), juce::Justification::centredLeft);
            }
        }

        // The rubber band.
        if (! marquee.isEmpty())
        {
            g.setColour (Palette::text.withAlpha (0.08f));
            g.fillRect (marquee);
            g.setColour (Palette::text.withAlpha (0.7f));
            g.drawRect (marquee, 1.0f);
        }

        if (playhead >= 0.0)
        {
            g.setColour (Palette::text);
            g.fillRect (xOf (playhead) - 0.75f, grid.getY() - PianoRollView::chordLaneHeight, 1.5f, grid.getHeight() + PianoRollView::chordLaneHeight);
        }
    }

    // Keyboard.
    for (int note = lowNote; note <= highNote; ++note)
    {
        const auto key = juce::Rectangle<float> (keys.getX(), yOf (note), keys.getWidth() - 2.0f, rowH);
        g.setColour (isBlack (note) ? Palette::panel.darker (0.5f) : Palette::text.withAlpha (0.85f));
        g.fillRect (key.reduced (0.0f, 0.3f));
        if (note % 12 == 0)
        {
            g.setColour (Palette::background);
            g.setFont (font (juce::jlimit (8.0f, 11.0f, rowH * 1.4f), true));
            g.drawText ("C" + juce::String (note / 12 - 1), key.withTrimmedRight (3.0f).expanded (0.0f, 3.0f), juce::Justification::centredRight);
        }
    }

    // Velocity lane.
    {
        const auto lane = velocityArea();
        g.setColour (Palette::background);
        g.fillRect (lane);
        g.setColour (Palette::dim);
        g.setFont (font (10.0f, true));
        g.drawFittedText (Localisation::upper (tr ("Velocity")), juce::Rectangle<float> (keys.getX(), lane.getY(), keys.getWidth() - 4.0f, lane.getHeight()).toNearestInt(),
                          juce::Justification::centredRight, 2, 0.6f);
        juce::Graphics::ScopedSaveState clip (g);
        g.reduceClipRegion (lane.toNearestInt());
        for (int i = 0; i < (int) notes.size(); ++i)
        {
            const auto& n = notes[(size_t) i];
            if (! isShown (n))
                continue;
            const float x = xOf (n.start);
            const float h = lane.getHeight() * (float) n.velocity / 127.0f;
            const auto colour = n.part == sgk::Part::bass ? Palette::accent2 : Palette::accent;
            g.setColour (isPicked (i) ? Palette::text : colour.withAlpha (0.8f));
            g.fillRect (x - 1.0f, lane.getBottom() - h, 3.0f, h);
            g.fillEllipse (x - 3.0f, lane.getBottom() - h - 3.0f, 7.0f, 7.0f);
        }
    }

    g.setColour (processor.hasEditedLoop() ? Palette::accent2 : Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (status, juce::Rectangle<float> (c.getX() + 16.0f, c.getBottom() - 26.0f, c.getWidth() - 70.0f, 18.0f), juce::Justification::centredLeft);
}

//==============================================================================
MorphOverlay::MorphOverlay (FaseGroovePianoProcessor& p) : processor (p)
{
    // The target: a button that opens the groove browser (genres, search), plus previous / next.
    targetButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    targetButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    targetButton.onClick = [this]
    {
        if (onChooseTarget != nullptr)
            onChooseTarget (targetPattern, [this] (int pattern) { setTarget (pattern); });
    };
    addAndMakeVisible (targetButton);
    auto step = [this] (int direction)
    {
        const auto order = GrooveBrowser::menuOrder();
        auto it = std::find (order.begin(), order.end(), targetPattern);
        const int index = it == order.end() ? 0 : (int) std::distance (order.begin(), it);
        setTarget (order[(size_t) ((index + direction + (int) order.size()) % (int) order.size())]);
    };
    targetPrev.onClick = [step] { step (-1); };
    targetNext.onClick = [step] { step (1); };
    addAndMakeVisible (targetPrev);
    addAndMakeVisible (targetNext);

    amount.setSliderStyle (juce::Slider::LinearHorizontal);
    amount.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 24);
    amount.setRange (0.0, 100.0, 1.0);
    amount.setTextValueSuffix (" %");
    amount.setColour (juce::Slider::trackColourId, Palette::accent);
    amount.setColour (juce::Slider::thumbColourId, Palette::accent);
    amount.onValueChange = [this] { apply(); };
    addAndMakeVisible (amount);

    keepButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
    keepButton.setColour (juce::TextButton::textColourOffId, Palette::background);
    keepButton.onClick = [this] { setVisible (false); };
    cancelButton.onClick = [this]
    {
        processor.restoreSnapshot (before);
        setVisible (false);
    };
    addAndMakeVisible (keepButton);
    addAndMakeVisible (cancelButton);
    refreshText();
}

juce::Rectangle<float> MorphOverlay::card() const
{
    return getLocalBounds().toFloat().withSizeKeepingCentre (700.0f, 250.0f).withY (150.0f);
}

void MorphOverlay::refreshText()
{
    keepButton.setButtonText (tr ("KEEP"));
    cancelButton.setButtonText (tr ("CANCEL"));
    repaint();
}

void MorphOverlay::show()
{
    before = processor.snapshot();
    const auto s = processor.currentSettings();
    fromName = sgk::GrooveEngine::basePattern (s).name;
    processor.beginMorph();

    // The first target: the next groove in the browser's order.
    const auto order = GrooveBrowser::menuOrder();
    const auto it = std::find (order.begin(), order.end(), s.pattern);
    targetPattern = it == order.end() || std::next (it) == order.end() ? order.front() : *std::next (it);
    targetButton.setButtonText (juce::String (sgk::groovePatterns()[(size_t) targetPattern].name) + juce::String::fromUTF8 ("  \xe2\x96\xbe"));
    amount.setValue (0.0, juce::dontSendNotification);

    refreshText();
    setVisible (true);
    toFront (true);
}

void MorphOverlay::setTarget (int pattern)
{
    if (! juce::isPositiveAndBelow (pattern, (int) sgk::groovePatterns().size()))
        return;
    targetPattern = pattern;
    const auto& groove = sgk::groovePatterns()[(size_t) pattern];
    targetButton.setButtonText (juce::String (groove.name) + juce::String::fromUTF8 ("  \xe2\x96\xbe"));
    targetButton.setTooltip (tr (groove.category));
    apply();
    repaint();
}

void MorphOverlay::apply()
{
    if (targetPattern >= 0)
        processor.setMorph (targetPattern, (float) amount.getValue() / 100.0f);
}

void MorphOverlay::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
        setVisible (false);   // keeps the blend, like KEEP
}

void MorphOverlay::resized()
{
    auto inner = card().toNearestInt().reduced (24, 18);
    inner.removeFromTop (58);
    auto row = inner.removeFromTop (30);
    row.removeFromLeft (90);
    targetButton.setBounds (row.removeFromLeft (380));
    row.removeFromLeft (8);
    targetPrev.setBounds (row.removeFromLeft (22).withSizeKeepingCentre (22, 20));
    targetNext.setBounds (row.removeFromLeft (22).withSizeKeepingCentre (22, 20));
    inner.removeFromTop (14);
    auto slider = inner.removeFromTop (34);
    slider.removeFromLeft (90);
    amount.setBounds (slider);
    auto buttons = inner.removeFromBottom (32);
    cancelButton.setBounds (buttons.removeFromRight (110));
    buttons.removeFromRight (10);
    keepButton.setBounds (buttons.removeFromRight (110));
}

void MorphOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.35f));   // the grid below stays visible while you morph
    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto inner = c.reduced (24.0f, 18.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    g.drawText (Localisation::upper (tr ("Groove morph")), inner.removeFromTop (26.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (tr ("From") + ": " + fromName + "   " + tr ("(locked parts stay as they are)"), inner.removeFromTop (26.0f), juce::Justification::centredLeft);

    g.setColour (Palette::text);
    g.setFont (font (13.0f, true));
    inner.removeFromTop (6.0f);
    g.drawText (tr ("Towards"), inner.removeFromTop (30.0f).withWidth (90.0f), juce::Justification::centredLeft);
    inner.removeFromTop (14.0f);
    g.drawText (tr ("Amount"), inner.removeFromTop (34.0f).withWidth (90.0f), juce::Justification::centredLeft);
}

//==============================================================================
ChordLab::ChordLab (FaseGroovePianoProcessor& p) : processor (p)
{
    keepButton.setColour (juce::TextButton::buttonColourId, Palette::accent);
    keepButton.setColour (juce::TextButton::textColourOffId, Palette::background);
    keepButton.onClick = [this] { setVisible (false); };
    cancelButton.onClick = [this]
    {
        processor.restoreSnapshot (before);
        setVisible (false);
    };
    addAndMakeVisible (keepButton);
    addAndMakeVisible (cancelButton);
    refreshText();
}

void ChordLab::refreshText()
{
    for (auto* b : hearButtons) b->setButtonText (tr ("HEAR"));
    keepButton.setButtonText (tr ("KEEP"));
    cancelButton.setButtonText (tr ("CANCEL"));
    repaint();
}

juce::String ChordLab::namesOf (const std::vector<int>& codes) const
{
    auto s = processor.currentSettings();
    juce::StringArray names;
    for (int code : codes)
    {
        const int kind = code / 12;
        const int scale = kind == 2 ? 3 : kind == 1 ? 1 : 0;   // dominant / minor / major, as free chords are built
        names.add (sgk::chordName (sgk::buildChord ((s.keyRoot + code % 12) % 12, scale, 0, s.extension)));
    }
    return names.joinIntoString ("  ");
}

void ChordLab::show()
{
    before = processor.snapshot();
    const auto s = processor.currentSettings();
    rows.clear();
    rows.push_back ({ tr ("Original"), sgk::GrooveEngine::progressionCodes (s) });
    for (const auto& v : sgk::GrooveEngine::progressionVariants (s))
        rows.push_back ({ tr (v.name), v.codes });

    hearButtons.clear();
    for (int i = 0; i < (int) rows.size(); ++i)
    {
        auto* b = hearButtons.add (new juce::TextButton (tr ("HEAR")));
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::buttonOnColourId, Palette::accent);
        b->setColour (juce::TextButton::textColourOnId, Palette::background);
        b->onClick = [this, i] { hear (i); };
        addAndMakeVisible (b);
    }
    playing = 0;
    hearButtons[0]->setToggleState (true, juce::dontSendNotification);
    resized();
    refreshText();
    setVisible (true);
    toFront (true);
}

void ChordLab::hear (int row)
{
    if (! juce::isPositiveAndBelow (row, (int) rows.size()))
        return;
    processor.applyProgressionCodes (rows[(size_t) row].second);   // only the chords: rhythm, key and length stay
    startPreviewIfSilent (processor, startedPreview);
    playing = row;
    for (int i = 0; i < hearButtons.size(); ++i)
        hearButtons[i]->setToggleState (i == row, juce::dontSendNotification);
    repaint();
}

void ChordLab::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
        setVisible (false);   // keeps what you are hearing, like KEEP
}

juce::Rectangle<int> ChordLab::rowArea (int row) const
{
    auto inner = card().toNearestInt().reduced (24, 18);
    inner.removeFromTop (56);
    return inner.removeFromTop (54 * (row + 1)).removeFromBottom (54).reduced (0, 5);
}

void ChordLab::resized()
{
    for (int i = 0; i < hearButtons.size(); ++i)
        hearButtons[i]->setBounds (rowArea (i).removeFromRight (110).reduced (0, 4));
    auto buttons = card().toNearestInt().reduced (24, 18).removeFromBottom (32);
    cancelButton.setBounds (buttons.removeFromRight (110));
    buttons.removeFromRight (10);
    keepButton.setBounds (buttons.removeFromRight (110));
}

void ChordLab::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.35f));
    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto inner = c.reduced (24.0f, 18.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    g.drawText (Localisation::upper (tr ("Chord variations")), inner.removeFromTop (26.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (tr ("Alternatives of your progression in the same key, same number of chords (the rhythm stays). HEAR them while the loop plays."),
                inner.removeFromTop (22.0f), juce::Justification::centredLeft);

    for (int i = 0; i < (int) rows.size(); ++i)
    {
        auto row = rowArea (i).toFloat();
        g.setColour (i == playing ? Palette::accent.withAlpha (0.18f) : Palette::background);
        g.fillRoundedRectangle (row, 6.0f);
        row.removeFromRight (120.0f);
        auto name = row.removeFromLeft (170.0f).reduced (10.0f, 0.0f);
        g.setColour (i == 0 ? Palette::dim : Palette::text);
        g.setFont (font (14.0f, true));
        g.drawText (rows[(size_t) i].first, name, juce::Justification::centredLeft);
        g.setColour (i == playing ? Palette::accent : Palette::text);
        g.setFont (font (16.0f, true));
        g.drawText (namesOf (rows[(size_t) i].second), row, juce::Justification::centredLeft);
    }
}

//==============================================================================
DnaOverlay::DnaOverlay (FaseGroovePianoProcessor& p) : processor (p)
{
    for (int i = 0; i < sgk::numIntentions; ++i)
    {
        auto* b = intentions.add (new juce::TextButton());
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::textColourOffId, i % 2 == 0 ? Palette::accent : Palette::accent2.brighter (0.3f));
        b->onClick = [this, i]
        {
            processor.applyIntention (i, keepRhythm.getToggleState(), keepHarmony.getToggleState());
            ++steps;
            repaint();
        };
        addAndMakeVisible (b);
    }
    addAndMakeVisible (keepRhythm);
    addAndMakeVisible (keepHarmony);
    closeButton.onClick = [this] { setVisible (false); };
    addAndMakeVisible (closeButton);
    refreshText();
}

juce::Rectangle<float> DnaOverlay::card() const
{
    return getLocalBounds().toFloat().withSizeKeepingCentre (860.0f, 270.0f).withY (150.0f);
}

void DnaOverlay::refreshText()
{
    for (int i = 0; i < intentions.size(); ++i)
        intentions[i]->setButtonText (Localisation::upper (tr (sgk::intentionName (i))));
    keepRhythm.setButtonText (tr ("Keep the rhythm (grid and feel)"));
    keepHarmony.setButtonText (tr ("Keep the harmony (extensions and voicing)"));
    closeButton.setButtonText (tr ("CLOSE"));
    repaint();
}

void DnaOverlay::show()
{
    steps = 0;
    refreshText();
    setVisible (true);
    toFront (true);
}

void DnaOverlay::mouseUp (const juce::MouseEvent& e)
{
    if (! card().contains (e.position))
        setVisible (false);
}

void DnaOverlay::resized()
{
    auto inner = card().toNearestInt().reduced (24, 18);
    closeButton.setBounds (inner.removeFromTop (40).removeFromRight (100).withSizeKeepingCentre (100, 30));
    inner.removeFromTop (30);
    auto row = inner.removeFromTop (54);
    const int gap = 10, w = (row.getWidth() - gap * (intentions.size() - 1)) / intentions.size();
    for (auto* b : intentions)
    {
        b->setBounds (row.removeFromLeft (w));
        row.removeFromLeft (gap);
    }
    inner.removeFromTop (14);
    auto toggles = inner.removeFromTop (26);
    keepRhythm.setBounds (toggles.removeFromLeft (toggles.getWidth() / 2));
    keepHarmony.setBounds (toggles);
}

void DnaOverlay::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.35f));   // the grid stays visible
    auto c = card();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto inner = c.reduced (24.0f, 18.0f);
    g.setColour (Palette::accent);
    g.setFont (font (20.0f, true));
    g.drawText ("MUSICAL DNA", inner.removeFromTop (26.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (12.0f));
    g.drawText (tr ("Each click goes one step further. The chords never change, locked parts stay; undo with the arrow."),
                inner.removeFromTop (22.0f), juce::Justification::centredLeft);

    if (steps > 0)
    {
        g.setColour (Palette::text);
        g.setFont (font (12.0f));
        g.drawText (juce::String (steps) + " " + tr ("steps applied"), c.reduced (24.0f, 18.0f).removeFromBottom (22.0f), juce::Justification::centredLeft);
    }
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
        const auto report = Updater::install (result);
        setVisible (false);

        if (report.relaunching)
        {
            // Standalone app: close now; the new version opens by itself.
            juce::MessageManager::callAsync ([] { if (auto* app = juce::JUCEApplicationBase::getInstance()) app->systemRequestedQuit(); });
            return;
        }

        if (! report.updated.isEmpty())
            juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Fase Groove Piano",
                                                    tr ("Updated to version") + " " + result.version + ":\n\n"
                                                        + report.updated.joinIntoString ("\n") + "\n\n"
                                                        + tr ("Restart your DAW (or the app) to use the new version."));
        else if (! report.done && ! report.installerStarted)
            juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Fase Groove Piano",
                                                    tr ("The files could not be replaced automatically: the download is shown, copy it as the first time."));
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
    g.fillRoundedRectangle (c, 10.0f);
    g.setColour (Palette::panelEdge.brighter (0.3f));
    g.drawRoundedRectangle (c, 10.0f, 1.0f);

    auto r = c.reduced (28.0f, 22.0f);
    g.setColour (Palette::accent);
    g.setFont (font (22.0f, true));
    g.drawText (tr ("New version") + " " + result.version + " " + tr ("downloaded"), r.removeFromTop (30.0f), juce::Justification::centredLeft);
    g.setColour (Palette::dim);
    g.setFont (font (13.0f));
    g.drawText (tr ("You have") + " " + juce::String (JucePlugin_VersionString) + ". " + tr ("What's new:"),
                r.removeFromTop (26.0f), juce::Justification::centredLeft);

    const char* how = result.kind == Updater::InstallKind::installer
                        ? "INSTALL NOW updates it with the setup, without the wizard (same folders and parts). Close your DAW if it asks."
                    : result.kind == Updater::InstallKind::portable
                        ? "INSTALL NOW replaces the files of this copy where they are. Then restart your DAW."
                       #if JUCE_MAC
                        : "INSTALL NOW opens the zip: copy the plug-ins to the same folders as the first time, then restart your DAW.";
                       #else
                        : "INSTALL NOW shows the downloaded zip: copy the plug-ins to the same folders as the first time.";
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
      pumpRelease (p.apvts, ParamIDs::pumpRelease, "Release"),
      pumpSource (p.apvts, ParamIDs::pumpSource, "Sidechain"),
      pumpRate (p.apvts, ParamIDs::pumpRate, "Rate"),
      syncAttachment (p.apvts, ParamIDs::tempoSync, syncButton),
      tempoAttachment (p.apvts, ParamIDs::tempo, tempoSlider)
{
    setLookAndFeel (&lookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);   // fonts are resolved by the default one
    Localisation::loadSaved();

    styleCaption (presetLabel, "Progression presets");
    addAndMakeVisible (presetLabel);

    presetBox.setTextWhenNothingSelected ("Load a progression...");
    buildPresetMenu();
    presetBox.onChange = [this] { loadPreset (presetBox.getSelectedId() - 1); };
    presetBox.onPopup = [this] { grooveBrowser.show (GrooveBrowser::Mode::progressions, presetBox.getSelectedId() - 1); };
    grooveBrowser.onProgressionChosen = [this] (int preset) { presetBox.setSelectedId (preset + 1, juce::dontSendNotification); };
    grooveBrowser.onImportMidi = [this] { chooseMidiToImport(); };
    addChildComponent (voicingCoach);
    addChildComponent (songOverlay);
    addChildComponent (morphOverlay);
    addChildComponent (dnaOverlay);
    addChildComponent (chordLab);
    for (auto* b : { &songButton, &morphButton, &dnaButton, &variantsButton, &feelStyleButton, &newFeelButton })
    {
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::textColourOffId, Palette::accent);
        addAndMakeVisible (b);
    }
    songButton.onClick = [this] { songOverlay.show(); };
    morphButton.onClick = [this] { morphOverlay.show(); };
    morphOverlay.onChooseTarget = [this] (int current, std::function<void (int)> picked) { grooveBrowser.pickGroove (current, std::move (picked)); };
    dnaButton.onClick = [this] { dnaOverlay.show(); };
    variantsButton.onClick = [this] { chordLab.show(); };
    feelStyleButton.onClick = [this]
    {
        // How Humanize is shared between the voices: a short menu, ticked on the current one.
        const int style = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::humanizeStyle)->load());
        juce::PopupMenu menu;
        for (int i = 0; i < sgk::numHumanizeStyles; ++i)
            menu.addItem (i + 1, tr (sgk::humanizeStyleName (i)), true, i == style);
        juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&feelStyleButton), [safe] (int result)
        {
            if (safe != nullptr && result > 0)
                safe->processor.setParameterFromUI (ParamIDs::humanizeStyle, (float) (result - 1));
        });
    };
    newFeelButton.onClick = [this] { processor.newFeel(); };
    // The whole song: drag it, or save it as one .mid (the parts chosen under EXPORT MIDI).
    songOverlay.songTile.createFile = [this] { return processor.writeMidiClip (FaseGroovePianoProcessor::songArrangement, selectedClipPart()); };
    songOverlay.songTile.onDragStart = [this]
    {
        if (processor.shrinkWhileDragging)
        {
            juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
            juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->enterMiniMode (&safe->songOverlay); });
        }
    };
    songOverlay.songTile.onDragEnd = [this]
    {
        juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
        juce::MessageManager::callAsync ([safe] { if (safe != nullptr) safe->exitMiniMode(); });
    };
    songOverlay.onSaveSong = [this]
    {
        const auto part = selectedClipPart();
        auto folder = FaseGroovePianoProcessor::clipFolder();
        if (! folder.createDirectory())
            folder = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
        fileChooser = std::make_unique<juce::FileChooser> (tr ("Save MIDI clip"),
                                                           folder.getChildFile (processor.clipFileName (FaseGroovePianoProcessor::songArrangement, part)), "*.mid");
        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                                      | juce::FileBrowserComponent::warnAboutOverwriting,
                                  [this, part] (const juce::FileChooser& chooser)
                                  {
                                      auto file = chooser.getResult();
                                      if (file == juce::File())
                                          return;
                                      file = file.withFileExtension ("mid");
                                      file.deleteFile();
                                      juce::FileOutputStream stream (file);
                                      if (stream.openedOk())
                                          processor.createMidiClip (FaseGroovePianoProcessor::songArrangement, part).writeTo (stream);
                                  });
    };
    songOverlay.onSaveSections = [this]
    {
        FaseGroovePianoProcessor::clipFolder().createDirectory();
        fileChooser = std::make_unique<juce::FileChooser> (tr ("Choose a folder"), FaseGroovePianoProcessor::clipFolder());
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                                  [this] (const juce::FileChooser& chooser)
                                  {
                                      const auto folder = chooser.getResult();
                                      if (folder == juce::File())
                                          return;
                                      const int n = processor.saveSongSections (folder, selectedClipPart());
                                      juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Fase Groove Piano",
                                                                              juce::String (n) + " " + tr ("MIDI files saved in") + "\n" + folder.getFullPathName());
                                  });
    };
    coachButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    coachButton.setColour (juce::TextButton::textColourOffId, Palette::accent);
    coachButton.onClick = [this] { voicingCoach.show(); };
    addAndMakeVisible (coachButton);
    addAndMakeVisible (presetBox);

    for (int i = 0; i < sgk::maxProgressionLength; ++i)
        addAndMakeVisible (slots.add (new ChordSlot (p.apvts, i)));

    for (juce::Component* c : std::initializer_list<juce::Component*> {
             &progLength, &chordLength, &key, &scale, &extension, &voicing, &bass, &approach, &topLine, &movement, &variation, &grooveBars, &chorus, &pump, &pumpRelease, &pumpSource, &pumpRate, &sidechainMeter, &registerKnob, &strumKnob,
             &octaveToggle, &circle,
             &diceProgression, &pattern, &section, &newRiffButton, &fills, &fillStyle, &playMode, &soundType,
             &swing, &gate, &velocity, &humanize, &riffDensity,
             &soundToggle, &bassChannel, &tone, &reverb, &volume, &patternView,
             &clipLengthLabel, &clipPartLabel, &clipLength, &clipPart, &dragTile, &saveButton })
        addAndMakeVisible (c);

    octaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::octaveTop, octaveToggle);
    octaveToggle.setTooltip ("Double the top note of every chord an octave higher (bright house piano)");

    newRiffButton.setTooltip ("Generate a brand new random funky riff");
    for (auto* b : { &newRiffButton, &diceProgression, &rndPianoButton, &rndBassButton })
        b->makeRandomButton();
    newRiffButton.onClick = [this] { if (onVelocityPage()) processor.randomizeVelocities (true, true); else processor.rollNewRiff(); };

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
    sequencerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (p.apvts, ParamIDs::sequencer, sequencerToggle);
    addAndMakeVisible (sequencerToggle);
    syncButton.setClickingTogglesState (true);
    syncButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    syncButton.setColour (juce::TextButton::buttonOnColourId, Palette::accent);
    syncButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    syncButton.setColour (juce::TextButton::textColourOnId, Palette::background);
    syncButton.setTooltip ("On: follow the DAW tempo. Off: set your own BPM.");

    tempoSlider.setSliderStyle (juce::Slider::LinearBar);
    tempoSlider.setColour (juce::Slider::trackColourId, Palette::panelEdge.brighter (0.25f));
    tempoSlider.setColour (juce::Slider::backgroundColourId, Palette::field);
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

            // Columns: piano house | soulful, deep, gospel | jackin', jackin' piano | garage, afro, funky | '90s, neo soul, chill, dance stabs, generate
            group.columnBreakAfter = g == 0 || g == 3 || g == 5 || g == 8;
            groups.push_back (group);
        }
        groups.push_back ({ "Generate", { sgk::randomPatternIndex(), sgk::userPatternIndex() }, false });
        pattern.setGroups (std::move (groups));
    }
    pattern.box.onPopup = [this] { grooveBrowser.show(); };

    // Bass menu: the modes that follow the groove's rhythm, then the bass lines by genre.
    {
        std::vector<Choice::Group> groups;
        Choice::Group rhythm { "Follow the groove", {}, true };
        for (int i = 0; i < sgk::numBassPatternModes; ++i)
            rhythm.items.push_back (i);
        rhythm.items.push_back (sgk::stabsOwnRhythmMode);
        groups.push_back (rhythm);
        for (int i = 0; i < sgk::numBassLines; ++i)
        {
            const juce::String genre (sgk::bassLines()[(size_t) i].genre);
            if (groups.back().heading != "Bass lines: " + genre)
                groups.push_back ({ "Bass lines: " + genre, {}, false });
            groups.back().items.push_back (sgk::numBassPatternModes + i);
        }
        bass.setGroups (std::move (groups));
    }
    addChildComponent (grooveBrowser);
    for (auto* arrow : { &groovePrev, &grooveNext })
        addAndMakeVisible (arrow);
    groovePrev.onClick = [this] { stepGroove (-1); };
    grooveNext.onClick = [this] { stepGroove (1); };

    for (juce::Button* b : std::initializer_list<juce::Button*> { &styleMenuButton, &resetButton, &optionsButton })
    {
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::textColourOffId, Palette::text);
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
    Updater::cleanupAfterUpdate();
    if (Updater::autoCheckEnabled() && Updater::firstAutoCheckThisSession())
        updateChecker.start (false);
    applyTips();

    // Transport of the preview (the groove without the DAW): to the start, play / pause, stop.
    rewindButton.onClick = [this] { processor.previewToStart(); };
    playButton.onClick = [this] { processor.previewToggle(); };
    stopButton.onClick = [this] { processor.previewStop(); };
    playButton.onColour = Palette::accent2;
    rewindButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    playButton.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    stopButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    for (auto* b : { &rewindButton, &playButton, &stopButton })
        addAndMakeVisible (b);

    styleCaption (clipLengthLabel, "Clip length");
    styleCaption (clipPartLabel, "Notes");
    clipLength.addItemList ({ "Full loop", "1 bar", "2 bars", "4 bars", "8 bars", "16 bars", "32 bars",
                              "Song: intro > drop > outro" }, 1);
    clipLength.setSelectedId (1, juce::dontSendNotification);
    clipPart.addItemList ({ "Chords + bass", "Chords only", "Bass only", "Separate tracks" }, 1);
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
    addChildComponent (shrinkToggle);   // the same choice is in OPTIONS > SETTINGS

    // Rhythm grid: editable cells, A/B/C/D segments, quick bass on/off.
    patternView.onCellClicked = [this] (bool bassRow, int step, const juce::MouseEvent& e) { cellClicked (bassRow, step, e); };
    patternView.setTooltip ("Click a step: hit > accent > ghost > rest.  Right-click: more options");

    // What you hear: two switches, the piano and the bass (lit = you hear it; never both off).
    hearPianoButton.onClick = [this] { hearPart (false); };
    hearBassButton.onClick  = [this] { hearPart (true); };
    hearPianoButton.setConnectedEdges (juce::Button::ConnectedOnRight);
    hearBassButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
    hearBassButton.onColour = Palette::accent2;
    for (auto* b : { &hearPianoButton, &hearBassButton })
        addAndMakeVisible (b);

    // Piano roll (what plays) or rhythm grid (what you edit).
    rollTab.setConnectedEdges (juce::Button::ConnectedOnRight);
    gridTab.setConnectedEdges (juce::Button::ConnectedOnLeft);
    rollTab.onClick = [this] { showRoll (true); };
    gridTab.onClick = [this] { velocityShown = false; showRoll (false); };
    for (auto* b : { &rollTab, &gridTab })
    {
        b->setClickingTogglesState (false);
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::buttonOnColourId, Palette::accent);
        b->setColour (juce::TextButton::textColourOffId, Palette::dim);
        b->setColour (juce::TextButton::textColourOnId, Palette::background);
        addAndMakeVisible (b);
    }
    addAndMakeVisible (pianoRoll);

    // The velocity page (grid view only): the hits as faders.
    velocityTab.setClickingTogglesState (false);
    velocityTab.setColour (juce::TextButton::buttonColourId, Palette::background);
    velocityTab.setColour (juce::TextButton::buttonOnColourId, Palette::level);
    velocityTab.setColour (juce::TextButton::textColourOffId, Palette::level);
    velocityTab.setColour (juce::TextButton::textColourOnId, Palette::background);
    velocityTab.onClick = [this] { showVelocity (! velocityShown); };
    addChildComponent (velocityTab);
    velocityView.onLevel = [this] (bool bassRow, int step, int level) { processor.setStepVelocity (bassRow, step, level); };
    velocityView.onReset = [this] (bool chords, bool bass) { processor.resetStepVelocities (chords, bass); };
    addChildComponent (velocityView);

    // The big piano roll, to edit the notes one by one.
    auto openRollEditor = [this]
    {
        updatePianoRoll (processor.currentSettings());
        rollEditor.show (processor.naturalLoopBeats());
    };
    expandRollButton.onClick = openRollEditor;
    pianoRoll.onDoubleClick = openRollEditor;
    rollEditor.chordSpans = [this] (double length) { return chordSpansFor (processor.currentSettings(), length); };
    addChildComponent (rollEditor);
    generatedButton.onClick = [this] { processor.clearEditedLoop(); };
    for (auto* b : { &expandRollButton, &generatedButton })
    {
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::textColourOffId, b == &generatedButton ? Palette::accent2 : Palette::accent);
        addAndMakeVisible (b);
    }
    for (auto* b : { &rndPianoButton, &rndBassButton })
    {
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::textColourOffId, Palette::accent);
        addAndMakeVisible (b);
    }
    myGroovesButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    myGroovesButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    myGroovesButton.onClick = [this] { showGrooveMenu(); };
    addAndMakeVisible (myGroovesButton);
    // On the velocity page the random buttons change only the levels.
    rndPianoButton.onClick = [this] { if (onVelocityPage()) processor.randomizeVelocities (true, false); else processor.rerollHand (false); };
    rndBassButton.onClick = [this] { if (onVelocityPage()) processor.randomizeVelocities (false, true); else processor.rerollHand (true); };

    for (int i = 0; i < sgk::numSegments; ++i)
    {
        auto* box = segmentBoxes.add (new juce::ComboBox());
        fillChoiceItems (*box, p.apvts.getParameter (ParamIDs::segment (i)));
        box->setJustificationType (juce::Justification::centred);
        box->setTooltip ("Which part (A-D) plays here: combine parts like A A B C");
        segmentAttachments.add (new juce::AudioProcessorValueTreeState::ComboBoxAttachment (p.apvts, ParamIDs::segment (i), *box));
        addAndMakeVisible (box);

        auto* dice = segmentDice.add (new IconButton (IconButton::Icon::dice));
        dice->makeRandomButton();
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

        // Velocity page: which part's levels play here, new random levels, lock.
        auto* vbox = velocityBoxes.add (new juce::ComboBox());
        vbox->addItemList ({ "A", "B", "C", "D" }, 1);
        vbox->setJustificationType (juce::Justification::centred);
        vbox->setColour (juce::ComboBox::textColourId, Palette::level);
        vbox->setColour (juce::ComboBox::arrowColourId, Palette::level);
        vbox->onChange = [this, i, vbox]
        {
            if (vbox->getSelectedId() > 0 && vbox->getSelectedId() - 1 != processor.velocityPartAt (i))
                processor.setVelocityPartAt (i, vbox->getSelectedId() - 1);
        };
        addChildComponent (vbox);

        auto* vdice = velocityDice.add (new IconButton (IconButton::Icon::dice));
        vdice->makeRandomButton();
        vdice->setColour (juce::TextButton::buttonColourId, Palette::background);
        vdice->onClick = [this, i] { processor.randomizeVelocities (true, true, processor.velocityPartAt (i)); };
        addChildComponent (vdice);

        auto* vlock = velocityLocks.add (new juce::TextButton ("LOCK"));
        vlock->setClickingTogglesState (true);
        vlock->setColour (juce::TextButton::buttonColourId, Palette::background);
        vlock->setColour (juce::TextButton::buttonOnColourId, Palette::level);
        vlock->setColour (juce::TextButton::textColourOffId, Palette::dim);
        vlock->setColour (juce::TextButton::textColourOnId, Palette::background);
        vlock->onClick = [this, i, vlock] { processor.setVelocityLocked (processor.velocityPartAt (i), vlock->getToggleState()); };
        addChildComponent (vlock);
    }

    newRiffButton.setTooltip ("New random funky riff (locked parts are kept)");
    showRoll (true);

    saveButton.setColour (juce::TextButton::buttonColourId, Palette::background);
    saveButton.setColour (juce::TextButton::textColourOffId, Palette::text);
    saveButton.onClick = [this] { showSaveMenu(); };

    // Presets, undo / redo, learn.
    for (auto* b : { &presetsButton, &learnButton })
    {
        b->setColour (juce::TextButton::buttonColourId, Palette::background);
        b->setColour (juce::TextButton::textColourOffId, Palette::text);
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
    fullScreenButton.onClick = [this] { toggleFullScreen(); };
    fullScreenButton.setTooltip ("Full screen");
    addAndMakeVisible (fullScreenButton);
    addAndMakeVisible (undoButton);
    addAndMakeVisible (redoButton);
    undoButton.onClick = [this] { undo(); };
    redoButton.onClick = [this] { redo(); };
    setWantsKeyboardFocus (true);

    // Buttons don't keep the keyboard focus, so the space bar always plays / stops the preview
    // instead of pressing the last button clicked.
    std::function<void (juce::Component&)> noButtonFocus = [&noButtonFocus] (juce::Component& c)
    {
        for (auto* child : c.getChildren())
        {
            if (dynamic_cast<juce::Button*> (child) != nullptr)
                child->setWantsKeyboardFocus (false);
            noButtonFocus (*child);
        }
    };
    noButtonFocus (*this);

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
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
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
    if (key == juce::KeyPress::spaceKey && ! mods.isAnyModifierKeyDown())
    {
        // Space: play / stop the preview (the plug-in can't start the DAW's transport).
        processor.previewToggle();
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
            if (! safe->processor.loadPreset (files[result - 100]))
            {
                juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Fase Groove Piano",
                                                        tr ("This preset file is damaged or is not a Fase Groove Piano preset: nothing was changed."));
                return;
            }
            safe->afterRestore();
        }
    });
}

void FaseGroovePianoEditor::stepGroove (int direction)
{
    const auto order = GrooveBrowser::menuOrder();
    const int current = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::pattern)->load());
    const auto it = std::find (order.begin(), order.end(), current);
    int index = it == order.end() ? (direction > 0 ? -1 : 0) : (int) (it - order.begin());
    index = (index + direction + (int) order.size()) % (int) order.size();
    processor.setParameterFromUI (ParamIDs::pattern, (float) order[(size_t) index]);
}

bool FaseGroovePianoEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (const auto& f : files)
        if (juce::File (f).hasFileExtension ("mid;midi"))
            return true;
    return false;
}

void FaseGroovePianoEditor::filesDropped (const juce::StringArray& files, int, int)
{
    for (const auto& f : files)
        if (juce::File (f).hasFileExtension ("mid;midi"))
        {
            importMidi (juce::File (f));
            return;
        }
}

void FaseGroovePianoEditor::chooseMidiToImport()
{
    fileChooser = std::make_unique<juce::FileChooser> (tr ("Import a MIDI file..."),
                                                       juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.mid;*.midi");
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& chooser)
                              {
                                  if (chooser.getResult().existsAsFile())
                                      importMidi (chooser.getResult());
                              });
}

void FaseGroovePianoEditor::importMidi (const juce::File& file)
{
    const auto analysis = FaseGroovePianoProcessor::analyzeMidiFile (file);
    if (! analysis.ok)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::WarningIcon, "Fase Groove Piano",
                                                tr ("No chords found in this MIDI file."));
        return;
    }

    processor.applyAnalysis (analysis);
    presetBox.setSelectedId (0, juce::dontSendNotification);

    // What was found, in words (it is an estimate).
    const auto s = processor.currentSettings();
    juce::StringArray chords;
    for (int i = 0; i < s.progressionLength; ++i)
        chords.add (sgk::chordName (sgk::GrooveEngine::chordForSlot (s, i)));

    static const char* const lengths[] { "1/2 bar", "1 bar", "2 bars", "4 bars" };
    juce::String text;
    text << tr ("Key") << ": " << sgk::noteName (analysis.keyRoot) << " " << tr (sgk::scaleName (analysis.scale)) << "\n"
         << tr ("Chords") << ": " << chords.joinIntoString ("  ") << "\n"
         << tr ("Each chord") << ": " << tr (lengths[juce::jlimit (0, 3, analysis.chordLengthChoice)]) << "\n"
         << tr ("Notes outside the scale") << ": " << juce::roundToInt (analysis.outOfScale * 100.0) << "%";
    juce::StringArray outside;
    for (int pc = 0; pc < 12; ++pc)
        if (analysis.outsideNotes[(size_t) pc])
            outside.add (sgk::noteName (pc));
    if (! outside.isEmpty())
        text << " (" << outside.joinIntoString (", ") << ")";
    text << "\n" << tr ("Reliability") << ": "
         << (analysis.confidence >= 0.8 ? tr ("high") : analysis.confidence >= 0.6 ? tr ("medium") : tr ("low: check the chords"));
    text << "\n\n" << tr ("This is an estimate: inversions and passing notes can mislead it. Fix any chord on the circle, or undo with the arrow.");

    juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, file.getFileNameWithoutExtension(), text);
}

void FaseGroovePianoEditor::showGrooveMenu()
{
    juce::PopupMenu menu, remove;
    menu.addItem (1, tr ("Save this groove as..."));
    menu.addSeparator();

    const auto files = FaseGroovePianoProcessor::grooveFiles();
    if (files.isEmpty())
        menu.addItem (-1, tr ("No saved grooves yet"), false);
    for (int i = 0; i < files.size(); ++i)
    {
        menu.addItem (100 + i, files[i].getFileNameWithoutExtension());
        remove.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    }

    menu.addSeparator();
    menu.addSubMenu (tr ("Delete"), remove, ! files.isEmpty());
    menu.addItem (2, tr ("Open the Fase Groove Piano folder"));

    juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&myGroovesButton), [safe, files] (int result)
    {
        if (safe == nullptr || result <= 0)
            return;

        if (result == 1)
            safe->saveGrooveAs();
        else if (result == 2)
        {
            FaseGroovePianoProcessor::grooveFolder().createDirectory();
            FaseGroovePianoProcessor::userFolder().startAsProcess();
        }
        else if (result >= 1000 && result - 1000 < files.size())
            files[result - 1000].deleteFile();
        else if (result >= 100 && result - 100 < files.size())
            safe->processor.loadGroove (files[result - 100]);
    });
}

void FaseGroovePianoEditor::saveGrooveAs()
{
    auto* window = new juce::AlertWindow (tr ("Save groove"), tr ("Name:"), juce::MessageBoxIconType::NoIcon, this);
    window->addTextEditor ("name", tr ("My groove"));
    window->addButton (tr ("Save"), 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton (tr ("Cancel"), 0, juce::KeyPress (juce::KeyPress::escapeKey));

    juce::Component::SafePointer<FaseGroovePianoEditor> safe (this);
    juce::Component::SafePointer<juce::AlertWindow> safeWindow (window);
    window->enterModalState (true, juce::ModalCallbackFunction::create ([safe, safeWindow] (int result)
    {
        if (safe != nullptr && safeWindow != nullptr && result == 1)
            safe->processor.saveGroove (safeWindow->getTextEditorContents ("name"));
    }), true);
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
    FaseGroovePianoProcessor::clipFolder().createDirectory();
    fileChooser = std::make_unique<juce::FileChooser> (tr ("Choose a folder"), FaseGroovePianoProcessor::clipFolder());

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
    for (auto* k : { &registerKnob, &strumKnob, &swing, &gate, &velocity, &humanize, &riffDensity, &tone, &reverb, &volume, &chorus, &pump, &pumpRelease })
        k->refreshText();

    for (auto* c : { &progLength, &chordLength, &key, &scale, &extension, &voicing, &bass, &approach, &topLine, &movement, &variation, &grooveBars, &pattern, &section,
                     &fills, &fillStyle, &playMode, &bassChannel, &soundType, &pumpSource, &pumpRate })
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

    static const char* const parts[] { "Chords + bass", "Chords only", "Bass only", "Separate tracks" };
    const int selectedPart = clipPart.getSelectedId();
    for (int i = 0; i < clipPart.getNumItems(); ++i)
        clipPart.changeItemText (i + 1, tr (parts[i]));
    clipPart.setSelectedId (selectedPart, juce::dontSendNotification);

    diceProgression.setButtonText (tr ("RANDOM"));
    newRiffButton.setButtonText (tr ("NEW RIFF"));
    syncButton.setButtonText (tr ("SYNC DAW"));
    sequencerToggle.setButtonText (tr ("Sequencer"));
    sequencerToggle.setTooltip (tr ("Off: the groove doesn't play with the DAW, e.g. when the dragged MIDI clip plays on this track (no doubled notes). Incoming MIDI still plays the guide sound; the play button still previews."));
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
    saveButton.setButtonText (tr ("SAVE .MID..."));
    shrinkToggle.setButtonText (tr ("Mini window while dragging"));

    octaveToggle.setTooltip (tr ("Double the top note of every chord an octave higher (bright house piano)"));
    freeToggle.setButtonText (tr ("Free chords"));
    freeToggle.setTooltip (tr ("Free chords: drag the dots anywhere on the circle, even outside the scale"));
    buildPresetMenu();
    approach.box.setTooltip (tr ("Gospel slide: on the last 16th before a chord change, play the next chord a half step above or below"));
    newRiffButton.setTooltip (tr ("New random funky riff (locked parts are kept)"));
    groovePrev.setTooltip (tr ("Previous groove"));
    grooveNext.setTooltip (tr ("Next groove"));
    diceProgression.setTooltip (tr ("Random soulful-house progression"));
    syncButton.setTooltip (tr ("On: follow the DAW tempo. Off: set your own BPM."));
    tempoSlider.setTooltip (tr ("Drag, or double-click to type the BPM"));
    rewindButton.setTooltip (tr ("Back to the start"));
    playButton.setTooltip (tr ("Play / pause the groove without starting the DAW (space bar)"));
    stopButton.setTooltip (tr ("Stop (the next play starts from the top)"));
    shrinkToggle.setTooltip (tr ("Shrink the plug-in window while you drag a clip, so you can see your DAW"));
    patternView.setTooltip (tr ("Click a step: hit > accent > ghost > rest.  Right-click: more options"));
    hearPianoButton.setTooltip (tr ("Hear the piano (lit) or mute it: exported clips keep everything"));
    hearBassButton.setTooltip (tr ("Hear the bass (lit) or switch it off"));
    rndPianoButton.setButtonText (tr ("PIANO"));
    myGroovesButton.setButtonText (tr ("MY GROOVES"));
    myGroovesButton.setTooltip (tr ("Save the rhythm grid as your own groove, or load one you saved (Documents/Fase Groove Piano/Grooves)"));
    rndBassButton.setButtonText (tr ("BASS"));
    updateRandomTips();

    for (int i = 0; i < sgk::numSegments; ++i)
    {
        velocityBoxes[i]->setTooltip (tr ("Whose velocities play here (A-D): e.g. A A A A gives every half bar the accents of the first"));
        velocityDice[i]->setButtonText ({});
        velocityDice[i]->setTooltip (tr ("New random velocities for this part (the rhythm stays)"));
        velocityLocks[i]->setButtonText (tr ("LOCK"));
        velocityLocks[i]->setTooltip (tr ("Keep these velocities when you press the random buttons"));
        segmentBoxes[i]->setTooltip (tr ("Which part (A-D) plays here: combine parts like A A B C"));
        segmentDice[i]->setButtonText ({});
        segmentDice[i]->setTooltip (tr ("New random rhythm for this part"));
        segmentLocks[i]->setButtonText (tr ("LOCK"));
        segmentLocks[i]->setTooltip (tr ("Keep this part when you press NEW RIFF"));
    }

    options.refreshText();
    updateOverlay.refreshText();
    grooveBrowser.refreshText();
    voicingCoach.refreshText();
    songOverlay.refreshText();
    morphOverlay.refreshText();
    dnaOverlay.refreshText();
    chordLab.refreshText();
    variantsButton.setButtonText (tr ("VARIANTS"));
    variantsButton.setTooltip (tr ("Chord variations: more soulful, gospel, deep or chromatic versions of your progression, to hear and keep"));
    newFeelButton.setButtonText (tr ("NEW FEEL"));
    newFeelButton.setTooltip (tr ("Another humanization of the same groove (repeatable: saved with the project)"));
    feelStyleButton.setTooltip (tr ("How Humanize is shared: Even, Pianist (the top note sings), Laid-back bass, Tight bass. Click to change"));
    dnaButton.setTooltip (tr ("Musical DNA: variations by intention (more soulful, funkier, more minimal, more energy, more sophisticated)"));
    songButton.setButtonText (tr ("FULL SONG..."));
    rollTab.setButtonText (tr ("PIANO ROLL"));
    expandRollButton.setButtonText (tr ("EDIT NOTES"));
    expandRollButton.setTooltip (tr ("Open the piano roll big, zoom in and edit the notes one by one (or double-click the piano roll)"));
    generatedButton.setButtonText (tr ("BACK TO GENERATED"));
    generatedButton.setTooltip (tr ("Forget your edits: the plug-in plays the generated notes again"));
    rollEditor.refreshText();
    gridTab.setButtonText (tr ("EDIT GRID"));
    velocityTab.setButtonText (tr ("VELOCITY"));
    velocityTab.setTooltip (tr ("The velocity of every hit, as faders: drag to set it, double-click for automatic"));
    velocityView.setTooltip (tr ("Drag a fader up or down (or across several) to set the velocity. Double-click: automatic again. Right-click: reset"));
    rollTab.setTooltip (tr ("Every note the plug-in plays: chords, bass and fills, with the chord names"));
    gridTab.setTooltip (tr ("The rhythm grid: click the steps to write your own groove"));
    hearPianoButton.setButtonText (tr ("PIANO"));
    hearBassButton.setButtonText (tr ("BASS"));
    morphButton.setButtonText (tr ("MORPH"));
    songButton.setTooltip (tr ("The whole song: arrange the sections, then drag or save it as one .mid, or one .mid per section"));
    morphButton.setTooltip (tr ("Morph the groove step by step towards another one (locked parts stay)"));
    coachButton.setTooltip (tr ("Voicing Coach: compare two voicings of your progression"));
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
    pump.setTip (tr ("Sidechain depth: ducks the guide sound like a kick sidechain"));
    pumpRelease.setTip (tr ("How fast the sound comes back after each duck"));
    pumpSource.box.setTooltip (tr ("Tempo: the duck follows the beat. Sidechain input: it follows the track you send to the plug-in's sidechain input (e.g. the kick)"));
    sidechainMeter.setTooltip (tr ("The signal arriving at the sidechain input: if it stays grey, nothing is routed to the plug-in"));
    pumpRate.box.setTooltip (tr ("How often the tempo sidechain ducks: every beat, 8th, half bar or 16th"));
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
    return (FaseGroovePianoProcessor::ClipPart) juce::jlimit (0, 3, clipPart.getSelectedItemIndex());
}

void FaseGroovePianoEditor::saveClip()
{
    const int bars = selectedClipBars();
    const auto part = selectedClipPart();
    auto folder = FaseGroovePianoProcessor::clipFolder();
    if (! folder.createDirectory())
        folder = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);

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

void FaseGroovePianoEditor::enterMiniMode (juce::Component* keep)
{
    if (miniMode)
        return;

    miniMode = true;
    miniKeep = keep != nullptr ? keep : &dragTile;
    if (miniKeep == &songOverlay)
        songOverlay.setMini (true);
    fullSize = { getWidth(), getHeight() };
    miniStartedAt = juce::Time::getMillisecondCounter();

    hiddenForMini.clear();
    for (auto* c : getChildren())
    {
        if (c == miniKeep || ! c->isVisible()
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
    if (miniKeep == &songOverlay)
        songOverlay.setMini (false);
    miniKeep = nullptr;
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
void FaseGroovePianoEditor::hearPart (bool bass)
{
    // Lit = you hear it. Switching off the only part you hear switches the other one on.
    const bool pianoOn = ! processor.chordsMuted.load() || ! processor.isBassOn();
    const bool bassOn = processor.isBassOn();
    if (bass)
    {
        if (bassOn) { processor.toggleBass(); processor.chordsMuted = false; }   // off (the piano plays)
        else        processor.toggleBass();
    }
    else
    {
        if (pianoOn && bassOn)  processor.chordsMuted = true;                       // bass only
        else if (pianoOn)      { processor.chordsMuted = true; processor.toggleBass(); }
        else                   processor.chordsMuted = false;
    }
}

void FaseGroovePianoEditor::showRoll (bool roll)
{
    rollShown = roll;
    rollTab.setToggleState (roll, juce::dontSendNotification);
    gridTab.setToggleState (! roll, juce::dontSendNotification);
    pianoRoll.setVisible (roll);
    expandRollButton.setVisible (roll);
    velocityTab.setVisible (! roll);
    if (roll)
        velocityShown = false;
    velocityTab.setToggleState (velocityShown, juce::dontSendNotification);
    patternView.setVisible (! roll && ! velocityShown);
    velocityView.setVisible (! roll && velocityShown);
    const bool rhythmStrip = ! roll && ! velocityShown, levelStrip = ! roll && velocityShown;
    for (auto* c : segmentBoxes)  c->setVisible (rhythmStrip);
    for (auto* c : segmentDice)   c->setVisible (rhythmStrip);
    for (auto* c : segmentLocks)  c->setVisible (rhythmStrip);
    for (auto* c : velocityBoxes) c->setVisible (levelStrip);
    for (auto* c : velocityDice)  c->setVisible (levelStrip);
    for (auto* c : velocityLocks) c->setVisible (levelStrip);
    updateRandomTips();   // the random buttons say what they change
    if (getWidth() > 0)
        resized();
}

void FaseGroovePianoEditor::updateRandomTips()
{
    const bool levels = onVelocityPage();
    rndPianoButton.setTooltip (levels ? tr ("New random velocities for the chords (unlocked parts): the rhythm stays")
                                      : tr ("New random rhythm for the piano (chords) only: the bass stays"));
    rndBassButton.setTooltip (levels ? tr ("New random velocities for the bass (unlocked parts): the rhythm stays")
                                     : tr ("New random rhythm for the bass only: the piano stays"));
    newRiffButton.setTooltip (levels ? tr ("On the velocity page: new random velocities for chords and bass (unlocked parts)")
                                     : tr ("New random funky riff (locked parts are kept)"));
}

void FaseGroovePianoEditor::showVelocity (bool show)
{
    velocityShown = show;
    if (show)
        updateVelocityView (processor.currentSettings());
    showRoll (false);
}

void FaseGroovePianoEditor::updateVelocityView (const sgk::GrooveSettings& s)
{
    const auto p = sgk::GrooveEngine::resolvePattern (s);
    const int len = p.length();

    VelocityView::Row chords, bass;
    chords.line = p.rightHand;
    chords.levels.resize ((size_t) len);
    bass.levels.resize ((size_t) len);
    for (int i = 0; i < len; ++i)
    {
        chords.levels[(size_t) i] = processor.stepVelocity (false, i);
        bass.levels[(size_t) i] = processor.stepVelocity (true, i);
    }

    // The bass row shows what the bass plays: with the piano-stab bass, the piano's stabs.
    bass.line = s.bass == sgk::BassMode::withChords ? sgk::stabBassLine (p) : p.leftHand;
    bass.line.resize ((size_t) len, '.');
    if (! processor.isBassOn())
    {
        bass.enabled = false;
        bass.message = tr ("Bass off  -  click BASS to turn it back on");
    }
    else if (sgk::isBassLine (s.bass))
    {
        bass.enabled = false;
        bass.message = tr ("A bass line from the library plays its own levels: write the bass in the grid to set them here");
    }

    velocityView.setState (chords, bass, processor.uiStep.load());
}

std::vector<PianoRollView::ChordSpan> FaseGroovePianoEditor::chordSpansFor (const sgk::GrooveSettings& s, double length) const
{
    std::vector<PianoRollView::ChordSpan> spans;
    const double chordBeats = s.stepsPerChord / 4.0;
    for (int k = 0; k * chordBeats < length - 1.0e-6; ++k)
    {
        const int slot = sgk::GrooveEngine::slotForStep ((long long) (k * s.stepsPerChord), s);
        spans.push_back ({ k * chordBeats, juce::jmin (chordBeats, length - k * chordBeats),
                           sgk::chordName (sgk::GrooveEngine::chordForSlot (s, slot)) });
    }
    return spans;
}

void FaseGroovePianoEditor::updatePianoRoll (const sgk::GrooveSettings& s)
{
    // The loop: long enough for the groove and the progression to come round together (8 bars at most).
    const int patternBars = juce::jmax (1, sgk::GrooveEngine::patternLength (s) / 16);
    const int progressionBars = juce::jmax (1, (s.progressionLength * s.stepsPerChord + 15) / 16);
    int bars = std::lcm (patternBars, progressionBars);
    if (bars > 8)
        bars = juce::jmin (8, juce::jmax (patternBars, progressionBars));
    rollLength = s.playsEditedLoop() ? s.editedLoopLength : bars * 4.0;

    std::vector<PianoRollView::Note> notes;
    {
        const auto events = sgk::GrooveEngine::renderClip (s, rollLength);
        std::map<int, std::pair<double, int>> open;   // note -> start, velocity (per part)
        std::map<int, sgk::Part> parts;
        for (const auto& e : events)
        {
            const int id = e.note + 128 * (int) e.part;
            if (e.isNoteOn())
            {
                open[id] = { e.ppq, e.velocity };
                parts[id] = e.part;
            }
            else if (auto it = open.find (id); it != open.end())
            {
                notes.push_back ({ it->second.first, juce::jmax (0.02, e.ppq - it->second.first), e.note, it->second.second, e.part });
                open.erase (it);
            }
        }
        for (const auto& [id, on] : open)
            notes.push_back ({ on.first, juce::jmax (0.05, rollLength - on.first), id % 128, on.second, parts[id] });
    }

    auto chordSpans = chordSpansFor (s, rollLength);
    pianoRoll.setContent (std::move (notes), std::move (chordSpans), rollLength, processor.chordsMuted.load());
    pianoRoll.setEdited (s.playsEditedLoop());
    const double ppq = processor.uiPpq.load();
    pianoRoll.setPlayhead (ppq < 0.0 ? -1.0 : std::fmod (ppq, rollLength));
}

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

    // Back on the window (Alt+Tab, a click on the title bar): the keys work again at once, so
    // the space bar plays without clicking inside first.
    if (auto* peer = getPeer(); peer != nullptr && peer->isFocused() && isShowing()
        && juce::Component::getCurrentlyFocusedComponent() == nullptr)
        (rollEditor.isVisible() ? static_cast<juce::Component&> (rollEditor) : static_cast<juce::Component&> (*this)).grabKeyboardFocus();

    fullScreenButton.setToggleState (isFullScreen(), juce::dontSendNotification);
    fullScreenButton.setTooltip (tr (isFullScreen() ? "Exit full screen" : "Full screen"));

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
    undoButton.setAlpha (undoStack.isEmpty() ? 0.35f : 1.0f);
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
        repaint (chordDisplayArea.toFloat().transformedBy (uiTransform()).getSmallestIntegerContainer().expanded (2));
    }

    {
        const bool external = juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::pumpSource)->load()) == 1;
        sidechainMeter.setVisible (external);
        sidechainMeter.setState (processor.sidechainMeter.load(), processor.apvts.getRawParameterValue (ParamIDs::pump)->load() < 0.001f);
        processor.sidechainMeter = processor.sidechainMeter.load() * 0.7f;   // falls when nothing arrives
    }

    const bool bassOn = processor.isBassOn();
    patternView.setState (sgk::GrooveEngine::resolvePattern (s), processor.uiStep.load(), bassOn);
    if (velocityShown && ! rollShown)
        updateVelocityView (s);

    if (rollShown)
        updatePianoRoll (s);
    generatedButton.setVisible (rollShown && s.playsEditedLoop());
    dragTile.setSubtitle (clipPart.getText() + juce::String::fromUTF8 ("  \xc2\xb7  ") + clipLength.getText());
    if (songOverlay.isVisible())
        songOverlay.songTile.setSubtitle (clipPart.getText() + juce::String::fromUTF8 ("  \xc2\xb7  ") + tr ("Song"));

    feelStyleButton.setButtonText (tr ("Feel") + ": " + tr (sgk::humanizeStyleName (juce::roundToInt (processor.apvts.getRawParameterValue (ParamIDs::humanizeStyle)->load()))) + "  " + juce::String::fromUTF8 ("\xe2\x96\xbe"));
    {
        const bool pianoHeard = ! processor.chordsMuted.load() || ! bassOn;
        hearPianoButton.setToggleState (pianoHeard, juce::dontSendNotification);
        hearBassButton.setToggleState (bassOn, juce::dontSendNotification);
        hearPianoButton.setIcon (pianoHeard ? IconButton::Icon::speaker : IconButton::Icon::speakerOff);
        hearBassButton.setIcon (bassOn ? IconButton::Icon::speaker : IconButton::Icon::speakerOff);
    }
    {
        // Transport: play shows pause while the preview runs.
        const bool previewing = processor.previewOn.load();
        playButton.setIcon (previewing ? IconButton::Icon::pause : IconButton::Icon::play);
        playButton.setToggleState (previewing, juce::dontSendNotification);
        fullScreenButton.setIcon (isFullScreen() ? IconButton::Icon::exitFullScreen : IconButton::Icon::fullScreen);
    }

    for (int i = 0; i < sgk::numSegments; ++i)
    {
        segmentLocks[i]->setToggleState (processor.isSegmentLocked (processor.segmentAtStep (i * sgk::userPatternLength / sgk::numSegments)),
                                         juce::dontSendNotification);
        const int part = processor.velocityPartAt (i);
        if (velocityBoxes[i]->getSelectedId() != part + 1)
            velocityBoxes[i]->setSelectedId (part + 1, juce::dontSendNotification);
        velocityLocks[i]->setToggleState (processor.isVelocityLocked (part), juce::dontSendNotification);
    }

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


    const bool synced = syncButton.getToggleState();
    tempoSlider.setVisible (! synced);
    dawTempo.setVisible (synced);
    dawTempo.setText (juce::String (processor.hostBpm.load(), 1) + " BPM  " + tr ("(DAW)"), juce::dontSendNotification);
}

void FaseGroovePianoEditor::drawPanel (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& title)
{
    // Flat panel, hairline border; the title is a small letter-spaced caption with an accent tick.
    const auto r = area.toFloat();
    g.setColour (Palette::panel);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (Palette::panelEdge);
    g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);

    if (title.isNotEmpty())
    {
        auto head = area.reduced (14, 8).removeFromTop (16).toFloat();
        g.setColour (Palette::accent);
        g.fillRoundedRectangle (head.getX(), head.getCentreY() - 5.0f, 2.5f, 10.0f, 1.0f);
        juce::Font f (juce::FontOptions (11.0f).withStyle ("Bold"));
        f.setExtraKerningFactor (0.12f);
        g.setFont (f);
        g.setColour (Palette::text.withAlpha (0.85f));
        g.drawText (title, head.withTrimmedLeft (9.0f), juce::Justification::centredLeft);
    }
}

void FaseGroovePianoEditor::paint (juce::Graphics& g)
{
    g.fillAll (Palette::background);
    g.addTransform (uiTransform());

    if (miniMode)
    {
        g.setColour (Palette::dim);
        g.setFont (font (11.0f));
        g.drawText (tr ("Drop the clip on a track in your DAW"), juce::Rectangle<int> (0, miniHeight - 22, miniWidth, 18),
                    juce::Justification::centred);
        return;
    }

    // Top bar: a slightly lighter band with a hairline under it.
    {
        const auto band = juce::Rectangle<float> (0.0f, 0.0f, (float) baseWidth, 100.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff15181e), 0.0f, 0.0f, juce::Colour (0xff101317), 0.0f, band.getBottom(), false));
        g.fillRect (band);
        g.setColour (Palette::panelEdge);
        g.fillRect (band.withTop (band.getBottom() - 1.0f));
    }

    // Header: logo + credit line
    auto header = headerArea.toFloat();
    Branding::drawFullLogo (g, header.removeFromTop (56.0f).withWidth (300.0f));
    g.setColour (Palette::dim);
    g.setFont (font (11.5f));
    g.drawText (tr ("by") + " " + Branding::artist, header.withTrimmedLeft (66.0f), juce::Justification::centredLeft);

    // Current chord: a display, like the screen of a hardware unit.
    {
        const auto d = chordDisplayArea.toFloat();
        g.setColour (Palette::field);
        g.fillRoundedRectangle (d, 6.0f);
        g.setColour (Palette::panelEdge);
        g.drawRoundedRectangle (d.reduced (0.5f), 6.0f, 1.0f);
    }
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

    drawPanel (g, tempoArea, tr ("TRANSPORT & TEMPO"));
    drawPanel (g, progressionPanel, tr ("PROGRESSION"));
    drawPanel (g, harmonyPanel, tr ("HARMONY & SCALES"));
    drawPanel (g, circlePanel, tr ("CIRCLE OF FIFTHS"));
    drawPanel (g, groovePanel, tr ("GROOVE"));
    drawPanel (g, soundPanel, tr ("GUIDE SOUND & MIDI OUT"));
    drawPanel (g, gridPanel, {});
    {
        auto f = font (9.5f, true);
        f.setExtraKerningFactor (0.1f);
        g.setFont (f);
        g.setColour (Palette::dim);
        for (const auto& [area, caption] : gridCaptions)
            g.drawText (Localisation::upper (caption), area.withWidth (juce::jmax (area.getWidth(), 110)), juce::Justification::centredLeft);
    }
    drawPanel (g, clipPanel, tr ("EXPORT MIDI"));

    // FEEL: a small heading with a line, so the knobs read as their own group.
    {
        auto h = feelHeader.toFloat();
        g.setColour (Palette::accent);
        g.setFont (font (11.0f, true));
        const juce::String title = Localisation::upper (tr ("Feel"));
        const float tw = juce::GlyphArrangement::getStringWidth (font (11.0f, true), title) + 8.0f;
        g.drawText (title, h.withWidth (tw), juce::Justification::centredLeft);
        g.setColour (Palette::panelEdge);
        g.fillRect (h.getX() + tw, h.getCentreY() - 0.5f, juce::jmax (0.0f, (float) feelStyleButton.getX() - 8.0f - h.getX() - tw), 1.0f);
    }

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
        // Full screen can have another shape: the interface keeps its proportions, centred.
        uiScale = juce::jmax (0.3f, juce::jmin (getWidth() / (float) baseWidth, getHeight() / (float) baseHeight));
        if (! isFullScreen())
            processor.editorScale = uiScale;
        layoutFull();
    }
    uiOffset = miniMode ? juce::Point<float>()
                        : juce::Point<float> ((getWidth() - baseWidth * uiScale) * 0.5f, (getHeight() - baseHeight * uiScale) * 0.5f);

    // Everything is laid out at base size and scaled as a whole.
    for (auto* c : getChildren())
        if (dynamic_cast<juce::ResizableCornerComponent*> (c) == nullptr && dynamic_cast<juce::TooltipWindow*> (c) == nullptr)
            c->setTransform (uiTransform());
}

bool FaseGroovePianoEditor::isFullScreen() const
{
    if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
        return window->isFullScreen();
    return ! sizeBeforeFullScreen.isEmpty();
}

void FaseGroovePianoEditor::toggleFullScreen()
{
    // The standalone app: the real full screen. In a DAW the plug-in fills the screen instead
    // (the DAW owns its window).
    if (auto* window = findParentComponentOfClass<juce::ResizableWindow>())
    {
        window->setFullScreen (! window->isFullScreen());
        return;
    }
    if (! sizeBeforeFullScreen.isEmpty())
    {
        setSize (sizeBeforeFullScreen.getWidth(), sizeBeforeFullScreen.getHeight());
        sizeBeforeFullScreen = {};
        return;
    }
    if (const auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds()))
    {
        sizeBeforeFullScreen = getBounds();
        const auto area = display->userArea.reduced (8).withTrimmedTop (30);
        const float scale = juce::jmin (area.getWidth() / (float) baseWidth, area.getHeight() / (float) baseHeight, 2.0f);
        setSize (juce::roundToInt (baseWidth * scale), juce::roundToInt (baseHeight * scale));
    }
}

void FaseGroovePianoEditor::layoutMini()
{
    if (miniKeep == &songOverlay)
        songOverlay.setBounds (0, 0, miniWidth, miniHeight);
    else
        dragTile.setBounds (juce::Rectangle<int> (0, 0, miniWidth, miniHeight).reduced (10).withTrimmedBottom (14));
}

void FaseGroovePianoEditor::layoutFull()
{
    auto r = juce::Rectangle<int> (0, 0, baseWidth, baseHeight).reduced (20, 14);

    // Header
    auto top = r.removeFromTop (76);
    chordDisplayArea = top.removeFromRight (300).reduced (0, 4);
    top.removeFromRight (10);
    tempoArea = top.removeFromRight (330).reduced (0, 4);
    headerArea = top;
    {
        // Two rows of utilities at the right of the logo, all the same height.
        auto bottomRow = juce::Rectangle<int> (headerArea.getRight() - 238, headerArea.getBottom() - 30, 234, 26);
        optionsButton.setBounds (bottomRow.removeFromRight (100));
        bottomRow.removeFromRight (4);
        resetButton.setBounds (bottomRow.removeFromRight (60));
        bottomRow.removeFromRight (4);
        styleMenuButton.setBounds (bottomRow);

        auto topRow = juce::Rectangle<int> (headerArea.getRight() - 238, headerArea.getY() + 8, 234, 26);
        fullScreenButton.setBounds (topRow.removeFromRight (30));
        topRow.removeFromRight (4);
        redoButton.setBounds (topRow.removeFromRight (30));
        undoButton.setBounds (topRow.removeFromRight (30));
        undoButton.setConnectedEdges (juce::Button::ConnectedOnRight);
        redoButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
        topRow.removeFromRight (6);
        presetsButton.setBounds (topRow);
    }
    options.setBounds (0, 0, baseWidth, baseHeight);
    updateOverlay.setBounds (0, 0, baseWidth, baseHeight);
    grooveBrowser.setBounds (0, 0, baseWidth, baseHeight);
    voicingCoach.setBounds (0, 0, baseWidth, baseHeight);
    rollEditor.setBounds (0, 0, baseWidth, baseHeight);
    songOverlay.setBounds (0, 0, baseWidth, baseHeight);
    morphOverlay.setBounds (0, 0, baseWidth, baseHeight);
    dnaOverlay.setBounds (0, 0, baseWidth, baseHeight);
    chordLab.setBounds (0, 0, baseWidth, baseHeight);
    {
        // Transport: to the start, play / pause, stop; then the tempo.
        sequencerToggle.setBounds (tempoArea.reduced (12, 6).removeFromTop (18).removeFromRight (104));
        auto controls = tempoArea.reduced (12, 8).withTrimmedTop (22);
        rewindButton.setBounds (controls.removeFromLeft (34));
        playButton.setBounds (controls.removeFromLeft (40));
        stopButton.setBounds (controls.removeFromLeft (34));
        controls.removeFromLeft (12);
        syncButton.setBounds (controls.removeFromLeft (86));
        controls.removeFromLeft (6);
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
        variantsButton.setBounds (progressionPanel.getX() + 140, progressionPanel.getY() + 6, 96, 17);
        auto presetRow = leftCol.removeFromTop (28);
        learnButton.setBounds (presetRow.removeFromRight (58));
        presetRow.removeFromRight (4);
        diceProgression.setBounds (presetRow.removeFromRight (92));
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
    circlePanel = mid.removeFromLeft (262);
    mid.removeFromLeft (10);
    soundPanel = mid.removeFromRight (252);
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
        coachButton.setBounds (voicing.getRight() - 34, voicing.getY(), 34, 15);
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
        grooveRow.removeFromRight (6);
        auto arrows = grooveRow.removeFromRight (40).withTrimmedTop (16).withSizeKeepingCentre (40, 20);
        groovePrev.setBounds (arrows.removeFromLeft (20));
        grooveNext.setBounds (arrows);
        pattern.setBounds (grooveRow.withTrimmedRight (4));
        {
            auto three = row (inner, 44);
            const int w = (three.getWidth() - 16) / 3;
            section.setBounds (three.removeFromLeft (w));
            three.removeFromLeft (8);
            grooveBars.setBounds (three.removeFromLeft (w));
            three.removeFromLeft (8);
            variation.setBounds (three);
        }
        split (row (inner, 44), fills, fillStyle, (inner.getWidth() - 10) * 2 / 5);

        // FEEL: its own small header (style menu and NEW FEEL), then the five knobs.
        feelHeader = inner.removeFromTop (20);
        {
            auto h = feelHeader;
            newFeelButton.setBounds (h.removeFromRight (78));
            h.removeFromRight (6);
            feelStyleButton.setBounds (h.removeFromRight (130));
        }
        inner.removeFromTop (4);
        const int w = inner.getWidth() / 5;
        auto knobs = inner.withSizeKeepingCentre (inner.getWidth(), juce::jmin (inner.getHeight(), w + 40));
        for (auto* k : { &swing, &gate, &velocity, &humanize, &riffDensity })
            k->setBounds (knobs.removeFromLeft (w));
    }

    {
        auto inner = soundPanel.reduced (14, 8);
        inner.removeFromTop (22);
        // The on/off switch next to the sound menu, then MIDI out, the knobs and the sidechain.
        auto soundRow = row (inner, 44);
        soundToggle.setBounds (soundRow.removeFromLeft (118).withTrimmedTop (16));
        soundType.setBounds (soundRow.withTrimmedLeft (8));
        split (row (inner, 44), bassChannel, playMode, (inner.getWidth() - 10) / 2);
        const int w = inner.getWidth() / 3;
        auto top = inner.removeFromTop ((inner.getHeight() - 52) / 2);
        for (auto* k : { &tone, &reverb, &volume })
            k->setBounds (top.removeFromLeft (w));
        split (row (inner, 44), pumpSource, pumpRate, (inner.getWidth() - 10) * 3 / 5);
        sidechainMeter.setBounds (pumpSource.getRight() - 56, pumpSource.getY() + 2, 56, 10);
        for (auto* k : { &chorus, &pump, &pumpRelease })
            k->setBounds (inner.removeFromLeft (w));
    }

    r.removeFromTop (10);
    clipPanel = r.removeFromRight (300);
    r.removeFromRight (10);
    gridPanel = r;
    {
        auto inner = gridPanel.reduced (12, 8);
        {
            // Left: what you look at (piano roll / grid) and what you hear; right: groove tools.
            // Groups of buttons with a caption above each: view | hear | new rhythm | groove tools.
            auto captions = inner.removeFromTop (13);
            auto header = inner.removeFromTop (24);
            gridCaptions.clear();
            auto group = [&] (const juce::String& caption, std::initializer_list<std::pair<juce::Component*, int>> items)
            {
                const int x = header.getX();
                for (const auto& [c, w] : items)
                    c->setBounds (header.removeFromLeft (w));
                gridCaptions.push_back ({ juce::Rectangle<int> (x, captions.getY(), header.getX() - x, captions.getHeight()), caption });
                header.removeFromLeft (12);
            };
            group (tr ("View"), { { &rollTab, 78 }, { &gridTab, 64 }, { &expandRollButton, 96 } });
            group (tr ("Hear"), { { &hearPianoButton, 82 }, { &hearBassButton, 82 } });
            group (onVelocityPage() ? tr ("New velocity") : tr ("New rhythm"), { { &rndPianoButton, 76 }, { &rndBassButton, 76 } });
            group (tr ("Groove"), { { &myGroovesButton, 92 }, { &morphButton, 60 }, { &dnaButton, header.getWidth() - 152 } });
            velocityTab.setBounds (expandRollButton.getBounds());
            velocityTab.setConnectedEdges (juce::Button::ConnectedOnLeft);
            expandRollButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
            gridTab.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
            hearPianoButton.setConnectedEdges (juce::Button::ConnectedOnRight);
            hearBassButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
            rndPianoButton.setConnectedEdges (juce::Button::ConnectedOnRight);
            rndBassButton.setConnectedEdges (juce::Button::ConnectedOnLeft);
        }
        inner.removeFromTop (6);

        pianoRoll.setBounds (inner);
        generatedButton.setBounds (inner.getRight() - 300, inner.getY() + 24, 160, 18);

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
            velocityBoxes[i]->setBounds (segmentBoxes[i]->getBounds());
            velocityDice[i]->setBounds (segmentDice[i]->getBounds());
            velocityLocks[i]->setBounds (segmentLocks[i]->getBounds());
        }

        inner.removeFromTop (2);
        patternView.setBounds (inner);
        velocityView.setBounds (inner);
    }

    {
        // EXPORT MIDI: what and how long, the big drag tile, then save / the whole song.
        auto inner = clipPanel.reduced (14, 8);
        inner.removeFromTop (24);
        auto labels = inner.removeFromTop (16);
        clipPartLabel.setBounds (labels.removeFromLeft (labels.getWidth() / 2));
        clipLengthLabel.setBounds (labels.withTrimmedLeft (8));
        auto boxes = inner.removeFromTop (28);
        clipPart.setBounds (boxes.removeFromLeft (boxes.getWidth() / 2 - 4));
        clipLength.setBounds (boxes.withTrimmedLeft (8));
        inner.removeFromTop (10);
        inner.removeFromBottom (18);   // hint
        auto buttons = inner.removeFromBottom (30);
        saveButton.setBounds (buttons.removeFromLeft (buttons.getWidth() / 2 - 4));
        songButton.setBounds (buttons.withTrimmedLeft (8));
        inner.removeFromBottom (8);
        dragTile.setBounds (inner);
    }
}
