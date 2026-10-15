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

    struct Model { const char* const tags[3]; int id; bool push; };
    constexpr Model models[] {
        { { "LPMiniMK3", "Launchpad Mini MK3", nullptr }, 0x0D, false },
        { { "LPX",       "Launchpad X",        nullptr }, 0x0C, false },
        { { "LPProMK3",  "Launchpad Pro MK3",  nullptr }, 0x0E, false },
        { { "Push 2",    nullptr,              nullptr }, 2,    true },
        { { "Push 3",    nullptr,              nullptr }, 3,    true },
    };

    const Model* modelOf (const juce::String& deviceName)
    {
        for (const auto& m : models)
            for (auto* tag : m.tags)
                if (tag != nullptr && deviceName.containsIgnoreCase (tag))
                    return &m;
        return nullptr;
    }

    /** Push: the "User" port (the "Live" one belongs to Live): "... User Port" on macOS,
        "MIDIIN2 / MIDIOUT2 (Ableton Push 3)" on Windows, ":1" on Linux. */
    int userPortScore (const juce::String& deviceName)
    {
        if (deviceName.containsIgnoreCase ("Live Port"))
            return -1;
        if (deviceName.containsIgnoreCase ("User") || deviceName.startsWithIgnoreCase ("MIDIIN2")
            || deviceName.startsWithIgnoreCase ("MIDIOUT2") || deviceName.endsWith (":1"))
            return 2;
        return -1;
    }

    // Push has no fixed colour palette: the plug-in writes its colours into entries 64.. (RGB + white).
    struct PushColour { int launchpad, r, g, b; };
    constexpr PushColour pushColours[] {
        { 1, 40, 40, 40 },     { 3, 255, 255, 255 },  { 5, 255, 0, 0 },     { 7, 100, 0, 0 },
        { 9, 255, 130, 10 },   { 10, 140, 70, 5 },    { 11, 55, 28, 2 },    { 13, 255, 225, 30 },
        { 21, 0, 255, 0 },     { 23, 0, 60, 0 },      { 37, 40, 200, 255 }, { 38, 20, 100, 140 },
        { 39, 8, 40, 60 },     { 45, 40, 60, 255 },   { 47, 10, 15, 90 },   { 49, 170, 80, 255 },
        { 50, 70, 35, 120 },
    };

    int pushIndex (int launchpadColour)
    {
        for (int i = 0; i < (int) std::size (pushColours); ++i)
            if (pushColours[i].launchpad == launchpadColour)
                return 64 + i;
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
                const bool heard = bassRow ? bassOn : (! processor.chordsMuted.load() || ! bassOn);
                colour = heard ? symbolColour (c, bassRow) : Colour::off;   // a part you don't hear goes dark
                if (colour == Colour::off && step % 4 == 0 && heard)
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
        leds.side[(size_t) i] = Colour::violetDim;       // new rhythm for part A..D
        leds.side[(size_t) (4 + i)] = Colour::violet;    // NEW RIFF
    }
    return leds;
}

void PadController::pressPad (int row, int column)
{
    bool bassRow = false;
    int step = 0;
    padToStep (row, column, bassRow, step);
    if (step >= sgk::GrooveEngine::resolvePattern (processor.currentSettings()).length())
        return;
    if (! bassRow && processor.chordsMuted.load() && processor.isBassOn())
        return;   // the piano is off: its rows are dark

    // Like a click on the grid: rest -> hit -> accent -> ghost -> rest.
    const char current = processor.stepSymbol (bassRow, step);
    processor.editStep (bassRow, step, current == 'x' ? 'X' : current == 'X' ? 'g' : current == 'g' ? '.' : 'x');
}

void PadController::pressTop (int index)
{
    switch (index)
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
}

void PadController::pressSide (int index)
{
    // Top four: a new rhythm for part A..D; bottom four: NEW RIFF (parts locked in the plug-in stay).
    if (index < 4)
        processor.randomizeSegment (processor.segmentAtStep (index * sgk::userPatternLength / sgk::numSegments));
    else
        processor.rollNewRiff();
}

void PadController::handlePress (int number, bool isControlChange)
{
    if (! isControlChange)
    {
        const int row = 8 - number / 10, column = number % 10 - 1;
        if (row >= 0 && row <= 7 && column >= 0 && column <= 7)
            pressPad (row, column);
    }
    else if (number >= 91 && number <= 98)
        pressTop (number - 91);
    else if (number % 10 == 9 && number >= 19 && number <= 89)
        pressSide (8 - number / 10);
}

void PadController::handlePushPress (int number, bool isControlChange)
{
    if (! isControlChange)
    {
        if (number >= 36 && number <= 99)
            pressPad (7 - (number - 36) / 8, (number - 36) % 8);
    }
    else if (number >= 102 && number <= 109)
        pressTop (number - 102);
    else if (number >= 36 && number <= 43)
        pressSide (43 - number);
    else if (number == pushPlayButton)
        pressTop (0);
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
    // Only one instance drives the device.
    PadController* expected = nullptr;
    if (! owner.compare_exchange_strong (expected, this) && expected != this)
    {
        state = Status::taken;
        return;
    }

    // The first supported device found (a Launchpad before a Push).
    const Model* model = nullptr;
    for (const auto& d : juce::MidiOutput::getAvailableDevices())
        if (const auto* m = modelOf (d.name); m != nullptr && (model == nullptr || (model->push && ! m->push)))
            model = m;

    if (model == nullptr)
    {
        owner = nullptr;
        state = Status::searching;
        return;
    }

    push = model->push;
    deviceId = model->id;
    auto score = [this] (const juce::String& n) { return push ? userPortScore (n) : midiInterfaceScore (n); };

    int bestScore = -1;
    for (const auto& d : juce::MidiOutput::getAvailableDevices())
    {
        if (modelOf (d.name) != model || (push && score (d.name) < 0))
            continue;   // Push: never touch Live's own port
        if (auto out = juce::MidiOutput::openDevice (d.identifier))
        {
            if (const int sc = score (d.name); sc > bestScore)
            {
                bestScore = sc;
                ledOutput = out.get();
                name = d.name;
            }
            outputs.push_back (std::move (out));
        }
    }
    for (const auto& d : juce::MidiInput::getAvailableDevices())
    {
        if (modelOf (d.name) != model || (push && score (d.name) < 0))
            continue;
        if (auto in = juce::MidiInput::openDevice (d.identifier, this))
        {
            in->start();
            inputs.push_back (std::move (in));
        }
    }

    if (inputs.empty() || ledOutput == nullptr)
    {
        disconnect();
        owner = nullptr;
        state = Status::busy;   // there, but held by another program (or by the DAW)
        return;
    }

    sendModeSelect (true);
    sentOnce = false;
    state = Status::connected;
}

