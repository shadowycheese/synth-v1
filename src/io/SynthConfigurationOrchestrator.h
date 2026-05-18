#ifndef SYNTHCONFIGURATIONORCHESTRATOR_H
#define SYNTHCONFIGURATIONORCHESTRATOR_H

#include "../Constants.h"
#include "MuxIo.h"
#include "MidiIo.h"
#include "ControllerIoListener.h"

enum ReadState : uint8_t
{
    STATE_SELECT_MUX = 0,
    STATE_WAIT_MUX = 1,
    STATE_READ_MUX = 2,
    STATE_NOTIFY = 3
};

#define MUX_ENABLE_PIN 28
#define MUX_SELECT3_PIN 29
#define MUX_SELECT2_PIN 30
#define MUX_SELECT1_PIN 31
#define MUX_SELECT0_PIN 32

#define DEBUG 1

const int MUX_COUNT = 1;

class SynthConfigurationOrchestrator
{
public:
    SynthConfigurationOrchestrator(ControllerIoListener *controllerListener) : _mux1(INPUT_GROUP_MUX1, 16),
                                                                               _mux2(INPUT_GROUP_MUX2, 16),
                                                                               _mux3(INPUT_GROUP_MUX3, 16),
                                                                               _ioReads("IO Read Cycles"),
                                                                               _commits("IO Commits")
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

    void task(uint32_t microSeconds)
    {
        switch (_state)
        {
        case STATE_SELECT_MUX:
            digitalWriteFast(MUX_SELECT3_PIN, (_currentInput & 8) ? HIGH : LOW);
            digitalWriteFast(MUX_SELECT2_PIN, (_currentInput & 4) ? HIGH : LOW);
            digitalWriteFast(MUX_SELECT1_PIN, (_currentInput & 2) ? HIGH : LOW);
            digitalWriteFast(MUX_SELECT0_PIN, (_currentInput & 1) ? HIGH : LOW);
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
            _mux1.read(_currentInput);
            _mux2.read(_currentInput);
            _mux3.read(_currentInput);

            _state = STATE_SELECT_MUX;
            _currentInput = (_currentInput + 1) & 0xF;

            if (_currentInput == 0)
            {
                _ioReads.inc(microSeconds);
            }
            break;
        }

        updateSynthConfiguration(microSeconds);
    }

    MidiIo *midiHandler() { return &_midi; }

private:
    ControllerIoListener *_controllerListener;

    int _currentInput;
    int _state;
    uint32_t _readTime;
    uint32_t _notifyTime;
    MuxIo _mux1;
    MuxIo _mux2;
    MuxIo _mux3;
    MidiIo _midi;

    CallCounter _ioReads;
    CallCounter _commits;

    uint8_t _debugWrite = 0;

    void updateSynthConfiguration(uint32_t microSeconds)
    {
        if (microSeconds >= _notifyTime)
        {
            if (DEBUG)
            {
                if (++_debugWrite > 50)
                {
                    _mux1.debug();
                    _mux2.debug();
                    _mux3.debug();
                    Serial.println();

                    _debugWrite = 0;
                }
            }

            _mux1.commitBufferChanges(_controllerListener);
            _mux2.commitBufferChanges(_controllerListener);
            _mux3.commitBufferChanges(_controllerListener);
            _midi.commitBufferChanges(_controllerListener);

            _controllerListener->commit();

            _notifyTime = microSeconds + 10000;

            _commits.inc(microSeconds);
        }
    }
};

#endif