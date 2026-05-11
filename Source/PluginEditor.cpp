#include "PluginEditor.h"
#include <cmath>

using APVTS = juce::AudioProcessorValueTreeState;

// ─────────────────────────────────────────────────────────────────────────────
// OscilloscopeComponent
// ─────────────────────────────────────────────────────────────────────────────
OscilloscopeComponent::OscilloscopeComponent(ChiptuneVSTProcessor& p) : proc(p) {}

void OscilloscopeComponent::refresh()
{
    int wp = proc.oscWritePos.load(std::memory_order_relaxed);
    int w  = (int)displayBuffer.size();
    for (int i = 0; i < w; ++i) {
        int idx = (wp - w + i + OSC_SIZE) & (OSC_SIZE - 1);
        displayBuffer[(size_t)i] = proc.oscBuffer[(size_t)idx];
    }
    repaint();
}

void OscilloscopeComponent::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    g.fillAll(GBC::ink);
    g.setColour(GBC::dark);
    g.drawRect(b, 1.0f);
    g.setColour(GBC::dark.withAlpha(0.55f));
    for (int i = 1; i < 6; ++i) {
        float x = b.getX() + b.getWidth() * (float)i / 6.0f;
        g.drawVerticalLine((int)x, b.getY(), b.getBottom());
    }
    g.drawHorizontalLine((int)b.getCentreY(), b.getX(), b.getRight());

    int w = (int)displayBuffer.size();
    if (w < 2) return;
    juce::Path path;
    for (int i = 0; i < w; ++i) {
        float x = b.getX() + (float)i / (float)(w - 1) * b.getWidth();
        float y = b.getCentreY() - displayBuffer[(size_t)i] * b.getHeight() * 0.43f;
        y = std::clamp(y, b.getY() + 1.f, b.getBottom() - 1.f);
        if (i == 0) path.startNewSubPath(x, y); else path.lineTo(x, y);
    }
    g.setColour(GBC::accent.withAlpha(0.32f));
    g.strokePath(path, juce::PathStrokeType(5.0f));
    g.setColour(GBC::bright);
    g.strokePath(path, juce::PathStrokeType(1.5f));
}

// ─────────────────────────────────────────────────────────────────────────────
// WaveTableEditor
// ─────────────────────────────────────────────────────────────────────────────
WaveTableEditor::WaveTableEditor(std::array<uint8_t, 32>& t) : waveTable(t) {}

void WaveTableEditor::setFromMouse(const juce::MouseEvent& e)
{
    auto b   = getLocalBounds().toFloat();
    float bw = b.getWidth() / 32.0f;
    int idx  = (int)(e.position.x / bw);
    if (idx < 0 || idx >= 32) return;
    float norm = 1.0f - std::clamp(e.position.y / b.getHeight(), 0.0f, 1.0f);
    waveTable[(size_t)idx] = (uint8_t)(norm * 15.0f + 0.5f);
    repaint();
    if (onChange) onChange();
}

void WaveTableEditor::mouseDown(const juce::MouseEvent& e) { setFromMouse(e); }
void WaveTableEditor::mouseDrag(const juce::MouseEvent& e) { setFromMouse(e); }

