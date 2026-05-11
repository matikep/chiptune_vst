#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
#include <random>

using APVTS = juce::AudioProcessorValueTreeState;

// ── Parameter layout ──────────────────────────────────────────────────────────
APVTS::ParameterLayout ChiptuneVSTProcessor::createParams()
{
    using NR = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;

    p.push_back(std::make_unique<juce::AudioParameterInt>  ("channelType","Channel",  0,3,0));
    p.push_back(std::make_unique<juce::AudioParameterInt>  ("duty",       "Duty",     0,3,2));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("attack",  "Attack",  NR(0.001f,2.0f,0.001f,0.3f), 0.005f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("decay",   "Decay",   NR(0.001f,2.0f,0.001f,0.3f), 0.1f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("sustain", "Sustain", NR(0.f,1.f),                  0.8f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("release", "Release", NR(0.001f,3.0f,0.001f,0.3f), 0.08f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("volume",  "Volume",  NR(0.f,1.f),                  0.7f));
    // Sweep
    p.push_back(std::make_unique<juce::AudioParameterBool> ("sweepOn",   "Sweep",    false));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("sweepRate", "Swp Rate", NR(0.1f,20.f),              2.f));
    p.push_back(std::make_unique<juce::AudioParameterBool> ("sweepUp",   "Swp Up",   true));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("sweepAmt",  "Swp Semi", NR(0.f,12.f),               1.f));
    // Noise
    p.push_back(std::make_unique<juce::AudioParameterBool> ("noiseShort","Short",    false));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("noiseFreq", "NoiseFreq",NR(200.f,20000.f,1.f,0.4f), 4000.f));
    // Vibrato
    p.push_back(std::make_unique<juce::AudioParameterBool> ("vibOn",    "Vibrato",  false));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("vibRate",  "Vib Rate", NR(0.1f,15.f),   5.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("vibDepth", "Vib Depth",NR(0.f,3.f),     0.3f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("vibDelay", "Vib Delay",NR(0.f,2.f),     0.1f));
    // PWM
    p.push_back(std::make_unique<juce::AudioParameterBool> ("pwmOn",   "PWM",      false));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("pwmRate", "PWM Rate", NR(0.1f,8.f),    1.f));
    // Arp
    p.push_back(std::make_unique<juce::AudioParameterBool> ("arpOn",      "Arp",      false));
    p.push_back(std::make_unique<juce::AudioParameterInt>  ("arpPattern", "Arp Pat",  0,6,2));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("arpSpeed",   "Arp Speed",NR(4.f,32.f),  16.f));
    // FX
    p.push_back(std::make_unique<juce::AudioParameterFloat>("bitcrush",   "Bitcrush",  NR(0.f,8.f),  0.f));
    p.push_back(std::make_unique<juce::AudioParameterFloat>("saturation", "Saturation",NR(0.f,1.f),  0.f));

    return { p.begin(), p.end() };
}

ChiptuneVSTProcessor::ChiptuneVSTProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParams())
{
    const char* ids[] = {
        "channelType","duty","attack","decay","sustain","release","volume",
        "sweepOn","sweepRate","sweepUp","sweepAmt",
        "noiseShort","noiseFreq",
        "vibOn","vibRate","vibDepth","vibDelay",
        "pwmOn","pwmRate",
        "arpOn","arpPattern","arpSpeed",
        "bitcrush","saturation"
    };
    for (auto* id : ids) apvts.addParameterListener(id, this);
}

ChiptuneVSTProcessor::~ChiptuneVSTProcessor() {}

void ChiptuneVSTProcessor::prepareToPlay(double sr, int) {
    currentSampleRate = sr;
    for (auto& s : voiceSlots) s = {};
    arpStep = 0; arpPhaseAccum = 0.0; arpCurrentOffset = 0.0f;
}
void ChiptuneVSTProcessor::releaseResources() {}
void ChiptuneVSTProcessor::parameterChanged(const juce::String&, float) {}

