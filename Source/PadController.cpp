// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "PadController.h"
#include "PluginProcessor.h"

namespace
{
    // Launchpad palette (Programmer mode): 0 off, 1-3 greys/white, then four shades per hue.
    namespace Colour
    {
        constexpr int off = 0, beat = 1, white = 3;
        constexpr int amber = 9, amberDim = 10, amberDark = 11, yellow = 13;
        constexpr int red = 5, redDark = 7;
        constexpr int green = 21, greenDark = 23;
        constexpr int cyan = 37, cyanDim = 38, cyanDark = 39;
        constexpr int blue = 45, blueDark = 47;
        constexpr int violet = 49, violetDim = 50;
    }

    // The setting is shared by every instance in the process (and saved like the language).
    std::atomic<int> choice { -1 };          // -1 = not read yet, 0 = off, 1 = automatic
    std::atomic<int> choiceVersion { 0 };
    std::atomic<PadController*> owner { nullptr };

    std::unique_ptr<juce::PropertiesFile> openSettings()
    {
        juce::PropertiesFile::Options o;
        o.applicationName = "Fase Groove Piano";
        o.folderName = "Fase Groove Piano";
        o.filenameSuffix = ".settings";
        o.osxLibrarySubFolder = "Application Support";
        return std::make_unique<juce::PropertiesFile> (o);
    }

    struct Model { const char* const tags[3]; int id; };
    constexpr Model models[] {
        { { "LPMiniMK3", "Launchpad Mini MK3", nullptr }, 0x0D },
        { { "LPX",       "Launchpad X",        nullptr }, 0x0C },
        { { "LPProMK3",  "Launchpad Pro MK3",  nullptr }, 0x0E },
    };

    int modelOf (const juce::String& deviceName)
    {
        for (const auto& m : models)
            for (auto* tag : m.tags)
                if (tag != nullptr && deviceName.containsIgnoreCase (tag))
                    return m.id;
        return 0;
    }

    /** How likely a port is the Launchpad's "MIDI" interface (Programmer mode lights), not its
        "DAW" one: "... MIDI In/Out" on macOS and Linux, "MIDIIN2 / MIDIOUT2 (...)" on Windows. */
    int midiInterfaceScore (const juce::String& deviceName)
    {
        if (deviceName.containsIgnoreCase ("DAW"))
            return -1;
        if (deviceName.startsWithIgnoreCase ("MIDIIN2") || deviceName.startsWithIgnoreCase ("MIDIOUT2"))
            return 2;
        return deviceName.containsIgnoreCase ("MIDI") ? 1 : 0;
    }

    int symbolColour (char c, bool bass)
    {
        switch (c)
        {
            case 'X': case 'U': case 'D': return bass ? Colour::cyan : Colour::amber;
            case 'x': case 'u': case 'd': return bass ? Colour::cyanDim : Colour::amberDim;
            case 'g':                     return bass ? Colour::cyanDark : Colour::amberDark;
            case '>':                     return bass ? Colour::blue : Colour::yellow;
            case '=':                     return bass ? Colour::blueDark : Colour::redDark;
            default:                      return Colour::off;
        }
    }
}

//==============================================================================
PadController::PadController (FaseGroovePianoProcessor& p) : processor (p)
{
    startTimer (40);
}

PadController::~PadController()
{
    *alive = false;
    stopTimer();
    disconnect();
}

bool PadController::enabled()
{
    if (choice.load() < 0)
        choice = openSettings()->getBoolValue ("launchpad", false) ? 1 : 0;
    return choice.load() == 1;
}

void PadController::setEnabled (bool on)
{
    choice = on ? 1 : 0;
    ++choiceVersion;
    auto settings = openSettings();
    settings->setValue ("launchpad", on);
    settings->saveIfNeeded();
}

juce::String PadController::deviceName() const
{
    return name;
}

//==============================================================================
void PadController::padToStep (int row, int column, bool& bassRow, int& step)
{
    // Rows 0-3: chords, rows 4-7: bass; in each, two rows per 16 steps (steps 1-16, then 17-32).
    bassRow = row >= 4;
    const int r = row % 4;
    step = (r / 2) * 16 + (r % 2) * 8 + column;
}

