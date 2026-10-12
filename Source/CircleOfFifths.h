// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// Interactive circle of fifths: outer ring = major keys, inner ring = relative
// minors. Shows the current key, its diatonic neighbours and the path the
// progression draws around the circle. Click a key to select it; drag a dot
// of the progression onto another chord of the scale to change that chord.

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

    /** Called when the dot of progression slot `slot` is dropped: on the chord of scale degree
        `degree` (>= 0), or, in free mode, on any other chord: then degree is -1 and
        `rootPc` / `minor` describe it. */
    std::function<void (int slot, int degree, int rootPc, bool minor)> onMoveChord;

    /** Free mode: every chord of the circle is a drop target, even outside the scale. */
    void setFreeMode (bool shouldBeFree)   { freeMode = shouldBeFree; repaint(); }

    /** `diatonic` holds the chord on each scale degree (0..6), used as drop targets. */
    void setState (int keyRoot, bool minorKey, const std::vector<ChordMark>& progression, int playingIndex,
                   const std::vector<ChordMark>& diatonic);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    struct Hit { int index = -1; bool inner = false; };

    Hit segmentAt (juce::Point<float>) const;
    juce::Point<float> segmentCentre (int index, bool inner) const;

    juce::Point<float> dotPosition (int slot) const;
    int dotAt (juce::Point<float>) const;
    /** Scale degree whose chord sits on that segment (same ring first, then same root), or -1. */
    int degreeAt (Hit) const;

    float outerRadius() const;
    float ringSplit() const { return outerRadius() * 0.68f; }
    float innerRadius() const { return outerRadius() * 0.38f; }

    int key = 0;
    bool minor = false;
    std::vector<ChordMark> chords, scaleChords;
    int dragging = -1;
    bool freeMode = false;
    juce::Point<float> dragPos;
    int playing = -1;
    Hit hover;
};