// ── Preset loading ────────────────────────────────────────────────────────────
void ChiptuneVSTProcessor::loadPreset(int index)
{
    if (index < 0 || index >= NUM_PRESETS) return;
    const Preset& pr = PRESETS[index];

    auto set = [&](const char* id, float v) {
        if (auto* p = apvts.getParameter(id))
            p->setValueNotifyingHost(p->convertTo0to1(v));
    };
    auto setBool = [&](const char* id, bool v) { set(id, v ? 1.f : 0.f); };
    auto setInt  = [&](const char* id, int v)  { set(id, (float)v); };

    setInt  ("channelType", pr.channelType);
    setInt  ("duty",        pr.duty);
    set     ("attack",      pr.attack);
    set     ("decay",       pr.decay);
    set     ("sustain",     pr.sustain);
    set     ("release",     pr.release);
    set     ("volume",      pr.volume);
    setBool ("sweepOn",     pr.sweepOn);
    set     ("sweepRate",   pr.sweepRate);
    setBool ("sweepUp",     pr.sweepUp);
    set     ("sweepAmt",    pr.sweepAmt);
    setBool ("noiseShort",  pr.noiseShort);
    set     ("noiseFreq",   pr.noiseFreq);
    setBool ("vibOn",       pr.vibOn);
    set     ("vibRate",     pr.vibRate);
    set     ("vibDepth",    pr.vibDepth);
    set     ("vibDelay",    pr.vibDelay);
    setBool ("pwmOn",       pr.pwmOn);
    set     ("pwmRate",     pr.pwmRate);
    setBool ("arpOn",       pr.arpOn);
    setInt  ("arpPattern",  pr.arpPattern);
    set     ("arpSpeed",    pr.arpSpeed);
    set     ("bitcrush",    pr.bitcrush);
    set     ("saturation",  pr.saturation);

    currentPreset = index;
}

// ── Sync params to voice structs ──────────────────────────────────────────────
void ChiptuneVSTProcessor::syncParams()
{
    auto g  = [&](const char* id) -> float { return apvts.getRawParameterValue(id)->load(); };
    auto gb = [&](const char* id) { return g(id) > 0.5f; };
    auto gi = [&](const char* id) { return (int)g(id); };

    channelType         = gi("channelType");
    int   duty          = gi("duty");
    float atk           = g("attack");
    float dec           = g("decay");
    float sus           = g("sustain");
    float rel           = g("release");
    bool  vibOn         = gb("vibOn");
    float vibRate       = g("vibRate");
    float vibDepth      = g("vibDepth");
    float vibDelay      = g("vibDelay");
    bool  pwmOn         = gb("pwmOn");
    float pwmRate       = g("pwmRate");
    bool  sweepOn       = gb("sweepOn");
    float sweepRate     = g("sweepRate");
    bool  sweepUp       = gb("sweepUp");
    float sweepAmt      = g("sweepAmt");
    bool  noiseShort    = gb("noiseShort");
    float noiseFreq     = g("noiseFreq");

    for (auto& v : sq1Voices) {
        v.duty = (uint8_t)duty; v.envAttack = atk; v.envDecay = dec; v.sustain = sus; v.envRelease = rel;
        v.vibEnabled = vibOn; v.vibRate = vibRate; v.vibDepth = vibDepth; v.vibDelay = vibDelay;
        v.pwmEnabled = pwmOn; v.pwmRate = pwmRate;
        v.sweepEnabled = sweepOn; v.sweepRate = sweepRate; v.sweepUp = sweepUp; v.sweepAmount = sweepAmt;
    }
    for (auto& v : sq2Voices) {
        v.duty = (uint8_t)duty; v.envAttack = atk; v.envDecay = dec; v.sustain = sus; v.envRelease = rel;
        v.vibEnabled = vibOn; v.vibRate = vibRate; v.vibDepth = vibDepth; v.vibDelay = vibDelay;
        v.pwmEnabled = pwmOn; v.pwmRate = pwmRate;
        v.sweepEnabled = false;
    }
    for (auto& v : waveVoices) {
        v.waveTable = waveTable; v.envAttack = atk; v.envDecay = dec; v.sustain = sus; v.envRelease = rel;
        v.vibEnabled = vibOn; v.vibRate = vibRate; v.vibDepth = vibDepth; v.vibDelay = vibDelay;
    }
    for (auto& v : noiseVoices) {
        v.shortMode = noiseShort; v.noiseFreq = noiseFreq;
        v.envAttack = atk; v.envDecay = dec; v.sustain = sus; v.envRelease = rel;
    }
}

