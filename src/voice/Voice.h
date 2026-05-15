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
    void task(bool print);

private:
    AudioMixer4 oscillatorMixer1;
    AudioMixer4 oscillatorMixer2;
    AudioMixer4 oscillatorMixerMain;
    AudioMixer4 modMixer;

    AudioSynthWaveform lfo1a;
    AudioSynthWaveform lfo2a;
    AudioSynthWaveform lfo1b;
    AudioSynthWaveform lfo2b;

    AudioSynthWaveformDc filterLevel;
    AudioSynthWaveformModulated oscillators[7];
    AudioSynthWaveformDc pulseWidths[4];
    AudioEffectMultiply modMultiply;
    AudioSynthNoisePink noise;
    AudioFilterStateVariable filter;
    AudioEffectEnvelope envelopeVoice;
    AudioEffectEnvelope envelopeLfo1a;
    AudioEffectEnvelope envelopeLfo1b;
    AudioEffectEnvelope envelopeLfo2a;
    AudioEffectEnvelope envelopeLfo2b;
    AudioEffectEnvelope envelopeFilter;
    AudioAnalyzePeak mixer1Analyze;
    AudioAnalyzePeak mixer2Analyze;
    AudioAnalyzePeak mainMixerAnalyze;
    AudioAnalyzePeak filterAnalyze;

    AudioMixer4 filterMixer;

    AudioConnection patches[40] =
        {
            AudioConnection(lfo1a, 0, envelopeLfo1a, 0),
            AudioConnection(lfo2a, 0, envelopeLfo2a, 0),

            AudioConnection(envelopeLfo1a, 0, oscillators[0], 0),
            AudioConnection(envelopeLfo1a, 0, oscillators[1], 0),
            AudioConnection(envelopeLfo1b, 0, oscillators[2], 0),
            AudioConnection(envelopeLfo2a, 0, oscillators[3], 0),
            AudioConnection(envelopeLfo2b, 0, oscillators[4], 0),
            AudioConnection(envelopeLfo2a, 0, oscillators[5], 0),
            AudioConnection(envelopeLfo2b, 0, oscillators[6], 0),

            AudioConnection(oscillators[0], 0, oscillatorMixer1, 0),
            AudioConnection(oscillators[1], 0, oscillatorMixer1, 1),
            AudioConnection(oscillators[2], 0, oscillatorMixer1, 2),
            AudioConnection(oscillators[3], 0, oscillatorMixer1, 3),
            AudioConnection(oscillators[4], 0, oscillatorMixer2, 0),
            AudioConnection(oscillators[5], 0, oscillatorMixer2, 1),
            AudioConnection(oscillators[6], 0, oscillatorMixer2, 2),
            AudioConnection(noise, 0, oscillatorMixer2, 3),

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

            AudioConnection(filterLevel, 0, envelopeFilter, 1),
            AudioConnection(envelopeFilter, 0, filter, 0),
            AudioConnection(filter, 0, filterMixer, 0),
            AudioConnection(filter, 1, filterMixer, 1),
            AudioConnection(oscillatorMixerMain, 0, filterMixer, 2),

            AudioConnection(filterMixer, 0, envelopeVoice, 0),

            AudioConnection(oscillatorMixer1, 0, mixer1Analyze, 0),
            AudioConnection(oscillatorMixer2, 0, mixer2Analyze, 0),
            AudioConnection(oscillatorMixerMain, 0, mainMixerAnalyze, 0),
            AudioConnection(filterMixer, 0, filterAnalyze, 0),
        };

    VoiceConfiguration _voiceConfiguration;

    uint32_t _timestamp;
    byte _note;
    float _frequency;
    float _gain;
    uint32_t _iteration;
    float peak1, peak2, peak3, peak4;

    void init();
    void configureVoice(bool restart);
    void configureGain();
    void configureFilter();
    void configureEffects();
    void configureOscilators();
    inline void configureLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency);
    inline void configureEnvelope(AudioEffectEnvelope *envelope, EnvelopeConfiguration *config);
};

#endif