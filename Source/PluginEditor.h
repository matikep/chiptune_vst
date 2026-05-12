#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace GBC {
    const juce::Colour bg      { 0xFF07140F };
    const juce::Colour panel   { 0xFF0B2417 };
    const juce::Colour dark    { 0xFF1B4D34 };
    const juce::Colour mid     { 0xFF89B80D };
    const juce::Colour bright  { 0xFFC7F20A };
    const juce::Colour accent  { 0xFF00E5D4 };
    const juce::Colour hot     { 0xFFFF2D9A };
    const juce::Colour ink     { 0xFF020705 };
}

// ── Oscilloscope ──────────────────────────────────────────────────────────────
class OscilloscopeComponent : public juce::Component
{
public:
    OscilloscopeComponent(ChiptuneVSTProcessor& p);
    void paint(juce::Graphics&) override;
    void refresh();
private:
    ChiptuneVSTProcessor& proc;
    std::array<float, 256> displayBuffer = {};
};

// ── Wave table editor ─────────────────────────────────────────────────────────
class WaveTableEditor : public juce::Component
{
public:
    WaveTableEditor(std::array<uint8_t, 32>& table);
    void paint(juce::Graphics&) override;
    void mouseDown(const juce::MouseEvent&) override;
    void mouseDrag(const juce::MouseEvent&) override;
    std::function<void()> onChange;
private:
    std::array<uint8_t, 32>& waveTable;
    void setFromMouse(const juce::MouseEvent&);
};

// ── Look and feel ─────────────────────────────────────────────────────────────
class GBLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GBLookAndFeel();
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawToggleButton(juce::Graphics&,juce::ToggleButton&,bool,bool) override;
    juce::Font getLabelFont(juce::Label&) override;
    void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
    void drawPopupMenuBackground(juce::Graphics&,int,int) override;
};

// ── Main editor ───────────────────────────────────────────────────────────────
class ChiptuneVSTEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    ChiptuneVSTEditor(ChiptuneVSTProcessor&);
    ~ChiptuneVSTEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateChannelVisibility();
    void buildPresetBox();
    void drawSectionLabel(juce::Graphics&, juce::Rectangle<int>, const char*);

    ChiptuneVSTProcessor& proc;
    GBLookAndFeel gbLAF;

    // ── Preset bar ────────────────────────────────────────────────────────────
    juce::TextButton presetPrev, presetNext;
    juce::ComboBox   presetBox;

    // ── Channel buttons ───────────────────────────────────────────────────────
    juce::TextButton chBtn[4];

    // ── Oscilloscope ──────────────────────────────────────────────────────────
    std::unique_ptr<OscilloscopeComponent> osc;

    // ── Drum kit ──────────────────────────────────────────────────────────────
    juce::TextButton drumBtn[5];

    // ── ADSR + Vol ────────────────────────────────────────────────────────────
    juce::Slider atkSlider, decSlider, susSlider, relSlider, volSlider;
    juce::Label  atkLabel,  decLabel,  susLabel,  relLabel,  volLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        atkA, decA, susA, relA, volA;

    // ── Duty ─────────────────────────────────────────────────────────────────
    juce::TextButton dutyBtn[4];

    // ── Vibrato ───────────────────────────────────────────────────────────────
    juce::ToggleButton vibOnBtn;
    juce::Slider       vibRateSlider, vibDepthSlider, vibDelaySlider;
    juce::Label        vibRateLbl, vibDepthLbl, vibDelayLbl;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>
        vibRateA, vibDepthA, vibDelayA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> vibOnA;

    // ── PWM ───────────────────────────────────────────────────────────────────
    juce::ToggleButton pwmOnBtn;
    juce::Slider       pwmRateSlider;
    juce::Label        pwmRateLbl;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pwmRateA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> pwmOnA;

    // ── Sweep ─────────────────────────────────────────────────────────────────
    juce::ToggleButton sweepOnBtn, sweepUpBtn;
    juce::Slider       sweepRateSlider, sweepAmtSlider;
    juce::Label        sweepRateLbl, sweepAmtLbl;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sweepRateA, sweepAmtA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> sweepOnA, sweepUpA;

    // ── Wave editor ───────────────────────────────────────────────────────────
    std::unique_ptr<WaveTableEditor> waveEditor;

    // ── Noise ─────────────────────────────────────────────────────────────────
    juce::ToggleButton noiseShortBtn;
    juce::Slider       noiseFreqSlider;
    juce::Label        noiseFreqLbl;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseFreqA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> noiseShortA;

    // ── Arp ───────────────────────────────────────────────────────────────────
    juce::ToggleButton arpOnBtn;
    juce::ComboBox     arpPatternBox;
    juce::Slider       arpSpeedSlider;
    juce::Label        arpSpeedLbl;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> arpSpeedA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpOnA;

    // ── FX ────────────────────────────────────────────────────────────────────
    juce::Slider crushSlider, satSlider;
    juce::Label  crushLbl, satLbl;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> crushA, satA;

    // ── Footer link ───────────────────────────────────────────────────────────
    juce::HyperlinkButton shoushiLink;

    // ── Layout constant ───────────────────────────────────────────────────────
    static constexpr int arpY = 604;  // y-start of ARP section header

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChiptuneVSTEditor)
};
