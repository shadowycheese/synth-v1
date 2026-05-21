#ifndef CONSTANTS_H
#define CONSTANTS_H

const float TWELTH_ROOT_OF_TWO = 1.0595;

#define midiNoteHz(note) (440.0 * pow(2.0, (note - 69) / 12.0))

#define DETUNE_MAX_SPREAD 1200.0f

#define MAX_VOICES 8

enum
{
    FILTER_NONE = 0,
    FILTER_LADDER = 1,
    FILTER_SVF = 2
};

#endif