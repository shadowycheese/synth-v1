#ifndef VOICECONFIGURATION_H
#define VOICECONFIGURATION_H

class VoiceConfiguration
{
public:
    float detune;
    float resonance;
    float pitch;
    float pulseWidth;
    int waveforms[4];
    float amplitudes[4];
    float noiseAmplitude;
    float cutoff;
    bool manualCutoff;
    bool superSawMode;

    inline int activeOscilators() { return activeOscilatorCount; }
    inline int audioWaveform(int waveform)
    {
        return WaveFormMap[waveforms[waveform]];
    }

    void copyOscillatorConfiguration(SynthConfiguration *source)
    {
        superSawMode = source->superSawMode;

        for (int i = 0; i < 4; i++)
        {
            waveforms[i] = source->waveforms[i];
        }

        updateOscilatorCount();
    }

    void copyFilterConfiguration(SynthConfiguration *source)
    {
        manualCutoff = source->manualCutoff;
        resonance = source->resonance;
        cutoff = source->cutoff;
    }

    void copyVoiceConfiguration(SynthConfiguration *source)
    {
        detune = source->detune;
        resonance = source->resonance;
        noiseAmplitude = source->noiseAmplitude;
        pitch = source->pitch;
        pulseWidth = source->pulseWidth;

        for (int i = 0; i < 4; i++)
        {
            amplitudes[i] = source->amplitudes[i];
        }

        updateOscilatorCount();
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

private:
    int activeOscilatorCount;
    void updateOscilatorCount()
    {
        activeOscilatorCount = 0;

        for (int i = 0; i < 4; i++)
        {
            if (amplitudes[i] > 0)
            {
                activeOscilatorCount += (i == 0) ? 1 : (superSawMode ? 2 : 1);
            }
        }
    }
};

#endif