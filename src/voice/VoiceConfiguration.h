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
        attack = source->attack;
        decay = source->decay;
        sustain = source->sustain;
        decay = source->decay;
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
        manualCutoff = source->manualCutoff;
        lfoFrequency = source->lfoFrequency;
        lfoAmplitude = source->lfoAmplitude;
        lfoPulseWidth = source->lfoPulseWidth;
        filterLevel = source->filterLevel;
    }

    void copyVoiceConfiguration(SynthConfiguration *source)
    {
        detune = source->detune;
        noiseAmplitude = source->noiseAmplitude;
        pitchBend = source->pitchBend;

        for (int i = 0; i < 4; i++)
        {
            amplitudes[i] = source->amplitudes[i];
        }
    }

    void copyVolumeConfiguration(SynthConfiguration *source)
    {
        masterVolume = source->masterVolume;
        filterGain = source->filterGain;
        voiceGain = source->voiceGain;
        pitchLevel = source->pitchLevel;
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
        2.0f,
        4.0f,
        7.0f};

    static constexpr float SUPER_SAW_GAIN_OFFSET[4] = {
        -0.3f,
        0.1f,
        0.3f,
        0.6f,
    };
};

#endif