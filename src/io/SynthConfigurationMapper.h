#ifndef SYNTHCONFIGURATIONMAPPER_H
#define SYNTHCONFIGURATIONMAPPER

#include "../Constants.h"
#include "ControllerIoListener.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"

class SynthConfigurationMapper : public ControllerIoListener
{
public:
    SynthConfigurationMapper(SynthConfiguration *configuration, SynthConfigurationListener *configuratonListener)
    {
        _synthConfiguration = configuration;
        _synthConfigurationListener = configuratonListener;
    }

    void commit()
    {
        if (_changeFlags)
        {
            _synthConfiguration->copy(&_localSynthConfiguration);

            _synthConfigurationListener->onSynthConfigurationChanged(_synthConfiguration, _changeFlags);
        }

        _changeFlags = 0;
    }

    void onControllerIoChanged(int group, int input, int value)
    {
        Func handler = getIoHandler(group, input);

        _changeFlags |= (this->*handler)(value);
    }

private:
    typedef int (SynthConfigurationMapper::*Func)(int);

    Func mux1Inputs[10] = {
        &SynthConfigurationMapper::updateAttack,
        &SynthConfigurationMapper::updateDecay,
        &SynthConfigurationMapper::updateSustain,
        &SynthConfigurationMapper::updateRelease,
        &SynthConfigurationMapper::updateDetune,
        &SynthConfigurationMapper::updateMainWaveForm,
        &SynthConfigurationMapper::updateOscillators,
        &SynthConfigurationMapper::updateVoiceGain,
        &SynthConfigurationMapper::updateMixerGain,
        &SynthConfigurationMapper::updateResonance,
    };

    Func getIoHandler(int group, int input)
    {
        return mux1Inputs[input];
    }

    SynthConfiguration _localSynthConfiguration;
    SynthConfiguration *_synthConfiguration;
    SynthConfigurationListener *_synthConfigurationListener;

    int _changeFlags;

    // Envelope
    int updateAttack(int value)
    {
        float newValue = value / 2.0f;

        if (newValue != _localSynthConfiguration.attack)
        {
            _localSynthConfiguration.attack = newValue;

            return ENVELOPE_CHANGED;
        }

        return 0;
    }

    int updateDecay(int value)
    {
        float newValue = value / 2.0f;

        if (newValue != _localSynthConfiguration.decay)
        {
            _localSynthConfiguration.decay = newValue;

            return ENVELOPE_CHANGED;
        }

        return 0;
    }

    int updateSustain(int value)
    {
        float newValue = value / 2.0f;

        if (newValue != _localSynthConfiguration.sustain)
        {
            _localSynthConfiguration.sustain = newValue;

            return ENVELOPE_CHANGED;
        }

        return 0;
    }

    int updateRelease(int value)
    {
        float newValue = value * 5.0f;

        if (newValue != _localSynthConfiguration.release)
        {
            _localSynthConfiguration.release = newValue;

            return ENVELOPE_CHANGED;
        }

        return 0;
    }

    // Volume
    int updateVoiceGain(int value)
    {
        float newValue = value / 1023.0f;

        if (newValue != _localSynthConfiguration.voiceGain)
        {
            _localSynthConfiguration.voiceGain = newValue;

            return VOLUME_CHANGED;
        }

        return 0;
    }

    int updateMixerGain(int value)
    {
        float newValue = value / 1023.0f;

        if (newValue != _localSynthConfiguration.mixerGain)
        {
            _localSynthConfiguration.mixerGain = newValue;

            return VOLUME_CHANGED;
        }

        return 0;
    }

    int updateFilterGain(int value)
    {
        float newValue = value / 1023.0f;

        if (newValue != _localSynthConfiguration.filterGain)
        {
            _localSynthConfiguration.filterGain = newValue;

            return VOLUME_CHANGED;
        }

        return 0;
    }

    int updateMasterGain(int value)
    {
        float newValue = value / 1023.0f;

        if (newValue != _localSynthConfiguration.masterVolume)
        {
            _localSynthConfiguration.masterVolume = newValue;

            return VOLUME_CHANGED;
        }

        return 0;
    }

    // Wave form
    int updateMainWaveForm(int value)
    {
        int newValue = getWaveForm(value);

        if (newValue != _localSynthConfiguration.mainWaveForm)
        {
            _localSynthConfiguration.mainWaveForm = newValue;
            _localSynthConfiguration.detuneWaveForm = newValue; // for now

            return WAVEFORM_CHANGED;
        }

        return 0;
    }

    int updateDetuneWaveForm(int value)
    {
        int newValue = getWaveForm(value);

        if (newValue != _localSynthConfiguration.detuneWaveForm)
        {
            _localSynthConfiguration.detuneWaveForm = newValue;

            return WAVEFORM_CHANGED;
        }

        return 0;
    }

    int updateOscillators(int value)
    {
        int oscillators = -1;
        if (value < 200)
        {
            oscillators = 1;
        }
        else if (value < 400)
        {
            oscillators = 3;
        }
        else if (value < 600)
        {
            oscillators = 5;
        }
        else if (value < 800)
        {
            oscillators = 7;
        }

        if (oscillators != _localSynthConfiguration.oscillators)
        {
            _localSynthConfiguration.oscillators = oscillators;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    // Voice configuration
    int updateDetune(int value)
    {
        float newValue = 1 + (value / 4095.0f);

        if (newValue != _localSynthConfiguration.detune)
        {
            _localSynthConfiguration.detune = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    int updateResonance(int value)
    {
        float newValue = 1.8f * (value / 1023.0f);

        if (newValue != _localSynthConfiguration.resonance)
        {
            _localSynthConfiguration.resonance = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    int getWaveForm(int value)
    {
        if (value < 200)
        {
            return WAVEFORM_SINE;
        }
        else if (value < 400)
        {
            return WAVEFORM_SAWTOOTH;
        }
        else if (value < 600)
        {
            return WAVEFORM_TRIANGLE;
        }
        else if (value < 800)
        {
            return WAVEFORM_SQUARE;
        }
        else
        {
            return WAVEFORM_ARBITRARY;
        }
    }
};

#endif