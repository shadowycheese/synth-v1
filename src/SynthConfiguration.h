#ifndef SYNTHCONFIGURATION_H
#define SYNTHCONFIGURATION_H

#include <Audio.h>
#include "Constants.h"

using SynthConfigurationFlags = uint16_t;

static constexpr SynthConfigurationFlags ENVELOPE_CHANGED = 0x0001;
static constexpr SynthConfigurationFlags FILTER_CHANGED = 0x0002;
static constexpr SynthConfigurationFlags OSCILLATOR_CHANGED = 0x0004;
static constexpr SynthConfigurationFlags VOLUME_CHANGED = 0x0008;
static constexpr SynthConfigurationFlags VOICE_CHANGED = 0x0010;

inline bool oscillatorChanged(SynthConfigurationFlags flags)
{
    return flags & OSCILLATOR_CHANGED;
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

class SynthConfiguration
{
public:
    SynthConfiguration() {}

    // Waveform parameters
    uint8_t waveforms[4];

    // Envelope parameters
    float attack = 10;
    float decay = 50;
    float sustain = 0.7;
    float release = 300;

    // Voice parameters
    float pitch = 0;
    float detune = 0;
    float resonance = 0;
    float amplitudes[4];
    float noiseAmplitude;
    float pulseWidth;
    float cutoff;
    bool manualCutoff;
    bool superSawMode;

    // Volume parameters
    float masterVolume = 1.0f;
    float filterGain = 0.5f;
    float voiceGain = 0.5f;

    void copy(SynthConfiguration *source)
    {
        attack = source->attack;
        decay = source->decay;
        sustain = source->sustain;
        release = source->release;

        detune = source->detune;
        resonance = source->resonance;
        pitch = source->pitch;
        pulseWidth = source->pulseWidth;
        cutoff = source->cutoff;
        manualCutoff = source->manualCutoff;
        superSawMode = source->superSawMode;

        noiseAmplitude = source->noiseAmplitude;

        for (int i = 0; i < 4; i++)
        {
            waveforms[i] = source->waveforms[i];
            amplitudes[i] = source->amplitudes[i];
        }

        masterVolume = source->masterVolume;
        filterGain = source->filterGain;
        voiceGain = source->voiceGain;
    }
};

#endif