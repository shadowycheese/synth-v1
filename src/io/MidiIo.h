#ifndef MIDIIO_H
#define MIDIIO_H

#include "../Constants.h"
#include "AnalogBuffer.h"
#include "ControllerIoListener.h"

#define MIDI_INPUT_COUNT 7

#define MIDI_INPUT_PITCH 0
#define MIDI_INPUT_RESONANCE 1
#define MIDI_INPUT_DETUNE 2
#define MIDI_INPUT_PW 3
#define MIDI_INPUT_WAVEFORM0 4
#define MIDI_INPUT_WAVEFORM1 5
#define MIDI_INPUT_CUTOFF 6

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

    void midiPitch(int pitch)
    {
        update(MIDI_INPUT_PITCH, pitch);
    }

    void midiControl(byte control, byte value)
    {
        if (control == 7)
        {
            update(MIDI_INPUT_CUTOFF, 8 * (int)value);
        }
        else if (control == 7)
        {
            update(MIDI_INPUT_RESONANCE, 8 * (int)value);
        }
        else if (control == 74)
        {
            update(MIDI_INPUT_DETUNE, 8 * (int)value);
        }
        else if (control == 71)
        {
            update(MIDI_INPUT_PW, 8 * (int)value);
        }
        else if (control == 72)
        {
            update(MIDI_INPUT_WAVEFORM0, 8 * (int)value);
        }
        else if (control == 73)
        {
            update(MIDI_INPUT_WAVEFORM1, 8 * (int)value);
        }
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

                Serial.printf("%d => %d\n", i, commitValues[i]);

                ioListener->onControllerIoChanged(INPUT_GROUP_MIDI, i, commitValues[i]);
            }
        }

        bufferChanged = false;
    }

private:
    int bufferValues[MIDI_INPUT_COUNT];
    int commitValues[MIDI_INPUT_COUNT];
    bool bufferChanged;

    void update(int input, int value)
    {
        bufferValues[input] = value;

        bufferChanged = true;
    }
};

#endif