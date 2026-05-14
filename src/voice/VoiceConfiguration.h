#ifndef VOICECONFIGURATION_H
#define VOICECONFIGURATION_H

class VoiceConfiguration : public SynthConfiguration
{
public:
    inline int audioWaveform(int waveform)
    {
        return WaveFormMap[oscillators[waveform].waveform];
    }

    inline int audioWaveformPitchLfo()
    {
        return WaveFormMap[pitchLfo.waveform];
    }

    inline int audioWaveformFilterLfo()
    {
        return WaveFormMap[filterLfo.waveform];
    }

    static constexpr int WaveFormMap[8] = {
        WAVEFORM_SINE,
        WAVEFORM_PULSE,
        WAVEFORM_TRIANGLE,
        WAVEFORM_SAWTOOTH,
        WAVEFORM_SAWTOOTH_REVERSE,
        WAVEFORM_SAMPLE_HOLD //
    };

    static constexpr char *WaveFormNames[8] = {
        "Sine",
        "Square",
        "Triangle",
        "Saw",
        "Reverse Saw",
        "Sample & Hold" //
    };

    static constexpr float CENTS[4] = {
        0.0f,
        TWELTH_ROOT_OF_TWO,
        3 * TWELTH_ROOT_OF_TWO,
        5 * TWELTH_ROOT_OF_TWO //
    };
};

#endif