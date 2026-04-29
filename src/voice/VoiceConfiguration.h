#ifndef VOICECONFIGURATION_H
#define VOICECONFIGURATION_H

class VoiceConfiguration : public SynthConfiguration
{
public:
    inline int audioWaveform(int waveform)
    {
        return WaveFormMap[waveforms[waveform]];
    }

    inline int audioWaveformLfo()
    {
        return WaveFormMap[lfoWaveform];
    }

    void copyEnvelopeConfiguration(SynthConfiguration *source)
    {
        voiceAttack = source->voiceAttack;
        voiceDecay = source->voiceDecay;
        voiceSustain = source->voiceSustain;
        voiceRelease = source->voiceRelease;

        filterAttack = source->filterAttack;
        filterDecay = source->filterDecay;
        filterSustain = source->voiceSustain;
        filterRelease = source->filterRelease;
    }

    void copyEffectConfiguration(SynthConfiguration *source)
    {
        reverb = source->reverb;
        delay = source->delay;
        delayEnabled = source->delayEnabled;
        reverbEnabled = source->reverbEnabled;
    }

    void copyWaveformConfiguration(SynthConfiguration *source)
    {
        lfoWaveform = source->lfoWaveform;

        for (int i = 0; i < 4; i++)
        {
            waveforms[i] = source->waveforms[i];
        }
    }

    void copyFilterConfiguration(SynthConfiguration *source)
    {
        autoCutoff = source->autoCutoff;
        resonance = source->resonance;
        filterCutoff = source->filterCutoff;
        lfoFrequency = source->lfoFrequency;
        lfoAmplitude = source->lfoAmplitude;
        lfoPulseWidth = source->lfoPulseWidth;
        filterLevel = source->filterLevel;
        octaveControl = source->octaveControl;
    }

    void copyVoiceConfiguration(SynthConfiguration *source)
    {
        detune = source->detune;
        noiseAmplitude = source->noiseAmplitude;
        pitchBend = source->pitchBend;
        halfSaw = source->halfSaw;

        for (int i = 0; i < 4; i++)
        {
            amplitudes[i] = source->amplitudes[i];
        }
    }

    void copyVolumeConfiguration(SynthConfiguration *source)
    {
        masterVolume = source->masterVolume;
        voiceGain = source->voiceGain;
        pitchLevel = source->pitchLevel;
        filterLevel = source->filterLevel;
    }

    static constexpr int WaveFormMap[8] = {
        WAVEFORM_SINE,
        WAVEFORM_SQUARE,
        WAVEFORM_TRIANGLE,
        WAVEFORM_SAWTOOTH,
        WAVEFORM_SAWTOOTH_REVERSE,
        WAVEFORM_PULSE,
        WAVEFORM_BANDLIMIT_SAWTOOTH,
        WAVEFORM_ARBITRARY};

    static constexpr float CENTS[4] = {
        0.0f,
        TWELTH_ROOT_OF_TWO,
        3 * TWELTH_ROOT_OF_TWO,
        5 * TWELTH_ROOT_OF_TWO};
};

#endif