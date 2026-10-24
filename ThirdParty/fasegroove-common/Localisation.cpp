// Fase Groove family - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "Localisation.h"
#include "ProductInfo.h"

#include <atomic>
#include <unordered_map>

namespace Localisation
{
namespace
{
    // Texts translated the same way in every product; each product adds its own table.
    // clang-format off
    const Entry commonTable[] {
#include "CommonTranslations.inc"
    };
    // clang-format on

    std::atomic<int> currentLanguage { 0 };

    const std::unordered_map<std::string, const Entry*>& index()
    {
        static const auto map = []
        {
            std::unordered_map<std::string, const Entry*> m;
            for (const auto& e : commonTable)
                m[e.en] = &e;
            const auto own = productTranslations();
            for (std::size_t i = 0; i < own.size; ++i)
                m[own.rows[i].en] = &own.rows[i];   // a later row wins, as in a single table
            return m;
        }();
        return map;
    }

    std::unique_ptr<juce::PropertiesFile> openSettings()
    {
        juce::PropertiesFile::Options o;
        o.applicationName = FaseGroove::product().productName;
        o.folderName = FaseGroove::product().productName;
        o.filenameSuffix = ".settings";
        o.osxLibrarySubFolder = "Application Support";
        return std::make_unique<juce::PropertiesFile> (o);
    }
}

const char* languageName (int language) noexcept
{
    static const char* const names[numLanguages] { "English", "Italiano", "Español", "Français", "Deutsch", "Português" };
    return names[juce::jlimit (0, numLanguages - 1, language)];
}

Language current() noexcept
{
    return (Language) currentLanguage.load();
}

namespace
{
    /** JUCE's own texts (the Standalone window's Options menu and audio settings use TRANS). */
    void applyToJuce()
    {
        static const char* const juceTexts[] {
            "Audio/MIDI Settings...", "Audio/MIDI Settings", "Save current state...", "Load a saved state...",
            "Reset to default state", "Save current state", "Load a saved state", "Error whilst saving",
            "Error whilst loading", "Couldn't write to the specified file!", "Couldn't read from the specified file!",
            "Audio device type:", "Device:", "Output:", "Input:", "Sample rate:", "Audio buffer size:",
            "Active MIDI inputs:", "MIDI Output:", "Active output channels:", "Active input channels:", "Test",
            "Plays a test tone", "Control Panel", "Reset Device", "Show advanced settings...",
            "No MIDI inputs available", "none", "OK"
        };

        if (currentLanguage.load() == 0)
        {
            juce::LocalisedStrings::setCurrentMappings (nullptr);
            return;
        }

        juce::String text ("language: " + juce::String (FaseGroove::product().productName) + "\n");
        for (auto* key : juceTexts)
            text << "\"" << juce::String (key).replace ("\"", "\\\"") << "\" = \""
                 << tr (key).replace ("\"", "\\\"") << "\"\n";

        juce::LocalisedStrings::setCurrentMappings (new juce::LocalisedStrings (text, false));
    }
}





//==============================================================================
namespace
{
    struct Binding
    {
        juce::Component::SafePointer<juce::Component> component;
        std::function<void()> apply;
    };

    std::vector<Binding>& bindings()
    {
        static std::vector<Binding> all;
        return all;
    }

    void refreshBindings()
    {
        auto& all = bindings();
        all.erase (std::remove_if (all.begin(), all.end(), [] (const Binding& b) { return b.component == nullptr; }), all.end());
        // By index: a binding may add another one while it runs.
        for (std::size_t i = 0; i < all.size(); ++i)
            if (all[i].component != nullptr)
            {
                auto apply = all[i].apply;   // the vector may grow during the call
                apply();
            }
    }
}

void bind (juce::Component& c, std::function<void()> apply)
{
    apply();
    bindings().push_back ({ juce::Component::SafePointer<juce::Component> (&c), std::move (apply) });
}

void bindText (juce::Button& b, const juce::String& english)
{
    juce::Component::SafePointer<juce::Button> safe (&b);
    bind (b, [safe, english] { if (safe != nullptr) safe->setButtonText (tr (english)); });
}

void bindTip (juce::Component& c, const juce::String& english)
{
    juce::Component::SafePointer<juce::Component> safe (&c);
    bind (c, [safe, english]
    {
        if (auto* client = dynamic_cast<juce::SettableTooltipClient*> (safe.getComponent()))
            client->setTooltip (tr (english));
    });
}

void bindLabel (juce::Label& l, const juce::String& english, bool upperCase)
{
    juce::Component::SafePointer<juce::Label> safe (&l);
    bind (l, [safe, english, upperCase]
    {
        if (safe != nullptr)
            safe->setText (upperCase ? upper (tr (english)) : tr (english), juce::dontSendNotification);
    });
}

void bindItems (juce::ComboBox& box, const juce::StringArray& english, int firstId)
{
    juce::Component::SafePointer<juce::ComboBox> safe (&box);
    bind (box, [safe, english, firstId]
    {
        if (safe == nullptr)
            return;
        for (int i = 0; i < english.size(); ++i)
            safe->changeItemText (firstId + i, tr (english[i]));
        safe->setText (safe->getItemText (safe->getSelectedItemIndex()), juce::dontSendNotification);
        if (safe->getSelectedId() == 0)
            safe->setText ({}, juce::dontSendNotification);
    });
}

namespace
{
    /** The settings file kept open for reading; it is opened again when its date or size changed
        or it was written in the last few seconds (another instance, or a coarse file clock).
        Writes still go through a fresh file, so they never overwrite another instance's keys. */
    struct SettingsCache : juce::DeletedAtShutdown
    {
        ~SettingsCache() override { clearSingletonInstance(); }

