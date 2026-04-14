#ifndef CONTROLLERIO_H
#define CONTROLLERIO_H

#include "../Constants.h"
#include "AnalogBuffer.h"
#include "ControllerIoListener.h"

const int STATE_SELECT_MUX1 = 0;
const int STATE_WAIT_MUX1 = 1;
const int STATE_READ_MUX1 = 2;
const int STATE_NOTIFY1 = 3;

const int MUX1_ENABLE_PIN = 28;
const int MUX1_SELECT0_PIN = 32;
const int MUX1_SELECT1_PIN = 31;
const int MUX1_SELECT2_PIN = 30;
const int MUX1_SELECT3_PIN = 29;
const int MUX1_READ_PIN = 27;

class ControllerIo
{
public:
    ControllerIo(ControllerIoListener *controllerListener)
    {
        _controllerListener = controllerListener;
    }

    void begin()
    {
        for (int i = 0; i < 10; i++)
        {
            _commitValues[i] = -1;
        }

        pinMode(MUX1_ENABLE_PIN, OUTPUT);
        pinMode(MUX1_SELECT0_PIN, OUTPUT);
        pinMode(MUX1_SELECT1_PIN, OUTPUT);
        pinMode(MUX1_SELECT2_PIN, OUTPUT);
        pinMode(MUX1_SELECT3_PIN, OUTPUT);

        digitalWriteFast(MUX1_ENABLE_PIN, LOW);

        analogReadResolution(10);
    }

    void task()
    {
        uint32_t microSeconds = micros();

        if ((_readTime - microSeconds) > 500)
        {
            // _readTime = microSeconds;
        }

        switch (_state)
        {
        case STATE_SELECT_MUX1:
            digitalWriteFast(MUX1_SELECT0_PIN, (_currentInput & 1) > 0 ? HIGH : LOW);
            digitalWriteFast(MUX1_SELECT1_PIN, (_currentInput & 2) > 0 ? HIGH : LOW);
            digitalWriteFast(MUX1_SELECT2_PIN, (_currentInput & 4) > 0 ? HIGH : LOW);
            digitalWriteFast(MUX1_SELECT3_PIN, (_currentInput & 8) > 0 ? HIGH : LOW);
            _readTime = microSeconds + 20;
            _state = STATE_WAIT_MUX1;
            break;
        case STATE_WAIT_MUX1:
            if (microSeconds > _readTime)
            {
                _state = STATE_READ_MUX1;
            }
            break;
        case STATE_READ_MUX1:
            int rawValue = analogRead(MUX1_READ_PIN);
            int oldValue = _currentValues[_currentInput].value();
            int value = _currentValues[_currentInput].read(rawValue);

            if (oldValue != value)
            {
                // Serial.printf("%d: %d (was %d)\n", _currentInput, value, oldValue);

                updateSynthConfiguration(_currentInput, value);
            }
            _state = STATE_SELECT_MUX1;

            _currentInput++;

            if (_currentInput >= 10)
            {
                _currentInput = 0;
            }
            break;
        }

        updateSynthConfiguration();
    }

private:
    ControllerIoListener *_controllerListener;

    int _currentInput;
    int _state;
    uint32_t _readTime;
    uint32_t _notifyTime;
    AnalogBuffer _currentValues[10];
    int _bufferValues[10];
    int _commitValues[10];
    bool _bufferChanged;
    int calls = 0, checks = 0;

    void updateSynthConfiguration(int input, int value)
    {
        _bufferValues[input] = value;

        _bufferChanged = true;
    }

    void commitBufferChanges()
    {
        for (int i = 0; i < 10; i++)
        {
            int smoothedValue = (_bufferValues[i] + _commitValues[i]) / 2;

            if (debounce(smoothedValue, _commitValues[i], 1))
            {
                _commitValues[i] = smoothedValue;

                _controllerListener->onControllerIoChanged(MUX0, i, smoothedValue);
            }
        }
    }

    void updateSynthConfiguration()
    {
        calls++;
        uint32_t m = millis();

        if (m >= _notifyTime)
        {
            checks++;

            commitBufferChanges();

            _controllerListener->commit();

            _notifyTime = m + 10;
        }
    }

    bool debounce(int value1, int value2, int minDiff)
    {
        if (value1 < 0 || value2 < 0)
        {
            return true;
        }

        int diff = value1 - value2;

        if (diff < 0)
        {
            diff = -diff;
        }

        return diff > minDiff;
    }
};

#endif