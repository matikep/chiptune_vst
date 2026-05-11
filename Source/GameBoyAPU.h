#pragma once
#include <cstdint>
#include <array>
#include <cmath>
#include <algorithm>

static constexpr double TWO_PI = 6.283185307179586;

static constexpr bool DUTY_TABLE[4][8] = {
    { 0,0,0,0,0,0,0,1 },  // 12.5%
    { 1,0,0,0,0,0,0,1 },  // 25%
    { 1,0,0,0,1,1,1,1 },  // 50%
    { 0,1,1,1,1,1,1,0 },  // 75%
};

// ── Square voice (CH1 / CH2) ──────────────────────────────────────────────────
struct GBSquareVoice {
    // Pitch
    double  baseFreq    = 440.0;
    float   arpOffset   = 0.0f;   // semitones, set by arp engine each sample
    float   driftCents  = 0.0f;   // ±cents hardware instability

    // Timbre
    float   gain        = 1.0f;
    uint8_t duty        = 2;
    double  phase       = 0.0;

    // ADSR
    double  envAttack   = 0.005;
    double  envDecay    = 0.1;
    double  sustain     = 0.8;
    double  envRelease  = 0.08;

    // Vibrato
    bool    vibEnabled  = false;
    float   vibRate     = 5.0f;   // Hz
    float   vibDepth    = 0.3f;   // semitones
    float   vibDelay    = 0.1f;   // seconds before vib starts

    // PWM fake (LFO on duty)
    bool    pwmEnabled  = false;
    float   pwmRate     = 1.0f;

    // Sweep (CH1 only)
    bool    sweepEnabled = false;
    float   sweepRate    = 2.0f;
    bool    sweepUp      = true;
    float   sweepAmount  = 1.0f;

    bool    active      = false;
    bool    releasing   = false;
    double  envTime     = 0.0;
    double  noteTime    = 0.0;

private:
    double  sweepBase   = 440.0;
    double  sweepAccum  = 0.0;
    double  vibPhase    = 0.0;
    double  pwmPhase    = 0.0;

public:
    void noteOn(double hz, float velocity) {
        baseFreq   = hz;
        sweepBase  = hz;
        gain       = velocity;
        phase      = 0.0;
        active     = true;
        releasing  = false;
        envTime    = 0.0;
        noteTime   = 0.0;
        sweepAccum = 0.0;
        vibPhase   = 0.0;
        pwmPhase   = 0.0;
    }

    void noteOff() {
        if (active && !releasing) { releasing = true; envTime = 0.0; }
    }

    float process(double sampleRate) {
        if (!active) return 0.0f;
        double dt = 1.0 / sampleRate;
        noteTime += dt;

        // Envelope
        float env;
        if (!releasing) {
            double a = std::max(envAttack,  1e-6);
            double d = std::max(envDecay,   1e-6);
            if      (envTime < a)     env = (float)(envTime / a);
            else if (envTime < a + d) env = 1.0f - (float)((envTime - a) / d) * (1.0f - (float)sustain);
            else                      env = (float)sustain;
            envTime += dt;
        } else {
            double r = std::max(envRelease, 1e-6);
            env = (float)(sustain * std::max(0.0, 1.0 - envTime / r));
            envTime += dt;
            if (envTime >= r) { active = false; return 0.0f; }
        }

        // Sweep
        if (sweepEnabled && sweepRate > 0.0f && sweepAmount > 0.0f) {
            sweepAccum += sweepRate * dt;
            if (sweepAccum >= 1.0) {
                sweepAccum -= 1.0;
                sweepBase *= std::pow(2.0, (sweepUp ? 1.0 : -1.0) * sweepAmount / 12.0);
                sweepBase  = std::clamp(sweepBase, 20.0, 20000.0);
            }
        }

        // Effective frequency: sweep + arp + drift
        double f = sweepBase
                 * std::pow(2.0, (double)arpOffset  / 12.0)
                 * std::pow(2.0, (double)driftCents / 1200.0);

        // Vibrato
        if (vibEnabled && vibDepth > 0.0f && vibRate > 0.0f && noteTime > vibDelay) {
            vibPhase += vibRate * dt;
            if (vibPhase >= 1.0) vibPhase -= 1.0;
            f *= std::pow(2.0, std::sin(vibPhase * TWO_PI) * vibDepth / 12.0);
        }
        f = std::clamp(f, 10.0, 20000.0);

        // PWM
        uint8_t effectiveDuty = duty;
        if (pwmEnabled) {
            pwmPhase += pwmRate * dt;
            if (pwmPhase >= 1.0) pwmPhase -= 1.0;
            effectiveDuty = (uint8_t)((int)(pwmPhase * 4.0) & 3);
        }

        phase += f / sampleRate;
        if (phase >= 1.0) phase -= std::floor(phase);
        float wave = DUTY_TABLE[effectiveDuty][(int)(phase * 8.0) & 7] ? 1.0f : -1.0f;

        return wave * env * gain;
    }

    bool isActive() const { return active; }
};

