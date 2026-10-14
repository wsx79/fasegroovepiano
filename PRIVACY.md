# Privacy – Fase Groove Piano

*Last updated: 13 October 2026 · 🇮🇹 [Versione italiana](PRIVACY.it.md)*

Fase Groove Piano respects your privacy in the simplest possible way: **it collects nothing**.

## In short

- **No personal data** is collected, stored or sent.
- **No analytics, telemetry, ads or license checks.**
- **One optional connection: the update check.** When you open the plug-in it asks GitHub
  (once per session) for the latest release and, if there is a newer one, downloads it. Like any
  web request, GitHub sees your IP address and a "FaseGroovePiano-Updater" user agent; nothing
  else is sent. Switch it off in **OPTIONS → SETTINGS → Check for updates automatically** and the
  plug-in never connects to the internet.
- **No account** and no registration.

## What stays on your computer

| What | Where | Why |
|---|---|---|
| Plug-in settings | Inside your DAW project | So you get your settings back when you reopen the project (saved by the DAW, not by the plug-in). |
| Dragged MIDI clips | `Documents/Fase Groove Piano/MIDI clips/` | Dragging to the DAW needs a real file. You can delete them at any time. |
| `.mid` files saved with *Save .mid…* | Wherever you choose | At your request. |
| Your presets (`.fgp`) | `Documents/Fase Groove Piano/Presets/` | Only when you save one, to load it in other projects. |
| Your grooves (`.fgg`) | `Documents/Fase Groove Piano/Grooves/` | Only when you save one with MY GROOVES. |
| Downloaded updates | `Downloads/Fase Groove Piano updates/` (or `Documents/` if there is no Downloads folder) | Only when a newer version is found. You can delete them at any time. |
| Interface language, favourites and recent grooves / progressions | `Fase Groove Piano.settings` in your system's user settings folder | So the plug-in keeps your language and your lists in every project. |
| Audio/MIDI settings of the Standalone app | Your system's user settings folder | Handled by the JUCE framework to remember your audio interface. |

None of these files ever leaves your computer.

## External links

The **OPTIONS → ABOUT** page has *SOURCE CODE* (GitHub), *DONATE: Ko-fi*, *DONATE: PayPal* and
*SPOTIFY* buttons.
They open **only if you click them**, in your browser; from then on those sites' privacy
policies apply. The language detection on first start only reads your computer's language setting, locally.
Donations are handled entirely by Ko-fi or PayPal; the author never receives your
payment details.

## Contact

For questions, open an *issue* at [github.com/wsx79/fasegroovepiano](https://github.com/wsx79/fasegroovepiano/issues).