void WaveTableEditor::paint(juce::Graphics& g)
{
    auto b  = getLocalBounds().toFloat();
    g.fillAll(GBC::ink);
    g.setColour(GBC::dark);
    g.drawRect(b, 1.0f);
    float bw = b.getWidth() / 32.0f;
    for (int i = 0; i < 32; ++i) {
        float h = (waveTable[(size_t)i] / 15.0f) * b.getHeight();
        g.setColour(i % 2 == 0 ? GBC::accent : GBC::bright);
        g.fillRect(i * bw + 1.0f, b.getHeight() - h, bw - 1.5f, h);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GBLookAndFeel
// ─────────────────────────────────────────────────────────────────────────────
GBLookAndFeel::GBLookAndFeel()
{
    setColour(juce::Slider::thumbColourId,                GBC::accent);
    setColour(juce::Slider::rotarySliderFillColourId,     GBC::bright);
    setColour(juce::Slider::rotarySliderOutlineColourId,  GBC::dark);
    setColour(juce::Slider::textBoxTextColourId,          GBC::accent);
    setColour(juce::Slider::textBoxBackgroundColourId,    GBC::ink);
    setColour(juce::Slider::textBoxOutlineColourId,       GBC::dark);
    setColour(juce::Label::textColourId,                  GBC::bright);
    setColour(juce::TextButton::buttonColourId,           GBC::panel);
    setColour(juce::TextButton::buttonOnColourId,         GBC::dark);
    setColour(juce::TextButton::textColourOffId,          GBC::accent);
    setColour(juce::TextButton::textColourOnId,           GBC::bright);
    setColour(juce::ToggleButton::textColourId,           GBC::bright);
    setColour(juce::ToggleButton::tickColourId,           GBC::bright);
    setColour(juce::ToggleButton::tickDisabledColourId,   GBC::dark);
    setColour(juce::ComboBox::backgroundColourId,         GBC::ink);
    setColour(juce::ComboBox::outlineColourId,            GBC::dark);
    setColour(juce::ComboBox::textColourId,               GBC::bright);
    setColour(juce::ComboBox::arrowColourId,              GBC::mid);
    setColour(juce::PopupMenu::backgroundColourId,        GBC::ink);
    setColour(juce::PopupMenu::textColourId,              GBC::bright);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, GBC::dark);
    setColour(juce::PopupMenu::highlightedTextColourId,   GBC::bright);
}

void GBLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int w, int h, float pos, float rotStart, float rotEnd, juce::Slider&)
{
    float rad = juce::jmin(w, h) * 0.40f;
    float cx  = x + w * 0.5f, cy = y + h * 0.5f;

    g.setColour(GBC::ink);
    g.fillEllipse(cx-rad, cy-rad, rad*2, rad*2);
    g.setColour(GBC::dark);
    g.drawEllipse(cx-rad, cy-rad, rad*2, rad*2, 2.f);

    // Track arc
    juce::Path track;
    track.addArc(cx-rad+2, cy-rad+2, (rad-2)*2, (rad-2)*2, rotStart, rotEnd, true);
    g.setColour(GBC::dark.brighter(0.15f));
    g.strokePath(track, juce::PathStrokeType(2.f));

    // Value arc
    float angle = rotStart + pos * (rotEnd - rotStart);
    juce::Path val;
    val.addArc(cx-rad+2, cy-rad+2, (rad-2)*2, (rad-2)*2, rotStart, angle, true);
    g.setColour(GBC::accent.withAlpha(0.28f));
    g.strokePath(val, juce::PathStrokeType(5.0f));
    g.setColour(GBC::bright);
    g.strokePath(val, juce::PathStrokeType(2.0f));

    // Pointer dot
    float px = cx + (rad-5) * std::cos(angle - juce::MathConstants<float>::halfPi);
    float py = cy + (rad-5) * std::sin(angle - juce::MathConstants<float>::halfPi);
    g.setColour(GBC::hot);
    g.fillEllipse(px-3, py-3, 6, 6);
}

void GBLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& btn,
    const juce::Colour&, bool hl, bool dn)
{
    auto b = btn.getLocalBounds().toFloat().reduced(1.f);
    g.setColour(btn.getToggleState() ? GBC::dark : (hl||dn ? GBC::dark.brighter(0.1f) : GBC::panel));
    g.fillRoundedRectangle(b, 3.f);
    g.setColour(btn.getToggleState() ? GBC::bright : GBC::dark);
    g.drawRoundedRectangle(b, 3.f, 1.5f);
}

void GBLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& btn, bool, bool)
{
    auto b = btn.getLocalBounds().toFloat().reduced(1.f);
    bool on = btn.getToggleState();
    g.setColour(on ? GBC::dark : GBC::panel);
    g.fillRoundedRectangle(b, 3.f);
    g.setColour(on ? GBC::hot : GBC::dark);
    g.drawRoundedRectangle(b, 3.f, 1.5f);
    g.setFont(juce::Font(juce::FontOptions().withHeight(9.5f)));
    g.setColour(on ? GBC::bright : GBC::accent);
    g.drawText(btn.getButtonText(), btn.getLocalBounds(), juce::Justification::centred);
}

juce::Font GBLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(juce::FontOptions().withHeight(9.5f));
}

void GBLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool,
    int,int,int,int, juce::ComboBox&)
{
    g.fillAll(GBC::ink);
    g.setColour(GBC::dark);
    g.drawRect(0, 0, w, h, 1);
    juce::Path arrow;
    arrow.addTriangle((float)w-12, (float)h*0.35f, (float)w-4, (float)h*0.35f,
                      (float)w-8, (float)h*0.65f);
    g.setColour(GBC::hot);
    g.fillPath(arrow);
}

void GBLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int w, int h)
{
    g.fillAll(GBC::ink);
    g.setColour(GBC::dark);
    g.drawRect(0, 0, w, h, 1);
}

// ─────────────────────────────────────────────────────────────────────────────
// Helper — creates a knob slider without attaching label to component
// ─────────────────────────────────────────────────────────────────────────────
static void makeKnob(juce::Slider& s, juce::Label& lbl,
                     const char* text, juce::Component* parent)
{
    s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 54, 13);
    s.setNumDecimalPlacesToDisplay(3);
    s.textFromValueFunction = [](double v) {
        if (std::abs(v) >= 100.0)
            return juce::String((int)std::round(v));
        if (std::abs(v) >= 10.0)
            return juce::String(v, 1);
        if (std::abs(v) >= 1.0)
            return juce::String(v, 2);
        return juce::String(v, 3);
    };
    parent->addAndMakeVisible(s);

    lbl.setText(text, juce::dontSendNotification);
    lbl.setFont(juce::Font(juce::FontOptions().withHeight(10.5f).withStyle("Bold")));
    lbl.setJustificationType(juce::Justification::centred);
    parent->addAndMakeVisible(lbl);
    // NOTE: no attachToComponent — caller positions label manually
}

// Helper — place one knob block (label above, slider below)
static void placeKnob(juce::Slider& s, juce::Label& lbl,
                      int x, int y, int kW, int kH, int lH)
{
    lbl.setBounds(x, y,       kW, lH);
    s  .setBounds(x, y + lH, kW, kH);
}

