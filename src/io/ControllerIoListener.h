#ifndef CONTROLLERIOLISTENER_H
#define CONTROLLERIOLISTENER_H

#include "../Constants.h"

#define INPUT_GROUP_MIDI 0
#define INPUT_GROUP_MUX1 1
#define INPUT_GROUP_MUX2 2

#define MIDI_INPUT_PITCH 0
#define MIDI_INPUT_RESONANCE 1
#define MIDI_INPUT_DETUNE 2
#define MIDI_INPUT_PW 3
#define MIDI_INPUT_WAVEFORM 4

class ControllerIoListener
{
public:
    ControllerIoListener() {};
    virtual ~ControllerIoListener() {};

    virtual void onControllerIoChanged(int group, int input, int level);

    virtual void commit();
};

#endif