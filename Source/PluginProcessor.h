#pragma once
#include <JuceHeader.h>
#include "GameBoyAPU.h"
#include "Presets.h"
#include <array>
#include <atomic>

static constexpr int MAX_VOICES  = 8;
static constexpr int OSC_SIZE    = 1024;

class ChiptuneVSTProcessor : public juce::AudioProcessor,
                              public juce::AudioProcessorValueTreeState::Listener
{
public:
    ChiptuneVSTProcessor();
    ~ChiptuneVSTProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ChiptuneVST"; }
    bool acceptsMidi()  const override { return true;  }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.5; }

    int  getNumPrograms() override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return "Default"; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    void parameterChanged(const juce::String& id, float value) override;

    // Preset management (called from UI thread)
    void loadPreset(int index);
    int  currentPreset = -1;

    juce::AudioProcessorValueTreeState apvts;

    // Wave table — shared with editor
    std::array<uint8_t, 32> waveTable = {
        15,14,12,10, 8, 6, 4, 2,
         0, 2, 4, 6, 8,10,12,14,
        15,14,12,10, 8, 6, 4, 2,
         0, 2, 4, 6, 8,10,12,14,
    };

    // Oscilloscope ring buffer (audio thread writes, UI thread reads)
    std::array<float, OSC_SIZE> oscBuffer = {};
    std::atomic<int>            oscWritePos { 0 };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    void handleMidi(const juce::MidiMessage&, int samplePos);
    void syncParams();

    double currentSampleRate = 44100.0;
    int    channelType       = 0;

    // Voice pools
    std::array<GBSquareVoice, MAX_VOICES> sq1Voices;
    std::array<GBSquareVoice, MAX_VOICES> sq2Voices;
    std::array<GBWaveVoice,   MAX_VOICES> waveVoices;
    std::array<GBNoiseVoice,  MAX_VOICES> noiseVoices;

    struct VoiceSlot { int note = -1; bool active = false; };
    std::array<VoiceSlot, MAX_VOICES> voiceSlots;

    // Arpeggiator state
    int    arpStep       = 0;
    double arpPhaseAccum = 0.0;
    float  arpCurrentOffset = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChiptuneVSTProcessor)
};
