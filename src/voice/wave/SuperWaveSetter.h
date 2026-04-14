#ifndef SUPERWAVESETTER_H
#define SUPERWAVESETTER_H

#include <Audio.h>
#include "WaveSetter.h"
#include "../../SynthConfiguration.h"

class SuperWaveSetter : public WaveSetter
{
public:
    SuperWaveSetter() {};

    virtual char *name()
    {
        return "SuperWaveSetter";
    }

    virtual void configure(float frequency, float amplitude, bool restart, WaveConfiguration *configuration, AudioSynthWaveform *waveForms)
    {
        Serial.printf("configure freq: %0.3f, volume: %0.3f, detune %0.3f\n", frequency, amplitude, configuration->detune);

        waveForms[0].frequency(frequency);

        if (restart)
        {
            waveForms[0].amplitude(amplitude);
            waveForms[0].begin(configuration->mainWaveForm);
        }

        float lf = frequency;
        float rf = frequency;

        for (int i = 0; i < 3; i++)
        {
            int l = 1 + (i * 2);
            int r = l + 1;

            lf /= configuration->detune;
            rf *= configuration->detune;
            amplitude *= 0.7;
            waveForms[l].frequency(lf);
            waveForms[r].frequency(rf);

            if (restart)
            {
                waveForms[l].amplitude(amplitude);
                waveForms[r].amplitude(amplitude);
                waveForms[l].begin(configuration->detuneWaveForm);
                waveForms[r].begin(configuration->detuneWaveForm);
            }

            Serial.printf("left freq: %0.3f, right freq: %0.3f amplitude: %0.3f\n", lf, rf, amplitude);
        }
    }
};

#endif