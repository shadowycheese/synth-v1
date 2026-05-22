#ifndef VOICE_H
#define VOICE_H

#include <Audio.h>
#include "../audio/AudioAnalyzeOverdrive.h"
#include "../config/SynthConfiguration.h"
#include "../config/SynthConfigurationListener.h"
#include "../store/WaveformStore.h"

#define DETUNE_MAX_SPREAD 1200

class Voice : public SynthConfigurationListener
{
public:
    Voice()
    {
        init();
    };

    void noteOn(byte note, float frequency, float velocity);
    void noteOff();

    AudioStream &getOutput() { return postDelayMixer; }

    inline bool isPlaying() { return envelopeVoice.isActive(); };
    inline uint32_t getTimestamp() { return _timestamp; };
    inline byte getLastPlayedNote() { return _note; };

    inline bool isLadderFilterSelected() { return _filterType == FILTER_LADDER; };
    inline bool isOverdriven() { return overdrive.isOverdriven(true); }
    void onSynthConfigurationChanged(SynthConfiguration *configuration, SynthConfigurationFlags changeFlags);
    void updateFilterAndEffects();
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
    AudioSynthNoiseWhite noise;
    AudioEffectDelay delay;
    AudioMixer4 preDelayMixer;
    AudioMixer4 postDelayMixer;

    AudioFilterLadder filterLadder;
    AudioFilterStateVariable filterSvf;

    AudioEffectEnvelope envelopeVoice;
    AudioEffectEnvelope envelopeLfo1a;
    AudioEffectEnvelope envelopeLfo1b;
    AudioEffectEnvelope envelopeLfo1c;
    AudioEffectEnvelope envelopeLfo2;
    AudioAnalyzeOverdrive overdrive;

    AudioMixer4 filterMixer;

    AudioConnection patches[39] =
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
            AudioConnection(oscillatorMixerMain, 0, filterMixer, 2),

            AudioConnection(filterMixer, 0, envelopeVoice, 0),
            AudioConnection(envelopeVoice, 0, preDelayMixer, 0),
            AudioConnection(postDelayMixer, 0, preDelayMixer, 1),
            AudioConnection(preDelayMixer, 0, delay, 0),
            AudioConnection(envelopeVoice, 0, postDelayMixer, 0),
            AudioConnection(delay, 0, postDelayMixer, 1),
            AudioConnection(filterMixer, 0, overdrive, 0),
        };

    static const int FILTER_PATCH_COUNT = 3;

    AudioConnection ladderPatches[FILTER_PATCH_COUNT] =
        {
            AudioConnection(oscillatorMixerMain, 0, filterLadder, 0),
            AudioConnection(envelopeLfo2, 0, filterLadder, 1),
            AudioConnection(filterLadder, 0, filterMixer, 0),
        };

    AudioConnection svfPatches[FILTER_PATCH_COUNT] =
        {
            AudioConnection(oscillatorMixerMain, 0, filterSvf, 0),
            AudioConnection(envelopeLfo2, 0, filterSvf, 1),
            AudioConnection(filterSvf, 0, filterMixer, 1),
        };

    SynthConfiguration _voiceConfiguration;
    WaveformStore *_waveformStore;

    int _filterType;
    uint32_t _timestamp;
    byte _note;
    float _frequency;
    float _gain;
    uint32_t _iteration;
    bool _isOverdriven;
    float _currentDelay;

    inline void connectFilterPatches(AudioConnection *patches);
    inline void disconnectFilterPatches(AudioConnection *patches);

    void init();
    void configureVoice();
    void configureGain();
    void configureFilter();
    void configureFilterPatches(int filterType);
    void configureEffects();
    void configureOscilators();
    void configuraOscillator(AudioSynthWaveformModulated *wf, OscillatorConfiguration *config);
    void configuraOscillator(AudioSynthWaveform *wf, OscillatorConfiguration *config);
    inline void configureLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency, float phase);
    inline void configureUniPolarLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency);
    inline void configureEnvelope(AudioEffectEnvelope *envelope, EnvelopeConfiguration *config);
};

#endif