void PadController::disconnect()
{
    if (ledOutput != nullptr)
    {
        // Lights off and back to the device's own mode (Push: back to Live).
        for (int i = 0; i < 64; ++i)
            sendLed (true, i, 0);
        for (int i = 0; i < 8; ++i)
        {
            sendLed (false, i, 0);
            sendLed (false, 8 + i, 0);
        }
        if (push)
            ledOutput->sendMessageNow (juce::MidiMessage::controllerEvent (1, pushPlayButton, 0));
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
    if (! push)
    {
        // F0 00 20 29 02 <device> 0E <mode> F7: Programmer (1) or Live (0) mode, on every port.
        const juce::uint8 data[] { 0x00, 0x20, 0x29, 0x02, (juce::uint8) deviceId, 0x0E, (juce::uint8) (programmer ? 1 : 0) };
        const auto message = juce::MidiMessage::createSysExMessage (data, (int) sizeof (data));
        for (auto& out : outputs)
            out->sendMessageNow (message);
        return;
    }

    // Push: F0 00 21 1D 01 <model> <command> ... F7 (model 01 = Push 2; Push 3 keeps the protocol,
    // its own model byte is sent too). First our colours into the palette (0x03, then 0x05 to
    // apply them), then the MIDI mode (0x0A): User (1) or Live (0).
    for (const juce::uint8 modelByte : { (juce::uint8) 0x01, (juce::uint8) 0x02 })
    {
        if (modelByte == 0x02 && deviceId != 3)
            continue;
        auto send = [this, modelByte] (std::initializer_list<juce::uint8> body)
        {
            std::vector<juce::uint8> data { 0x00, 0x21, 0x1D, 0x01, modelByte };
            data.insert (data.end(), body.begin(), body.end());
            ledOutput->sendMessageNow (juce::MidiMessage::createSysExMessage (data.data(), (int) data.size()));
        };
        if (programmer)
        {
            for (int i = 0; i < (int) std::size (pushColours); ++i)
            {
                const auto& c = pushColours[i];
                const int w = juce::jmax (c.r, c.g, c.b) / 2;
                send ({ 0x03, (juce::uint8) (64 + i),
                        (juce::uint8) (c.r & 0x7F), (juce::uint8) (c.r >> 7), (juce::uint8) (c.g & 0x7F), (juce::uint8) (c.g >> 7),
                        (juce::uint8) (c.b & 0x7F), (juce::uint8) (c.b >> 7), (juce::uint8) (w & 0x7F), (juce::uint8) (w >> 7) });
            }
            send ({ 0x05 });
        }
        send ({ 0x0A, (juce::uint8) (programmer ? 1 : 0) });
    }
}

void PadController::sendLed (bool pad, int number, int launchpadColour)
{
    // pad: number 0..63 (row * 8 + column); button: 0..7 top row, 8..15 right column.
    if (ledOutput == nullptr)
        return;

    const int colour = push ? pushIndex (launchpadColour) : launchpadColour;
    if (pad)
    {
        const int row = number / 8, column = number % 8;
        ledOutput->sendMessageNow (juce::MidiMessage::noteOn (1, push ? pushPadNote (row, column) : padNote (row, column), (juce::uint8) colour));
        return;
    }

    const int cc = number < 8 ? (push ? pushTopButton (number) : topButton (number))
                              : (push ? pushSideButton (number - 8) : sideButton (number - 8));
    ledOutput->sendMessageNow (juce::MidiMessage::controllerEvent (1, cc, colour));
}

void PadController::sendLeds (bool everything)
{
    if (ledOutput == nullptr)
        return;

    const auto leds = computeLeds();
    for (int i = 0; i < 64; ++i)
        if (everything || leds.pads[(size_t) i] != sent.pads[(size_t) i])
            sendLed (true, i, leds.pads[(size_t) i]);
    for (int i = 0; i < 8; ++i)
    {
        if (everything || leds.top[(size_t) i] != sent.top[(size_t) i])
            sendLed (false, i, leds.top[(size_t) i]);
        if (everything || leds.side[(size_t) i] != sent.side[(size_t) i])
            sendLed (false, 8 + i, leds.side[(size_t) i]);
    }
    if (push && (everything || leds.top[0] != sent.top[0]))   // Play (a white LED: brightness)
        ledOutput->sendMessageNow (juce::MidiMessage::controllerEvent (1, pushPlayButton, processor.previewOn.load() ? 127 : 16));
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

    // Push: the encoder and touch strip touches (notes 0..12) are not pads.
    juce::MessageManager::callAsync ([this, guard = alive, number, controlChange]
    {
        if (! guard->load())
            return;
        if (push) handlePushPress (number, controlChange);
        else      handlePress (number, controlChange);
    });
}