PadController::Leds PadController::computeLeds() const
{
    Leds leds;
    const auto s = processor.currentSettings();
    const auto pattern = sgk::GrooveEngine::resolvePattern (s);
    const int length = juce::jmin (pattern.length(), sgk::userPatternLength);
    const bool bassOn = processor.isBassOn();
    const auto bassLine = s.bass == sgk::BassMode::withChords ? sgk::stabBassLine (pattern) : pattern.leftHand;
    const int playing = processor.uiStep.load() >= 0 ? processor.uiStep.load() % sgk::userPatternLength : -1;

    for (int row = 0; row < 8; ++row)
        for (int column = 0; column < 8; ++column)
        {
            bool bassRow = false;
            int step = 0;
            padToStep (row, column, bassRow, step);
            int colour = Colour::off;
            if (step < length)
            {
                const auto& line = bassRow ? bassLine : pattern.rightHand;
                const char c = step < (int) line.size() ? line[(size_t) step] : '.';
                colour = (bassRow && ! bassOn) ? Colour::off : symbolColour (c, bassRow);
                if (colour == Colour::off && step % 4 == 0)
                    colour = Colour::beat;   // the beats, to find your way
                if (step == playing)
                    colour = Colour::white;
            }
            leds.pads[(size_t) (row * 8 + column)] = colour;
        }

    const bool previewing = processor.previewOn.load();
    const bool sequencerOn = processor.apvts.getRawParameterValue (ParamIDs::sequencer)->load() > 0.5f;
    const bool pianoHeard = ! processor.chordsMuted.load() || ! bassOn;
    leds.top = { previewing ? Colour::green : Colour::greenDark, Colour::redDark,
                 Colour::violet, Colour::violet, Colour::violetDim,
                 sequencerOn ? Colour::green : Colour::red,
                 pianoHeard ? Colour::amber : Colour::amberDark,
                 bassOn ? Colour::cyan : Colour::cyanDark };

    for (int i = 0; i < sgk::numSegments; ++i)
    {
        leds.side[(size_t) i] = Colour::violetDim;   // new rhythm for part A..D
        const bool locked = processor.isSegmentLocked (processor.segmentAtStep (i * sgk::userPatternLength / sgk::numSegments));
        leds.side[(size_t) (4 + i)] = locked ? Colour::cyan : Colour::beat;
    }
    return leds;
}

void PadController::handlePress (int number, bool isControlChange)
{
    if (! isControlChange)
    {
        const int row = 8 - number / 10, column = number % 10 - 1;
        if (row < 0 || row > 7 || column < 0 || column > 7)
            return;

        bool bassRow = false;
        int step = 0;
        padToStep (row, column, bassRow, step);
        if (step >= sgk::GrooveEngine::resolvePattern (processor.currentSettings()).length())
            return;

        // Like a click on the grid: rest -> hit -> accent -> ghost -> rest.
        const char current = processor.stepSymbol (bassRow, step);
        processor.editStep (bassRow, step, current == 'x' ? 'X' : current == 'X' ? 'g' : current == 'g' ? '.' : 'x');
        return;
    }

    if (number >= 91 && number <= 98)
    {
        switch (number - 91)
        {
            case 0: processor.previewToggle(); break;
            case 1: processor.previewStop(); break;
            case 2: processor.rerollHand (false); break;
            case 3: processor.rerollHand (true); break;
            case 4: processor.rollNewRiff(); break;
            case 5: processor.setParameterFromUI (ParamIDs::sequencer,
                                                  processor.apvts.getRawParameterValue (ParamIDs::sequencer)->load() > 0.5f ? 0.0f : 1.0f); break;
            case 6: processor.hearPart (false); break;
            case 7: processor.hearPart (true); break;
            default: break;
        }
        return;
    }

    if (number % 10 == 9 && number >= 19 && number <= 89)
    {
        const int index = 8 - number / 10;   // 0 = top
        const int part = processor.segmentAtStep ((index % 4) * sgk::userPatternLength / sgk::numSegments);
        if (index < 4)
            processor.randomizeSegment (part);
        else
            processor.setSegmentLocked (part, ! processor.isSegmentLocked (part));
    }
}

//==============================================================================
void PadController::timerCallback()
{
    ++ticks;

    // The setting changed (here or in another instance).
    if (const int version = choiceVersion.load(); version != seenChoice)
    {
        seenChoice = version;
        if (! enabled())
        {
            disconnect();
            state = Status::off;
        }
        else if (state.load() == Status::off)
        {
            state = Status::searching;
            ticks = 0;
        }
    }

    if (! enabled())
        return;

    if (inputs.empty() || ledOutput == nullptr)
    {
        if (ticks % 50 == 1)   // every 2 seconds
            tryConnect();
        return;
    }

    // Unplugged?
    if (ticks % 50 == 0)
    {
        bool present = false;
        for (const auto& d : juce::MidiOutput::getAvailableDevices())
            present = present || d.name == name;
        if (! present)
        {
            disconnect();
            state = Status::searching;
            return;
        }
    }

    sendLeds (! sentOnce);
}

