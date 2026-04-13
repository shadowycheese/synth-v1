#ifndef CONTROLLERIO_H
#define CONTROLLERIO_H

#include "../Constants.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"

const int STATE_SELECT_MUX1 = 0;
const int STATE_WAIT_MUX1 = 1;
const int STATE_READ_MUX1 = 2;
const int STATE_NOTIFY1 = 3;

const int MUX1_ENABLE_PIN = 28;
const int MUX1_SELECT0_PIN  = 32;
const int MUX1_SELECT1_PIN = 31;
const int MUX1_SELECT2_PIN = 30;
const int MUX1_SELECT3_PIN = 29;
const int MUX1_READ_PIN = A13;

class ControllerIo {
public:
    ControllerIo(SynthConfiguration *configuration, SynthConfigurationListener *configuratonListener) {
        _synthConfiguration = configuration;
        _synthConfigurationListener = configuratonListener;
    }   
    
    void begin()
    {
        pinMode(MUX1_ENABLE_PIN, OUTPUT);
        pinMode(MUX1_SELECT0_PIN, OUTPUT);
        pinMode(MUX1_SELECT1_PIN, OUTPUT);
        pinMode(MUX1_SELECT2_PIN, OUTPUT);
        pinMode(MUX1_SELECT3_PIN, OUTPUT);
        pinMode(MUX1_READ_PIN, OUTPUT);

        digitalWriteFast(MUX1_ENABLE_PIN, LOW);

        analogReadAveraging(5);
    }

    void task() 
    {
        uint32_t microSeconds = micros();

        if ((_readTime - microSeconds) > 500) 
        {
            _readTime = microSeconds;
        }

        switch(_state)
        {
            case STATE_SELECT_MUX1:
                digitalWriteFast(MUX1_SELECT0_PIN, (_currentInput & 1) > 0 ? HIGH : LOW);
                digitalWriteFast(MUX1_SELECT1_PIN, (_currentInput & 2) > 0 ? HIGH : LOW);
                digitalWriteFast(MUX1_SELECT2_PIN, (_currentInput & 4) > 0 ? HIGH : LOW);
                digitalWriteFast(MUX1_SELECT3_PIN, (_currentInput & 8) > 0 ? HIGH : LOW);
                _readTime = microSeconds + 10;
                _state = STATE_WAIT_MUX1;
                break;
            case STATE_WAIT_MUX1:
                if (microSeconds > _readTime) 
                {
                    _state = STATE_READ_MUX1;
                }
                break;
            case STATE_READ_MUX1:
                int value = analogRead(MUX1_READ_PIN);
                int oldValue = _currentValues[_currentInput];

                if (oldValue != value) 
                {
                    _currentValues[_currentInput] = value;

                    Serial.printf("%d: %d (was %d)\n", _currentInput, value, oldValue);

                    updateSynthConfiguration(_currentInput, value);
                }
                _state = STATE_SELECT_MUX1;

                _currentInput ++;

                if (_currentInput >= 10) 
                {
                    _currentInput = 0;
                }
                break;
        }

        notifyIfNecessary();
    }
private:
    SynthConfiguration *_synthConfiguration;
    SynthConfigurationListener *_synthConfigurationListener;

    int _currentInput;
    int _state;
    uint32_t _readTime;
    uint32_t _notifyTime;
    int _currentValues[10];
    bool _configChanged;

    void updateSynthConfiguration(int input, int value) 
    {
        Serial.println("Change");
        switch(input) 
        {
            case 0: 
                _synthConfiguration -> attack = value;
            case 1: 
                _synthConfiguration -> decay = 2 * value;
            case 2: 
                _synthConfiguration -> sustain = value / 1024.0f;
            case 3: 
                _synthConfiguration -> release = 2 * value;
            case 4: 
                _synthConfiguration -> detune = 1.0f + value / 2048.0f;
        }
        _configChanged = true;
    }

    void notifyIfNecessary() 
    {
        uint32_t m = millis();

        if (m >= _notifyTime) 
        {
            if (_configChanged) 
            {
                _synthConfigurationListener -> onSynthConfigurationChanged(true, true, true, true);

                _configChanged = false;
            }

            _notifyTime = m + 2;
        }
    }

};

#endif