// ─────────────────────────────────────────────────────────────────────────────
// ChiptuneVSTEditor
// ─────────────────────────────────────────────────────────────────────────────
ChiptuneVSTEditor::ChiptuneVSTEditor(ChiptuneVSTProcessor& p)
    : AudioProcessorEditor(&p), proc(p)
{
    setLookAndFeel(&gbLAF);
    auto& apvts = proc.apvts;

    // Oscilloscope
    osc = std::make_unique<OscilloscopeComponent>(proc);
    addAndMakeVisible(*osc);

    // Preset bar
    presetPrev.setButtonText("<");
    presetNext.setButtonText(">");
    presetPrev.onClick = [this] {
        int n = proc.currentPreset <= 0 ? NUM_PRESETS-1 : proc.currentPreset-1;
        proc.loadPreset(n);
        presetBox.setSelectedItemIndex(n, juce::dontSendNotification);
    };
    presetNext.onClick = [this] {
        int n = (proc.currentPreset+1) % NUM_PRESETS;
        proc.loadPreset(n);
        presetBox.setSelectedItemIndex(n, juce::dontSendNotification);
    };
    addAndMakeVisible(presetPrev);
    addAndMakeVisible(presetNext);
    buildPresetBox();
    presetBox.onChange = [this] {
        int idx = presetBox.getSelectedItemIndex();
        if (idx >= 0) proc.loadPreset(idx);
    };
    addAndMakeVisible(presetBox);

    // Channel buttons
    const char* chNames[] = { "CH1  SQ+SWP","CH2  SQUARE","CH3  WAVE","CH4  NOISE" };
    for (int i = 0; i < 4; ++i) {
        chBtn[i].setButtonText(chNames[i]);
        chBtn[i].onClick = [this, i, &apvts] {
            apvts.getParameter("channelType")->setValueNotifyingHost(
                apvts.getParameter("channelType")->convertTo0to1((float)i));
            updateChannelVisibility();
        };
        addAndMakeVisible(chBtn[i]);
    }

    // ADSR + Vol
    makeKnob(atkSlider, atkLabel, "ATK", this);
    makeKnob(decSlider, decLabel, "DEC", this);
    makeKnob(susSlider, susLabel, "SUS", this);
    makeKnob(relSlider, relLabel, "REL", this);
    makeKnob(volSlider, volLabel, "VOL", this);
    atkA = std::make_unique<APVTS::SliderAttachment>(apvts, "attack",  atkSlider);
    decA = std::make_unique<APVTS::SliderAttachment>(apvts, "decay",   decSlider);
    susA = std::make_unique<APVTS::SliderAttachment>(apvts, "sustain", susSlider);
    relA = std::make_unique<APVTS::SliderAttachment>(apvts, "release", relSlider);
    volA = std::make_unique<APVTS::SliderAttachment>(apvts, "volume",  volSlider);

    // Duty
    const char* dutyNames[] = { "12.5%","25%","50%","75%" };
    for (int i = 0; i < 4; ++i) {
        dutyBtn[i].setButtonText(dutyNames[i]);
        dutyBtn[i].onClick = [i, &apvts] {
            apvts.getParameter("duty")->setValueNotifyingHost(
                apvts.getParameter("duty")->convertTo0to1((float)i));
        };
        addAndMakeVisible(dutyBtn[i]);
    }

    // Vibrato
    vibOnBtn.setButtonText("VIB ON");
    vibOnA = std::make_unique<APVTS::ButtonAttachment>(apvts, "vibOn", vibOnBtn);
    addAndMakeVisible(vibOnBtn);
    makeKnob(vibRateSlider, vibRateLbl,   "RATE",  this);
    makeKnob(vibDepthSlider, vibDepthLbl, "DEPTH", this);
    makeKnob(vibDelaySlider, vibDelayLbl, "DELAY", this);
    vibRateA  = std::make_unique<APVTS::SliderAttachment>(apvts, "vibRate",  vibRateSlider);
    vibDepthA = std::make_unique<APVTS::SliderAttachment>(apvts, "vibDepth", vibDepthSlider);
    vibDelayA = std::make_unique<APVTS::SliderAttachment>(apvts, "vibDelay", vibDelaySlider);

    // PWM
    pwmOnBtn.setButtonText("PWM");
    pwmOnA = std::make_unique<APVTS::ButtonAttachment>(apvts, "pwmOn", pwmOnBtn);
    addAndMakeVisible(pwmOnBtn);
    makeKnob(pwmRateSlider, pwmRateLbl, "RATE", this);
    pwmRateA = std::make_unique<APVTS::SliderAttachment>(apvts, "pwmRate", pwmRateSlider);

    // Sweep
    sweepOnBtn.setButtonText("SWP ON");
    sweepUpBtn.setButtonText("UP");
    sweepOnA = std::make_unique<APVTS::ButtonAttachment>(apvts, "sweepOn", sweepOnBtn);
    sweepUpA = std::make_unique<APVTS::ButtonAttachment>(apvts, "sweepUp", sweepUpBtn);
    addAndMakeVisible(sweepOnBtn);
    addAndMakeVisible(sweepUpBtn);
    makeKnob(sweepRateSlider, sweepRateLbl, "RATE", this);
    makeKnob(sweepAmtSlider,  sweepAmtLbl,  "SEMI", this);
    sweepRateA = std::make_unique<APVTS::SliderAttachment>(apvts, "sweepRate", sweepRateSlider);
    sweepAmtA  = std::make_unique<APVTS::SliderAttachment>(apvts, "sweepAmt",  sweepAmtSlider);

    // Wave editor
    waveEditor = std::make_unique<WaveTableEditor>(proc.waveTable);
    addAndMakeVisible(*waveEditor);

    // Noise
    noiseShortBtn.setButtonText("7-BIT");
    noiseShortA = std::make_unique<APVTS::ButtonAttachment>(apvts, "noiseShort", noiseShortBtn);
    addAndMakeVisible(noiseShortBtn);
    makeKnob(noiseFreqSlider, noiseFreqLbl, "FREQ", this);
    noiseFreqA = std::make_unique<APVTS::SliderAttachment>(apvts, "noiseFreq", noiseFreqSlider);

    // Arp
    arpOnBtn.setButtonText("ARP ON");
    arpOnA = std::make_unique<APVTS::ButtonAttachment>(apvts, "arpOn", arpOnBtn);
    addAndMakeVisible(arpOnBtn);
    for (int i = 0; i < 7; ++i)
        arpPatternBox.addItem(ARP_NAMES[i], i+1);
    arpPatternBox.onChange = [this, &apvts] {
        int idx = arpPatternBox.getSelectedItemIndex();
        apvts.getParameter("arpPattern")->setValueNotifyingHost(
            apvts.getParameter("arpPattern")->convertTo0to1((float)idx));
    };
    addAndMakeVisible(arpPatternBox);
    makeKnob(arpSpeedSlider, arpSpeedLbl, "SPEED", this);
    arpSpeedA = std::make_unique<APVTS::SliderAttachment>(apvts, "arpSpeed", arpSpeedSlider);

    // FX
    makeKnob(crushSlider, crushLbl, "CRUSH", this);
    makeKnob(satSlider,   satLbl,   "SAT",   this);
    crushA = std::make_unique<APVTS::SliderAttachment>(apvts, "bitcrush",   crushSlider);
    satA   = std::make_unique<APVTS::SliderAttachment>(apvts, "saturation", satSlider);

    // Spotify footer link
    shoushiLink.setButtonText("shoushi bit");
    shoushiLink.setURL(juce::URL("https://open.spotify.com/intl-es/artist/0ShkJFfceqTKxlBpenaATY"));
    shoushiLink.setColour(juce::HyperlinkButton::textColourId, GBC::accent);
    addAndMakeVisible(shoushiLink);

    updateChannelVisibility();
    startTimerHz(20);
    setSize(640, 740);
}

