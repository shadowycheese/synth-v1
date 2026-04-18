#ifndef CONTROLLERIO_H
#define CONTROLLERIO_H

#include "../Constants.h"
#include "Mux.h"
#include "MidiIo.h"
#include "ControllerIoListener.h"

const int STATE_SELECT_MUX = 0;
const int STATE_WAIT_MUX = 1;
const int STATE_READ_MUX = 2;
const int STATE_NOTIFY = 3;

const int MUX_ENABLE_PIN = 28;
const int MUX_SELECT0_PIN = 32;
const int MUX_SELECT1_PIN = 31;
const int MUX_SELECT2_PIN = 30;
const int MUX_SELECT3_PIN = 29;

const int MUX_COUNT = 1;

class ControllerIo
{
public:
    ControllerIo(ControllerIoListener *controllerListener) : mux1(INPUT_GROUP_MUX1, 10)
    {
        _controllerListener = controllerListener;
    }

    void begin()
    {
        pinMode(MUX_ENABLE_PIN, OUTPUT);
        pinMode(MUX_SELECT0_PIN, OUTPUT);
        pinMode(MUX_SELECT1_PIN, OUTPUT);
        pinMode(MUX_SELECT2_PIN, OUTPUT);
        pinMode(MUX_SELECT3_PIN, OUTPUT);

        digitalWriteFast(MUX_ENABLE_PIN, LOW);

        analogReadResolution(10);
    }

    void task()
    {
        uint32_t microSeconds = micros();

        if (_readTime < microSeconds)
        {
            _readTime = microSeconds;
        }

        switch (_state)
        {
        case STATE_SELECT_MUX:
            digitalWriteFast(MUX_SELECT0_PIN, (_currentInput & 1) > 0 ? HIGH : LOW);
            digitalWriteFast(MUX_SELECT1_PIN, (_currentInput & 2) > 0 ? HIGH : LOW);
            digitalWriteFast(MUX_SELECT2_PIN, (_currentInput & 4) > 0 ? HIGH : LOW);
            digitalWriteFast(MUX_SELECT3_PIN, (_currentInput & 8) > 0 ? HIGH : LOW);
            _readTime = microSeconds + 20;
            _state = STATE_WAIT_MUX;
            break;
        case STATE_WAIT_MUX:
            if (microSeconds >= _readTime)
            {
                _state = STATE_READ_MUX;
            }
            break;
        case STATE_READ_MUX:
            mux1.read(_currentInput);

            _state = STATE_SELECT_MUX;
            _currentInput = (_currentInput + 1) & 0xF;
            break;
        }

        updateSynthConfiguration();
    }

    void midiControl(int input, int value)
    {
        midiIo.update(input, value);
    }

private:
    ControllerIoListener *_controllerListener;

    int _currentInput;
    int _state;
    uint32_t _readTime;
    uint32_t _notifyTime;
    Mux mux1;
    MidiIo midiIo;

    void updateSynthConfiguration()
    {
        uint32_t m = millis();

        if (m >= _notifyTime)
        {
            mux1.commitBufferChanges(_controllerListener);
            midiIo.commitBufferChanges(_controllerListener);

            _controllerListener->commit();

            _notifyTime = m + 10;
        }
    }
};

#endif