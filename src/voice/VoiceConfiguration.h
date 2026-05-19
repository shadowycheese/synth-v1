#ifndef VOICECONFIGURATION_H
#define VOICECONFIGURATION_H

const int CUSTOM_WAVEFORM_OFFSET = 6;

class VoiceConfiguration : public SynthConfiguration
{
public:
    inline int audioWaveform(int waveform)
    {
        return WaveFormMap[oscillators[waveform].waveform];
    }

    inline int audioWaveformLfo1()
    {
        return WaveFormMap[lfo1.waveform];
    }

    inline int audioWaveformLfo2()
    {
        return WaveFormMap[lfo2.waveform];
    }

    static constexpr int WaveFormMap[12] = {
        WAVEFORM_SINE,
        WAVEFORM_PULSE,
        WAVEFORM_TRIANGLE,
        WAVEFORM_SAWTOOTH,
        WAVEFORM_SAWTOOTH_REVERSE,
        WAVEFORM_SAMPLE_HOLD,
        WAVEFORM_ARBITRARY,
        WAVEFORM_ARBITRARY,
        WAVEFORM_ARBITRARY,
        WAVEFORM_ARBITRARY,
        WAVEFORM_ARBITRARY,
        WAVEFORM_ARBITRARY //
    };

    static constexpr char *WaveFormNames[12] = {
        "Sine",
        "Square",
        "Triangle",
        "Saw",
        "Reverse Saw",
        "Sample & Hold",
        "Custom 1",
        "Custom 2",
        "Custom 3",
        "Custom 4",
        "Custom 5",
        "Custom 6", //
    };

    static constexpr float CENTS[4] = {
        0.0f,
        TWELTH_ROOT_OF_TWO,
        3 * TWELTH_ROOT_OF_TWO,
        5 * TWELTH_ROOT_OF_TWO //
    };
};

#endif