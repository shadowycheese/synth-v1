#ifndef MUXIO_H
#define MUXIO_H

#include "../Constants.h"
#include "AnalogBuffer.h"
#include "ControllerIoListener.h"

#define DEBOUNCE_DISTANCE 1

const int MUX1_READ_PIN = 27;

class MuxIo
{
public:
    MuxIo(int id, int activePinCount)
    {
        muxId = id;
        activePins = activePinCount;

        for (int i = 0; i < 16; i++)
        {
            commitValues[i] = -1;
        }
    }

    void read(int input)
    {
        if (input >= activePins)
        {
            return;
        }

        analogRead(MUX1_READ_PIN);

        int rawValue = analogRead(MUX1_READ_PIN);
        int oldValue = currentValues[input].value();
        int value = currentValues[input].read(rawValue);

        if (oldValue != value)
        {
            bufferValues[input] = value;

            bufferChanged = true;
        }
    }

    void commitBufferChanges(ControllerIoListener *ioListener)
    {
        for (int i = 0; i < activePins; i++)
        {
            int smoothedValue = (bufferValues[i] + commitValues[i]) / 2;

            if (debounce(smoothedValue, commitValues[i], DEBOUNCE_DISTANCE))
            {
                commitValues[i] = smoothedValue;

                ioListener->onControllerIoChanged(muxId, i, smoothedValue);
            }
        }
    }

private:
    int activePins;
    int muxId;
    AnalogBuffer currentValues[16];
    int bufferValues[16];
    int commitValues[16];
    bool bufferChanged;

    bool debounce(int value1, int value2, int minDiff)
    {
        if (value1 < 0 || value2 < 0)
        {
            return true;
        }

        int diff = value1 - value2;

        diff = diff < 0 ? -diff : diff;

        return diff > minDiff;
    }
};

#endif