// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Updater.h"
#include "Localisation.h"


namespace Updater
{
namespace
{
    constexpr const char* latestReleaseApi = "https://api.github.com/repos/wsx79/fasegroovepiano/releases/latest";
    constexpr const char* rawBase = "https://raw.githubusercontent.com/wsx79/fasegroovepiano/";

    const char* assetName()
    {
       #if JUCE_WINDOWS
        return "FaseGroovePiano-Windows-Setup.exe";
       #elif JUCE_MAC
        return "FaseGroovePiano-macOS.zip";
       #else
        return "FaseGroovePiano-Linux.zip";
       #endif
    }

   #if JUCE_LINUX || JUCE_BSD
    // The plug-in is built without libcurl (no extra dependency), so on Linux the system's
    // curl command does the HTTPS work.
    bool curl (juce::Thread& thread, juce::StringArray args, juce::String* output)
    {
        args.insert (0, "curl");
        args.insert (1, "-sSfL");
        args.insert (2, "--max-time");
        args.insert (3, "600");
        args.insert (4, "-A");
        args.insert (5, "FaseGroovePiano-Updater");

        juce::ChildProcess process;
        if (! process.start (args, output != nullptr ? juce::ChildProcess::wantStdOut : 0))
            return false;

        juce::MemoryOutputStream out;
        while (process.isRunning())
        {
            if (thread.threadShouldExit())
            {
                process.kill();
                return false;
            }
            if (output != nullptr)
            {
                char buffer[4096];
                const int n = process.readProcessOutput (buffer, (int) sizeof (buffer));
                if (n > 0) out.write (buffer, (size_t) n);
                else juce::Thread::sleep (20);
            }
            else
            {
                juce::Thread::sleep (50);
            }
        }

        if (output != nullptr)
        {
            char buffer[4096];
            for (int n; (n = process.readProcessOutput (buffer, (int) sizeof (buffer))) > 0;)
                out.write (buffer, (size_t) n);
            *output = out.toUTF8();
        }
        return process.getExitCode() == 0;
    }
   #endif

   #if ! (JUCE_LINUX || JUCE_BSD)
    std::unique_ptr<juce::InputStream> open (const juce::String& url)
    {
        return juce::URL (url).createInputStream (juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                                                      .withConnectionTimeoutMs (15000)
                                                      .withExtraHeaders ("User-Agent: FaseGroovePiano-Updater\r\nAccept: application/vnd.github+json"));
    }
   #endif

    bool fetchText (juce::Thread& thread, const juce::String& url, juce::String& text)
    {
       #if JUCE_LINUX || JUCE_BSD
        return curl (thread, { url }, &text) && text.isNotEmpty();
       #else
        juce::ignoreUnused (thread);
        auto stream = open (url);
        if (stream == nullptr)
            return false;
        text = stream->readEntireStreamAsString();
        return text.isNotEmpty();
       #endif
    }

    bool fetchFile (juce::Thread& thread, const juce::String& url, const juce::File& target)
    {
        auto partial = target.getSiblingFile (target.getFileName() + ".part");
        partial.deleteFile();

       #if JUCE_LINUX || JUCE_BSD
        if (! curl (thread, { "-o", partial.getFullPathName(), url }, nullptr))
        {
            partial.deleteFile();
            return false;
        }
       #else
        {
            auto stream = open (url);
            if (stream == nullptr)
                return false;

            juce::FileOutputStream out (partial);
            if (! out.openedOk())
                return false;

            juce::HeapBlock<char> buffer (65536);
            for (;;)
            {
                if (thread.threadShouldExit())
                {
                    out.flush();
                    partial.deleteFile();
                    return false;
                }
                const int n = stream->read (buffer, 65536);
                if (n <= 0)
                    break;
                out.write (buffer, (size_t) n);
            }
            out.flush();
            if (out.getStatus().failed() || ! stream->isExhausted())
            {
                partial.deleteFile();
                return false;
            }
        }
       #endif

        target.deleteFile();
        return partial.getSize() > 0 && partial.moveFileTo (target);
    }

    juce::File downloadsFolder()
    {
        auto downloads = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile ("Downloads");
        if (! downloads.isDirectory())
            downloads = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
        return downloads.getChildFile ("Fase Groove Piano updates");
    }

