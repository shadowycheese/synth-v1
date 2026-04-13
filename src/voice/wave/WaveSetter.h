#ifndef WAVESETTER_H
#define WAVESETTER_H

#include <Audio.h>
#include "../../SynthConfiguration.h"

class WaveSetter {
public:
    virtual ~WaveSetter() {};

    virtual void configure(float frequency, float amplitude, bool restart, SynthConfiguration *configuration, AudioSynthWaveform *waveForms) = 0;
};

#endif