#ifndef WAVEFORMSTORE_H
#define WAVEFORMSTORE_H

#include <EEPROM.h>
#include <Wire.h>

const int SLOT_SIZE_BYTES = 512; // 256 samples * 2 bytes each

class WaveformStore
{
public:
    int16_t *waveformData(uint8_t id)
    {
        return waveformBank[id];
    }

    void midiHandleSystemExclusive(byte *array, unsigned int size)
    {
        if (size < 1029)
        {
            return;
        }

        if (array[1] != 0x7D || array[2] != 0x02)
        {
            return;
        }

        int targetSlot = array[3] % 4;
        int dataIndex = 4;

        // Parse data into the specific sub-array slot
        for (int i = 0; i < 256; i++)
        {
            uint32_t n1 = hexToNibble((char)array[dataIndex++]);
            uint32_t n2 = hexToNibble((char)array[dataIndex++]);
            uint32_t n3 = hexToNibble((char)array[dataIndex++]);
            uint32_t n4 = hexToNibble((char)array[dataIndex++]);

            uint32_t unsigned_sample = (n1 << 12) | (n2 << 8) | (n3 << 4) | n4;

            waveformBank[targetSlot][i] = (int16_t)(unsigned_sample - 32768);
        }

        int eepromAddress = targetSlot * SLOT_SIZE_BYTES;

        EEPROM.put(eepromAddress, waveformBank[targetSlot]);
    }

private:
    int16_t waveformBank[4][256];

    inline uint8_t hexToNibble(char c)
    {
        if (c >= '0' && c <= '9')
        {
            return c - '0';
        }
        if (c >= 'a' && c <= 'f')
        {
            return c - 'a' + 10;
        }
        if (c >= 'A' && c <= 'F')
        {
            return c - 'A' + 10;
        }
        return 0;
    }
};

#endif