// ── Wave voice (CH3) ──────────────────────────────────────────────────────────
struct GBWaveVoice {
    std::array<uint8_t, 32> waveTable = {
        15,14,12,10, 8, 6, 4, 2,
         0, 2, 4, 6, 8,10,12,14,
        15,14,12,10, 8, 6, 4, 2,
         0, 2, 4, 6, 8,10,12,14,
    };
    double  baseFreq   = 440.0;
    float   arpOffset  = 0.0f;
    float   driftCents = 0.0f;
    float   gain       = 1.0f;
    double  phase      = 0.0;
    double  envAttack  = 0.005;
    double  envDecay   = 0.1;
    double  sustain    = 0.8;
    double  envRelease = 0.1;
    bool    vibEnabled = false;
    float   vibRate    = 5.0f;
    float   vibDepth   = 0.3f;
    float   vibDelay   = 0.1f;
    bool    active     = false;
    bool    releasing  = false;
    double  envTime    = 0.0;
    double  noteTime   = 0.0;
private:
    double  vibPhase   = 0.0;
public:
    void noteOn(double hz, float velocity) {
        baseFreq  = hz;  gain = velocity;  phase = 0.0;
        active = true;   releasing = false; envTime = 0.0; noteTime = 0.0; vibPhase = 0.0;
    }
    void noteOff() { if (active && !releasing) { releasing = true; envTime = 0.0; } }

    float process(double sampleRate) {
        if (!active) return 0.0f;
        double dt = 1.0 / sampleRate;
        noteTime += dt;

        float env;
        if (!releasing) {
            double a = std::max(envAttack, 1e-6), d = std::max(envDecay, 1e-6);
            if      (envTime < a)     env = (float)(envTime / a);
            else if (envTime < a + d) env = 1.0f - (float)((envTime - a) / d) * (1.0f - (float)sustain);
            else                      env = (float)sustain;
            envTime += dt;
        } else {
            double r = std::max(envRelease, 1e-6);
            env = (float)(sustain * std::max(0.0, 1.0 - envTime / r));
            envTime += dt;
            if (envTime >= r) { active = false; return 0.0f; }
        }

        double f = baseFreq
                 * std::pow(2.0, (double)arpOffset  / 12.0)
                 * std::pow(2.0, (double)driftCents / 1200.0);
        if (vibEnabled && vibDepth > 0.0f && vibRate > 0.0f && noteTime > vibDelay) {
            vibPhase += vibRate * dt;
            if (vibPhase >= 1.0) vibPhase -= 1.0;
            f *= std::pow(2.0, std::sin(vibPhase * TWO_PI) * vibDepth / 12.0);
        }
        f = std::clamp(f, 10.0, 20000.0);

        phase += f / sampleRate;
        if (phase >= 1.0) phase -= std::floor(phase);
        float wave = ((float)waveTable[(int)(phase * 32.0) & 31] / 7.5f) - 1.0f;
        return wave * env * gain;
    }
    bool isActive() const { return active; }
};

// ── Noise voice (CH4) ─────────────────────────────────────────────────────────
struct GBNoiseVoice {
    bool    shortMode   = false;
    float   noiseFreq   = 4000.0f;
    float   gain        = 1.0f;
    float   arpOffset   = 0.0f;
    double  envAttack   = 0.001;
    double  envDecay    = 0.05;
    double  sustain     = 0.0;
    double  envRelease  = 0.08;
    bool    active      = false;
    bool    releasing   = false;
    double  envTime     = 0.0;
private:
    uint16_t lfsr       = 0x7FFF;
    double   lfsrAccum  = 0.0;
    float    lfsrOut    = 1.0f;
public:
    void noteOn(float velocity) {
        gain = velocity; active = true; releasing = false;
        envTime = 0.0; lfsr = 0x7FFF; lfsrAccum = 0.0;
    }
    void noteOff() { if (active && !releasing) { releasing = true; envTime = 0.0; } }

    float process(double sampleRate) {
        if (!active) return 0.0f;
        double dt = 1.0 / sampleRate;

        float env;
        if (!releasing) {
            double a = std::max(envAttack, 1e-6), d = std::max(envDecay, 1e-6);
            if      (envTime < a)     env = (float)(envTime / a);
            else if (envTime < a + d) env = 1.0f - (float)((envTime - a) / d) * (1.0f - (float)sustain);
            else                      env = (float)sustain;
            envTime += dt;
        } else {
            double r = std::max(envRelease, 1e-6);
            env = (float)(sustain * std::max(0.0, 1.0 - envTime / r));
            envTime += dt;
            if (envTime >= r) { active = false; return 0.0f; }
        }

        lfsrAccum += noiseFreq * dt;
        while (lfsrAccum >= 1.0) {
            lfsrAccum -= 1.0;
            uint16_t bit = (lfsr ^ (lfsr >> 1)) & 1;
            lfsr >>= 1;
            lfsr |= (uint16_t)(bit << 14);
            if (shortMode) { lfsr &= ~(uint16_t)(1 << 6); lfsr |= (uint16_t)(bit << 6); }
            lfsrOut = (lfsr & 1) ? 1.0f : -1.0f;
        }
        return lfsrOut * env * gain;
    }
    bool isActive() const { return active; }
};
