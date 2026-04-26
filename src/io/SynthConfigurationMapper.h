#ifndef SYNTHCONFIGURATIONMAPPER_H
#define SYNTHCONFIGURATIONMAPPER

#include "../Constants.h"
#include "ControllerIoListener.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"

#define DEAD_ZONE 12

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
        currentGroup = group;

        _changeFlags |= (this->*handler)(value);
    }

private:
    typedef int (SynthConfigurationMapper::*Func)(int);

    int currentInput;
    int currentGroup;

    Func midiInputs[7] = {
        &SynthConfigurationMapper::updatePitchBend,
        &SynthConfigurationMapper::updateResonance,
        &SynthConfigurationMapper::updateLfoPulseWidth,
        &SynthConfigurationMapper::updateLfoFrequency,
        &SynthConfigurationMapper::updateReverb,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp};

    Func mux1Inputs[16] = {
        &SynthConfigurationMapper::updateAttack,
        &SynthConfigurationMapper::updateDecay,
        &SynthConfigurationMapper::updateSustain,
        &SynthConfigurationMapper::updateRelease,
        &SynthConfigurationMapper::updateManualCutoff,
        &SynthConfigurationMapper::updateNoiseAmplitude,
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
        &SynthConfigurationMapper::updateFilterLevel,
        &SynthConfigurationMapper::updateWaveform2,
        &SynthConfigurationMapper::updateDetune,
        &SynthConfigurationMapper::updateWaveform1,
        &SynthConfigurationMapper::updateWaveform0,
        &SynthConfigurationMapper::updateLfoAmplitude,
        &SynthConfigurationMapper::updateAutoCutoff,
        &SynthConfigurationMapper::updateReverbEnabled,
        &SynthConfigurationMapper::updateChorusEnabled,
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
        float newValue = getScaledValue(value, 2);

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
        float newValue = getScaledValue(value, 3);

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
        float newValue = getScaledValue(value, 2);

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
        float newValue = getMidScaledValue(value, 1);

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
        float valueF = getScaledValue(value, 2);

        float newValue = (valueF * 1000.0f);

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
        float newValue = getScaledValue(value, 2);

        if (newValue != _localSynthConfiguration.lfoAmplitude)
        {
            Serial.printf("LFO Amplitude = %0.3f\n", newValue);

            _localSynthConfiguration.lfoAmplitude = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateOctaveControl(int value)
    {
        float valueF = getMidScaledValue(value, 1);
        float newValue = valueF * 8.0f;

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

    int updateReverbEnabled(int value)
    {
        bool newValue = value >= 512 ? true : false;

        if (newValue != _localSynthConfiguration.reverbEnabled)
        {
            Serial.printf("Reverb Enabled = %s\n", newValue ? "true" : "false");

            _localSynthConfiguration.reverbEnabled = newValue;

            return EFFECT_CHANGED;
        }

        return 0;
    }

    int updateFilterLevel(int value)
    {
        float valueF = getScaledValue(value, 1);

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
        float valueF = getScaledValue(value, 2);

        if (valueF != _localSynthConfiguration.pitchLevel)
        {
            Serial.printf("Pitch Level = %0.3f\n", valueF);

            _localSynthConfiguration.pitchLevel = valueF;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateReverb(int value)
    {
        float newValue = getScaledValue(value, 1);

        if (newValue != _localSynthConfiguration.reverb)
        {
            Serial.printf("Reverb = %0.3f\n", newValue);

            _localSynthConfiguration.reverb = newValue;

            return EFFECT_CHANGED;
        }

        return 0;
    }

    int updateChorusEnabled(int value)
    {
        bool newValue = value >= 512 ? true : false;

        if (newValue != _localSynthConfiguration.chorusEnabled)
        {
            Serial.printf("Chorus Enabled = %s\n", newValue ? "true" : "false");

            _localSynthConfiguration.chorusEnabled = newValue;

            return EFFECT_CHANGED;
        }

        return 0;
    }

    int updateDetune(int value)
    {
        float valueF = getScaledValue(value, 3);

        if (valueF != _localSynthConfiguration.detune)
        {
            Serial.printf("Detune = %0.3f\n", valueF);

            _localSynthConfiguration.detune = valueF;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateResonance(int value)
    {
        float valueF = getScaledValue(value, 3);

        float newValue = 0.7f + 4.3f * valueF;

        if (newValue != _localSynthConfiguration.resonance)
        {
            Serial.printf("Resonance = %0.3f\n", newValue);

            _localSynthConfiguration.resonance = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateManualCutoff(int value)
    {
        float newValue = getScaledValue(value, 2);

        if (newValue != _localSynthConfiguration.filterCutoff)
        {
            Serial.printf("Cutoff = %0.3f\n", newValue);

            _localSynthConfiguration.filterCutoff = newValue;

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
        return 0;
    }

    float getScaledValue(int value, int order)
    {
        if (value > (1023 - DEAD_ZONE))
        {
            return 1.0f;
        }

        if (value < DEAD_ZONE)
        {
            return 0.0f;
        }

        float range = 1023.0f - (DEAD_ZONE * 2);

        value -= (DEAD_ZONE * 2);

        float valueF = (float)value / range;

        return fastPow(valueF, order);
    }

    float getMidScaledValue(int value, int order)
    {
        if (value > (1023 - DEAD_ZONE))
        {
            if (currentGroup == 2 && currentInput == 6)
            {
                Serial.printf("= %d => 1.000\n", value);
            }

            return 1.0f;
        }

        if (value < DEAD_ZONE)
        {
            if (currentGroup == 2 && currentInput == 6)
            {
                Serial.printf("= %d => 0.000\n", value);
            }

            return 0.0f;
        }

        int centreMin = 512 - (DEAD_ZONE >> 1);
        int centreMax = centreMin + DEAD_ZONE;

        if (value < centreMin)
        {
            float range = centreMin - DEAD_ZONE;
            float valueF = (float)(value - DEAD_ZONE);

            if (currentGroup == 2 && currentInput == 6)
            {
                Serial.printf("< %0.3f / %0.3f\n", range, valueF);
            }
            valueF = (0.5f - (valueF / range));
            valueF = fastPow(valueF, order);

            return 0.5f - valueF;
        }
        else if (value > centreMax)
        {
            float range = 1023.0f - (centreMax + DEAD_ZONE);
            float valueF = (float)(value - centreMax);

            if (currentGroup == 2 && currentInput == 6)
            {
                Serial.printf("> %d => %0.3f / %0.3f\n", value, range, valueF);
            }
            valueF = valueF / range;
            valueF = fastPow(valueF, order);

            return valueF + 0.5f;
        }
        else
        {
            if (currentGroup == 2 && currentInput == 6)
            {
                Serial.printf("= %d => 0.500\n", value);
            }

            return 0.5f;
        }
    }

    inline float fastPow(float value, int p)
    {
        for (int i = 1; i < p; i++)
        {
            value *= value;
        }

        return value;
    }

    int tmpStore[128];
};

#endif