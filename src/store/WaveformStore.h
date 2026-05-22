#ifndef WAVEFORM_STORE_H
#define WAVEFORM_STORE_H

#include <EEPROM.h>
#include <Wire.h>
#include "../io/Indicators.h"

const int SLOT_SIZE_BYTES = 512;
const int CHUNK_SIZE_BYTES = 128;
const int CHUNK_SIZE_WORDS = 64;

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
            uint32_t eepromAddress = i * SLOT_SIZE_BYTES;

            EEPROM.get(eepromAddress, _waveformBank[i]);
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
        int targetOffset = targetChunk * CHUNK_SIZE_WORDS;
        int targetSlot = array[4] % 6;
        int dataIndex = 5;

        Serial.printf("MIDI SysEx: %02x %02x %02x; SLOT %d, CHUNK %d\n", array[0], array[1], array[2], targetSlot, targetChunk);

        for (int i = 0; i < 64; i++)
        {
            uint32_t unsignedValue = 0;

            for (int j = 0; j < 4; j++)
            {
                unsignedValue = (unsignedValue << 4) + hexToNibble((char)array[dataIndex++]);
            }

            int16_t signedValue = (int16_t)(uint16_t)unsignedValue;

            _waveformBank[targetSlot][targetOffset + i] = signedValue;
        }

        if (targetChunk == 3)
        {
            uint32_t eepromAddress = targetSlot * SLOT_SIZE_BYTES;

            EEPROM.put(eepromAddress, _waveformBank[targetSlot]);

            Serial.printf("Last chunk received.  Updated EEPROM Slot %d [%03X]\n", targetSlot, eepromAddress);
        }

        _indicators->waveformUploaded(micros(), targetSlot);
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