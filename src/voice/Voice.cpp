#include "../Constants.h"
#include "Voice.h"

void Voice::init()
{
    for (int i = 0; i < 4; i++)
    {
        oscillatorMixer1.gain(i, 0.25f);
    }

    for (int i = 0; i < 4; i++)
    {
        oscillatorMixer2.gain(i, 0.25f);
    }

    for (int i = 0; i < 7; i++)
    {
        oscillators[i].phaseModulation(180);
        oscillators[i].amplitude(1.0f);
    }

    filterLevel.amplitude(1.0f);
    noise.amplitude(1.0f);
    oscillatorMixerMain.gain(0, 0.25f);
    oscillatorMixerMain.gain(1, 0.25f);

    lfo1a.amplitude(1.0f);
    lfo1b.amplitude(1.0f);
    lfo1c.amplitude(1.0f);
    lfo2.amplitude(1.0f);

    lfo1a.phase(0.0f);
    lfo1b.phase(120.0f);
    lfo1c.phase(240.0f);
}

void Voice::noteOn(byte note, float frequency, float gain)
{
    _gain = 0.25f;
    _frequency = frequency;

    configureGain();
    configureFilter();
    configureVoice(false);

    envelopeFilter.noteOn();
    envelopeVoice.noteOn();
    envelopeLfo1a.noteOn();
    envelopeLfo1b.noteOn();
    envelopeLfo1c.noteOn();
    envelopeLfo2.noteOn();

    _note = note;
    _timestamp = millis();
}

void Voice::noteOff()
{
    envelopeVoice.noteOff();
    envelopeFilter.noteOff();
    envelopeLfo1a.noteOff();
    envelopeLfo1b.noteOff();
    envelopeLfo1c.noteOff();
    envelopeLfo2.noteOff();
}

void Voice::updateFilter()
{
}

void Voice::task(bool print)
{
    if (mixer1Analyze.available())
    {
        peak1 = max(peak1, mixer1Analyze.read());
    }
    if (mixer2Analyze.available())
    {
        peak2 = max(peak2, mixer2Analyze.read());
    }
    if (mainMixerAnalyze.available())
    {
        peak3 = max(peak3, mainMixerAnalyze.read());
    }
    if (filterAnalyze.available())
    {
        peak4 = max(peak4, filterAnalyze.read());
    }

    if (print)
    {
        Serial.printf("%0.2f %0.2f %0.2f %0.2f", peak1, peak2, peak3, peak4);

        peak1 = peak2 = peak3 = peak4 = 0.0f;
    }
}

void Voice::onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags)
{
    _voiceConfiguration.copy(configuration);

    if (envelopeChanged(changeFlags))
    {
        configureEnvelope(&envelopeLfo1a, &_voiceConfiguration.lfoEnvelope);
        configureEnvelope(&envelopeLfo1b, &_voiceConfiguration.lfoEnvelope);
        configureEnvelope(&envelopeLfo1c, &_voiceConfiguration.lfoEnvelope);
        configureEnvelope(&envelopeLfo2, &_voiceConfiguration.lfoEnvelope);
        configureEnvelope(&envelopeFilter, &_voiceConfiguration.filterEnvelope);
        configureEnvelope(&envelopeVoice, &_voiceConfiguration.voiceEnvelope);
    }

    if (lfoChanged(changeFlags))
    {
        configureLfo(&lfo1a, &_voiceConfiguration.lfo1, _frequency);
        configureLfo(&lfo1b, &_voiceConfiguration.lfo1, _frequency);
        configureLfo(&lfo1c, &_voiceConfiguration.lfo1, _frequency);
        configureLfo(&lfo2, &_voiceConfiguration.lfo2, _frequency);
    }

    if (effectChanged(changeFlags))
    {
        configureEffects();
    }

    if (volumeChanged(changeFlags))
    {
        configureGain();
    }

    bool updateFilter = filterChanged(changeFlags);
    bool restartOscillators = waveformChanged(changeFlags);
    bool updateVoice = voiceChanged(changeFlags);

    if (updateFilter)
    {
        configureFilter();
    }

    if (restartOscillators)
    {
        configureOscilators();
    }

    if (envelopeVoice.isActive() && (restartOscillators || updateVoice))
    {
        configureVoice(restartOscillators);
    }
}

bool Voice::isPlaying()
{
    return envelopeVoice.isActive();
}

