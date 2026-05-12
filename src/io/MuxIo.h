#ifndef MUXIO_H
#define MUXIO_H

#include "../Constants.h"
#include "AnalogBuffer.h"
#include "ControllerIoListener.h"

#define DEBOUNCE_DISTANCE 1
#define INVERTED 1

const int MUX1_READ_PIN = 27;
const int MUX2_READ_PIN = 26;
const int MUX3_READ_PIN = 25;

class MuxIo
{
public:
    MuxIo(int id, int activePinCount)
    {
        muxId = id;
        activePins = activePinCount;
        inputPin = muxId == 1
                       ? MUX1_READ_PIN
                   : muxId == 2
                       ? MUX2_READ_PIN
                       : MUX3_READ_PIN;

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

        analogRead(inputPin);

        int rawValue = analogRead(inputPin);

        if (INVERTED)
        {
            rawValue = 1023 - rawValue;
        }

        int oldValue = currentValues[input].value();
        int value = currentValues[input].read(rawValue);

        if (oldValue != value)
        {
            bufferValues[input] = value;

            bufferChanged = true;
        }
    }

    void debug()
    {
        Serial.printf("%d:", muxId);

        for (int i = 0; i < activePins; i++)
        {
            if ((i % 4) == 0)
            {
                Serial.print(" ");
            }

            Serial.printf("%01X", bufferValues[i] >> 6);
        }

        Serial.print("  ");
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
    int inputPin;
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