        template <typename Fn>
        auto read (Fn&& fn)
        {
            const juce::ScopedLock sl (lock);
            return fn (current());
        }
        void invalidate() { const juce::ScopedLock sl (lock); size = -2; }

        JUCE_DECLARE_SINGLETON_INLINE (SettingsCache, false)

    private:
        juce::PropertiesFile& current()
        {
            const auto f = settings != nullptr ? settings->getFile() : juce::File();
            const auto t = f.getLastModificationTime();
            const auto n = f.getSize();
            if (settings == nullptr || n != size || t != modified || (juce::Time::getCurrentTime() - t).inMilliseconds() < 3000)
            {
                settings = openSettings();
                modified = settings->getFile().getLastModificationTime();
                size = settings->getFile().getSize();
            }
            return *settings;
        }

        juce::CriticalSection lock;
        std::unique_ptr<juce::PropertiesFile> settings;
        juce::Time modified;
        juce::int64 size = -2;
    };
}

bool tipsEnabled()
{
    return setting ("tips", "1") != "0";
}

void setTipsEnabled (bool enabled)
{
    setSetting ("tips", enabled ? "1" : "0");
}

juce::String setting (const char* key, const juce::String& fallback)
{
    return SettingsCache::getInstance()->read ([&] (juce::PropertiesFile& settings) { return settings.getValue (key, fallback); });
}

void setSetting (const char* key, const juce::var& value)
{
    if (auto settings = openSettings())
    {
        settings->setValue (key, value);
        settings->saveIfNeeded();
    }
    SettingsCache::getInstance()->invalidate();
}

void setCurrent (Language language)
{
    currentLanguage = juce::jlimit (0, numLanguages - 1, (int) language);
    applyToJuce();
    refreshBindings();

    if (auto settings = openSettings())
    {
        settings->setValue ("language", currentLanguage.load());
        settings->saveIfNeeded();
    }
    SettingsCache::getInstance()->invalidate();
}

void loadSaved()
{
    if (auto settings = openSettings(); settings != nullptr && settings->containsKey ("language"))
    {
        currentLanguage = juce::jlimit (0, numLanguages - 1, settings->getIntValue ("language"));
        applyToJuce();
        refreshBindings();   // texts and tips set up before the language was read
        return;
    }

    // First run: follow the computer's language.
    const auto system = juce::SystemStats::getUserLanguage().toLowerCase();
    static const char* const codes[numLanguages] { "en", "it", "es", "fr", "de", "pt" };
    for (int i = 0; i < numLanguages; ++i)
        if (system.startsWith (codes[i]))
            currentLanguage = i;
    applyToJuce();
    refreshBindings();
}

juce::String upper (const juce::String& text)
{
    juce::String out;
    for (auto c : text)
    {
        if (c >= 0xe0 && c <= 0xfe && c != 0xf7)
            c -= 0x20;   // Latin-1: à -> À, é -> É, ñ -> Ñ ...
        else
            c = juce::CharacterFunctions::toUpperCase (c);
        out += juce::String::charToString (c);
    }
    return out;
}

juce::String tr (const juce::String& english)
{
    const int lang = currentLanguage.load();
    if (lang == 0)
        return english;

    const auto& map = index();
    const auto it = map.find (english.toStdString());
    if (it == map.end())
        return english;

    const Entry& e = *it->second;
    const char* text = lang == 1 ? e.it : lang == 2 ? e.es : lang == 3 ? e.fr : lang == 4 ? e.de : e.pt;
    return juce::String::fromUTF8 (text);
}
}
