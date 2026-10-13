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

    const char* assetName (InstallKind kind)
    {
       #if JUCE_WINDOWS
        return kind == InstallKind::installer ? "FaseGroovePiano-Windows-Setup.exe" : "FaseGroovePiano-Windows.zip";
       #elif JUCE_MAC
        juce::ignoreUnused (kind);
        return "FaseGroovePiano-macOS.zip";
       #else
        juce::ignoreUnused (kind);
        return "FaseGroovePiano-Linux.zip";
       #endif
    }

    const juce::String productName ("Fase Groove Piano");

    /** The plug-in bundle (or app) this code is running from. */
    juce::File runningBundle()
    {
        auto module = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
        juce::File bundle = module;
        for (auto f = module.getParentDirectory(); f != f.getParentDirectory(); f = f.getParentDirectory())
            if (f.hasFileExtension ("vst3;component;app"))
                bundle = f;   // the outermost one
        return bundle;
    }

    /** The same product in the unpacked zip (VST3/, AU/, Standalone/). */
    juce::File stagedCounterpart (const juce::File& staged, const juce::File& target)
    {
        if (target.hasFileExtension ("vst3"))      return staged.getChildFile ("VST3").getChildFile (target.getFileName());
        if (target.hasFileExtension ("component")) return staged.getChildFile ("AU").getChildFile (target.getFileName());
        return staged.getChildFile ("Standalone").getChildFile (target.getFileName());
    }

   #if JUCE_MAC
    bool runTool (const juce::StringArray& args)
    {
        juce::ChildProcess p;
        return p.start (args, 0) && p.waitForProcessToFinish (120000) && p.getExitCode() == 0;
    }
   #endif

    bool unpack (const juce::File& zip, const juce::File& folder)
    {
        folder.deleteRecursively();
        if (! folder.createDirectory())
            return false;
       #if JUCE_MAC
        return runTool ({ "/usr/bin/ditto", "-x", "-k", zip.getFullPathName(), folder.getFullPathName() });   // keeps bundles intact
       #else
        juce::ZipFile archive (zip);
        if (archive.getNumEntries() == 0 || archive.uncompressTo (folder).failed())
            return false;
        #if JUCE_LINUX || JUCE_BSD
        for (const auto& f : folder.findChildFiles (juce::File::findFiles, true))   // zip entries lose the execute bit
            if (f.getFileExtension().isEmpty() || f.hasFileExtension ("so"))
                f.setExecutePermission (true);
        #endif
        return true;
       #endif
    }

    /** Every copy of the plug-in we can update: the running one, its neighbours from the same zip,
        and the usual plug-in folders. */
    juce::Array<juce::File> updateTargets (const juce::File& staged)
    {
        juce::Array<juce::File> candidates;
        const auto running = runningBundle();
        candidates.add (running);

        const auto zipRoot = running.getParentDirectory().getParentDirectory();
        candidates.add (zipRoot.getChildFile ("VST3").getChildFile (productName + ".vst3"));
        candidates.add (zipRoot.getChildFile ("AU").getChildFile (productName + ".component"));
       #if JUCE_WINDOWS
        candidates.add (zipRoot.getChildFile ("Standalone").getChildFile (productName + ".exe"));
        candidates.add (running.getSiblingFile (productName + ".exe"));
        candidates.add (running.getSiblingFile (productName + ".vst3"));
       #elif JUCE_MAC
        candidates.add (zipRoot.getChildFile ("Standalone").getChildFile (productName + ".app"));
        const auto home = juce::File::getSpecialLocation (juce::File::userHomeDirectory);
        for (const auto& base : { home, juce::File ("/") })
        {
            candidates.add (base.getChildFile ("Library/Audio/Plug-Ins/VST3").getChildFile (productName + ".vst3"));
            candidates.add (base.getChildFile ("Library/Audio/Plug-Ins/Components").getChildFile (productName + ".component"));
        }
        candidates.add (juce::File ("/Applications").getChildFile (productName + ".app"));
        candidates.add (home.getChildFile ("Applications").getChildFile (productName + ".app"));
       #else
        candidates.add (zipRoot.getChildFile ("Standalone").getChildFile (productName));
        candidates.add (juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (".vst3").getChildFile (productName + ".vst3"));
       #endif

        juce::Array<juce::File> targets;
        for (const auto& f : candidates)
            if (f.exists() && f.getParentDirectory().hasWriteAccess() && stagedCounterpart (staged, f).exists()
                && ! targets.contains (f))
                targets.add (f);
        return targets;
    }

    /** Puts the new copy in place of the old one. */
    bool swapIn (const juce::File& fresh, const juce::File& target)
    {
       #if JUCE_WINDOWS
        // A loaded DLL (or the running .exe) can't be overwritten or its folder renamed, but the
        // file itself can be renamed: move each old file aside, copy the new one in.
        juce::Array<juce::File> files;
        if (fresh.isDirectory())
            files = fresh.findChildFiles (juce::File::findFiles, true);
        else
            files.add (fresh);

        for (const auto& f : files)
        {
            const auto dest = fresh.isDirectory() ? target.getChildFile (f.getRelativePathFrom (fresh)) : target;
            dest.getParentDirectory().createDirectory();
            if (dest.exists())
            {
                const auto old = dest.getSiblingFile (dest.getFileName() + ".old");
                old.deleteFile();
                if (old.exists() || ! dest.moveFileTo (old))   // still loaded from the last update
                    return false;
            }
            if (! f.copyFileTo (dest))
                return false;
        }
        return true;
       #else
        // macOS / Linux: build the new copy next to the old one, then swap them by renaming
        // (the running code keeps working from the old files until the DAW restarts).
        const auto incoming = target.getSiblingFile (target.getFileName() + ".new");
        const auto old = target.getSiblingFile (target.getFileName() + ".old");
        incoming.deleteRecursively();
        old.deleteRecursively();
        #if JUCE_MAC
        if (! runTool ({ "/usr/bin/ditto", fresh.getFullPathName(), incoming.getFullPathName() }))
            return false;
        runTool ({ "/usr/bin/xattr", "-dr", "com.apple.quarantine", incoming.getFullPathName() });
        #else
        if (! (fresh.isDirectory() ? fresh.copyDirectoryTo (incoming) : fresh.copyFileTo (incoming)))
            return false;
        for (const auto& f : incoming.isDirectory() ? incoming.findChildFiles (juce::File::findFiles, true) : juce::Array<juce::File> { incoming })
            if (f.getFileExtension().isEmpty() || f.hasFileExtension ("so") || f == incoming)
                f.setExecutePermission (true);
        #endif
        if (! target.moveFileTo (old))
        {
            incoming.deleteRecursively();
            return false;
        }
        if (! incoming.moveFileTo (target))
        {
            old.moveFileTo (target);   // put the old one back
            return false;
        }
        old.deleteRecursively();
        return true;
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

InstallKind installKind()
{
    const auto bundle = runningBundle();
   #if JUCE_WINDOWS
    // The setup installs into Program Files (VST3 in Common Files); anything else was copied by hand.
    for (auto location : { juce::File::globalApplicationsDirectory, juce::File::globalApplicationsDirectoryX86 })
        if (bundle.isAChildOf (juce::File::getSpecialLocation (location)))
            return InstallKind::installer;
   #endif
    return bundle.exists() && bundle.getParentDirectory().hasWriteAccess() ? InstallKind::portable : InstallKind::manual;
}

InstallReport install (const Result& result)
{
    InstallReport report;

    if (result.kind == InstallKind::installer && result.file.existsAsFile())
    {
        // Same setup, no wizard: it keeps the folders and parts you chose last time and asks to
        // close the DAW only if the plug-in is in use.
        report.installerStarted = report.done = result.file.startAsProcess ("/SILENT /NORESTART");
        if (report.done)
            return report;
    }

    if (result.kind == InstallKind::portable && result.staged.isDirectory())
    {
        bool allOk = true;
        for (const auto& target : updateTargets (result.staged))
        {
            if (swapIn (stagedCounterpart (result.staged, target), target))
                report.updated.add (target.getFullPathName());
            else
                allOk = false;
        }
        report.done = allOk && ! report.updated.isEmpty();
        if (report.done)
            return report;
    }

    // Fallback: show the download so it can be installed by hand.
   #if JUCE_MAC
    result.file.startAsProcess();   // Archive Utility unzips it next to the zip
    result.file.getParentDirectory().revealToUser();
   #elif JUCE_WINDOWS
    if (result.file.hasFileExtension ("exe"))
        report.installerStarted = result.file.startAsProcess();
    else
        result.file.revealToUser();
   #else
    result.file.revealToUser();
   #endif
    return report;
}

void cleanupAfterUpdate()
{
    const auto bundle = runningBundle();
    juce::Array<juce::File> leftovers;
    if (bundle.isDirectory())
        leftovers = bundle.findChildFiles (juce::File::findFiles, true, "*.old");
    leftovers.add (bundle.getSiblingFile (bundle.getFileName() + ".old"));
    for (const auto& f : leftovers)
        if (f.existsAsFile())
            f.deleteFile();   // fails quietly while something still has it loaded
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
        result.kind = installKind();
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
                if (asset.getProperty ("name", {}).toString() == assetName (result.kind))
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
        auto file = folder.getChildFile (juce::File::createLegalFileName (juce::String (assetName (result.kind))
                                             .replace ("FaseGroovePiano-", "FaseGroovePiano-" + result.version + "-")));
        if (! file.existsAsFile() || file.getSize() == 0)
            if (! fetchFile (*this, assetUrl, file))
                return;

        result.file = file;

        // A portable copy: unpack now, so INSTALL NOW only has to swap the files.
        if (result.kind == InstallKind::portable)
        {
            const auto staged = folder.getChildFile (result.version);
            if (unpack (file, staged))
                result.staged = staged;
            else
                result.kind = InstallKind::manual;
        }
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
