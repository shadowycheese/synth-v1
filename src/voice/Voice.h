#ifndef VOICE_H
#define VOICE_H

#include <Audio.h>
#include "../Constants.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"
#include "VoiceConfiguration.h"

class Voice : public SynthConfigurationListener
{
public:
    Voice();
    void noteOn(byte note, float frequency, float velocity);
    void noteOff();

    AudioStream &getOutput() { return envelope1; }

    float volume();
    bool isPlaying();
    uint32_t timestamp() { return _timestamp; };
    byte noteLastPlayed() { return _note; };

    void onSynthConfigurationChanged(SynthConfiguration *configuration, int changeFlags);
    void updateFilter();

private:
    AudioMixer4 mixer1;
    AudioMixer4 mixer2;
    AudioMixer4 voiceMix;

    AudioAnalyzePeak analyze;
    AudioSynthWaveform waveforms[7];
    AudioSynthNoiseWhite noise;
    AudioFilterLadder ladderFilter;

    AudioEffectEnvelope envelope1;

    AudioConnection patch1, patch2, patch3, patch4;
    AudioConnection patch5, patch6, patch7, patch8;
    AudioConnection patchM1, patchM2;

    AudioConnection patchFilter;
    AudioConnection patchEnv;
    AudioConnection patchAnalyze;

    VoiceConfiguration _voiceConfiguration;

    uint32_t _timestamp;
    byte _note;
    float _frequency;
    float _amplitudeScale;
    uint32_t _iteration;

    void configure(bool restart);
};

#endif