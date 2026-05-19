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

    AudioStream &getOutput() { return delayMixer; }

    bool isPlaying();
    uint32_t timestamp() { return _timestamp; };
    byte noteLastPlayed() { return _note; };

    void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags);
    void updateFilter();
    void setWaveformStore(WaveformStore *waveformStore) { _waveformStore = waveformStore; }

private:
    AudioMixer4 oscillatorMixer1;
    AudioMixer4 oscillatorMixer2;
    AudioMixer4 oscillatorMixerMain;
    AudioMixer4 modMixer;

    AudioSynthWaveform lfo1a;
    AudioSynthWaveform lfo1b;
    AudioSynthWaveform lfo1c;
    AudioSynthWaveform lfo2;

    AudioSynthWaveformModulated oscillators[7];
    AudioSynthWaveformDc pulseWidths[4];
    AudioEffectMultiply modMultiply;
    AudioSynthNoisePink noise;
    AudioFilterStateVariable filter;
    AudioEffectDelay delay;
    AudioMixer4 delayMixer;

    AudioEffectEnvelope envelopeVoice;
    AudioEffectEnvelope envelopeLfo1a;
    AudioEffectEnvelope envelopeLfo1b;
    AudioEffectEnvelope envelopeLfo1c;
    AudioEffectEnvelope envelopeLfo2;

    AudioMixer4 filterMixer;

    AudioConnection patches[40] =
        {
            AudioConnection(lfo1a, 0, envelopeLfo1a, 0),
            AudioConnection(lfo1b, 0, envelopeLfo1b, 0),
            AudioConnection(lfo1c, 0, envelopeLfo1c, 0),
            AudioConnection(lfo2, 0, envelopeLfo2, 0),

            AudioConnection(envelopeLfo1a, 0, oscillators[0], 0),
            AudioConnection(envelopeLfo1b, 0, oscillators[1], 0),
            AudioConnection(envelopeLfo1b, 0, oscillators[2], 0),
            AudioConnection(envelopeLfo1c, 0, oscillators[3], 0),
            AudioConnection(envelopeLfo1c, 0, oscillators[4], 0),
            AudioConnection(envelopeLfo1a, 0, oscillators[5], 0),
            AudioConnection(envelopeLfo1a, 0, oscillators[6], 0),

            AudioConnection(oscillators[0], 0, oscillatorMixer1, 0),
            AudioConnection(oscillators[1], 0, oscillatorMixer1, 1),
            AudioConnection(oscillators[3], 0, oscillatorMixer1, 2),
            AudioConnection(oscillators[5], 0, oscillatorMixer1, 3),
            AudioConnection(oscillators[2], 0, oscillatorMixer2, 1),
            AudioConnection(oscillators[4], 0, oscillatorMixer2, 2),
            AudioConnection(oscillators[6], 0, oscillatorMixer2, 3),
            AudioConnection(noise, 0, oscillatorMixer2, 0),

            AudioConnection(pulseWidths[0], 0, oscillators[0], 1),
            AudioConnection(pulseWidths[1], 0, oscillators[1], 1),
            AudioConnection(pulseWidths[1], 0, oscillators[2], 1),
            AudioConnection(pulseWidths[2], 0, oscillators[3], 1),
            AudioConnection(pulseWidths[2], 0, oscillators[4], 1),
            AudioConnection(pulseWidths[3], 0, oscillators[5], 1),
            AudioConnection(pulseWidths[3], 0, oscillators[6], 1),

            AudioConnection(oscillatorMixer1, 0, oscillatorMixerMain, 0),
            AudioConnection(oscillatorMixer2, 0, oscillatorMixerMain, 1),
            AudioConnection(oscillatorMixerMain, 0, filter, 0),

            AudioConnection(envelopeLfo2, 0, filter, 1),
            AudioConnection(filter, 0, filterMixer, 0),
            AudioConnection(filter, 2, filterMixer, 1),
            AudioConnection(oscillatorMixerMain, 0, filterMixer, 2),

            AudioConnection(filterMixer, 0, envelopeVoice, 0),
            AudioConnection(envelopeVoice, 0, delay, 0),
            AudioConnection(envelopeVoice, 0, delayMixer, 0),
            AudioConnection(delay, 0, delayMixer, 1),
        };

    VoiceConfiguration _voiceConfiguration;
    WaveformStore *_waveformStore;

    uint32_t _timestamp;
    byte _note;
    float _frequency;
    float _gain;
    uint32_t _iteration;

    void init();
    void configureVoice();
    void configureGain();
    void configureFilter();
    void configureEffects();
    void configureOscilators();
    void configuraOscillator(AudioSynthWaveformModulated *wf, OscillatorConfiguration *config);
    void configuraOscillator(AudioSynthWaveform *wf, OscillatorConfiguration *config);
    inline void configureLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency, float phase);
    inline void configureUniPolarLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency);
    inline void configureEnvelope(AudioEffectEnvelope *envelope, EnvelopeConfiguration *config);
};

#endif