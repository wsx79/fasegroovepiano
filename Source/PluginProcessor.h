#pragma once

#include "engine/GrooveEngine.h"

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include <array>
#include <atomic>

namespace ParamIDs
{
    inline constexpr const char* key         = "key";
    inline constexpr const char* scale       = "scale";
    inline constexpr const char* extension   = "extension";
    inline constexpr const char* voicing     = "voicing";
    inline constexpr const char* registerPos = "register";
    inline constexpr const char* bass        = "bass";
    inline constexpr const char* pattern     = "pattern";
    inline constexpr const char* chordLength = "chordLength";
    inline constexpr const char* progLength  = "progLength";
    inline constexpr const char* swing       = "swing";
    inline constexpr const char* gate        = "gate";
    inline constexpr const char* velocity    = "velocity";
    inline constexpr const char* humanize    = "humanize";
    inline constexpr const char* fills       = "fills";
    inline constexpr const char* playMode    = "playMode";
    inline constexpr const char* sound       = "sound";
    inline constexpr const char* tone        = "tone";
    inline constexpr const char* reverb      = "reverb";
    inline constexpr const char* volume      = "volume";

    inline juce::String slot (int index) { return "slot" + juce::String (index + 1); }
}

class SoulfulGrooveKeysProcessor : public juce::AudioProcessor
{
public:
    SoulfulGrooveKeysProcessor();
    ~SoulfulGrooveKeysProcessor() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    /** Current parameter values as engine settings (safe from any thread). */
    sgk::GrooveSettings currentSettings() const;

    /** Sets a parameter from the UI, notifying the host. */
    void setParameterFromUI (const juce::String& id, float plainValue);

    juce::AudioProcessorValueTreeState apvts;

    // Read by the editor for display.
    std::atomic<int> uiStep { -1 };      // current 16th within the pattern, -1 = stopped
    std::atomic<int> uiSlot { -1 };      // current chord slot, -1 = stopped
    std::atomic<int> uiHeldRoot { -1 };  // root picked by a held key (hold mode)

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
    float raw (const char* id) const;

    std::array<std::atomic<float>*, sgk::maxProgressionLength> slotParams {};

    sgk::GrooveEngine engine;
    std::vector<sgk::NoteEvent> events;
    juce::MidiBuffer outMidi;

    juce::Synthesiser synth;
    juce::Reverb reverb;
    juce::SmoothedValue<float> outputGain;

    // Hold-key mode
    std::array<int, 16> heldStack {};
    int heldCount = 0;

    double internalPpq = 0.0;
    double expectedNextPpq = 0.0;
    bool wasRunning = false;
    bool soundWasOn = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoulfulGrooveKeysProcessor)
};
