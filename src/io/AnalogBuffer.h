#ifndef ANALOGBUFFER_H
#define ANALOGBUFFER_H

#define BUFFER_SIZE 5

class AnalogBuffer
{
public:
    AnalogBuffer()
    {
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            values[i] = 0;
        }
    }

    int read(int newVal)
    {
        values[pos++] = newVal;

        pos = pos % BUFFER_SIZE;

        return value();
    }

    int value()
    {
        int value = 0;

        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            value += values[i];
        }

        value /= BUFFER_SIZE;

        return value;
    }

private:
    int values[BUFFER_SIZE];
    int pos = 0;
};

#endif