#ifndef PRESET_STORE_H
#define PRESET_STORE_H

#include <EEPROM.h>
#include <Wire.h>
#include "../config/SynthConfiguration.h"
#include "../config/SynthConfigurationListener.h"

class PresetStore : public SynthConfigurationListener
{
public:
    PresetStore(SynthConfigurationListener *configurationListener)
    {
        _currentPreset = 0;
        _configurationListener = configurationListener;
    }

    void init()
    {
        for (int i = 1; i <= 3; i++)
        {
            read(i);
        }
    }

    // This will save the preset when the top and bottom two notes on my midi keyboard
    // are pressed (I ran out of inputs for a dedicated button and this is my hacky solution)
    void checkStorePreset(byte note, bool on)
    {
        if (on)
        {
            switch (note)
            {
            case 0x24:
                _storePresetBitMask |= 1;
                break;
            case 0x25:
                _storePresetBitMask |= 2;
                break;
            case 0x5F:
                _storePresetBitMask |= 4;
                break;
            case 0x60:
                _storePresetBitMask |= 8;
                break;
            }
        }
        else
        {
            switch (note)
            {
            case 0x24:
                _storePresetBitMask &= 0x0E;
                break;
            case 0x25:
                _storePresetBitMask &= 0x0D;
                break;
            case 0x5F:
                _storePresetBitMask &= 0x0B;
                break;
            case 0x60:
                _storePresetBitMask &= 0x07;
                break;
            }
        }

        if (_storePresetBitMask == 0xF)
        {
            storePreset();
        }
    }

    void storePreset()
    {
        if (_currentPreset == 0)
        {
            Serial.println("Not saving preset");

            return;
        }

        Serial.printf("Saving current preset: %d\n", _currentPreset);

        int presetLoc = presetLocation(_currentPreset);
        int presetIndex = _currentPreset - 1;

        _presetConfiguration[presetIndex].copy(&_buttonConfiguration);
        _presetValid[presetIndex] = true;

        EEPROM.put(presetLoc, _presetConfiguration[presetIndex]);

        notifyPresetUpdate();
    }

    void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags)
    {
        _buttonConfiguration.copy(configuration);

        if (presetChanged(changeFlags))
        {
            uint8_t newPreset = configuration->preset1 + (configuration->preset2 << 1);

            if (newPreset != _currentPreset)
            {
                _currentPreset = newPreset;

                if (_currentPreset == 0)
                {
                    Serial.printf("Selecting button configuration settings %d\n", _currentPreset);

                    notifyStandardUpdate(ALL_CHANGED);
                }
                else
                {
                    if (!isValid(_currentPreset))
                    {
                        storePreset();
                    }
                    else
                    {
                        Serial.printf("Selecting preset %d\n", _currentPreset);

                        notifyPresetUpdate();

                        return;
                    }
                }
            }
        }

        if (_currentPreset == 0)
        {
            notifyStandardUpdate(changeFlags);
        }
        else if (isPassthroughChange(changeFlags))
        {
            notifyPassThroughUpdate(configuration, changeFlags);
        }
    }

private:
    SynthConfiguration _presetConfiguration[3];
    SynthConfiguration _buttonConfiguration;
    SynthConfigurationListener *_configurationListener;
    bool _presetValid[3];
    uint8_t _currentPreset;
    uint8_t _storePresetBitMask;

    const uint32_t PRESET_LOCATION = 3100;
    const uint32_t PRESET_SIZE = 300;

    bool isValid(int presetId)
    {
        if (presetId <= 0 || presetId >= 4)
        {
            return false;
        }
        return _presetValid[presetId - 1];
    }

    void notifyPresetUpdate()
    {
        _configurationListener->onSynthConfigurationChanged(&_presetConfiguration[_currentPreset - 1], ALL_CHANGED);
    }

    void notifyStandardUpdate(uint16_t changeFlags)
    {
        _configurationListener->onSynthConfigurationChanged(&_buttonConfiguration, changeFlags);
    }

    void notifyPassThroughUpdate(SynthConfiguration *configuration, uint16_t changeFlags)
    {
        int presetIndex = _currentPreset - 1;
        bool notify = false;

        if (configuration->pitchBend != _presetConfiguration[presetIndex].pitchBend)
        {
            _presetConfiguration[presetIndex].pitchBend = configuration->pitchBend;

            notify = true;
        }
        if (configuration->ampGain != _presetConfiguration[presetIndex].ampGain)
        {
            _presetConfiguration[presetIndex].ampGain = configuration->ampGain;

            notify = true;
        }

        if (notify)
        {
            _configurationListener->onSynthConfigurationChanged(&_presetConfiguration[presetIndex], changeFlags);
        }
    }

    void read(int presetId)
    {
        int presetLoc = presetLocation(presetId);
        int presetIndex = presetId - 1;

        uint32_t magic;

        EEPROM.get(presetLoc, magic);

        _presetValid[presetIndex] = magic == MAGIC_CODE;

        if (_presetValid[presetIndex])
        {
            Serial.printf("Loaded valid preset: %d\n", presetId);

            _presetValid[presetIndex] = true;

            EEPROM.get(presetLoc, _presetConfiguration[presetIndex]);
        }
        else
        {
            Serial.printf("Not loaded invalid preset: %d\n", presetId);
        }
    }

    uint32_t presetLocation(int presetId)
    {
        if (presetId <= 0 || presetId >= 4)
        {
            return -1;
        }

        return PRESET_LOCATION + ((presetId - 1) * PRESET_SIZE);
    }
};

#endif