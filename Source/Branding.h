#pragma once

// App identity: the Fase Groove Piano logo (icon + wordmark) and the credits
// artwork. Shared by the editor, the About page and the icon/README renders.

#include <juce_gui_basics/juce_gui_basics.h>

namespace Branding
{
    inline constexpr const char* appName   = "Fase Groove Piano";
    inline constexpr const char* tagline   = "funky piano grooves for soulful & deep house";
    inline constexpr const char* artist    = "Fabio 'Pitch' Serra";

    /** The square app icon (slanted groove keys). */
    std::unique_ptr<juce::Drawable> createIcon();

    /** Fabio 'Pitch' Serra's FS PITCH logo (white on transparent). */
    juce::Image artistLogo();

    /** "FASE" + "GROOVE PIANO", slanted like the FS logo, fitted inside `area`. */
    void drawWordmark (juce::Graphics&, juce::Rectangle<float> area);

    /** Icon on the left, wordmark on the right. */
    void drawFullLogo (juce::Graphics&, juce::Rectangle<float> area);
}
