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

class VoiceConfiguration
{
public:
    float detune;
    int mainWaveForm;
    int detuneWaveForm;
    float resonance;
    float pitch;
    float pulseWidth;
    float amplitudes[4];
    float noiseAmplitude;
    int activeOscilators;
    bool manualCutoff;

    int waveform(int waveform)
    {
        return WaveFormMap[waveform];
    }

    void copyWaveFormConfiguration(SynthConfiguration *source)
    {
        mainWaveForm = 1;
        source->mainWaveForm;
        detuneWaveForm = 3;
        source->detuneWaveForm;
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