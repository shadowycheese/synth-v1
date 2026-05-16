#ifndef INDICATORS_H
#define INDICATORS_H

#include "../Constants.h"
#include "../voice/VoiceController.h"

const int PIN_VU = 33;
const int PIN_LED_595_DATA = 34;
const int PIN_LED_595_CLOCK = 35;
const int PIN_LED_595_LATCH = 36;
const int PIN_LED_OVERDRIVE = 37;

class Indicators
{
public:
    Indicators()
    {
        pinMode(PIN_LED_595_LATCH, OUTPUT);
        pinMode(PIN_LED_595_DATA, OUTPUT);
        pinMode(PIN_LED_595_CLOCK, OUTPUT);
        pinMode(PIN_LED_OVERDRIVE, OUTPUT);

        analogWriteFrequency(PIN_VU, 22000);
        analogWriteResolution(8);
    }

    void level(float level)
    {
        uint8_t level2 = (uint8_t)(pow(level, 0.4f) * 255.0f);

        _targetLevel = level2;
    }

    void keyboardConnected(uint32_t microSeconds, bool connected)
    {
        _kbConnected = connected;

        if (connected)
        {
            digitalWriteFast(PIN_LED_OVERDRIVE, LOW);
        }
        else
        {
            digitalWriteFast(PIN_LED_OVERDRIVE, HIGH);

            _targetLevel = 0.0f;
            _targetAdjTime = microSeconds;
        }

        write595(0);
    }

    void overdrive(uint32_t microSeconds)
    {
        if (!_active)
        {
            digitalWriteFast(PIN_LED_OVERDRIVE, HIGH);
        }

        _active = true;
        _timeOff = microSeconds + 200000;
    }

    void voices(uint8_t count)
    {
        if (_voiceCount == count)
        {
            return;
        }

        _voiceCount = count;

        write595(_voiceLedValues[count]);
    }

    void task(uint32_t microSeconds)
    {
        if (!_kbConnected)
        {
            if (microSeconds > _targetAdjTime)
            {
                write595(_kbLedValues[_kbLeds]);

                _kbLeds = (_kbLeds + 1) % 14;

                _targetAdjTime = microSeconds + 150000;
            }

            return;
        }

        if (_active && microSeconds > _timeOff)
        {
            _active = false;

            digitalWriteFast(PIN_LED_OVERDRIVE, LOW);
        }

        if (_level == _targetLevel)
        {
            return;
        }

        if (microSeconds > _targetAdjTime)
        {
            if (_level > _targetLevel)
            {
                _level = max(_targetLevel, _level - 2);

                analogWrite(PIN_VU, _level);
            }
            else if (_level < _targetLevel)
            {
                _level = min(_targetLevel, _level + 20);

                analogWrite(PIN_VU, _level);
            }

            _targetAdjTime = microSeconds + 20000;
        }
    }

private:
    uint32_t _timeOff;
    uint32_t _targetAdjTime;
    bool _active;
    uint8_t _voiceCount;
    uint8_t _level;
    uint8_t _targetLevel;
    bool _kbConnected;
    uint8_t _kbLeds;

    void write595(uint8_t value)
    {
        digitalWriteFast(PIN_LED_595_LATCH, LOW);
        shiftOut(PIN_LED_595_DATA, PIN_LED_595_CLOCK, MSBFIRST, value);
        digitalWriteFast(PIN_LED_595_LATCH, HIGH);
    }

    static constexpr uint8_t _kbLedValues[14] = {4, 2, 1, 8, 32, 16, 64, 128, 64, 16, 32, 8, 1, 2};

    static constexpr uint8_t _voiceLedValues[9] = {0, 4, 6, 7, 15, 47, 63, 127, 255};
};

#endif