#ifndef CALLCOUNTER_H
#define CALLCOUNTER_H

#include <Audio.h>

#define COUNT_ENABLED 0

class CallCounter
{
public:
    CallCounter(const char *str)
    {
        strcpy(_debugTitle, str);
        _nextDebug = micros();
    }

    void inc(uint32_t time)
    {
        if (COUNT_ENABLED == 0)
        {
            return;
        }

        if (time >= _nextDebug)
        {
            Serial.printf("%s: %d/s\n", _debugTitle, _calls);

            _nextDebug = time + 1000000;
            _calls = 0;
        }
        else
        {
            _calls++;
        }
    }

private:
    char _debugTitle[256];
    uint32_t _nextDebug;
    int _calls;
};

#endif