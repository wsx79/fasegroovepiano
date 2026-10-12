#include "CircleOfFifths.h"
#include "PluginEditor.h"

namespace
{
    // Clockwise from the top, one fifth apart.
    const char* const majorNames[12] { "C", "G", "D", "A", "E", "B", "F#", "Db", "Ab", "Eb", "Bb", "F" };
    const char* const minorNames[12] { "Am", "Em", "Bm", "F#m", "C#m", "G#m", "D#m", "Bbm", "Fm", "Cm", "Gm", "Dm" };

    constexpr float segment = juce::MathConstants<float>::twoPi / 12.0f;

    int mod12 (int v) { return ((v % 12) + 12) % 12; }

    /** Position on the circle of a major key (or of the relative major of a minor one). */
    int indexOfMajor (int pc) { return mod12 (pc * 7); }          // 7 is its own inverse mod 12
    int indexOfMinor (int pc) { return mod12 ((pc - 9) * 7); }
    int majorAt (int index)   { return mod12 (index * 7); }
    int minorAt (int index)   { return mod12 (index * 7 + 9); }

    float angleOf (int index) { return -juce::MathConstants<float>::halfPi + index * segment; }

    juce::Font font (float h, bool bold)
    {
        return juce::Font (juce::FontOptions (h).withStyle (bold ? "Bold" : "Regular"));
    }
}

void CircleOfFifths::setState (int keyRoot, bool minorKey, const std::vector<ChordMark>& progression, int playingIndex)
{
    bool same = keyRoot == key && minorKey == minor && playingIndex == playing && progression.size() == chords.size();
    for (size_t i = 0; same && i < chords.size(); ++i)
        same = chords[i].rootPc == progression[i].rootPc && chords[i].minor == progression[i].minor;

    if (same)
        return;

    key = keyRoot;
    minor = minorKey;
    chords = progression;
    playing = playingIndex;
    repaint();
}

float CircleOfFifths::outerRadius() const
{
    return juce::jmin (getWidth(), getHeight()) * 0.5f - 3.0f;
}

juce::Point<float> CircleOfFifths::segmentCentre (int index, bool inner) const
{
    const float r = inner ? (innerRadius() + ringSplit()) * 0.5f : (ringSplit() + outerRadius()) * 0.5f;
    return getLocalBounds().toFloat().getCentre().getPointOnCircumference (r, angleOf (index) + juce::MathConstants<float>::halfPi);
}

CircleOfFifths::Hit CircleOfFifths::segmentAt (juce::Point<float> p) const
{
    const auto c = getLocalBounds().toFloat().getCentre();
    const float d = p.getDistanceFrom (c);

    if (d < innerRadius() || d > outerRadius())
        return {};

    const float angle = std::atan2 (p.y - c.y, p.x - c.x) + juce::MathConstants<float>::halfPi;
    return { mod12 (juce::roundToInt (angle / segment)), d < ringSplit() };
}

void CircleOfFifths::mouseDown (const juce::MouseEvent& e)
{
    const auto h = segmentAt (e.position);
    if (h.index >= 0 && onPick != nullptr)
        onPick (h.inner ? minorAt (h.index) : majorAt (h.index), h.inner);
}

void CircleOfFifths::mouseMove (const juce::MouseEvent& e)
{
    const auto h = segmentAt (e.position);
    if (h.index != hover.index || h.inner != hover.inner)
    {
        hover = h;
        repaint();
    }
}

void CircleOfFifths::mouseExit (const juce::MouseEvent&)
{
    hover = {};
    repaint();
}

void CircleOfFifths::paint (juce::Graphics& g)
{
    const auto c = getLocalBounds().toFloat().getCentre();
    const float rOut = outerRadius(), rMid = ringSplit(), rIn = innerRadius();

    const int keyIndex = minor ? indexOfMinor (key) : indexOfMajor (key);

    for (int i = 0; i < 12; ++i)
    {
        const float a0 = angleOf (i) - segment * 0.5f + juce::MathConstants<float>::halfPi;
        const float a1 = a0 + segment;
        const int distance = juce::jmin (mod12 (i - keyIndex), mod12 (keyIndex - i));
        const bool family = distance <= 1;   // I, IV, V and their relative minors

        for (const bool inner : { false, true })
        {
            juce::Path seg;
            seg.addPieSegment (c.x - (inner ? rMid : rOut), c.y - (inner ? rMid : rOut),
                               (inner ? rMid : rOut) * 2.0f, (inner ? rMid : rOut) * 2.0f,
                               a0, a1, (inner ? rIn : rMid) / (inner ? rMid : rOut));

            const bool isKey = i == keyIndex && inner == minor;
            juce::Colour fill = Palette::panel.brighter (inner ? 0.05f : 0.12f);
            if (family)                                      fill = Palette::panel.interpolatedWith (Palette::accent, 0.16f);
            if (isKey)                                       fill = Palette::accent;
            if (hover.index == i && hover.inner == inner)    fill = fill.brighter (0.25f);

            g.setColour (fill);
            g.fillPath (seg);
            g.setColour (Palette::background);
            g.strokePath (seg, juce::PathStrokeType (1.5f));

            g.setColour (isKey ? Palette::background : (family ? Palette::text : Palette::dim));
            g.setFont (font (inner ? 10.5f : 13.0f, isKey || family));
            const auto p = segmentCentre (i, inner);
            g.drawText (inner ? minorNames[i] : majorNames[i],
                        juce::Rectangle<float> (40.0f, 16.0f).withCentre (p), juce::Justification::centred);
        }
    }

    // The progression's path around the circle.
    if (! chords.empty())
    {
        std::vector<juce::Point<float>> points;
        for (const auto& ch : chords)
        {
            const auto p = segmentCentre (ch.minor ? indexOfMinor (ch.rootPc) : indexOfMajor (ch.rootPc), ch.minor);
            points.push_back (c + (p - c) * 0.82f);
        }

        juce::Path path;
        path.startNewSubPath (points.front());
        for (size_t i = 1; i < points.size(); ++i)
            path.lineTo (points[i]);
        if (points.size() > 2)
            path.closeSubPath();

        g.setColour (Palette::accent2.withAlpha (0.7f));
        g.strokePath (path, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        for (size_t i = 0; i < points.size(); ++i)
        {
            const bool now = (int) i == playing;
            const float d = now ? 11.0f : 7.0f;
            g.setColour (now ? Palette::text : Palette::accent2);
            g.fillEllipse (juce::Rectangle<float> (d, d).withCentre (points[i]));
        }
    }

    // Centre label
    g.setColour (Palette::text);
    g.setFont (font (12.0f, true));
    g.drawText (minor ? juce::String (minorNames[indexOfMinor (key)]) : juce::String (majorNames[indexOfMajor (key)]),
                juce::Rectangle<float> (rIn * 2.0f, 16.0f).withCentre (c.translated (0.0f, -6.0f)), juce::Justification::centred);
    g.setColour (Palette::dim);
    g.setFont (font (9.5f, false));
    g.drawText ("5ths", juce::Rectangle<float> (rIn * 2.0f, 12.0f).withCentre (c.translated (0.0f, 9.0f)), juce::Justification::centred);
}
