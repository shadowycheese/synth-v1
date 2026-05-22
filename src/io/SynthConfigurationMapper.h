#ifndef SYNTH_CONFIGURATION_MAPPER_H
#define SYNTH_CONFIGURATION_MAPPER_H

#include "ControllerIoListener.h"
#include "../config/SynthConfiguration.h"
#include "../config/SynthConfigurationListener.h"

#define DEAD_ZONE 12

class SynthConfigurationMapper : public ControllerIoListener
{
public:
    SynthConfigurationMapper(SynthConfiguration *configuration, SynthConfigurationListener *configuratonListener, Indicators *indicators)
    {
        _synthConfiguration = configuration;
        _synthConfigurationListener = configuratonListener;

        _indicators = indicators;
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

        _currentInput = input;
        _currentGroup = group;

        _changeFlags |= (this->*handler)(value);
    }

private:
    typedef int (SynthConfigurationMapper::*Func)(int);

    int _currentInput;
    int _currentGroup;
    Indicators *_indicators;

    Func midiInputs[7] = {
        &SynthConfigurationMapper::updatePitchBend,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp,
        &SynthConfigurationMapper::noOp};

    Func mux1Inputs[16] = {
        &SynthConfigurationMapper::updatePreset1,
        &SynthConfigurationMapper::updatePreset2,
        &SynthConfigurationMapper::updateFilterType,
        &SynthConfigurationMapper::updateHalfSaw,
        &SynthConfigurationMapper::updateLfo1Release,
        &SynthConfigurationMapper::updateLfo1Sustain,
        &SynthConfigurationMapper::updateLfo1Decay,
        &SynthConfigurationMapper::updateLfo1Attack,
        &SynthConfigurationMapper::updateLfo2Decay,
        &SynthConfigurationMapper::updateLfo2Release,
        &SynthConfigurationMapper::updateLfo2Attack,
        &SynthConfigurationMapper::updateLfo2Sustain,
        &SynthConfigurationMapper::updateVoiceRelease,
        &SynthConfigurationMapper::updateVoiceSustain,
        &SynthConfigurationMapper::updateVoiceDecay,
        &SynthConfigurationMapper::updateVoiceAttack};

    Func mux2Inputs[16] = {
        &SynthConfigurationMapper::updateOscillatorPulseWidth3,
        &SynthConfigurationMapper::updateOscillatorPulseWidth2,
        &SynthConfigurationMapper::updateOscillatorPulseWidth0,
        &SynthConfigurationMapper::updateOscillatorPulseWidth1,
        &SynthConfigurationMapper::updateFilterTracking,
        &SynthConfigurationMapper::updateFilterCutoff,
        &SynthConfigurationMapper::updateResonance,
        &SynthConfigurationMapper::updateOctaveControl,
        &SynthConfigurationMapper::updateLfo2Waveform,
        &SynthConfigurationMapper::updateLfo2Frequency,
        &SynthConfigurationMapper::updateLfo2PulseWidth,
        &SynthConfigurationMapper::updateLfo2Amplitude,
        &SynthConfigurationMapper::updateLfo1Amplitude,
        &SynthConfigurationMapper::updateLfo1PulseWidth,
        &SynthConfigurationMapper::updateLfo1Frequency,
        &SynthConfigurationMapper::updateLfo1Waveform};

