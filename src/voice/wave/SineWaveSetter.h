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
        float volume = 0.15;//0.5 + (velocity > 0.5 ? 0.5 : velocity);

        Serial.printf("configure freq: %0.3f, volume: %0.3f\n",frequency, volume);

        waveForms[0].frequency(frequency);
        waveForms[0].amplitude(volume);
        waveForms[0].begin(WAVEFORM_SAWTOOTH);

        float lf = frequency;
        float rf = frequency;

        for(int i = 1; i < 3; i++) 
        {
            int l = i << 2;
            int r = l + 1;

            lf /= TWELTH_ROOT_OF_TWO;
            rf *= TWELTH_ROOT_OF_TWO;
            volume *= 0.9;

            waveForms[l].frequency(lf);
            waveForms[l].amplitude(volume);

            waveForms[r].frequency(rf);
            waveForms[r].amplitude(volume);
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