ChiptuneVSTEditor::~ChiptuneVSTEditor() { setLookAndFeel(nullptr); }

void ChiptuneVSTEditor::buildPresetBox()
{
    presetBox.clear(juce::dontSendNotification);
    const char* lastCat = "";
    for (int i = 0; i < NUM_PRESETS; ++i) {
        if (std::strcmp(PRESETS[i].category, lastCat) != 0) {
            presetBox.addSectionHeading(PRESETS[i].category);
            lastCat = PRESETS[i].category;
        }
        presetBox.addItem(PRESETS[i].name, i+1);
    }
    if (proc.currentPreset >= 0)
        presetBox.setSelectedItemIndex(proc.currentPreset, juce::dontSendNotification);
}

void ChiptuneVSTEditor::timerCallback()
{
    osc->refresh();

    int duty = (int)*proc.apvts.getRawParameterValue("duty");
    for (int i = 0; i < 4; ++i)
        dutyBtn[i].setToggleState(i == duty, juce::dontSendNotification);

    int ch = (int)*proc.apvts.getRawParameterValue("channelType");
    for (int i = 0; i < 4; ++i)
        chBtn[i].setToggleState(i == ch, juce::dontSendNotification);

    int arpPat = (int)*proc.apvts.getRawParameterValue("arpPattern");
    arpPatternBox.setSelectedItemIndex(arpPat, juce::dontSendNotification);

    if (proc.currentPreset >= 0)
        presetBox.setSelectedItemIndex(proc.currentPreset, juce::dontSendNotification);
}

void ChiptuneVSTEditor::updateChannelVisibility()
{
    int ch     = (int)*proc.apvts.getRawParameterValue("channelType");
    bool isSq  = (ch == 0 || ch == 1);
    bool isSq1 = (ch == 0);
    bool isWave= (ch == 2);
    bool isNoise=(ch == 3);

    for (auto& b : dutyBtn) b.setVisible(isSq);

    bool showVib = isSq;  // vibrato only shown for square channels (CH1/CH2)
    vibOnBtn.setVisible(showVib);
    vibRateSlider.setVisible(showVib);  vibRateLbl.setVisible(showVib);
    vibDepthSlider.setVisible(showVib); vibDepthLbl.setVisible(showVib);
    vibDelaySlider.setVisible(showVib); vibDelayLbl.setVisible(showVib);

    pwmOnBtn.setVisible(isSq);
    pwmRateSlider.setVisible(isSq); pwmRateLbl.setVisible(isSq);

    sweepOnBtn.setVisible(isSq1);
    sweepUpBtn.setVisible(isSq1);
    sweepRateSlider.setVisible(isSq1); sweepRateLbl.setVisible(isSq1);
    sweepAmtSlider.setVisible(isSq1);  sweepAmtLbl.setVisible(isSq1);

    waveEditor->setVisible(isWave);

    noiseShortBtn.setVisible(isNoise);
    noiseFreqSlider.setVisible(isNoise); noiseFreqLbl.setVisible(isNoise);

    resized();
    repaint();
}

