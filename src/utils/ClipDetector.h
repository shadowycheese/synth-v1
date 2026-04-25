#ifndef CLIPDETECTOR_H
#define CLIPDETECTOR_H

#include <Audio.h>

class ClipDetector
{
public:
    ClipDetector(const char *name, AudioStream &source);

    void detect()
    {
        if (analyzer.available())
        {
            _max = max(_max, analyzer.read());
        }
    }

    void debug()
    {
        Serial.printf("%s: %.4f ", _name, _max);

        _max = 0.0f;
    }

private:
    AudioConnection patch;
    AudioAnalyzePeak analyzer;
    char _name[64];
    float _max;
};

class ClipDetectorTask
{
public:
    static void Add(ClipDetector *detector)
    {
        for (int i = 0; i < 16; i++)
        {
            if (_detectors[i] == 0)
            {
                _detectors[i] = detector;
                break;
            }
        }
    }

public:
    void task(uint32_t microSeconds)
    {
        if (microSeconds >= _nextPrint)
        {
            for (int i = 0; i < 16; i++)
            {
                if (_detectors[i] != 0)
                {
                    _detectors[i]->debug();
                }
            }

            _nextPrint = microSeconds + 1000000;

            Serial.println();

            return;
        }

        for (int i = 0; i < 16; i++)
        {
            if (_detectors[i] != 0)
            {
                _detectors[i]->detect();
                break;
            }
        }
    }

private:
    static ClipDetector *_detectors[16];

    uint32_t _nextPrint;
};

#endif