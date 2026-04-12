#ifndef SINEWAVESETTER_H
#define SINEWAVESETTER_H

#include <Audio.h>
#include "WaveSetter.h"

class SineWaveSetter : public WaveSetter {
public:
    SineWaveSetter() {

    };

    void configure(float frequency, float velocity, AudioSynthWaveform *waveForms)
    {
        float volume = 0.5 + (velocity > 0.5 ? 0.5 : velocity);

        Serial.printf("configure freq: %0.3f, volume: %0.3f\n",frequency, volume);

        waveForms[0].frequency(frequency);
        waveForms[0].amplitude(0.1);
        waveForms[0].begin(WAVEFORM_SINE);

        for(int i = 1; i < 7; i++) {
            waveForms[i].amplitude(0);
        }        
    }

    void update(AudioSynthWaveform **waveForms) 
    {
        for(int i = 1; i < 7; i++) {
            waveForms[i] -> amplitude(0);
        }        
    }
};

#endif