    /** Markdown -> plain text that reads well in a text box. */
    juce::String plain (juce::String text)
    {
        text = text.replace ("\r\n", "\n").replace ("**", "").replace ("*", "").replace ("`", "");
        juce::StringArray raw, lines;
        raw.addLines (text);
        for (auto& line : raw)
        {
            // Changelog lines are wrapped at ~100 columns: join them back (the text box wraps).
            if (line.startsWith ("  ") && line.trim().isNotEmpty() && ! lines.isEmpty() && lines[lines.size() - 1].isNotEmpty())
                lines.getReference (lines.size() - 1) << " " << line.trim();
            else
                lines.add (line);
        }
        for (auto& line : lines)
        {
            if (line.startsWith ("### ")) line = Localisation::upper (line.substring (4));
            else if (line.startsWith ("## ")) line = line.substring (3);
            else if (line.startsWith ("- ")) line = juce::String::fromUTF8 ("\xe2\x80\xa2 ") + line.substring (2);
        }
        return lines.joinIntoString ("\n").trim();
    }
}

bool autoCheckEnabled()
{
    return Localisation::setting ("autoUpdate", "1") != "0";
}

void setAutoCheckEnabled (bool enabled)
{
    Localisation::setSetting ("autoUpdate", enabled ? "1" : "0");
}

void skipVersion (const juce::String& version)
{
    Localisation::setSetting ("skippedVersion", version);
}

bool isNewer (const juce::String& a, const juce::String& b)
{
    auto parts = [] (juce::String v)
    {
        v = v.trim().trimCharactersAtStart ("vV");
        juce::StringArray tokens;
        tokens.addTokens (v.upToFirstOccurrenceOf ("-", false, false), ".", {});
        std::array<int, 3> n {};
        for (int i = 0; i < 3 && i < tokens.size(); ++i)
            n[(size_t) i] = tokens[i].getIntValue();
        return n;
    };
    return parts (a) > parts (b);
}

juce::String changelogSection (const juce::String& changelog, const juce::String& version)
{
    const auto v = version.trimCharactersAtStart ("vV");
    juce::StringArray lines;
    lines.addLines (changelog);

    juce::StringArray section;
    bool inside = false;
    for (auto& line : lines)
    {
        if (line.startsWith ("## "))
        {
            if (inside)
                break;
            inside = line.startsWith ("## [" + v + "]");
            continue;
        }
        if (inside)
            section.add (line);
    }
    return plain (section.joinIntoString ("\n"));
}

void install (const juce::File& file)
{
   #if JUCE_LINUX || JUCE_BSD
    file.revealToUser();
   #elif JUCE_MAC
    file.startAsProcess();   // Archive Utility unzips it next to the zip
    file.getParentDirectory().revealToUser();
   #else
    file.startAsProcess();   // the installer
   #endif
}

bool firstAutoCheckThisSession()
{
    static std::atomic<bool> checked { false };
    return ! checked.exchange (true);
}

//==============================================================================
Checker::Checker() : juce::Thread ("Fase Groove Piano update check") {}

Checker::~Checker()
{
    stopThread (4000);
}

void Checker::start (bool manual)
{
    if (isThreadRunning())
        return;
    manualCheck = manual;
    startThread (juce::Thread::Priority::low);
}

void Checker::run()
{
    Result result;
    const bool manual = manualCheck;

    [&]
    {
        juce::String json;
        if (! fetchText (*this, latestReleaseApi, json))
            return;

        const auto release = juce::JSON::parse (json);
        const auto tag = release.getProperty ("tag_name", {}).toString();
        if (tag.isEmpty())
            return;

        result.version = tag.trimCharactersAtStart ("vV");
        if (! isNewer (tag, JucePlugin_VersionString))
        {
            result.status = Result::Status::upToDate;
            return;
        }
        if (! manual && Localisation::setting ("skippedVersion") == result.version)
        {
            result.status = Result::Status::upToDate;
            return;
        }

        juce::String assetUrl;
        if (auto* assets = release.getProperty ("assets", {}).getArray())
            for (auto& asset : *assets)
                if (asset.getProperty ("name", {}).toString() == assetName())
                    assetUrl = asset.getProperty ("browser_download_url", {}).toString();
        if (assetUrl.isEmpty())
            return;

        // What's new: the changelog in the UI language when there is one, else the release notes.
        if (Localisation::current() == Localisation::Language::italian)
        {
            juce::String changelog;
            if (fetchText (*this, rawBase + tag + "/CHANGELOG.it.md", changelog))
                result.notes = changelogSection (changelog, tag);
        }
        if (result.notes.isEmpty())
            result.notes = plain (release.getProperty ("body", {}).toString());

        const auto folder = downloadsFolder();
        folder.createDirectory();
        auto file = folder.getChildFile (juce::File::createLegalFileName (juce::String (assetName())
                                             .replace ("FaseGroovePiano-", "FaseGroovePiano-" + result.version + "-")));
        if (! file.existsAsFile() || file.getSize() == 0)
            if (! fetchFile (*this, assetUrl, file))
                return;

        result.file = file;
        result.status = Result::Status::ready;
    }();

    if (threadShouldExit())
        return;

    juce::WeakReference<Checker> self (this);
    juce::MessageManager::callAsync ([self, result, manual]
    {
        if (self != nullptr && self->onResult != nullptr)
            self->onResult (result, manual);
    });
}
}
