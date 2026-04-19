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

        Serial.printf("Change %d %d %d\n", group, input, value);

        _changeFlags |= (this->*handler)(value);
    }

private:
    typedef int (SynthConfigurationMapper::*Func)(int);

    Func midiInputs[6] = {
        &SynthConfigurationMapper::updatePitch,
        &SynthConfigurationMapper::updateResonance,
        &SynthConfigurationMapper::updateDetune,
        &SynthConfigurationMapper::updatePulseWidth,
        &SynthConfigurationMapper::updateWaveform0,
        &SynthConfigurationMapper::updateWaveform1,
    };

    Func mux1Inputs[16] = {
        &SynthConfigurationMapper::updateAttack,
        &SynthConfigurationMapper::updateDecay,
        &SynthConfigurationMapper::updateSustain,
        &SynthConfigurationMapper::updateRelease,
        &SynthConfigurationMapper::updatePulseWidth,
        &SynthConfigurationMapper::updateNoise,
        &SynthConfigurationMapper::updateOscillators3,
        &SynthConfigurationMapper::updateOscillators2,
        &SynthConfigurationMapper::updateOscillators1,
        &SynthConfigurationMapper::updateOscillators0,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp};

    Func mux2Inputs[16] = {
        &SynthConfigurationMapper::updateAttack,
        &SynthConfigurationMapper::updateDecay,
        &SynthConfigurationMapper::updateSustain,
        &SynthConfigurationMapper::updateRelease,
        &SynthConfigurationMapper::updateMixerGain,
        &SynthConfigurationMapper::updateDetune,
        &SynthConfigurationMapper::updateResonance,
        &SynthConfigurationMapper::updateManualCutoff,
        &SynthConfigurationMapper::updatePulseWidth,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
    };

    Func getIoHandler(int group, int input)
    {
        if (group == INPUT_GROUP_MIDI)
        {
            return midiInputs[input];
        }

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
            Serial.printf("Attack %f\n", newValue);

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
            Serial.printf("Decay %f\n", newValue);

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
            Serial.printf("Sustain %f\n", newValue);

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
            Serial.printf("Release %f\n", newValue);

            _localSynthConfiguration.release = newValue;

            return ENVELOPE_CHANGED;
        }

        return 0;
    }

    // Volume
    int updateMixerGain(int value)
    {
        float newValue = value / 1023.0f;

        if (newValue != _localSynthConfiguration.mixerGain)
        {
            Serial.printf("Mixer Gain %f\n", newValue);

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
            Serial.printf("Filter Gain %f\n", newValue);

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
            Serial.printf("Master Gain %f\n", newValue);

            _localSynthConfiguration.masterVolume = newValue;

            return VOLUME_CHANGED;
        }

        return 0;
    }

    // Wave form
    int updateWaveform0(int value)
    {
        return updateWaveform(0, value);
    }

    int updateWaveform1(int value)
    {
        updateWaveform(2, value);
        updateWaveform(3, value);

        return updateWaveform(1, value);
    }

    int updateWaveform2(int value)
    {
        return updateWaveform(2, value);
    }

    int updateWaveform3(int value)
    {
        return updateWaveform(3, value);
    }

    int updateWaveform(int waveform, int value)
    {
        int newValue = (value / 128) & 7;

        Serial.printf("updateWaveform %d: %d  %d\n", waveform, _localSynthConfiguration.waveforms[waveform], newValue);

        if (newValue != _localSynthConfiguration.waveforms[waveform])
        {
            Serial.printf("Waveform %d  %d\n", waveform, newValue);

            _localSynthConfiguration.waveforms[waveform] = newValue;

            return WAVEFORM_CHANGED;
        }

        return 0;
    }

    // Voice configuration
    int updateOscillators0(int value)
    {
        return updateOscillators(0, value);
    }

    int updateOscillators1(int value)
    {
        return updateOscillators(1, value);
    }

    int updateOscillators2(int value)
    {
        return updateOscillators(2, value);
    }

    int updateOscillators3(int value)
    {
        return updateOscillators(3, value);
    }

    int updateNoise(int value)
    {
        float newValue = getScaledValue(value, 12, 12);

        newValue *= newValue;

        if (newValue != _localSynthConfiguration.noiseAmplitude)
        {
            Serial.printf("Noise %0.3f\n", newValue);

            _localSynthConfiguration.noiseAmplitude = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    int updateOscillators(int oscilator, int value)
    {
        float newValue = getScaledValue(value, 12, 12);

        newValue *= newValue;

        if (newValue != _localSynthConfiguration.amplitudes[oscilator])
        {
            Serial.printf("Oscillator%d = %0.3f\n", oscilator, newValue);

            _localSynthConfiguration.amplitudes[oscilator] = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    int updatePulseWidth(int value)
    {
        float newValue = value / 1023.0f;

        if (newValue != _localSynthConfiguration.pulseWidth)
        {
            Serial.printf("Pulse Width = %0.3f\n", newValue);

            _localSynthConfiguration.pulseWidth = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    int updateManualCutoff(int value)
    {
        bool newValue = value >= 512 ? 1 : 0;

        if (newValue != _localSynthConfiguration.manualCutoff)
        {
            Serial.printf("Manual Cutoff = %d\n", newValue);

            _localSynthConfiguration.manualCutoff = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    int updateDetune(int value)
    {
        float valueF = getScaledValue(value, 12, 12);
        float newValue = valueF * valueF;

        if (newValue != _localSynthConfiguration.detune)
        {
            Serial.printf("Detune = %0.3f\n", newValue);

            _localSynthConfiguration.detune = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    int updateResonance(int value)
    {
        float newValue = 1.8f * ((value * value) / 1023.0f);

        if (newValue != _localSynthConfiguration.resonance)
        {
            Serial.printf("Resonance Cutoff = %0.3f\n", newValue);

            _localSynthConfiguration.resonance = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    int updatePitch(int value)
    {
        float newValue = (value / 16384.0f);
        Serial.printf("Pitch: %d = %0.3f\n", value, newValue);

        if (newValue != _localSynthConfiguration.pitch)
        {
            Serial.printf("Pitch = %0.3f\n", newValue);

            _localSynthConfiguration.pitch = newValue;

            return VOICE_CONFIGURAITON_CHANGED;
        }

        return 0;
    }

    // Helpers
    int noOp(int value)
    {
        return 0;
    }

    float getScaledValue(int value, int zeroThreshold, int maxThreshold)
    {
        if (value > (1023 - maxThreshold))
        {
            return 1.0f;
        }

        if (value < zeroThreshold)
        {
            return 0.0f;
        }

        float range = 1023.0f - (maxThreshold + zeroThreshold);

        value -= (maxThreshold + zeroThreshold);

        return value / range;
    }
};

#endif