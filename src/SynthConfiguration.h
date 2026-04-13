#ifndef SYNTHCONFIGURATION_H
#define SYNTHCONFIGURATION_H

#include <Audio.h>
#include "Constants.h"

class SynthConfiguration {
public:
    SynthConfiguration() 
    {
        waveForm = WAVEFORM_SINE;
    }

    int waveForm;
    float attack = 10;
    float decay = 50;
    float sustain = 0.7;
    float release = 300;
    float detune = TWELTH_ROOT_OF_TWO;
};

#endif