// ─────────────────────────────────────────────────────────────────────────────
// Paint — draws section headers and background elements only
// ─────────────────────────────────────────────────────────────────────────────
void ChiptuneVSTEditor::paint(juce::Graphics& g)
{
    g.fillAll(GBC::bg);

    auto hdr = [&](int y, int h, const char* txt) {
        g.setColour(GBC::dark.withAlpha(0.82f));
        g.fillRect(0, y, getWidth(), h);
        g.setColour(GBC::hot.withAlpha(0.85f));
        g.drawLine(0.0f, (float)y, (float)getWidth(), (float)y, 1.0f);
        g.setColour(GBC::bright);
        g.setFont(juce::Font(juce::FontOptions().withHeight(10.5f).withStyle("Bold")));
        g.drawText(txt, 14, y, getWidth()-28, h, juce::Justification::centredLeft);
    };

    // Header bar
    g.setColour(GBC::ink);
    g.fillRect(0, 0, getWidth(), 38);
    g.setColour(GBC::dark.withAlpha(0.7f));
    for (int x = 0; x < getWidth(); x += 16)
        g.drawVerticalLine(x, 0, 38);
    g.setColour(GBC::hot);
    g.drawLine(0, 37, (float)getWidth(), 37, 1.5f);
    g.setColour(GBC::accent.withAlpha(0.55f));
    g.drawLine(0, 34, (float)getWidth(), 34, 1.0f);
    g.setColour(GBC::bright);
    g.setFont(juce::Font(juce::FontOptions().withHeight(17.f).withStyle("Bold")));
    g.drawText("CHIPTUNE VST", 0, 2, getWidth(), 22, juce::Justification::centred);
    g.setColour(GBC::accent);
    g.setFont(juce::Font(juce::FontOptions().withHeight(9.5f)));
    g.drawText("DMG-01 / CYBERDECK", 0, 21, getWidth(), 13, juce::Justification::centred);

    hdr(44,  16, "PRESET");
    hdr(110, 16, "CHANNEL");
    hdr(162, 16, "OSCILLOSCOPE");
    hdr(246, 16, "ENVELOPE + VOLUME");

    int ch = (int)*proc.apvts.getRawParameterValue("channelType");

    if (ch == 0 || ch == 1) {
        hdr(360, 16, "DUTY CYCLE");
        hdr(416, 16, ch == 0 ? "VIBRATO / PWM / SWEEP" : "VIBRATO / PWM");
    }
    if (ch == 2) hdr(360, 16, "WAVE TABLE - DRAW WITH MOUSE");
    if (ch == 3) hdr(360, 16, "NOISE CHANNEL");

    hdr(arpY,    16, "ARPEGGIATOR");
    hdr(arpY+94, 16, "FX - BITCRUSH / SATURATION");

    // Footer
    int fy = getHeight() - 24;
    g.setColour(GBC::ink);
    g.fillRect(0, fy, getWidth(), 24);
    g.setColour(GBC::hot);
    g.drawLine(0.f, (float)fy, (float)getWidth(), (float)fy, 1.f);
    g.setColour(GBC::mid);
    g.setFont(juce::Font(juce::FontOptions().withHeight(10.0f)));
    g.drawText("powered by mkep.dev /", 14, fy, 160, 24, juce::Justification::centredLeft);
}

