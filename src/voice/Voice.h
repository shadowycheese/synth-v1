#ifndef VOICE_H
#define VOICE_H

#include <Audio.h>
#include "../Constants.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"
#include "VoiceConfiguration.h"

#define CHORUS_DELAY_LEN (AUDIO_BLOCK_SAMPLES * 50) // 35ms

class Voice : public SynthConfigurationListener
{
public:
    Voice()
    {
        init();
    };

    void noteOn(byte note, float frequency, float velocity);
    void noteOff();

    AudioStream &getOutput() { return envelopeVoice; }

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
    AudioMixer4 modMixer;

    AudioSynthWaveform filterLfo;
    AudioSynthWaveform modLfo;

    AudioSynthWaveformDc modLevel;
    AudioSynthWaveformDc filterLevel;
    AudioSynthWaveformDc amplitudeOne;
    AudioSynthWaveformModulated oscillators[7];
    AudioSynthWaveformDc pulseWidths[4];
    AudioEffectMultiply modMultiply;
    AudioEffectMultiply filterMultiply1;
    AudioEffectMultiply filterMultiply2;
    AudioSynthNoiseWhite noise;
    AudioFilterStateVariable filter;
    AudioEffectEnvelope envelopeVoice;
    AudioEffectEnvelope envelopeFilter;
    AudioEffectEnvelope envelopeLfo;
    AudioAnalyzePeak analyze;
    AudioMixer4 filterMixer;

    AudioConnection patches[40] =
        {
            AudioConnection(modLevel, 0, modMultiply, 0),
            AudioConnection(modLfo, 0, modMultiply, 1),
            AudioConnection(modMultiply, 0, envelopeLfo, 0),

            AudioConnection(envelopeLfo, 0, oscillators[0], 0),
            AudioConnection(envelopeLfo, 0, oscillators[1], 0),
            AudioConnection(envelopeLfo, 0, oscillators[2], 0),
            AudioConnection(envelopeLfo, 0, oscillators[3], 0),
            AudioConnection(envelopeLfo, 0, oscillators[4], 0),
            AudioConnection(envelopeLfo, 0, oscillators[5], 0),
            AudioConnection(envelopeLfo, 0, oscillators[6], 0),

            AudioConnection(oscillators[0], 0, oscilatorMixer1, 0),
            AudioConnection(oscillators[1], 0, oscilatorMixer1, 1),
            AudioConnection(oscillators[2], 0, oscilatorMixer1, 2),
            AudioConnection(oscillators[3], 0, oscilatorMixer1, 3),
            AudioConnection(oscillators[4], 0, oscilatorMixer2, 0),
            AudioConnection(oscillators[5], 0, oscilatorMixer2, 1),
            AudioConnection(oscillators[6], 0, oscilatorMixer2, 1),
            AudioConnection(noise, 0, oscilatorMixer2, 3),

            AudioConnection(pulseWidths[0], 0, oscillators[0], 1),
            AudioConnection(pulseWidths[1], 0, oscillators[1], 1),
            AudioConnection(pulseWidths[1], 0, oscillators[2], 1),
            AudioConnection(pulseWidths[2], 0, oscillators[3], 1),
            AudioConnection(pulseWidths[2], 0, oscillators[4], 1),
            AudioConnection(pulseWidths[3], 0, oscillators[5], 1),
            AudioConnection(pulseWidths[3], 0, oscillators[6], 1),

            AudioConnection(oscilatorMixer1, 0, oscilatorMixerMain, 0),
            AudioConnection(oscilatorMixer2, 0, oscilatorMixerMain, 1),
            AudioConnection(oscilatorMixerMain, 0, filter, 0),

            AudioConnection(filterLfo, 0, filterMultiply1, 0),
            AudioConnection(filterLevel, 0, filterMultiply1, 1),

            AudioConnection(filterMultiply1, 0, envelopeFilter, 1),
            AudioConnection(envelopeFilter, 0, filter, 0),
            AudioConnection(filter, 0, filterMixer, 0),
            AudioConnection(filter, 1, filterMixer, 1),
            AudioConnection(oscilatorMixerMain, 0, filterMixer, 2),
            AudioConnection(filterMixer, 0, envelopeVoice, 0),
        };

    VoiceConfiguration _voiceConfiguration;

    uint32_t _timestamp;
    byte _note;
    float _frequency;
    float _gain;
    uint32_t _iteration;

    void init();
    void configureVoice(bool restart);
    void configureGain();
    void configureFilter();
    void configureEffects();
    void configureOscilators();
    inline void configureEnvelope();
};

#endif