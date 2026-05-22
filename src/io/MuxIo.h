#ifndef MUXIO_H
#define MUXIO_H

#include <Arduino.h>
#include "AnalogBuffer.h"
#include "ControllerIoListener.h"

#define DEBOUNCE_DISTANCE 1
#define MUX_SIZE 16

// Wired up the pots the wrong way round!
#define INVERTED 1

const int MUX1_READ_PIN = 27;
const int MUX2_READ_PIN = 26;
const int MUX3_READ_PIN = 25;

class MuxIo
{
public:
    MuxIo(int id)
    {
        _muxId = id;
        _activePins = MUX_SIZE;
        _inputPin = _muxId == 1
                        ? MUX1_READ_PIN
                    : _muxId == 2
                        ? MUX2_READ_PIN
                        : MUX3_READ_PIN;

        for (int i = 0; i < MUX_SIZE; i++)
        {
            _commitValues[i] = -1;
        }
    }

    void read(int input)
    {
        if (input >= _activePins)
        {
            return;
        }

        analogRead(_inputPin);

        int rawValue = analogRead(_inputPin);

        if (INVERTED)
        {
            rawValue = 1023 - rawValue;
        }

        int oldValue = _currentValues[input].value();
        int value = _currentValues[input].read(rawValue);

        if (oldValue != value)
        {
            _bufferValues[input] = value;

            _bufferChanged = true;
        }
    }

    void debug()
    {
        Serial.printf("%d:", _muxId);

        for (int i = 0; i < _activePins; i++)
        {
            if ((i % 4) == 0)
            {
                Serial.print(" ");
            }

            Serial.printf("%01X", _bufferValues[i] >> 6);
        }

        Serial.print("  ");
    }

    void commitBufferChanges(ControllerIoListener *ioListener)
    {
        for (int i = 0; i < _activePins; i++)
        {
            int smoothedValue = (_bufferValues[i] + _commitValues[i]) / 2;

            if (debounce(smoothedValue, _commitValues[i], DEBOUNCE_DISTANCE))
            {
                _commitValues[i] = smoothedValue;

                ioListener->onControllerIoChanged(_muxId, i, smoothedValue);
            }
        }
    }

private:
    int _activePins;
    int _muxId;
    int _inputPin;
    AnalogBuffer _currentValues[MUX_SIZE];
    int _bufferValues[MUX_SIZE];
    int _commitValues[MUX_SIZE];
    bool _bufferChanged;

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