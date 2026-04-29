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

class SynthConfiguration
{
public:
    SynthConfiguration() {}

    // Waveform parameters
    uint8_t waveforms[4];
    uint8_t lfoWaveform;

    // Envelope parameters
    float voiceAttack = 10;
    float voiceDecay = 50;
    float voiceSustain = 0.7;
    float voiceRelease = 300;
    float filterAttack = 10;
    float filterDecay = 50;
    float filterSustain = 0.7;
    float filterRelease = 300;

    // Filter parameters
    bool autoCutoff;
    float filterCutoff;
    float filterLevel;
    float lfoFrequency;
    float lfoAmplitude;
    float lfoPulseWidth;
    float resonance = 0;
    float octaveControl;

    // Voice parameters
    float pitchBend = 0;
    float detune = 0;
    bool halfSaw = 0;
    float amplitudes[4];
    float noiseAmplitude;
    float pitchLevel = 0.2;

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
        voiceAttack = source->voiceAttack;
        voiceDecay = source->voiceDecay;
        voiceSustain = source->voiceSustain;
        voiceRelease = source->voiceRelease;

        filterAttack = source->filterAttack;
        filterDecay = source->filterDecay;
        filterSustain = source->voiceSustain;
        filterRelease = source->filterRelease;

        delayEnabled = source->delayEnabled;
        reverb = source->reverb;
        delay = source->delay;

        halfSaw = source->halfSaw;
        detune = source->detune;
        resonance = source->resonance;
        pitchBend = source->pitchBend;
        lfoAmplitude = source->lfoAmplitude;
        lfoFrequency = source->lfoFrequency;
        lfoPulseWidth = source->lfoPulseWidth;
        octaveControl = source->octaveControl;

        filterCutoff = source->filterCutoff;
        autoCutoff = source->autoCutoff;
        filterLevel = source->filterLevel;

        noiseAmplitude = source->noiseAmplitude;
        lfoWaveform = source->lfoWaveform;

        for (int i = 0; i < 4; i++)
        {
            waveforms[i] = source->waveforms[i];
            amplitudes[i] = source->amplitudes[i];
        }

        masterVolume = source->masterVolume;
        voiceGain = source->voiceGain;
    }
};

#endif