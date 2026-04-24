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

    void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags);
    void updateFilter();

private:
    AudioMixer4 oscilatorMixer1;
    AudioMixer4 oscilatorMixer2;
    AudioMixer4 oscilatorMixerMain;

    AudioSynthWaveformDc pitch;
    AudioSynthWaveformDc filterLevel;
    AudioSynthWaveformModulated oscillators[7];
    AudioSynthWaveform lfo;
    AudioEffectMultiply pitchMultiply;
    AudioEffectMultiply filterMultiply;
    AudioSynthNoiseWhite noise;
    AudioFilterStateVariable filter;
    AudioEffectEnvelope envelope1;

    AudioConnection pitchPatch, lfoPatch1, filterLevelPatch, lfoPatch2;
    AudioConnection pitchMultiplyPatch, filterMultiplyPatch;

    AudioConnection patchOscIn0, patchOscIn1, patchOscIn2, patchOscIn3;
    AudioConnection patchOscIn4, patchOscIn5, patchOscIn6;

    AudioConnection patchOscOut0, patchOscOut1, patchOscOut2, patchOscOut3;
    AudioConnection patchOscOut4, patchOscOut5, patchOscOut6, patchOscOut7;

    AudioConnection patchOscMain1, patchOscMain2;

    AudioConnection patchFilter1, patchFilter2;

    AudioConnection patchEnv;

    VoiceConfiguration _voiceConfiguration;

    uint32_t _timestamp;
    byte _note;
    float _frequency;
    float _amplitudeScale;
    uint32_t _iteration;

    void configureVoice(bool restart);
    void configureFilter();
    inline void configureEnvelope();
    inline void configureOscilator(int id, float frequency, float ampliture, float phase);
};

#endif