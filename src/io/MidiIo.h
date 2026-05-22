#ifndef MIDIIO_H
#define MIDIIO_H

#include <Arduino.h>
#include "ControllerIoListener.h"

#define MIDI_INPUT_COUNT 7

#define MIDI_INPUT_0 0
#define MIDI_INPUT_1 1
#define MIDI_INPUT_2 2
#define MIDI_INPUT_3 3
#define MIDI_INPUT_4 4
#define MIDI_INPUT_5 5
#define MIDI_INPUT_6 6

class MidiIo
{
public:
    MidiIo()
    {
        for (int i = 0; i < MIDI_INPUT_COUNT; i++)
        {
            _commitValues[i] = -1;
        }
    }

    void midiPitch(int pitch)
    {
        update(MIDI_INPUT_0, pitch);
    }

    void midiControl(byte control, byte value)
    {
        // Additional inputs from MIDI controller can go here
    }

    void commitBufferChanges(ControllerIoListener *ioListener)
    {
        if (!_bufferChanged)
        {
            return;
        }

        for (int i = 0; i < MIDI_INPUT_COUNT; i++)
        {
            if (_bufferValues[i] != _commitValues[i])
            {
                _commitValues[i] = _bufferValues[i];

                Serial.printf("%d => %d\n", i, _commitValues[i]);

                ioListener->onControllerIoChanged(INPUT_GROUP_MIDI, i, _commitValues[i]);
            }
        }

        _bufferChanged = false;
    }

private:
    int _bufferValues[MIDI_INPUT_COUNT];
    int _commitValues[MIDI_INPUT_COUNT];
    bool _bufferChanged;

    void update(int input, int value)
    {
        _bufferValues[input] = value;

        _bufferChanged = true;
    }
};

#endif