// ─────────────────────────────────────────────────────────────────────────────
// Resized — all explicit positions, NO attachToComponent
// ─────────────────────────────────────────────────────────────────────────────
void ChiptuneVSTEditor::resized()
{
    const int W   = getWidth();
    const int kW  = 58;   // knob slot width
    const int kH  = 64;   // slider height (knob dial + textbox)
    const int lH  = 14;   // label height
    const int bH  = 28;   // button height
    const int G   = 10;   // gap between knob slots

    // ── Preset bar ────────────────────────────────────────────────────────────
    presetPrev.setBounds(14,   66, 36, 30);
    presetBox .setBounds(58,   66, W-116, 30);
    presetNext.setBounds(W-50, 66, 36, 30);

    // ── Channel buttons ────────────────────────────────────────────────────────
    int cw = (W-34)/4;
    for (int i = 0; i < 4; ++i)
        chBtn[i].setBounds(14 + i*(cw+2), 132, cw-1, 30);

    // ── Oscilloscope ───────────────────────────────────────────────────────────
    osc->setBounds(14, 190, W-28, 42);

    // ── ADSR + Vol ─────────────────────────────────────────────────────────────
    int envLy = 274;
    placeKnob(atkSlider, atkLabel, 14 + 0*(kW+G), envLy, kW, kH, lH);
    placeKnob(decSlider, decLabel, 14 + 1*(kW+G), envLy, kW, kH, lH);
    placeKnob(susSlider, susLabel, 14 + 2*(kW+G), envLy, kW, kH, lH);
    placeKnob(relSlider, relLabel, 14 + 3*(kW+G), envLy, kW, kH, lH);
    placeKnob(volSlider, volLabel, W-kW-14,        envLy, kW, kH, lH);

    // ── Channel-specific content ──────────────────────────────────────────────
    int ch   = (int)*proc.apvts.getRawParameterValue("channelType");
    int chY1 = 384;

    if (ch == 0 || ch == 1) {
        // Duty buttons
        int dw = (W-34)/4;
        for (int i = 0; i < 4; ++i)
            dutyBtn[i].setBounds(14 + i*(dw+2), chY1, dw-1, bH);

        // VIB / PWM / SWEEP row
        int vy = 440;
        int cx = 14;
        int vcy = vy + (kH - bH) / 2;  // vertical centre for buttons

        vibOnBtn.setBounds(cx, vcy, 68, bH);              cx += 74;
        placeKnob(vibRateSlider,  vibRateLbl,  cx, vy, kW, kH, lH); cx += kW + 6;
        placeKnob(vibDepthSlider, vibDepthLbl, cx, vy, kW, kH, lH); cx += kW + 6;
        placeKnob(vibDelaySlider, vibDelayLbl, cx, vy, kW, kH, lH); cx += kW + 10;

        pwmOnBtn.setBounds(cx, vcy, 54, bH);              cx += 60;
        placeKnob(pwmRateSlider, pwmRateLbl, cx, vy, kW, kH, lH);   cx += kW + 10;

        if (ch == 0) {
            sweepOnBtn.setBounds(cx, vcy, 72, bH);        cx += 78;
            sweepUpBtn.setBounds(cx, vcy, 40, bH);        cx += 46;
            placeKnob(sweepRateSlider, sweepRateLbl, cx, vy, kW, kH, lH); cx += kW + 6;
            placeKnob(sweepAmtSlider,  sweepAmtLbl,  cx, vy, kW, kH, lH);
        }
    }

    if (ch == 2) {
        waveEditor->setBounds(14, chY1, W-28, 118);
    }

    if (ch == 3) {
        int ny = chY1;
        noiseShortBtn.setBounds(14, ny + (kH-bH)/2, 72, bH);
        placeKnob(noiseFreqSlider, noiseFreqLbl, 100, ny, kW, kH, lH);
    }

    // ── Arpeggiator ──────────────────────────────────────────────────────────
    int ay = arpY + 22;
    arpOnBtn.setBounds(14, ay + (kH-bH)/2, 72, bH);
    arpPatternBox.setBounds(96, ay + (kH-bH)/2, 140, bH);
    placeKnob(arpSpeedSlider, arpSpeedLbl, 250, ay, kW, kH, lH);

    // ── FX ───────────────────────────────────────────────────────────────────
    int fy2 = arpY + 116;
    placeKnob(crushSlider, crushLbl, 14,       fy2, kW, kH, lH);
    placeKnob(satSlider,   satLbl,   14+kW+G,  fy2, kW, kH, lH);

    // Spotify footer link
    shoushiLink.setBounds(174, getHeight()-24, 110, 24);
}
