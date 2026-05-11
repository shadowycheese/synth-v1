#ifndef VOICECONFIGURATION_H
#define VOICECONFIGURATION_H

class VoiceConfiguration : public SynthConfiguration
{
public:
    float maxDetune;

    inline int audioWaveform(int waveform)
    {
        return WaveFormMap[waveforms[waveform].waveform];
    }

    inline int audioWaveformPitchLfo()
    {
        return WaveFormMap[pitchLfo.waveform];
    }

    inline int audioWaveformFilterLfo()
    {
        return WaveFormMap[filterLfo.waveform];
    }

    void copyEnvelopeConfiguration(SynthConfiguration *source)
    {
        voiceEnvelope.copy(&(source->voiceEnvelope));
        filterEnvelope.copy(&(source->filterEnvelope));
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
        filterLfo.copy(&(source->filterLfo));

        for (int i = 0; i < 4; i++)
        {
            waveforms[i].copy(&(source->waveforms[i]));
        }
    }

    void copyFilterConfiguration(SynthConfiguration *source)
    {
        autoCutoff = source->autoCutoff;
        resonance = source->resonance;
        filterCutoff = source->filterCutoff;
        octaveControl = source->octaveControl;

        filterLfo.copy(&(source->filterLfo));
    }

    void copyVoiceConfiguration(SynthConfiguration *source)
    {
        pitchLfo.copy(&(source->pitchLfo));

        noiseAmplitude = source->noiseAmplitude;
        pitchBend = source->pitchBend;
        halfSaw = source->halfSaw;
        maxDetune = 0;

        for (int i = 0; i < 4; i++)
        {
            waveforms[i].copy(&(source->waveforms[i]));

            if (waveforms[i].detune > maxDetune)
            {
                maxDetune = waveforms[i].detune;
            }
        }
    }

    void copyVolumeConfiguration(SynthConfiguration *source)
    {
        masterVolume = source->masterVolume;
        voiceGain = source->voiceGain;
        pitchLfo.level = source->pitchLfo.level;
        filterLfo.level = source->filterLfo.level;
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