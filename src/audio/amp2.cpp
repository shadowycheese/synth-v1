/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Arduino.h>
#include "amp2.h"
#include "utility/dspinst.h"

int32_t AudioAmplifier2::applySaturation(int32_t value)
{
    int32_t space, sattmp;

    // value >>= 16;

    if (value >= 32767)
    {
        saturation += value - 32767;
        value = 32767;
    }
    else if (value <= -32768)
    {
        saturation += value + 32768;
        value = -32768;
    }
    else if (value >= 0)
    {
        space = 32767 - value;

        sattmp = min(space, saturation / 4);
        saturation = max(0, saturation - sattmp);

        value += sattmp;
    }
    else
    {
        space = (value + 32768);

        sattmp = min(space, saturation / 4);
        saturation = max(0, saturation - sattmp);

        value -= sattmp;
    }
    value &= 0xFFFF;

    return value;
}

void AudioAmplifier2::applyGain2(int16_t *data, int32_t mult)
{
    uint32_t *p = (uint32_t *)data;
    const uint32_t *end = (uint32_t *)(data + AUDIO_BLOCK_SAMPLES);

    do
    {
        uint32_t tmp32 = *p; // read 2 samples from *data
        int32_t val1 = signed_multiply_32x16b(mult, tmp32);
        int32_t val2 = signed_multiply_32x16t(mult, tmp32);
        val1 = applySaturation(val1);
        val2 = applySaturation(val2);
        *p++ = pack_16b_16b(val2, val1);
    } while (p < end);
}

void AudioAmplifier2::update(void)
{
    audio_block_t *block;
    int32_t mult = multiplier;

    if (mult == 0)
    {
        // zero gain, discard any input and transmit nothing
        block = receiveReadOnly(0);
        if (block)
            release(block);
    }
    else if (mult == 65536)
    {
        // unity gain, pass input to output without any change
        block = receiveReadOnly(0);
        if (block)
        {
            transmit(block);
            release(block);
        }
    }
    else
    {
        // apply gain to signal
        block = receiveWritable(0);
        if (block)
        {
            applyGain2(block->data, mult);
            transmit(block);
            release(block);
        }
    }
}
