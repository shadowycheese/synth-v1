#ifndef MIDIIO_H
#define MIDIIO_H

#include "../Constants.h"
#include "AnalogBuffer.h"
#include "ControllerIoListener.h"

#define MIDI_INPUT_COUNT 5

class MidiIo
{
public:
    MidiIo()
    {
        for (int i = 0; i < MIDI_INPUT_COUNT; i++)
        {
            commitValues[i] = -1;
        }
    }

    void update(int input, int value)
    {
        bufferValues[input] = value;

        bufferChanged = true;
    }

    void commitBufferChanges(ControllerIoListener *ioListener)
    {
        if (!bufferChanged)
        {
            return;
        }

        for (int i = 0; i < MIDI_INPUT_COUNT; i++)
        {
            if (bufferValues[i] != commitValues[i])
            {
                commitValues[i] = bufferValues[i];

                ioListener->onControllerIoChanged(INPUT_GROUP_MIDI, i, commitValues[i]);
            }
        }
    }

private:
    int bufferValues[MIDI_INPUT_COUNT];
    int commitValues[MIDI_INPUT_COUNT];
    bool bufferChanged;
};

#endif