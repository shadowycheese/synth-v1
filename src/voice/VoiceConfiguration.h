#ifndef VOICECONFIGURATION_H
#define VOICECONFIGURATION_H

static int WaveFormMap[8] = {
    WAVEFORM_SINE,
    WAVEFORM_SQUARE,
    WAVEFORM_TRIANGLE,
    WAVEFORM_SAWTOOTH,
    WAVEFORM_SAWTOOTH_REVERSE,
    WAVEFORM_PULSE,
    WAVEFORM_BANDLIMIT_SAWTOOTH,
    WAVEFORM_ARBITRARY};

static float CENTS[4] = {
    0.0f,
    2.0f,
    4.0f,
    7.0f};

static float SUPER_SAW_GAIN_OFFSET[4] = {
    -0.3f,
    0.1f,
    0.3f,
    0.6f,
};

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
    int activeOscilators;
    bool manualCutoff;

    inline int audioWaveform(int waveform)
    {
        return WaveFormMap[waveforms[waveform]];
    }

    void copyWaveFormConfiguration(SynthConfiguration *source)
    {
        for (int i = 0; i < 4; i++)
        {
            waveforms[i] = source->waveforms[i];
        }
    }

    void copyVoiceConfiguration(SynthConfiguration *source)
    {
        detune = source->detune;
        resonance = source->resonance;
        noiseAmplitude = source->noiseAmplitude;
        pitch = source->pitch;
        pulseWidth = source->pulseWidth;
        manualCutoff = source->manualCutoff;

        activeOscilators = 0;

        for (int i = 0; i < 4; i++)
        {
            amplitudes[i] = source->amplitudes[i];

            if (amplitudes[i] > 0)
            {
                activeOscilators += (i == 0) ? 1 : 2;
            }
        }
    }

private:
};

#endif