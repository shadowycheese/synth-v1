#ifndef WAVESETTER_H
#define WAVESETTER_H

#include <Audio.h>
#include "WaveConfiguration.h"

class WaveSetter
{
public:
    virtual ~WaveSetter() {};

    virtual void configure(float frequency, float amplitude, bool restart, WaveConfiguration *configuration, AudioSynthWaveform *waveForms) = 0;

    virtual char *name() = 0;
};

#endif