    Func mux3Inputs[16] = {
        &SynthConfigurationMapper::updateWaveform0,
        &SynthConfigurationMapper::updateWaveform1,
        &SynthConfigurationMapper::updateWaveform2,
        &SynthConfigurationMapper::updateWaveform3,
        &SynthConfigurationMapper::updateOscillatorGain0,
        &SynthConfigurationMapper::updateOscillatorGain1,
        &SynthConfigurationMapper::updateOscillatorGain2,
        &SynthConfigurationMapper::updateOscillatorGain3,
        &SynthConfigurationMapper::updateAmpGain,
        &SynthConfigurationMapper::updateDecoherence,
        &SynthConfigurationMapper::updateDelay,
        &SynthConfigurationMapper::updateReverb,
        &SynthConfigurationMapper::updateNoiseAmplitude,
        &SynthConfigurationMapper::updateDetune1,
        &SynthConfigurationMapper::updateDetune2,
        &SynthConfigurationMapper::updateDetune3};

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
        case INPUT_GROUP_MUX3:
            return mux3Inputs[input];
        default:
            return &SynthConfigurationMapper::noOp;
        }
    }

    SynthConfiguration _localSynthConfiguration;
    SynthConfiguration *_synthConfiguration;
    SynthConfigurationListener *_synthConfigurationListener;

    int _changeFlags;

    // Envelope
    int updateVoiceAttack(int value)
    {
        return updateAttack(&_localSynthConfiguration.voiceEnvelope, "Voice", ENVELOPE_CHANGED, value);
    }

    int updateVoiceDecay(int value)
    {
        return updateDecay(&_localSynthConfiguration.voiceEnvelope, "Voice", ENVELOPE_CHANGED, value);
    }

    int updateVoiceSustain(int value)
    {
        return updateSustain(&_localSynthConfiguration.voiceEnvelope, "Voice", ENVELOPE_CHANGED, value);
    }

    int updateVoiceRelease(int value)
    {
        return updateRelease(&_localSynthConfiguration.voiceEnvelope, "Voice", ENVELOPE_CHANGED, value);
    }

    int updateLfo2Attack(int value)
    {
        return updateAttack(&_localSynthConfiguration.lfo2Envelope, "LFO 2", ENVELOPE_CHANGED, value);
    }

    int updateLfo2Decay(int value)
    {
        return updateDecay(&_localSynthConfiguration.lfo2Envelope, "LFO 2", ENVELOPE_CHANGED, value);
    }

    int updateLfo2Sustain(int value)
    {
        return updateSustain(&_localSynthConfiguration.lfo2Envelope, "LFO 2", ENVELOPE_CHANGED, value);
    }

    int updateLfo2Release(int value)
    {
        return updateRelease(&_localSynthConfiguration.lfo2Envelope, "LFO 2", ENVELOPE_CHANGED, value);
    }

    int updateLfo1Attack(int value)
    {
        return updateAttack(&_localSynthConfiguration.lfo1Envelope, "LFO 1", ENVELOPE_CHANGED, value);
    }

    int updateLfo1Decay(int value)
    {
        return updateDecay(&_localSynthConfiguration.lfo1Envelope, "LFO 1", ENVELOPE_CHANGED, value);
    }

    int updateLfo1Sustain(int value)
    {
        return updateSustain(&_localSynthConfiguration.lfo1Envelope, "LFO 1", ENVELOPE_CHANGED, value);
    }

    int updateLfo1Release(int value)
    {
        return updateRelease(&_localSynthConfiguration.lfo1Envelope, "LFO 1", ENVELOPE_CHANGED, value);
    }

    // Volume
    int updateAmpGain(int value)
    {
        float newValue = getScaledValue(value, 2);

        if (newValue != _localSynthConfiguration.ampGain)
        {
            Serial.printf("Amp gain %f\n", newValue);

            _localSynthConfiguration.ampGain = newValue;

            return VOLUME_CHANGED | PASSTHROUGH_CHANGED;
        }

        return 0;
    }

    int updateMasterGain(int value)
    {
        float newValue = value / 1023.0f;

        if (newValue != _localSynthConfiguration.masterVolume)
        {
            Serial.printf("Master gain %f\n", newValue);

            _localSynthConfiguration.masterVolume = newValue;

            return VOLUME_CHANGED;
        }

        return 0;
    }

    // Wave form
    int updateWaveform0(int value)
    {
        return updateOscillatorWaveform(&_localSynthConfiguration.oscillators[0], "Oscillator 0", WAVEFORM_CHANGED, value);
    }

    int updateWaveform1(int value)
    {
        return updateOscillatorWaveform(&_localSynthConfiguration.oscillators[1], "Oscillator 1", WAVEFORM_CHANGED, value);
    }

    int updateWaveform2(int value)
    {
        return updateOscillatorWaveform(&_localSynthConfiguration.oscillators[2], "Oscillator 2", WAVEFORM_CHANGED, value);
    }

    int updateWaveform3(int value)
    {
        return updateOscillatorWaveform(&_localSynthConfiguration.oscillators[3], "Oscillator 3", WAVEFORM_CHANGED, value);
    }

    int updateLfo1Waveform(int value)
    {
        return updateOscillatorWaveform(&_localSynthConfiguration.lfo1, "LFO 1", WAVEFORM_CHANGED, value);
    }

    int updateLfo2Waveform(int value)
    {
        return updateOscillatorWaveform(&_localSynthConfiguration.lfo2, "LFO 2", WAVEFORM_CHANGED, value);
    }

    int updateLfo3Waveform(int value)
    {
        return updateOscillatorWaveform(&_localSynthConfiguration.lfo2, "LFO 3", WAVEFORM_CHANGED, value);
    }

    // Voice configuration
    int updateOscillatorGain0(int value)
    {
        return updateOscillatorGain(&_localSynthConfiguration.oscillators[0], "Oscillator 0", VOLUME_CHANGED, value);
    }

    int updateOscillatorGain1(int value)
    {
        return updateOscillatorGain(&_localSynthConfiguration.oscillators[1], "Oscillator 1", VOLUME_CHANGED, value);
    }

    int updateOscillatorGain2(int value)
    {
        return updateOscillatorGain(&_localSynthConfiguration.oscillators[2], "Oscillator 2", VOLUME_CHANGED, value);
    }

    int updateOscillatorGain3(int value)
    {
        return updateOscillatorGain(&_localSynthConfiguration.oscillators[3], "Oscillator 3", VOLUME_CHANGED, value);
    }

    int updateNoiseAmplitude(int value)
    {
        float newValue = getScaledValue(value, 2);

        if (newValue != _localSynthConfiguration.noiseGain)
        {
            Serial.printf("Noise %0.3f\n", newValue);

            _localSynthConfiguration.noiseGain = newValue;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateOscillatorPulseWidth0(int value)
    {
        return updateOscillatorPulseWidth(&_localSynthConfiguration.oscillators[0], "Oscillator 0", VOICE_CHANGED, value);
    }

    int updateOscillatorPulseWidth1(int value)
    {
        return updateOscillatorPulseWidth(&_localSynthConfiguration.oscillators[1], "Oscillator 1", VOICE_CHANGED, value);
    }

    int updateOscillatorPulseWidth2(int value)
    {
        return updateOscillatorPulseWidth(&_localSynthConfiguration.oscillators[2], "Oscillator 2", VOICE_CHANGED, value);
    }

    int updateOscillatorPulseWidth3(int value)
    {
        return updateOscillatorPulseWidth(&_localSynthConfiguration.oscillators[3], "Oscillator 3", VOICE_CHANGED, value);
    }

    int updateLfo1PulseWidth(int value)
    {
        return updateOscillatorPulseWidth(&_localSynthConfiguration.lfo1, "LFO 1", LFO_CHANGED, value);
    }

    int updateLfo1Frequency(int value)
    {
        return updateOscillatorFrequency(&_localSynthConfiguration.lfo1, "LFO 1", LFO_CHANGED, value);
    }

    int updateLfo1Amplitude(int value)
    {
        return updateOscillatorAmplitude(&_localSynthConfiguration.lfo1, "LFO 1", LFO_CHANGED, value);
    }

    int updateLfo2PulseWidth(int value)
    {
        return updateOscillatorPulseWidth(&_localSynthConfiguration.lfo2, "LFO 2", LFO_CHANGED, value);
    }

    int updateLfo2Frequency(int value)
    {
        return updateOscillatorFrequency(&_localSynthConfiguration.lfo2, "LFO 2", LFO_CHANGED, value);
    }

    int updateLfo2Amplitude(int value)
    {
        return updateOscillatorAmplitude(&_localSynthConfiguration.lfo2, "LFO 2", LFO_CHANGED, value);
    }

    int updateLfo3Frequency(int value)
    {
        return updateOscillatorFrequency(&_localSynthConfiguration.lfo3, "LFO 3", LFO_CHANGED, value);
    }

    int updateLfo3Amplitude(int value)
    {
        return updateOscillatorAmplitude(&_localSynthConfiguration.lfo3, "LFO 3", LFO_CHANGED, value);
    }

    int updateOctaveControl(int value)
    {
        float valueF = getScaledValue(value, 1);
        float newValue = valueF * 7.0f;

        if (newValue != _localSynthConfiguration.octaveControl)
        {
            Serial.printf("Octave control = %0.3f\n", newValue);

            _localSynthConfiguration.octaveControl = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateDecoherence(int value)
    {
        float newValue = getScaledValue(value, 1);

        if (newValue != _localSynthConfiguration.decoherence)
        {
            Serial.printf("LFO Decoherence = %0.3f\n", newValue);

            _localSynthConfiguration.decoherence = newValue;

            return LFO_CHANGED;
        }

        return 0;
    }

    int updateFilterType(int value)
    {
        int newValue = (value < 512) ? FILTER_LADDER : FILTER_SVF;

        if (newValue != _localSynthConfiguration.filterType)
        {
            Serial.printf("Filter = %s [%d]\n", newValue == FILTER_LADDER ? "Ladder" : "SVF", value);

            _localSynthConfiguration.filterType = newValue;

            return FILTER_CHANGED;
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

    int updateDelay(int value)
    {
        float newValue = getScaledValue(value, 1) * 250.0f;

        if (newValue != _localSynthConfiguration.delay)
        {
            Serial.printf("Delay = %0.3f\n", newValue);

            _localSynthConfiguration.delay = newValue;

            return EFFECT_CHANGED;
        }

        return 0;
    }

    int updatePreset1(int value)
    {
        bool newValue = (value < 512) ? true : false;

        if (newValue != _localSynthConfiguration.preset1)
        {
            Serial.printf("Preset 1 changed = %s\n", newValue ? "true" : "false");

            _localSynthConfiguration.preset1 = newValue;

            return PRESET_CHANGED;
        }

        return 0;
    }

    int updatePreset2(int value)
    {
        bool newValue = (value < 512) ? true : false;

        if (newValue != _localSynthConfiguration.preset2)
        {
            Serial.printf("Preset 2 changed = %s\n", newValue ? "true" : "false");

            _localSynthConfiguration.preset2 = newValue;

            return PRESET_CHANGED;
        }

        return 0;
    }

    int updateHalfSaw(int value)
    {
        bool newValue = (value < 512) ? true : false;

        if (newValue != _localSynthConfiguration.leftSideOnly)
        {
            Serial.printf("Half Saw enabled = %s\n", newValue ? "true" : "false");

            _localSynthConfiguration.leftSideOnly = newValue;

            return VOICE_CHANGED;
        }

        return 0;
    }

    int updateDetune1(int value)
    {
        return updateDetune(&_localSynthConfiguration.oscillators[1], "Oscillator 1", VOICE_CHANGED, value);
    }

    int updateDetune2(int value)
    {
        return updateDetune(&_localSynthConfiguration.oscillators[2], "Oscillator 2", VOICE_CHANGED, value);
    }

    int updateDetune3(int value)
    {
        return updateDetune(&_localSynthConfiguration.oscillators[3], "Oscillator 3", VOICE_CHANGED, value);
    }

    int updateResonance(int value)
    {
        float newValue = getScaledValue(value, 2);

        if (newValue != _localSynthConfiguration.resonance)
        {
            Serial.printf("Resonance = %0.3f\n", newValue);

            _localSynthConfiguration.resonance = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateFilterCutoff(int value)
    {
        float newValue = getScaledValue(value, 1);

        if (newValue != _localSynthConfiguration.filterCutoff)
        {
            Serial.printf("Cutoff = %0.3f\n", newValue);

            _localSynthConfiguration.filterCutoff = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updateFilterTracking(int value)
    {
        float newValue = getScaledValue(value, 1);

        if (newValue != _localSynthConfiguration.keyTracking)
        {
            Serial.printf("Key tracking = %0.3f\n", newValue);

            _localSynthConfiguration.keyTracking = newValue;

            return FILTER_CHANGED;
        }

        return 0;
    }

    int updatePitchBend(int value)
    {
        float newValue = (value / 16384.0f);

        if (newValue != _localSynthConfiguration.pitchBend)
        {
            Serial.printf("Pitch = %0.3f\n", newValue);

            _localSynthConfiguration.pitchBend = newValue;

            return VOICE_CHANGED | PASSTHROUGH_CHANGED;
        }

        return 0;
    }

    // Common
    int updateOscillatorFrequency(OscillatorConfiguration *oscillator, const char *name, int changeFlag, int value)
    {
        float valueF = getScaledValue(value, 2);

        float newValue = (valueF * 1000.0f);

        if (newValue != oscillator->frequency)
        {
            Serial.printf("%s frequncy = %0.1f\n", name, newValue);

            oscillator->frequency = newValue;

            return changeFlag;
        }

        return 0;
    }

    int updateOscillatorAmplitude(OscillatorConfiguration *oscillator, const char *name, int changeFlag, int value)
    {
        float newValue = getScaledValue(value, 2);

        if (newValue != oscillator->amplitude)
        {
            Serial.printf("%s amplitude = %0.3f\n", name, newValue);

            oscillator->amplitude = newValue;

            return changeFlag;
        }

        return 0;
    }

    int updateOscillatorPulseWidth(OscillatorConfiguration *oscillator, const char *name, int changeFlag, int value)
    {
        float newValue = getScaledValue(value, 1);

        if (newValue != oscillator->pulseWidth)
        {
            Serial.printf("%s pulse width = %0.3f\n", name, newValue);

            oscillator->pulseWidth = newValue;

            return changeFlag;
        }

        return 0;
    }

    int updateAttack(EnvelopeConfiguration *envelope, const char *name, int changeFlag, int value)
    {
        float newValue = getScaledValue(value, 1);
        newValue = 2.0f + (powf(newValue, 2) * 3998.0f);

        if (newValue != envelope->attack)
        {
            Serial.printf("%s attack %f\n", name, newValue);

            envelope->attack = newValue;

            return ENVELOPE_CHANGED;
        }

        return 0;
    }

    int updateDecay(EnvelopeConfiguration *envelope, const char *name, int changeFlag, int value)
    {
        float newValue = getScaledValue(value, 1);
        newValue = 5.0f + (powf(newValue, 2) * 7995.0f);

        if (newValue != envelope->decay)
        {
            Serial.printf("%s decay %f\n", name, newValue);

            envelope->decay = newValue;

            return ENVELOPE_CHANGED;
        }

        return 0;
    }

    int updateSustain(EnvelopeConfiguration *envelope, const char *name, int changeFlag, int value)
    {
        float newValue = getScaledValue(value, 1);

        if (newValue != envelope->sustain)
        {
            Serial.printf("%s sustain %f\n", name, newValue);

            envelope->sustain = newValue;

            return changeFlag;
        }

        return 0;
    }

    int updateRelease(EnvelopeConfiguration *envelope, const char *name, int changeFlag, int value)
    {
        float newValue = getScaledValue(value, 1);
        newValue = 5.0f + (powf(newValue, 2) * 7995.0f);

        if (newValue != envelope->release)
        {
            Serial.printf("%s release %f\n", name, newValue);

            envelope->release = newValue;

            return changeFlag;
        }

        return 0;
    }

    int updateOscillatorWaveform(OscillatorConfiguration *oscillator, const char *name, int changeFlag, int value)
    {
        bool showIndicator = abs(value - _lastWaveForm) > 1;

        int newValue = (value / 86) % 12;

        if (newValue != oscillator->waveform)
        {
            Serial.printf("%s waveform %d [%s]\n", name, newValue, SynthConfiguration::WaveFormNames[newValue]);

            oscillator->waveform = newValue;

            showIndicator = true;

            return changeFlag;
        }

        if (showIndicator)
        {
            _indicators->waveformSelected(micros(), oscillator->waveform);
        }

        return 0;
    }

    int updateOscillatorGain(OscillatorConfiguration *oscillator, const char *name, int changeFlag, int value)
    {
        float valueF = getScaledValue(value, 2);

        if (valueF != oscillator->gain)
        {
            Serial.printf("%s level = %0.3f\n", name, valueF);

            oscillator->gain = valueF;

            return changeFlag;
        }

        return 0;
    }

    int updateDetune(OscillatorConfiguration *oscillator, const char *name, int changeFlag, int value)
    {
        float valueF = getScaledValue(value, 2);

        if (valueF != oscillator->detune)
        {
            Serial.printf("%s detune %d = %0.3f\n", name, valueF);

            oscillator->detune = valueF;

            return changeFlag;
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
        if (value >= (1023 - DEAD_ZONE))
        {
            return 1.0f;
        }

        if (value < DEAD_ZONE)
        {
            return 0.0f;
        }

        float range = 1023.0f - (DEAD_ZONE * 2);

        value -= DEAD_ZONE;

        float valueF = (float)value / range;
        float value2 = valueF;

        while (--order > 0)
        {
            valueF *= value2;
        }

        return valueF;
    }

    int _lastWaveForm;
};

#endif