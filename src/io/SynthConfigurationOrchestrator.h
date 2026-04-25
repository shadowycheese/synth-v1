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
#define MUX_SELECT3_PIN 32
#define MUX_SELECT2_PIN 31
#define MUX_SELECT1_PIN 30
#define MUX_SELECT0_PIN 29

const int MUX_COUNT = 1;

class SynthConfigurationOrchestrator
{
public:
    SynthConfigurationOrchestrator(ControllerIoListener *controllerListener) : mux1(INPUT_GROUP_MUX1, 16),
                                                                               mux2(INPUT_GROUP_MUX2, 16),
                                                                               ioReads("IO Read Cycles"),
                                                                               commits("IO Commits")
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
            mux2.read(_currentInput);

            _state = STATE_SELECT_MUX;
            _currentInput = (_currentInput + 1) & 0xF;

            if (_currentInput == 0)
            {
                ioReads.inc(microSeconds);
            }
            break;
        }

        updateSynthConfiguration(microSeconds);
    }

    MidiIo *midiHandler() { return &midiIo; }

private:
    ControllerIoListener *_controllerListener;

    int _currentInput;
    int _state;
    uint32_t _readTime;
    uint32_t _notifyTime;
    MuxIo mux1;
    MuxIo mux2;
    MidiIo midiIo;

    CallCounter ioReads;
    CallCounter commits;

    void updateSynthConfiguration(uint32_t microSeconds)
    {
        if (microSeconds >= _notifyTime)
        {
            // mux1.debug();
            // mux2.debug();

            mux1.commitBufferChanges(_controllerListener);
            mux2.commitBufferChanges(_controllerListener);
            midiIo.commitBufferChanges(_controllerListener);

            _controllerListener->commit();

            _notifyTime = microSeconds + 10000;

            commits.inc(microSeconds);
        }
    }
};

#endif