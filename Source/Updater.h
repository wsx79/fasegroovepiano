// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

// Automatic updates from the GitHub releases page.
// A background thread asks GitHub for the latest release; if it is newer than this build it
// downloads the installer (Windows) or the zip (macOS / Linux) into the Downloads folder and
// reports back on the message thread, with the release notes, so the user can confirm.
// It can be turned off in OPTIONS -> SETTINGS; nothing else is ever sent or received.

#include <juce_events/juce_events.h>

namespace Updater
{
    struct Result
    {
        enum class Status { upToDate, failed, ready };
        Status status = Status::failed;
        juce::String version;   // e.g. "1.15.0"
        juce::String notes;     // what's new, in the UI language when available
        juce::File file;        // the downloaded installer or zip
    };

    /** Automatic check when the plug-in opens (on by default). */
    bool autoCheckEnabled();
    void setAutoCheckEnabled (bool);

    /** "Skip this version": that version is no longer offered automatically. */
    void skipVersion (const juce::String& version);

    /** True when version a (e.g. "v1.15.0") is newer than b (e.g. "1.14.0"). */
    bool isNewer (const juce::String& a, const juce::String& b);

    /** The "## [x.y.z]" section of a changelog, as plain text. */
    juce::String changelogSection (const juce::String& changelog, const juce::String& version);

    /** Opens the downloaded file: runs the Windows installer, opens the zip on macOS, shows it on Linux. */
    void install (const juce::File&);

    class Checker : private juce::Thread
    {
    public:
        Checker();
        ~Checker() override;

        /** Starts a check. manual = true also reports "up to date" / "failed" and ignores skipped versions. */
        void start (bool manual);
        bool isBusy() const { return isThreadRunning(); }

        /** Called on the message thread when a check ends. */
        std::function<void (const Result&, bool manual)> onResult;

    private:
        void run() override;
        bool manualCheck = false;

        JUCE_DECLARE_WEAK_REFERENCEABLE (Checker)
    };

    /** True only the first time it is called in this process: one automatic check per session. */
    bool firstAutoCheckThisSession();
}
