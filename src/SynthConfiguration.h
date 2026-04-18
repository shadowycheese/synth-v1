#ifndef SYNTHCONFIGURATION_H
#define SYNTHCONFIGURATION_H

#include <Audio.h>
#include "Constants.h"

class SynthConfiguration
{
public:
    SynthConfiguration() {}

    // Waveform parameters
    uint8_t mainWaveForm = WAVEFORM_SINE;
    int detuneWaveForm = WAVEFORM_SINE;

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
    bool manualCutoff;

    // Volume parameters
    float masterVolume = 1.0f;
    float filterGain = 0.5f;
    float mixerGain = 0.5f;
    float voiceGain = 0.5f;

    void copy(SynthConfiguration *source)
    {
        mainWaveForm = source->mainWaveForm;
        detuneWaveForm = source->detuneWaveForm;

        attack = source->attack;
        decay = source->decay;
        sustain = source->sustain;
        release = source->release;

        detune = source->detune;
        resonance = source->resonance;
        pitch = source->pitch;
        pulseWidth = source->pulseWidth;
        manualCutoff = source->manualCutoff;

        noiseAmplitude = source->noiseAmplitude;

        for (int i = 0; i < 4; i++)
        {
            amplitudes[i] = source->amplitudes[i];
        }

        masterVolume = source->masterVolume;
        filterGain = source->filterGain;
        mixerGain = source->mixerGain;
        voiceGain = source->mixerGain;
    }
};

#endif