// ── MIDI handling ─────────────────────────────────────────────────────────────
void ChiptuneVSTProcessor::handleMidi(const juce::MidiMessage& msg, int)
{
    float masterVol = apvts.getRawParameterValue("volume")->load();

    if (msg.isNoteOn()) {
        int    note = msg.getNoteNumber();
        float  vel  = (float)msg.getVelocity() / 127.0f * masterVol;
        double hz   = juce::MidiMessage::getMidiNoteInHertz(note);

        // Find free slot, or steal oldest
        int slot = -1;
        for (int i = 0; i < MAX_VOICES; ++i) if (!voiceSlots[i].active) { slot = i; break; }
        if (slot < 0) slot = 0;

        voiceSlots[slot] = { note, true };
        switch (channelType) {
            case 0: sq1Voices[slot].noteOn(hz, vel);  break;
            case 1: sq2Voices[slot].noteOn(hz, vel);  break;
            case 2: waveVoices[slot].noteOn(hz, vel); break;
            case 3: noiseVoices[slot].noteOn(vel);    break;
        }
    }
    else if (msg.isNoteOff()) {
        int note = msg.getNoteNumber();
        for (int i = 0; i < MAX_VOICES; ++i) {
            if (voiceSlots[i].note == note && voiceSlots[i].active) {
                voiceSlots[i].active = false;
                sq1Voices[i].noteOff(); sq2Voices[i].noteOff();
                waveVoices[i].noteOff(); noiseVoices[i].noteOff();
            }
        }
    }
    else if (msg.isAllNotesOff()) {
        for (int i = 0; i < MAX_VOICES; ++i) {
            voiceSlots[i] = {};
            sq1Voices[i].noteOff(); sq2Voices[i].noteOff();
            waveVoices[i].noteOff(); noiseVoices[i].noteOff();
        }
    }
}

// ── FX ────────────────────────────────────────────────────────────────────────
static float applyBitcrush(float x, float reduction) {
    if (reduction < 0.05f) return x;
    float bits   = 16.0f - reduction;
    float levels = std::pow(2.0f, bits);
    return std::round(x * levels) / levels;
}

static float applySaturation(float x, float amt) {
    if (amt < 0.005f) return x;
    float drive = 1.0f + amt * 6.0f;
    return std::tanh(x * drive) / std::tanh(drive);
}

