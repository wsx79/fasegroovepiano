// Fase Groove Piano - Copyright (C) 2026 Fabio 'Pitch' Serra
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_events/juce_events.h>

#include <array>
#include <atomic>
#include <memory>

class FaseGroovePianoProcessor;

/** A Novation Launchpad (Mini MK3, X, Pro MK3) or an Ableton Push 2 / Push 3 (in User mode) as a
    hardware rhythm grid. On a Launchpad, in Programmer mode its
    8x8 pads show the groove (chords on the top four rows, bass on the bottom four, 16 steps per two
    rows) with the playhead, a press edits a step like a click on the grid, and the round buttons
    play / stop, roll new rhythms, switch the sequencer and the parts on and off. On Push the same
    pads, the eight buttons above the display as the Launchpad's top row, the scene buttons as its
    right column, and Play.

    The device is opened directly (not through the DAW), so it works even with the plug-in's editor
    closed. The choice (off / automatic) is global, like the language, and only one plug-in instance
    drives the Launchpad at a time. */
class PadController : private juce::Timer, private juce::MidiInputCallback
{
public:
    explicit PadController (FaseGroovePianoProcessor&);
    ~PadController() override;

    // The setting in OPTIONS: off, or the first Launchpad found.
    static bool enabled();
    static void setEnabled (bool);

    enum class Status { off, searching, connected, busy, taken };
    Status status() const        { return state.load(); }
    juce::String deviceName() const;

    //==============================================================================
    // The layout, without hardware (used by the tests).

    /** Launchpad colours (palette indices) for the 64 pads (row 0 = top, left to right), the top
        row of round buttons and the right column (top to bottom). */
    struct Leds
    {
        std::array<int, 64> pads {};
        std::array<int, 8> top {}, side {};
        bool operator== (const Leds& o) const { return pads == o.pads && top == o.top && side == o.side; }
    };
    Leds computeLeds() const;

    /** Where a pad (row 0 = top, column 0 = left) writes: which row of the grid and which step. */
    static void padToStep (int row, int column, bool& bassRow, int& step);

    /** A pad or button pressed: a note (11..88 in Programmer mode) or a control change (the round buttons). */
    void handlePress (int number, bool isControlChange);

    /** The same for an Ableton Push (User mode): pads 36..99, buttons as control changes. */
    void handlePushPress (int number, bool isControlChange);

    static int padNote (int row, int column) { return (8 - row) * 10 + column + 1; }
    static int topButton (int index)         { return 91 + index; }
    static int sideButton (int index)        { return (8 - index) * 10 + 9; }

    // Push: pads from 36 (bottom left) to 99 (top right); the buttons above the display 102..109,
    // the scene buttons 43 (top) .. 36 (bottom), Play 85.
    static int pushPadNote (int row, int column) { return 36 + (7 - row) * 8 + column; }
    static int pushTopButton (int index)         { return 102 + index; }
    static int pushSideButton (int index)        { return 43 - index; }
    static constexpr int pushPlayButton = 85;

private:
    void timerCallback() override;
    void handleIncomingMidiMessage (juce::MidiInput*, const juce::MidiMessage&) override;

    void tryConnect();
    void disconnect();
    void sendLeds (bool everything);
    void sendModeSelect (bool programmer);
    void pressPad (int row, int column);
    void pressTop (int index);
    void pressSide (int index);
    void sendLed (bool pad, int number, int launchpadColour);

    FaseGroovePianoProcessor& processor;

    std::vector<std::unique_ptr<juce::MidiInput>> inputs;   // both of the Launchpad's ports
    std::vector<std::unique_ptr<juce::MidiOutput>> outputs;
    juce::MidiOutput* ledOutput = nullptr;                   // the "MIDI" interface: Programmer mode lights
    int deviceId = 0x0D;                                      // 0x0D Mini MK3, 0x0C X, 0x0E Pro MK3; Push: 2 or 3
    bool push = false;
    juce::String name;

    std::atomic<Status> state { Status::off };
    Leds sent;
    bool sentOnce = false;
    int ticks = 0;
    int seenChoice = -1;

    std::shared_ptr<std::atomic<bool>> alive = std::make_shared<std::atomic<bool>> (true);
};
