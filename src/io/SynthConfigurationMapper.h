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

        currentInput = input;

        _changeFlags |= (this->*handler)(value);
    }

private:
    typedef int (SynthConfigurationMapper::*Func)(int);

    int currentInput;

    Func midiInputs[7] = {
        &SynthConfigurationMapper::updatePitchBend,
        &SynthConfigurationMapper::updateResonance,
        &SynthConfigurationMapper::updateDetune,
        &SynthConfigurationMapper::updateLfoFrequency,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp};

    Func mux1Inputs[16] = {
        &SynthConfigurationMapper::updateAttack,
        &SynthConfigurationMapper::updateDecay,
        &SynthConfigurationMapper::updateSustain,
        &SynthConfigurationMapper::updateRelease,
        &SynthConfigurationMapper::updateManualCutoff,
        &SynthConfigurationMapper::updateOctaveControl,
        &SynthConfigurationMapper::updateOscillatorAmplitude3,
        &SynthConfigurationMapper::updateOscillatorAmplitude2,
        &SynthConfigurationMapper::updateOscillatorAmplitude1,
        &SynthConfigurationMapper::updateOscillatorAmplitude0,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp};

    Func mux2Inputs[16] = {
        &SynthConfigurationMapper::updateVoiceGain,
        &SynthConfigurationMapper::updateLfoWaveform,
        &SynthConfigurationMapper::updatePitchLevel,
        &SynthConfigurationMapper::updateWaveform3,
        &SynthConfigurationMapper::updateLfoAmplitude,
        &SynthConfigurationMapper::updateWaveform2,
        &SynthConfigurationMapper::updateLfoPulseWidth,
        &SynthConfigurationMapper::updateWaveform1,
        &SynthConfigurationMapper::updateWaveform0,
        &SynthConfigurationMapper::updateFilterLevel,
        &SynthConfigurationMapper::updateAutoCutoff,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
    };

    Func getIoHandler(int group, int input)
    {
        switch (group)
        {
        case INPUT_GROUP_MIDI:
            return midiInputs[input];
        case INPUT_GROUP_MUX1:
            return mux1Inputs[input];
        case INPUT_GROUP_MUX2:
            return mux2Inputs[input];
        default:
            return &SynthConfigurationMapper::noOp;
        }
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
    int updateVoiceGain(int value)
    {
        float newValue = value / 1023.0f;

        newValue *= newValue;

        if (newValue != _localSynthConfiguration.voiceGain)
        {
            Serial.printf("Voice Gain %f\n", newValue);

            _localSynthConfiguration.voiceGain = newValue;

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

        if (newValue != _localSynthConfiguration.waveforms[waveform])
        {
            Serial.printf("Waveform %d  %d\n", waveform, newValue);

            _localSynthConfiguration.waveforms[waveform] = newValue;

            return WAVEFORM_CHANGED;
        }

        return 0;
    }

    int updateLfoWaveform(int value)
    {
        int newValue = (value / 128) & 7;

        if (newValue != _localSynthConfiguration.lfoWaveform)
        {
            Serial.printf("LFO Waveform %d\n", newValue);

            _localSynthConfiguration.lfoWaveform = newValue;

            return WAVEFORM_CHANGED;
        }

        return 0;
    }

    // Voice configuration
    int updateOscillatorAmplitude0(int value)
    {
        return updateOscillatorAmplitude(0, value);
    }

    int updateOscillatorAmplitude1(int value)
    {
        return updateOscillatorAmplitude(1, value);
    }

    int updateOscillatorAmplitude2(int value)
    {
        return updateOscillatorAmplitude(2, value);
    }

    int updateOscillatorAmplitude3(int value)
    {
        return updateOscillatorAmplitude(3, value);
    }

    int updateNoiseAmplitude(int value)
    {
        float newValue = getScaledValue(value, 12, 12);

        newValue *= newValue;

        if (newValue != _localSynthConfiguration.noiseAmplitude)
        {
            Serial.printf("Noise %0.3f\n", newValue);

            _localSynthConfiguration.noiseAmplitude = newValue;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateOscillatorAmplitude(int oscilator, int value)
    {
        float newValue = getScaledValue(value, 12, 12);

        newValue *= newValue;

        if (newValue != _localSynthConfiguration.amplitudes[oscilator])
        {
            Serial.printf("Oscillator%d = %0.3f\n", oscilator, newValue);

            _localSynthConfiguration.amplitudes[oscilator] = newValue;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateLfoPulseWidth(int value)
    {
        float newValue = (getMidScaledValue(value, 12, 12, 12) + 1.0f) / 2;

        if (newValue != _localSynthConfiguration.lfoPulseWidth)
        {
            Serial.printf("LFO Pulse Width = %0.3f\n", newValue);

            _localSynthConfiguration.lfoPulseWidth = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateLfoFrequency(int value)
    {
        float newValue = getScaledValue(value, 12, 12);

        newValue *= newValue * 1000.0f;

        if (newValue != _localSynthConfiguration.lfoFrequency)
        {
            Serial.printf("LFO Frequncy = %0.1f\n", newValue);

            _localSynthConfiguration.lfoFrequency = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateLfoAmplitude(int value)
    {
        float newValue = getScaledValue(value, 12, 12);

        newValue *= newValue;

        if (newValue != _localSynthConfiguration.lfoAmplitude)
        {
            Serial.printf("LFO Amplitude = %0.1f\n", newValue);

            _localSynthConfiguration.lfoAmplitude = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateOctaveControl(int value)
    {
        float newValue = getMidScaledValue(value, 12, 12, 12);

        newValue *= 8.0f;

        if (newValue != _localSynthConfiguration.octaveControl)
        {
            Serial.printf("Octave Control = %0.3f\n", newValue);

            _localSynthConfiguration.octaveControl = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateAutoCutoff(int value)
    {
        bool newValue = value >= 512 ? true : false;

        if (newValue != _localSynthConfiguration.autoCutoff)
        {
            Serial.printf("Is Auto Cutoff = %s\n", newValue ? "true" : "false");

            _localSynthConfiguration.autoCutoff = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateFilterLevel(int value)
    {
        float valueF = getScaledValue(value, 12, 12);
        if (valueF != _localSynthConfiguration.filterLevel)
        {
            Serial.printf("Filter Level = %0.3f\n", valueF);

            _localSynthConfiguration.filterLevel = valueF;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updatePitchLevel(int value)
    {
        float valueF = getScaledValue(value, 12, 12);

        valueF *= valueF;

        if (valueF != _localSynthConfiguration.pitchLevel)
        {
            Serial.printf("Pitch Level = %0.3f\n", valueF);

            _localSynthConfiguration.pitchLevel = valueF;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateDetune(int value)
    {
        float valueF = getScaledValue(value, 12, 12);
        float newValue = valueF * valueF * valueF;

        if (newValue != _localSynthConfiguration.detune)
        {
            Serial.printf("Detune = %0.3f\n", newValue);

            _localSynthConfiguration.detune = newValue;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateResonance(int value)
    {
        float value2 = getScaledValue(value, 12, 12);

        float newValue = 1.8f * (value2 * value2);

        if (newValue != _localSynthConfiguration.resonance)
        {
            Serial.printf("Resonance = %0.3f\n", newValue);

            _localSynthConfiguration.resonance = newValue;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateManualCutoff(int value)
    {
        float newValue = getScaledValue(value, 12, 12);

        // newValue = newValue < 0 ? -newValue * newValue : newValue * newValue;

        if (newValue != _localSynthConfiguration.manualCutoff)
        {
            Serial.printf("Cutoff = %0.3f\n", newValue);

            _localSynthConfiguration.manualCutoff = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updatePitchBend(int value)
    {
        float newValue = (value / 16384.0f);
        Serial.printf("Pitch: %d = %0.3f\n", value, newValue);

        if (newValue != _localSynthConfiguration.pitchBend)
        {
            Serial.printf("Pitch = %0.3f\n", newValue);

            _localSynthConfiguration.pitchBend = newValue;

            return VOICE_CHANGED;
        }

        return 0;
    }

    // Helpers
    int noOp(int value)
    {
        // Serial.printf("Change %d => %d\n", currentInput, value);
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

    float getMidScaledValue(int value, int minThreshold, int maxThreshold, int centreThreshold)
    {
        if (value > (1023 - maxThreshold))
        {
            return 1.0f;
        }

        if (value < minThreshold)
        {
            return -1.0f;
        }

        int centreMin = 512 - (centreThreshold >> 1);
        int centreMax = centreMin + centreThreshold;

        if (value < centreMin)
        {
            float range = centreMin - minThreshold;

            value -= minThreshold;

            return -(1.0f - (value / range));
        }
        else if (value > centreMax)
        {
            float range = 1023.0f - (centreMax + maxThreshold);

            value -= centreMax;

            return value / range;
        }
        else
        {
            return 0.0f;
        }
    }
};

#endif