// ── processBlock ──────────────────────────────────────────────────────────────
void ChiptuneVSTProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                        juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    syncParams();
    buffer.clear();

    bool  arpOn      = apvts.getRawParameterValue("arpOn")->load()      > 0.5f;
    int   arpPattern = (int)apvts.getRawParameterValue("arpPattern")->load();
    float arpSpeed   = apvts.getRawParameterValue("arpSpeed")->load();
    float crush      = apvts.getRawParameterValue("bitcrush")->load();
    float sat        = apvts.getRawParameterValue("saturation")->load();

    auto* outL = buffer.getWritePointer(0);
    auto* outR = buffer.getWritePointer(1);
    int   numSamples = buffer.getNumSamples();

    int midiPos = 0;
    for (const auto meta : midi) {
        int evPos = meta.samplePosition;
        // process audio up to event
        for (int s = midiPos; s < evPos && s < numSamples; ++s) {
            // Advance arpeggiator
            if (arpOn) {
                arpPhaseAccum += arpSpeed / currentSampleRate;
                if (arpPhaseAccum >= 1.0) {
                    arpPhaseAccum -= 1.0;
                    arpStep = (arpStep + 1) % ARP_STEPS[arpPattern];
                }
                arpCurrentOffset = (float)ARP_TABLE[arpPattern][arpStep];
                for (int v = 0; v < MAX_VOICES; ++v) {
                    sq1Voices[v].arpOffset = arpCurrentOffset;
                    sq2Voices[v].arpOffset = arpCurrentOffset;
                    waveVoices[v].arpOffset = arpCurrentOffset;
                }
            } else {
                for (int v = 0; v < MAX_VOICES; ++v) {
                    sq1Voices[v].arpOffset = sq2Voices[v].arpOffset = waveVoices[v].arpOffset = 0.f;
                }
            }

            float sample = 0.0f;
            for (int v = 0; v < MAX_VOICES; ++v) {
                switch (channelType) {
                    case 0: sample += sq1Voices[v].process(currentSampleRate);  break;
                    case 1: sample += sq2Voices[v].process(currentSampleRate);  break;
                    case 2: sample += waveVoices[v].process(currentSampleRate); break;
                    case 3: sample += noiseVoices[v].process(currentSampleRate);break;
                }
            }
            sample *= 0.25f;
            sample  = applyBitcrush(sample, crush);
            sample  = applySaturation(sample, sat);

            outL[s] = outR[s] = sample;

            // Write to oscilloscope ring buffer
            int wp = oscWritePos.load(std::memory_order_relaxed);
            oscBuffer[(size_t)wp] = sample;
            oscWritePos.store((wp + 1) & (OSC_SIZE - 1), std::memory_order_relaxed);
        }
        midiPos = evPos;
        handleMidi(meta.getMessage(), evPos);
    }

    // remaining samples
    for (int s = midiPos; s < numSamples; ++s) {
        if (arpOn) {
            arpPhaseAccum += arpSpeed / currentSampleRate;
            if (arpPhaseAccum >= 1.0) {
                arpPhaseAccum -= 1.0;
                arpStep = (arpStep + 1) % ARP_STEPS[arpPattern];
            }
            arpCurrentOffset = (float)ARP_TABLE[arpPattern][arpStep];
            for (int v = 0; v < MAX_VOICES; ++v) {
                sq1Voices[v].arpOffset = arpCurrentOffset;
                sq2Voices[v].arpOffset = arpCurrentOffset;
                waveVoices[v].arpOffset = arpCurrentOffset;
            }
        } else {
            for (int v = 0; v < MAX_VOICES; ++v)
                sq1Voices[v].arpOffset = sq2Voices[v].arpOffset = waveVoices[v].arpOffset = 0.f;
        }

        float sample = 0.0f;
        for (int v = 0; v < MAX_VOICES; ++v) {
            switch (channelType) {
                case 0: sample += sq1Voices[v].process(currentSampleRate);  break;
                case 1: sample += sq2Voices[v].process(currentSampleRate);  break;
                case 2: sample += waveVoices[v].process(currentSampleRate); break;
                case 3: sample += noiseVoices[v].process(currentSampleRate);break;
            }
        }
        sample *= 0.25f;
        sample  = applyBitcrush(sample, crush);
        sample  = applySaturation(sample, sat);

        outL[s] = outR[s] = sample;

        int wp = oscWritePos.load(std::memory_order_relaxed);
        oscBuffer[(size_t)wp] = sample;
        oscWritePos.store((wp + 1) & (OSC_SIZE - 1), std::memory_order_relaxed);
    }
}

// ── State ─────────────────────────────────────────────────────────────────────
void ChiptuneVSTProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    juce::XmlElement xml("CTV2State");
    xml.addChildElement(apvts.copyState().createXml().release());
    auto* wt = xml.createNewChildElement("WaveTable");
    juce::String s;
    for (int i = 0; i < 32; ++i) s += juce::String(waveTable[(size_t)i]) + (i<31?",":"");
    wt->setAttribute("data", s);
    xml.setAttribute("preset", currentPreset);
    copyXmlToBinary(xml, dest);
}

void ChiptuneVSTProcessor::setStateInformation(const void* data, int size)
{
    auto xml = getXmlFromBinary(data, size);
    if (!xml || !xml->hasTagName("CTV2State")) return;
    if (auto* st = xml->getFirstChildElement()) apvts.replaceState(juce::ValueTree::fromXml(*st));
    if (auto* wt = xml->getChildByName("WaveTable")) {
        auto tk = juce::StringArray::fromTokens(wt->getStringAttribute("data"), ",", "");
        for (int i = 0; i < juce::jmin(32, tk.size()); ++i)
            waveTable[(size_t)i] = (uint8_t)tk[i].getIntValue();
    }
    currentPreset = xml->getIntAttribute("preset", -1);
}

juce::AudioProcessorEditor* ChiptuneVSTProcessor::createEditor()
{
    return new ChiptuneVSTEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChiptuneVSTProcessor();
}
