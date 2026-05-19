#ifndef PRESETSTORE_H
#define PRESETSTORE_H

#include <EEPROM.h>
#include <Wire.h>
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"

class PresetStore : public SynthConfigurationListener
{
public:
    PresetStore(SynthConfigurationListener *configurationListener)
    {
        _currentPreset = -1;
        _configurationListener = configurationListener;
    }

    void init()
    {
        for (int i = 0; i < 3; i++)
        {
            read(i);
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

        _presetConfiguration[_currentPreset - 1].copy(&_buttonConfiguration);

        int presetLocation = PRESET_LOCATION + (_currentPreset - 1) * PRESET_SIZE;

        EEPROM.put(presetLocation, _presetConfiguration[_currentPreset - 1]);
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

                if (_currentPreset == 0 || !_presetValid[_currentPreset - 1])
                {
                    Serial.printf("Selecting new preset %d\n", _currentPreset);

                    _configurationListener->onSynthConfigurationChanged(&_buttonConfiguration, ALL_CHANGED);
                }
                else
                {
                    Serial.printf("Selecting raw config %d\n", _currentPreset);

                    _configurationListener->onSynthConfigurationChanged(&_presetConfiguration[_currentPreset - 1], ALL_CHANGED);
                }
            }
        }

        if (_currentPreset == 0)
        {
            _configurationListener->onSynthConfigurationChanged(&_buttonConfiguration, changeFlags);
        }
    }

private:
    SynthConfiguration _presetConfiguration[3];
    SynthConfiguration _buttonConfiguration;
    SynthConfigurationListener *_configurationListener;
    bool _presetValid[3];
    uint8_t _currentPreset;

    const int PRESET_LOCATION = 3100;
    const int PRESET_SIZE = 300;

    void read(int id)
    {
        int presetLocation = PRESET_LOCATION + id * PRESET_SIZE;

        uint32_t magic;

        EEPROM.get(presetLocation, magic);

        _presetValid[id] = magic == MAGIC_CODE;

        if (_presetValid[id])
        {
            Serial.printf("Loaded valid preset: %d\n", id + 1);

            _presetValid[id] = true;

            EEPROM.get(presetLocation, _presetConfiguration[id]);
        }
        else
        {
            Serial.printf("Not loaded invalid preset: %d\n", id + 1);
        }
    }
};

#endif