#ifndef WAVEFORMSTORE_H
#define WAVEFORMSTORE_H

#include <EEPROM.h>
#include <Wire.h>
#include "../io/Indicators.h"

const int SLOT_SIZE_BYTES = 512;
const int CHUNK_SIZE_BYTES = 64;

class WaveformStore
{
public:
    WaveformStore(Indicators *indicators)
    {
        _indicators = indicators;
    }
    void init()
    {
        for (int i = 0; i < 6; i++)
        {
            EEPROM.get(i * SLOT_SIZE_BYTES, _waveformBank[i]);
        }
    }

    int16_t *waveformData(uint8_t id)
    {
        return _waveformBank[id % 6];
    }

    void midiHandleSystemExclusive(byte *array, unsigned int size)
    {
        if (size < 260)
        {
            Serial.printf("MIDI SysEx: Invalid size: %d\n", size);
            return;
        }

        int targetChunk = array[3] % 4;
        int targetSlot = array[4] % 4;
        int dataIndex = 5;

        Serial.printf("MIDI SysEx: %02x %02x %02x; SLOT %d, CHUNK %d\n", array[0], array[1], array[2], targetSlot, targetChunk);

        for (int i = 0; i < 64; i++)
        {
            uint32_t unsignedValue = 0;

            for (int j = 0; j < 4; j++)
            {
                unsignedValue = (unsignedValue << 4) + hexToNibble((char)array[dataIndex++]);
            }

            int16_t signedValue = (int16_t)(unsignedValue - 32768);

            _waveformBank[targetSlot][i] = signedValue;
        }

        uint32_t eepromAddress = targetSlot * SLOT_SIZE_BYTES + targetChunk * CHUNK_SIZE_BYTES;

        EEPROM.put(eepromAddress, _waveformBank[targetSlot]);

        _indicators->waveformUploaded(micros(), targetSlot);

        Serial.printf("wrote %d bytes to %08X\n", 256, eepromAddress);
    }

private:
    int16_t _waveformBank[6][256];
    Indicators *_indicators;

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