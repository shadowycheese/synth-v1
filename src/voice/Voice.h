#ifndef VOICE_H
#define VOICE_H

#include <Audio.h>
#include "../Constants.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"
#include "VoiceConfiguration.h"
#include "../utils/ClipDetector.h"

#define CHORUS_DELAY_LEN (AUDIO_BLOCK_SAMPLES * 50) // 35ms

class Voice : public SynthConfigurationListener
{
public:
    Voice() : patch1(pitchLevel, 0, pitchMultiply, 0),
              patch2(pitchLfo, 0, pitchMultiply, 1),

              patch3(filterLevel, 0, filterMultiply1, 0),
              patch4(filterLfo, 0, filterMultiply1, 1),

              patch5(pitchMultiply, 0, oscillators[0], 0),
              patch6(pitchMultiply, 0, oscillators[1], 0),
              patch7(pitchMultiply, 0, oscillators[2], 0),
              patch8(pitchMultiply, 0, oscillators[3], 0),
              patch9(pitchMultiply, 0, oscillators[4], 0),
              patch10(pitchMultiply, 0, oscillators[5], 0),
              patch11(pitchMultiply, 0, oscillators[6], 0),

              patch12(oscillators[0], 0, oscilatorMixer1, 0),
              patch13(oscillators[1], 0, oscilatorMixer1, 1),
              patch14(oscillators[2], 0, oscilatorMixer1, 2),
              patch15(oscillators[3], 0, oscilatorMixer1, 3),
              patch16(oscillators[4], 0, oscilatorMixer2, 0),
              patch17(oscillators[5], 0, oscilatorMixer2, 1),
              patch18(oscillators[6], 0, oscilatorMixer2, 1),

              patch19(pulseWidths[0], 0, oscillators[0], 1),
              patch20(pulseWidths[1], 0, oscillators[1], 1),
              patch21(pulseWidths[1], 0, oscillators[2], 1),
              patch22(pulseWidths[2], 0, oscillators[3], 1),
              patch23(pulseWidths[2], 0, oscillators[4], 1),
              patch24(pulseWidths[3], 0, oscillators[5], 1),
              patch25(pulseWidths[3], 0, oscillators[6], 1),

              patch26(noise, 0, oscilatorMixer2, 3),

              patch27(oscilatorMixer1, 0, oscilatorMixerMain, 0),
              patch28(oscilatorMixer2, 0, oscilatorMixerMain, 1),

              patch29(filterMultiply1, 0, envelopeFilter, 0),
              patch30(oscilatorMixerMain, 0, filter, 0),
              patch31(envelopeFilter, 0, filter, 1),
              patch32(filter, 0, envelopeVoice, 0),
              patch33(envelopeVoice, 0, reverb, 0),
              patch34(envelopeVoice, 0, reverbMixer, 0),
              patch35(reverb, 0, reverbMixer, 1)

    {
        init();
    };

    void noteOn(byte note, float frequency, float velocity);
    void noteOff();

    AudioStream &getOutput() { return reverbMixer; }

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

    AudioSynthWaveform filterLfo;
    AudioSynthWaveformDc pitchLevel;
    AudioSynthWaveformDc filterLevel;
    AudioSynthWaveformModulated oscillators[7];
    AudioSynthWaveformDc pulseWidths[4];
    AudioSynthWaveform pitchLfo;
    AudioEffectMultiply pitchMultiply;
    AudioEffectMultiply filterMultiply1;
    AudioEffectMultiply filterMultiply2;
    AudioSynthNoiseWhite noise;
    AudioFilterStateVariable filter;
    AudioEffectEnvelope envelopeVoice;
    AudioEffectEnvelope envelopeFilter;
    AudioEffectFreeverb reverb;
    AudioAnalyzePeak analyze;
    AudioMixer4 reverbMixer;

    AudioConnection patch1, patch2, patch3, patch4, patch5, patch6, patch7, patch8;
    AudioConnection patch9, patch10, patch11, patch12, patch13, patch14, patch15, patch16;
    AudioConnection patch17, patch18, patch19, patch20, patch21, patch22, patch23, patch24;
    AudioConnection patch25, patch26, patch27, patch28, patch29, patch30, patch31, patch32;
    AudioConnection patch33, patch34, patch35;

    VoiceConfiguration _voiceConfiguration;

    uint32_t _timestamp;
    byte _note;
    float _frequency;
    float _amplitudeScale;
    uint32_t _iteration;

    void init();
    void configureVoice(bool restart);
    void configureFilter();
    void configureEffects();
    void configureOscilators();
    inline void configureEnvelope();
};

#endif