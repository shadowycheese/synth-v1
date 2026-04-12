#ifndef WAVESETTER_H
#define WAVESETTER_H

#include <Audio.h>

class WaveSetter {
public:
    virtual ~WaveSetter() {};

    virtual void configure(float frequency, float velocity, AudioSynthWaveform *waveForms);
};

#endif