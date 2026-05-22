#ifndef ANALOG_BUFFER_H
#define ANALOG_BUFFER_H

#define BUFFER_SIZE 4

class AnalogBuffer
{
public:
    AnalogBuffer()
    {
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            _values[i] = 0;
        }
    }

    int read(int newVal)
    {
        _values[pos++] = newVal;

        pos = pos % BUFFER_SIZE;

        return value();
    }

    int value()
    {
        int value = 0;

        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            value += _values[i];
        }

        value /= BUFFER_SIZE;

        return value;
    }

private:
    int _values[BUFFER_SIZE];
    int pos = 0;
};

#endif