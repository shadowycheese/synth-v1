#ifndef INDICATORS_H
#define INDICATORS_H

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
            _nonNoteTime = microSeconds;
        }

        write595(0);
    }

    void waveformUploaded(uint32_t microSeconds, uint8_t waveform)
    {
        _wfUploadFlash = 11;
        _wfUploadId = _wfLedValues[(waveform % 6) & 0x3F];
        _nonNoteTime = microSeconds + 100000;

        write595(_wfUploadId);
    }

    void waveformSelected(uint32_t microSeconds, uint8_t waveform)
    {
        write595(_wfLedValues[waveform % 12]);

        _wfSelection = true;
        _nonNoteTime = microSeconds + 1000000;
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
            if (microSeconds > _nonNoteTime)
            {
                write595(_kbLedValues[_kbLeds]);

                _kbLeds = (_kbLeds + 1) % 14;

                _nonNoteTime = microSeconds + 150000;
            }

            return;
        }
        else if (_wfUploadFlash > 0)
        {
            if (microSeconds > _nonNoteTime)
            {
                _wfUploadFlash--;

                _nonNoteTime = microSeconds + 100000;

                write595(_wfUploadFlash % 2 ? _wfUploadId : 0);
            }
        }
        else if (_wfSelection)
        {
            if (microSeconds > _nonNoteTime)
            {
                _wfSelection = false;

                write595(_voiceLedValues[_voiceCount]);
            }
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
    uint32_t _nonNoteTime;
    bool _kbConnected;
    bool _wfSelection;
    uint8_t _wfUploadFlash;
    uint8_t _wfUploadId;
    uint8_t _kbLeds;

    void write595(uint8_t value)
    {
        digitalWriteFast(PIN_LED_595_LATCH, LOW);
        shiftOut(PIN_LED_595_DATA, PIN_LED_595_CLOCK, MSBFIRST, value);
        digitalWriteFast(PIN_LED_595_LATCH, HIGH);
    }

    // Looks weird because the 8 LED's are not wired up in the correct order.
    static constexpr uint8_t _wfLedValues[12] = {132, 130, 129, 136, 160, 144, 196, 194, 193, 200, 224, 208};

    static constexpr uint8_t _kbLedValues[14] = {4, 2, 1, 8, 32, 16, 64, 128, 64, 16, 32, 8, 1, 2};

    static constexpr uint8_t _voiceLedValues[9] = {0, 4, 6, 7, 15, 47, 63, 127, 255};
};

#endif