void PadController::tryConnect()
{
    // Only one instance drives the Launchpad.
    PadController* expected = nullptr;
    if (! owner.compare_exchange_strong (expected, this) && expected != this)
    {
        state = Status::taken;
        return;
    }

    int found = 0, bestScore = -2;
    for (const auto& d : juce::MidiOutput::getAvailableDevices())
    {
        if (const int id = modelOf (d.name); id != 0)
        {
            found = id;
            if (auto out = juce::MidiOutput::openDevice (d.identifier))
            {
                if (const int score = midiInterfaceScore (d.name); score > bestScore)
                {
                    bestScore = score;
                    ledOutput = out.get();
                    name = d.name;
                }
                outputs.push_back (std::move (out));
            }
        }
    }
    for (const auto& d : juce::MidiInput::getAvailableDevices())
        if (modelOf (d.name) != 0)
            if (auto in = juce::MidiInput::openDevice (d.identifier, this))
            {
                in->start();
                inputs.push_back (std::move (in));
            }

    if (inputs.empty() || ledOutput == nullptr)
    {
        disconnect();
        owner = nullptr;
        state = found != 0 ? Status::busy : Status::searching;   // there, but held by another program
        return;
    }

    deviceId = found;
    sendModeSelect (true);
    sentOnce = false;
    state = Status::connected;
}

void PadController::disconnect()
{
    if (ledOutput != nullptr)
    {
        // Lights off and back to the Launchpad's own mode.
        for (int row = 0; row < 8; ++row)
            for (int column = 0; column < 8; ++column)
                ledOutput->sendMessageNow (juce::MidiMessage::noteOn (1, padNote (row, column), (juce::uint8) 0));
        for (int i = 0; i < 8; ++i)
        {
            ledOutput->sendMessageNow (juce::MidiMessage::controllerEvent (1, topButton (i), 0));
            ledOutput->sendMessageNow (juce::MidiMessage::controllerEvent (1, sideButton (i), 0));
        }
        sendModeSelect (false);
    }

    for (auto& in : inputs)
        in->stop();
    inputs.clear();
    ledOutput = nullptr;
    outputs.clear();
    name.clear();
    sentOnce = false;

    PadController* self = this;
    owner.compare_exchange_strong (self, nullptr);
}

void PadController::sendModeSelect (bool programmer)
{
    // F0 00 20 29 02 <device> 0E <mode> F7: Programmer (1) or Live (0) mode, on every port.
    const juce::uint8 data[] { 0x00, 0x20, 0x29, 0x02, (juce::uint8) deviceId, 0x0E, (juce::uint8) (programmer ? 1 : 0) };
    const auto message = juce::MidiMessage::createSysExMessage (data, (int) sizeof (data));
    for (auto& out : outputs)
        out->sendMessageNow (message);
}

void PadController::sendLeds (bool everything)
{
    if (ledOutput == nullptr)
        return;

    const auto leds = computeLeds();
    for (int i = 0; i < 64; ++i)
        if (everything || leds.pads[(size_t) i] != sent.pads[(size_t) i])
            ledOutput->sendMessageNow (juce::MidiMessage::noteOn (1, padNote (i / 8, i % 8), (juce::uint8) leds.pads[(size_t) i]));
    for (int i = 0; i < 8; ++i)
    {
        if (everything || leds.top[(size_t) i] != sent.top[(size_t) i])
            ledOutput->sendMessageNow (juce::MidiMessage::controllerEvent (1, topButton (i), leds.top[(size_t) i]));
        if (everything || leds.side[(size_t) i] != sent.side[(size_t) i])
            ledOutput->sendMessageNow (juce::MidiMessage::controllerEvent (1, sideButton (i), leds.side[(size_t) i]));
    }
    sent = leds;
    sentOnce = true;
}

void PadController::handleIncomingMidiMessage (juce::MidiInput*, const juce::MidiMessage& m)
{
    // MIDI thread: hand the press to the message thread (the controls live there).
    int number = -1;
    bool controlChange = false;
    if (m.isNoteOn())
        number = m.getNoteNumber();
    else if (m.isController() && m.getControllerValue() > 0)
    {
        number = m.getControllerNumber();
        controlChange = true;
    }
    if (number < 0)
        return;

    juce::MessageManager::callAsync ([this, guard = alive, number, controlChange]
    {
        if (guard->load())
            handlePress (number, controlChange);
    });
}
