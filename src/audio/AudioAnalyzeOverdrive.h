#ifndef analyze_overdrive_h_
#define analyze_overdrive_h_

#include <Arduino.h>
#include <AudioStream.h>

class AudioAnalyzeOverdrive : public AudioStream
{
public:
    AudioAnalyzeOverdrive(void) : AudioStream(1, inputQueueArray)
    {
        _isOverdriven = false;
    }

    bool isOverdriven(bool reset)
    {
        bool tmp = _isOverdriven;

        if (reset)
        {
            _isOverdriven = false;
        }

        return tmp;
    }

    void update(void)
    {
        audio_block_t *block;

        block = receiveReadOnly();

        if (!block)
        {
            return;
        }

        for (int i = 0; i < AUDIO_BLOCK_SAMPLES && !_isOverdriven; i++)
        {
            int16_t s = block->data[i];

            if (s >= 32766 || s <= -32766)
            {
                _isOverdriven = true;
            }
        };

        release(block);
    }

private:
    audio_block_t *inputQueueArray[1];
    bool _isOverdriven;
};

#endif
