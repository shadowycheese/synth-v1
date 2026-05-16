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
    configureVoice();

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
        configureEnvelope(&envelopeLfo1a, &_voiceConfiguration.lfo1Envelope);
        configureEnvelope(&envelopeLfo1b, &_voiceConfiguration.lfo1Envelope);
        configureEnvelope(&envelopeLfo1c, &_voiceConfiguration.lfo1Envelope);

        Serial.printf("FILTER ");
        configureEnvelope(&envelopeLfo2, &_voiceConfiguration.lfo2Envelope);
        configureEnvelope(&envelopeVoice, &_voiceConfiguration.voiceEnvelope);
    }

    if (lfoChanged(changeFlags))
    {
        configureLfo(&lfo1a, &_voiceConfiguration.lfo1, _frequency);
        configureLfo(&lfo1b, &_voiceConfiguration.lfo1, _frequency);
        configureLfo(&lfo1c, &_voiceConfiguration.lfo1, _frequency);
        configureUniPolarLfo(&lfo2, &_voiceConfiguration.lfo2, _frequency);
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
        configureVoice();
    }
}

bool Voice::isPlaying()
{
    return envelopeVoice.isActive();
}

void Voice::configureVoice()
{
    float frequency = _frequency * (1.0 + _voiceConfiguration.pitchBend);

    Serial.printf("main freq: %0.3f (%0.3f), max detune: %0.5f\n",
                  frequency,
                  _frequency,
                  _voiceConfiguration.maxDetune);

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

        Serial.printf("freq: %5.3f / freq: %5.3f\n",
                      lf,
                      rf);

        oscillators[l].frequency(lf);
        oscillators[r].frequency(rf);
    }
}

void Voice::configureLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency)
{
    lfo->amplitude(config->amplitude);
    lfo->frequency(config->frequency == 0.0f ? frequency / 16 : config->frequency);
    lfo->pulseWidth(config->pulseWidth);
}

void Voice::configureUniPolarLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency)
{
    float amplitude = min(0.0001, config->amplitude);
    float offset = 1.0f - amplitude;

    lfo->amplitude(amplitude / 2);
    lfo->offset(offset);
    lfo->frequency(config->frequency == 0.0f ? frequency / 16 : config->frequency);
    lfo->pulseWidth(config->pulseWidth);
}

void Voice::configureEffects()
{
}

void Voice::configureGain()
{
    float gain = _voiceConfiguration.leftSideOnly ? _gain * 2.0f : _gain;

    float osc0Gain = gain * _voiceConfiguration.oscillators[0].gain;
    float osc1Gain = gain * _voiceConfiguration.oscillators[1].gain;
    float osc2Gain = gain * _voiceConfiguration.oscillators[2].gain;
    float osc3Gain = gain * _voiceConfiguration.oscillators[3].gain;

    oscillatorMixer1.gain(0, osc0Gain);
    oscillatorMixer1.gain(1, osc1Gain);
    oscillatorMixer1.gain(2, osc2Gain);
    oscillatorMixer1.gain(3, osc3Gain);

    oscillatorMixer2.gain(0, _gain * _voiceConfiguration.noiseGain);

    if (_voiceConfiguration.leftSideOnly)
    {
        oscillatorMixer2.gain(1, 0.0f);
        oscillatorMixer2.gain(2, 0.0f);
        oscillatorMixer2.gain(3, 0.0f);
    }
    else
    {
        oscillatorMixer2.gain(1, osc1Gain);
        oscillatorMixer2.gain(2, osc2Gain);
        oscillatorMixer2.gain(3, osc3Gain);
    }
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
        float refFrequency = 440.0f;

        cutoffFrequency = cutoffFrequency * powf(noteFrequency / refFrequency, _voiceConfiguration.keyTracking);
    }

    cutoffFrequency = min(cutoffFrequency, 12000);

    float maxOctaves = log2f(12000 / cutoffFrequency);
    float octaves = max(0, min(maxOctaves, _voiceConfiguration.octaveControl));

    filter.frequency(cutoffFrequency);
    filter.octaveControl(octaves);
    filter.resonance(_voiceConfiguration.resonance);

    if (!_voiceConfiguration.filterEnabled)
    {
        Serial.printf("DISABLED: cutoff %0.1f, omax %0.3f, oct %0.3f\n", cutoffFrequency, maxOctaves, octaves);

        filterMixer.gain(0, 0.0f);
        filterMixer.gain(1, 0.0f);
        filterMixer.gain(2, 1.0f);
    }
    else if (_voiceConfiguration.lowPass)
    {
        Serial.printf("LOWPASS: cutoff %0.1f, omax %0.3f, oct %0.3f\n", cutoffFrequency, maxOctaves, octaves);

        filterMixer.gain(0, 1.0f);
        filterMixer.gain(1, 0.0f);
        filterMixer.gain(2, 0.0f);
    }
    else
    {
        Serial.printf("BANDPASS: cutoff %0.1f, omax %0.3f, oct %0.3f\n", cutoffFrequency, maxOctaves, octaves);

        filterMixer.gain(0, 0.0f);
        filterMixer.gain(1, 1.0f);
        filterMixer.gain(2, 0.0f);
    }

    updateFilter();
}

inline void Voice::configureEnvelope(AudioEffectEnvelope *envelope, EnvelopeConfiguration *config)
{
    Serial.printf("ENVELOPE: A %0.1f, D %0.3f, S %0.3f, R %0.3f\n", config->attack, config->decay, config->sustain, config->release);

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
