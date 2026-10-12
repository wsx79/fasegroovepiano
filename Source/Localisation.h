// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// User-interface translations. The English text is the key: tr ("PREVIEW")
// returns the text in the current language, or the English text itself when no
// translation exists (musical terms like "Dorian" stay as they are).
// The chosen language is a user preference shared by every project.

#include <juce_data_structures/juce_data_structures.h>

namespace Localisation
{
    enum class Language { english = 0, italian, spanish, french, german, portuguese };
    constexpr int numLanguages = 6;

    /** Native name of a language, e.g. "Italiano". */
    const char* languageName (int language) noexcept;

    Language current() noexcept;

    /** Changes the language and saves it for next time. */
    void setCurrent (Language);

    /** Loads the saved language, or picks the computer's language the first time. */
    void loadSaved();

    /** Upper case that also handles accented letters (the C library may not). */
    juce::String upper (const juce::String&);

    /** Translates an English UI text into the current language. */
    juce::String tr (const juce::String& english);
}

using Localisation::tr;
