#ifndef SIMPLEWAVESETTER_H
#define SIMPLEWAVESETTER_H

#include <Audio.h>
#include "WaveSetter.h"
#include "../../SynthConfiguration.h"

class SimpleWaveSetter : public WaveSetter {
public:
    SimpleWaveSetter() {
    };

    virtual void configure(float frequency, float amplitude, bool restart, SynthConfiguration *configuration, AudioSynthWaveform *waveForms)
    {
        Serial.printf("configure freq: %0.3f, volume: %0.3f\n",frequency, amplitude);

        waveForms[0].frequency(frequency);
        waveForms[0].amplitude(amplitude);

        if (restart) 
        {
            waveForms[0].begin(configuration -> waveForm);
        }

        for(int i = 1; i < 7; i++) 
        {
            waveForms[i].amplitude(0);
        }    
    }
};

#endif