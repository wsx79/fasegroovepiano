// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// Interactive circle of fifths: outer ring = major keys, inner ring = relative
// minors. Shows the current key, its diatonic neighbours and the path the
// progression draws around the circle. Click a key to select it.

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <vector>

class CircleOfFifths : public juce::Component
{
public:
    struct ChordMark
    {
        int rootPc;
        bool minor;
    };

    /** Called with the clicked root and whether the inner (minor) ring was clicked. */
    std::function<void (int pitchClass, bool minor)> onPick;

    void setState (int keyRoot, bool minorKey, const std::vector<ChordMark>& progression, int playingIndex);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    struct Hit { int index = -1; bool inner = false; };

    Hit segmentAt (juce::Point<float>) const;
    juce::Point<float> segmentCentre (int index, bool inner) const;

    float outerRadius() const;
    float ringSplit() const { return outerRadius() * 0.68f; }
    float innerRadius() const { return outerRadius() * 0.38f; }

    int key = 0;
    bool minor = false;
    std::vector<ChordMark> chords;
    int playing = -1;
    Hit hover;
};
