#ifndef ANALOGBUFFER_H
#define ANALOGBUFFER_H

class AnalogBuffer
{
public:
    AnalogBuffer()
    {
        for (int i = 0; i < 4; i++)
        {
            values[i] = 0;
        }
    }

    int read(int newVal)
    {
        values[pos++] = newVal;

        pos &= 3;

        return value();
    }

    int value()
    {
        int value = 0;

        for (int i = 0; i < 4; i++)
        {
            value += values[i];
        }

        value /= 4;

        return value;
    }

private:
    int values[4];
    int pos = 0;
};

#endif