#ifndef SUPERWAVESETTER_H
#define SUPERWAVESETTER_H

#include <Audio.h>
#include "WaveSetter.h"
#include "../../SynthConfiguration.h"

class SuperWaveSetter : public WaveSetter {
public:
    SuperWaveSetter() {};

    virtual void configure(float frequency, float amplitude, bool restart, SynthConfiguration *configuration, AudioSynthWaveform *waveForms)
    {    
        Serial.printf("configure freq: %0.3f, volume: %0.3f\n",frequency, amplitude);

        waveForms[0].frequency(frequency);
        waveForms[0].amplitude(amplitude);

        if (restart) 
        {
            waveForms[0].begin(configuration -> waveForm);
        }

        float lf = frequency;
        float rf = frequency;

        for(int i = 0; i < 2; i++) 
        {
            int l = 1 + (i * 2);
            int r = l + 1;

            lf /= configuration -> detune;
            rf *= configuration -> detune;
            amplitude *= 0.8;

            waveForms[l].begin(configuration -> waveForm);
            waveForms[r].begin(configuration -> waveForm);

            waveForms[l].frequency(lf);
            waveForms[l].amplitude(amplitude);

            waveForms[r].frequency(rf);
            waveForms[r].amplitude(amplitude);
        }    
    }
};

#endif