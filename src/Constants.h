#ifndef CONSTANTS_H
#define CONSTANTS_H

const float TWELTH_ROOT_OF_TWO = 1.0595;

#define MIDI_FREQ(note) (440.0 * pow(2.0, (note - 45) / 12.0))

#define MAX_VOICES 8

#endif