void Voice::configureVoice(bool restartOscillators)
{
    float frequency = _frequency * (1.0 + _voiceConfiguration.pitchBend);

    Serial.printf("main freq: %0.3f (%0.3f), max detune: %0.5f, resonance: %0.3f, halfsaw: %d\n",
                  frequency,
                  _frequency,
                  _voiceConfiguration.maxDetune,
                  _voiceConfiguration.resonance,
                  _voiceConfiguration.halfSaw);

    if (_voiceConfiguration.lfo1.frequency == 0.0f)
    {
        lfo1a.frequency(frequency / 2);
        lfo1b.frequency(frequency / 2);
        lfo1c.frequency(frequency / 2);
    }

    pulseWidths[0].amplitude(2.0f * (_voiceConfiguration.oscillators[0].pulseWidth - 0.5f));

    oscillators[0].frequency(frequency);

    for (int i = 0; i < 3; i++)
    {
        int l = 1 + (i * 2);
        int r = l + 1;

        pulseWidths[i + 1].amplitude(_voiceConfiguration.oscillators[i + 1].pulseWidth);

        float centsR = _voiceConfiguration.oscillators[i + 1].detune * DETUNE_MAX_SPREAD;
        float centsL = -centsR;

        float lf = frequency * powf(2.0f, centsL / 1200.0f);
        float rf = frequency * powf(2.0f, centsR / 1200.0f);

        uint8_t wf = _voiceConfiguration.audioWaveform(i + 1);

        Serial.printf("freq: %0.3f / freq: %0.3f waveform:%d (%s)\n",
                      lf,
                      rf,
                      wf,
                      restartOscillators ? "(restart)" : "");

        oscillators[l].frequency(lf);
        oscillators[r].frequency(rf);
    }
}

void Voice::configureLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency)
{
    lfo->amplitude(config->amplitude);
    lfo->frequency(config->frequency == 0.0f ? frequency : config->frequency);
    lfo->pulseWidth(config->pulseWidth);
}

void Voice::configureEffects()
{
}

void Voice::configureGain()
{
    float osc0Gain = _gain * _voiceConfiguration.oscillators[0].gain;
    float osc1Gain = _gain * _voiceConfiguration.oscillators[1].gain;
    float osc2Gain = _gain * _voiceConfiguration.oscillators[2].gain;
    float osc3Gain = _gain * _voiceConfiguration.oscillators[3].gain;
    float noiseGain = _gain * _voiceConfiguration.noiseGain;

    Serial.printf("Gain: %0.2f %0.2f %0.2f %0.2f\n", osc0Gain, osc1Gain, osc2Gain, osc3Gain);

    oscillatorMixer1.gain(0, osc0Gain);
    oscillatorMixer1.gain(1, osc1Gain);
    oscillatorMixer1.gain(2, osc1Gain);
    oscillatorMixer1.gain(3, osc2Gain);

    oscillatorMixer2.gain(0, osc2Gain);
    oscillatorMixer2.gain(1, osc3Gain);
    oscillatorMixer2.gain(2, osc3Gain);
    oscillatorMixer2.gain(3, noiseGain);
}

void Voice::configureFilter()
{
    float minFrequency = 30.0f;
    float maxFrequency = 8000.0f;

    float shaped = powf(_voiceConfiguration.filterCutoff, 1.5f);
    float cutoffFrequency = minFrequency * powf(maxFrequency / minFrequency, shaped);

    if (_voiceConfiguration.keyTracking > 0.0f)
    {
        float noteFrequency = (_frequency * (_voiceConfiguration.pitchBend + 1.0f));
        float refFrequency = 110.0f;

        cutoffFrequency = cutoffFrequency * powf(noteFrequency / refFrequency, _voiceConfiguration.keyTracking);
    }

    float maxOctaves = log2f(12000 / cutoffFrequency);
    float octaves = max(0, min(maxOctaves, _voiceConfiguration.octaveControl));

    Serial.printf("cutoff %0.1f, omax %0.3f, oct %0.3f\n", cutoffFrequency, maxOctaves, octaves);

    filter.frequency(cutoffFrequency);
    filter.octaveControl(octaves);
    filter.resonance(_voiceConfiguration.resonance);

    if (!_voiceConfiguration.filterEnabled)
    {
        filterMixer.gain(0, 0.0f);
        filterMixer.gain(1, 0.0f);
        filterMixer.gain(2, 1.0f);
    }
    else if (_voiceConfiguration.lowPass)
    {
        filterMixer.gain(0, 1.0f);
        filterMixer.gain(1, 0.0f);
        filterMixer.gain(2, 0.0f);
    }
    else
    {
        filterMixer.gain(0, 0.0f);
        filterMixer.gain(1, 1.0f);
        filterMixer.gain(2, 0.0f);
    }

    updateFilter();
}

inline void Voice::configureEnvelope(AudioEffectEnvelope *envelope, EnvelopeConfiguration *config)
{
    envelope->attack(config->attack);
    envelope->decay(config->decay);
    envelope->sustain(config->sustain);
    envelope->release(config->release);
}

void Voice::configureOscilators()
{
    lfo1a.begin(_voiceConfiguration.audioWaveformLfo1());
    lfo1a.begin(_voiceConfiguration.audioWaveformLfo1());
    lfo1c.begin(_voiceConfiguration.audioWaveformLfo1());
    lfo2.begin(_voiceConfiguration.audioWaveformLfo2());

    uint8_t wf = _voiceConfiguration.audioWaveform(0);

    oscillators[0].begin(wf);

    for (int i = 0; i < 3; i++)
    {
        wf = _voiceConfiguration.audioWaveform(i + 1);

        int l = 1 + (i * 2);
        int r = l + 1;

        oscillators[l].begin(wf);
        oscillators[r].begin(wf);
    }
}
