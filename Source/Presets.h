#pragma once

struct Preset {
    const char* name;
    const char* category;
    int   channelType;
    int   duty;
    float attack, decay, sustain, release, volume;
    bool  sweepOn;   float sweepRate;  bool sweepUp;  float sweepAmt;
    bool  noiseShort; float noiseFreq;
    bool  vibOn;  float vibRate, vibDepth, vibDelay;
    bool  pwmOn;  float pwmRate;
    bool  arpOn;  int arpPattern;  float arpSpeed;
    float bitcrush;   // 0=off, 1-8 bits of reduction
    float saturation; // 0-1
};

// arpPattern: 0=Octave  1=Power(0,7)  2=Major(0,4,7)  3=Minor(0,3,7)
//             4=Maj7(0,4,7,11)  5=Dim(0,3,6)  6=Sus4(0,5,7)
static const char* const ARP_NAMES[] = { "Oct","Power","Major","Minor","Maj7","Dim","Sus4" };

static const int ARP_TABLE[7][8] = {
    { 0,12, 0,12, 0,12, 0,12 },
    { 0, 7, 0, 7, 0, 7, 0, 7 },
    { 0, 4, 7, 4, 0, 4, 7, 4 },
    { 0, 3, 7, 3, 0, 3, 7, 3 },
    { 0, 4, 7,11, 0, 4, 7,11 },
    { 0, 3, 6, 3, 0, 3, 6, 3 },
    { 0, 5, 7, 5, 0, 5, 7, 5 },
};
static const int ARP_STEPS[7] = { 2,2,3,3,4,3,3 };

