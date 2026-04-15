#ifndef SYNTHCONFIGURATION_H
#define SYNTHCONFIGURATION_H

#include <Audio.h>
#include "Constants.h"

class SynthConfiguration
{
public:
    SynthConfiguration() {}

    // Waveform parameters
    int mainWaveForm = WAVEFORM_SINE;
    int detuneWaveForm = WAVEFORM_SINE;

    // Envelope parameters
    float attack = 10;
    float decay = 50;
    float sustain = 0.7;
    float release = 300;

    // Voice parameters
    float detune = 0;
    float resonance = 0;
    int oscillators = 3;

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
        oscillators = source->oscillators;
        resonance = source->resonance;

        masterVolume = source->masterVolume;
        filterGain = source->filterGain;
        mixerGain = source->mixerGain;
        voiceGain = source->voiceGain;
    }
};

#endif