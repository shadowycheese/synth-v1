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

class OscillatorConfiguration
{
public:
    uint8_t waveform;
    float level;
    float amplitude;
    float detune;
    float frequency;
    float pulseWidth;

    void copy(OscillatorConfiguration *source)
    {
        waveform = source->waveform;
        level = source->level;
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
    OscillatorConfiguration waveforms[4];
    OscillatorConfiguration pitchLfo;
    OscillatorConfiguration filterLfo;

    // Envelope parameters
    EnvelopeConfiguration voiceEnvelope;
    EnvelopeConfiguration filterEnvelope;

    // Filter parameters
    bool autoCutoff;
    float filterCutoff;
    float resonance = 0;
    float octaveControl = 5;

    // Voice parameters
    float pitchBend = 0;
    bool halfSaw = 0;
    float noiseAmplitude;

    // Volume parameters
    float masterVolume = 1.0f;
    float voiceGain = 0.5f;

    // Effect parameters
    bool delayEnabled = 20;
    float reverb = 0.0f;
    bool reverbEnabled = false;
    float delay = 0.0f;

    void copy(SynthConfiguration *source)
    {
        voiceEnvelope.copy(&(source->voiceEnvelope));
        filterEnvelope.copy(&(source->filterEnvelope));

        delayEnabled = source->delayEnabled;
        reverb = source->reverb;
        delay = source->delay;
        halfSaw = source->halfSaw;
        resonance = source->resonance;
        pitchBend = source->pitchBend;

        pitchLfo.copy(&(source->pitchLfo));
        filterLfo.copy(&(source->filterLfo));
        octaveControl = source->octaveControl;
        filterCutoff = source->filterCutoff;
        autoCutoff = source->autoCutoff;
        noiseAmplitude = source->noiseAmplitude;

        for (int i = 0; i < 4; i++)
        {
            waveforms[i].copy(&(source->waveforms[i]));
        }

        masterVolume = source->masterVolume;
        voiceGain = source->voiceGain;
    }
};

#endif