static const Preset PRESETS[] = {
// ── TREY FREY ────────────────────────────────────────────────────────────────
{ "Soft Pulse Lead",  "TREY FREY", 0,1, 0.005f,0.08f,0.75f,0.15f,0.70f,
  false,2.f,true,1.f, false,4000.f, true, 4.5f,0.25f,0.08f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Warm Lead",        "TREY FREY", 0,2, 0.008f,0.10f,0.70f,0.20f,0.70f,
  false,2.f,true,1.f, false,4000.f, true, 3.5f,0.30f,0.12f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Emotional Pulse",  "TREY FREY", 0,1, 0.012f,0.12f,0.65f,0.30f,0.65f,
  false,2.f,true,1.f, false,4000.f, true, 5.0f,0.40f,0.15f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Warm Wave Bass",   "TREY FREY", 2,2, 0.005f,0.15f,0.80f,0.20f,0.75f,
  false,2.f,true,1.f, false,4000.f, true, 3.0f,0.15f,0.20f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Harmonic Lead",    "TREY FREY", 1,1, 0.006f,0.09f,0.72f,0.18f,0.65f,
  false,2.f,true,1.f, false,4000.f, true, 4.0f,0.22f,0.10f, false,1.f, false,2,16.f, 0.f,0.00f },

// ── LSDJ ─────────────────────────────────────────────────────────────────────
{ "Hyper Arp",        "LSDJ",      0,0, 0.001f,0.04f,0.00f,0.03f,0.80f,
  true, 3.f,true,7.f, false,4000.f, false,5.f,0.00f,0.00f, false,1.f, true, 2,20.f, 0.f,0.10f },
{ "Arp Bass",         "LSDJ",      1,2, 0.001f,0.05f,0.00f,0.03f,0.80f,
  false,2.f,true,1.f, false,4000.f, false,5.f,0.00f,0.00f, false,1.f, true, 0,16.f, 0.f,0.10f },
{ "Tracker Lead",     "LSDJ",      0,0, 0.001f,0.06f,0.00f,0.04f,0.85f,
  false,2.f,true,1.f, false,4000.f, false,5.f,0.00f,0.00f, false,1.f, true, 1,24.f, 0.f,0.00f },
{ "Power Chord",      "LSDJ",      0,2, 0.001f,0.05f,0.00f,0.03f,0.80f,
  false,2.f,true,1.f, false,4000.f, false,5.f,0.00f,0.00f, false,1.f, true, 1,16.f, 0.f,0.20f },
{ "LSDj Noise Kit",   "LSDJ",      3,0, 0.001f,0.08f,0.00f,0.05f,0.85f,
  false,2.f,true,1.f, false,5000.f, false,5.f,0.00f,0.00f, false,1.f, false,2,16.f, 0.f,0.10f },

// ── FEZ / DISASTERPEACE ───────────────────────────────────────────────────────
{ "Hollow Lead",      "FEZ",       0,0, 0.020f,0.15f,0.60f,0.40f,0.65f,
  false,2.f,true,1.f, false,4000.f, true, 2.0f,0.15f,0.30f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Ambient Wave Pad", "FEZ",       2,2, 0.250f,0.30f,0.85f,0.60f,0.60f,
  false,2.f,true,1.f, false,4000.f, true, 1.5f,0.10f,0.40f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Soft Sine",        "FEZ",       2,2, 0.150f,0.20f,0.80f,0.50f,0.60f,
  false,2.f,true,1.f, false,4000.f, true, 2.5f,0.20f,0.25f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Minimal Pulse",    "FEZ",       0,2, 0.010f,0.10f,0.55f,0.35f,0.55f,
  false,2.f,true,1.f, false,4000.f, true, 1.8f,0.12f,0.20f, false,1.f, false,2,16.f, 0.f,0.00f },

// ── POKEMON GB ────────────────────────────────────────────────────────────────
{ "Pokemon Lead",     "POKEMON",   0,1, 0.005f,0.08f,0.70f,0.10f,0.75f,
  false,2.f,true,1.f, false,4000.f, true, 6.0f,0.35f,0.05f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Pokemon Bass",     "POKEMON",   1,2, 0.003f,0.06f,0.65f,0.08f,0.75f,
  false,2.f,true,1.f, false,4000.f, true, 6.0f,0.20f,0.05f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Pokemon SFX",      "POKEMON",   0,0, 0.001f,0.10f,0.00f,0.05f,0.80f,
  true, 8.f,true,5.f, false,4000.f, false,5.f,0.00f,0.00f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Pokemon Wave",     "POKEMON",   2,2, 0.005f,0.08f,0.75f,0.12f,0.70f,
  false,2.f,true,1.f, false,4000.f, true, 5.5f,0.28f,0.05f, false,1.f, false,2,16.f, 0.f,0.00f },

// ── HYPERPOP CHIP ─────────────────────────────────────────────────────────────
{ "Harsh Pulse",      "HYPERPOP",  0,3, 0.001f,0.05f,0.50f,0.05f,0.85f,
  false,2.f,true,1.f, false,4000.f, true, 8.0f,0.60f,0.00f, true, 3.f, true, 2,20.f, 2.f,0.40f },
{ "PWM Growl",        "HYPERPOP",  0,2, 0.001f,0.10f,0.70f,0.08f,0.80f,
  false,2.f,true,1.f, false,4000.f, false,5.f,0.00f,0.00f, true, 2.5f, false,2,16.f, 1.f,0.50f },
{ "Broken Wave",      "HYPERPOP",  2,2, 0.001f,0.10f,0.60f,0.05f,0.80f,
  false,2.f,true,1.f, false,4000.f, true, 7.0f,0.50f,0.00f, false,1.f, true, 2,24.f, 3.f,0.60f },
{ "Digital Glitch",   "HYPERPOP",  3,0, 0.001f,0.08f,0.40f,0.04f,0.85f,
  false,2.f,true,1.f, true, 8000.f, false,5.f,0.00f,0.00f, false,1.f, false,2,16.f, 2.f,0.30f },

// ── FAKE SID ──────────────────────────────────────────────────────────────────
{ "Fake SID Lead",    "FAKE SID",  0,2, 0.005f,0.10f,0.75f,0.15f,0.75f,
  false,2.f,true,1.f, false,4000.f, true, 4.0f,0.20f,0.10f, true, 1.5f, false,2,16.f, 0.f,0.20f },
{ "SID Bass",         "FAKE SID",  0,3, 0.003f,0.12f,0.60f,0.10f,0.75f,
  false,2.f,true,1.f, false,4000.f, false,5.f,0.00f,0.00f, true, 2.0f, false,2,16.f, 0.f,0.35f },
{ "SID Arp",          "FAKE SID",  0,1, 0.001f,0.06f,0.00f,0.03f,0.80f,
  false,2.f,true,1.f, false,4000.f, false,5.f,0.00f,0.00f, true, 1.8f, true, 2,18.f, 0.f,0.30f },

// ── TRACKER DRUMS ─────────────────────────────────────────────────────────────
{ "Kick",             "DRUMS",     0,0, 0.001f,0.08f,0.00f,0.05f,0.85f,
  true, 6.f,false,4.f, false,4000.f, false,5.f,0.00f,0.00f, false,1.f, false,2,16.f, 0.f,0.20f },
{ "Snare",            "DRUMS",     3,0, 0.001f,0.12f,0.00f,0.08f,0.85f,
  false,2.f,true,1.f, false,6000.f, false,5.f,0.00f,0.00f, false,1.f, false,2,16.f, 0.f,0.10f },
{ "Hi Hat",           "DRUMS",     3,0, 0.001f,0.04f,0.00f,0.02f,0.80f,
  false,2.f,true,1.f, true, 12000.f,false,5.f,0.00f,0.00f, false,1.f, false,2,16.f, 0.f,0.00f },
{ "Noise Perc",       "DRUMS",     3,0, 0.001f,0.06f,0.00f,0.04f,0.80f,
  false,2.f,true,1.f, false,3500.f, false,5.f,0.00f,0.00f, false,1.f, false,2,16.f, 1.f,0.00f },
{ "Wave Thump",       "DRUMS",     2,2, 0.001f,0.08f,0.00f,0.06f,0.85f,
  false,2.f,true,1.f, false,4000.f, false,5.f,0.00f,0.00f, false,1.f, false,2,16.f, 0.f,0.20f },
};

static const int NUM_PRESETS = (int)(sizeof(PRESETS) / sizeof(PRESETS[0]));
