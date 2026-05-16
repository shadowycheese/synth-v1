#ifndef SYNTHCONFIGURATION_H
#define SYNTHCONFIGURATION_H

#include <Audio.h>
#include "Constants.h"

using SynthConfigurationFlags = uint16_t;

static constexpr SynthConfigurationFlags ENVELOPE_CHANGED = 0x0001;
static constexpr SynthConfigurationFlags FILTER_CHANGED = 0x0002;
static constexpr SynthConfigurationFlags WAVEFORM_CHANGED = 0x0004;
static constexpr SynthConfigurationFlags VOLUME_CHANGED = 0x0008;
static constexpr SynthConfigurationFlags VOICE_CHANGED = 0x0010;
static constexpr SynthConfigurationFlags EFFECT_CHANGED = 0x0020;
static constexpr SynthConfigurationFlags LFO_CHANGED = 0x0040;

inline bool waveformChanged(SynthConfigurationFlags flags)
{
    return flags & WAVEFORM_CHANGED;
}

inline bool filterChanged(SynthConfigurationFlags flags)
{
    return flags & FILTER_CHANGED;
}

inline bool envelopeChanged(SynthConfigurationFlags flags)
{
    return flags & ENVELOPE_CHANGED;
}

inline bool volumeChanged(SynthConfigurationFlags flags)
{
    return flags & VOLUME_CHANGED;
}

inline bool voiceChanged(SynthConfigurationFlags flags)
{
    return flags & VOICE_CHANGED;
}

inline bool effectChanged(SynthConfigurationFlags flags)
{
    return flags & EFFECT_CHANGED;
}

inline bool lfoChanged(SynthConfigurationFlags flags)
{
    return flags & LFO_CHANGED;
}

class OscillatorConfiguration
{
public:
    uint8_t waveform;
    float gain;
    float amplitude;
    float detune;
    float frequency;
    float pulseWidth;

    void copy(OscillatorConfiguration *source)
    {
        waveform = source->waveform;
        gain = source->gain;
        detune = source->detune;
        amplitude = source->amplitude;
        frequency = source->frequency;
        pulseWidth = source->pulseWidth;
    }
};

class EnvelopeConfiguration
{
public:
    float attack = 10;
    float decay = 50;
    float sustain = 0.7;
    float release = 300;

    void copy(EnvelopeConfiguration *source)
    {
        attack = source->attack;
        decay = source->decay;
        sustain = source->sustain;
        release = source->release;
    }
};

class SynthConfiguration
{
public:
    SynthConfiguration() {}

    // Waveform parameters
    OscillatorConfiguration oscillators[4];
    OscillatorConfiguration lfo1;
    OscillatorConfiguration lfo2;
    OscillatorConfiguration lfo3;

    // Envelope parameters
    EnvelopeConfiguration voiceEnvelope;
    EnvelopeConfiguration lfo1Envelope;
    EnvelopeConfiguration lfo2Envelope;

    // Filter parameters
    bool filterEnabled;
    bool lowPass;
    float filterCutoff;
    float resonance = 0;
    float octaveControl = 5;
    float keyTracking = 0.0f;

    // Voice parameters
    float pitchBend = 0;
    bool leftSideOnly = 0;
    float noiseGain;
    float maxDetune;

    // Volume parameters
    float masterVolume = 1.0f;
    float ampGain = 0.5f;

    // Effect parameters
    bool delayEnabled = 20;
    float reverb = 0.0f;
    bool reverbEnabled = false;
    float delay = 0.0f;

    void copy(SynthConfiguration *source)
    {
        voiceEnvelope.copy(&(source->voiceEnvelope));
        lfo1Envelope.copy(&(source->lfo1Envelope));
        lfo2Envelope.copy(&(source->lfo2Envelope));

        delayEnabled = source->delayEnabled;
        reverbEnabled = source->reverbEnabled;
        reverb = source->reverb;
        keyTracking = source->keyTracking;
        delay = source->delay;
        leftSideOnly = source->leftSideOnly;
        lowPass = source->lowPass;
        resonance = source->resonance;
        pitchBend = source->pitchBend;

        lfo1.copy(&(source->lfo1));
        lfo2.copy(&(source->lfo2));
        lfo3.copy(&(source->lfo3));

        octaveControl = source->octaveControl;
        filterCutoff = source->filterCutoff;
        filterEnabled = source->filterEnabled;
        noiseGain = source->noiseGain;
        maxDetune = 0;

        for (int i = 0; i < 4; i++)
        {
            oscillators[i].copy(&(source->oscillators[i]));

            if (oscillators[i].detune > maxDetune)
            {
                maxDetune = oscillators[i].detune;
            }
        }

        masterVolume = source->masterVolume;
        ampGain = source->ampGain;
    }
};

#endif