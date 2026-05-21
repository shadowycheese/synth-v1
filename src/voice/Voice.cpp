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

void Voice::updateFilterAndEffects()
{
    if (_voiceConfiguration.delay > 0.0f)
    {
        if (_currentDelay < _voiceConfiguration.delay)
        {
            _currentDelay += 0.5f;

            _currentDelay = min(_currentDelay, _voiceConfiguration.delay);

            delay.delay(0, _currentDelay);
        }
        else if (_currentDelay > _voiceConfiguration.delay)
        {
            _currentDelay -= 0.5f;

            _currentDelay = max(_currentDelay, _voiceConfiguration.delay);

            delay.delay(0, _currentDelay);
        }
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
        configureEnvelope(&envelopeLfo2, &_voiceConfiguration.lfo2Envelope);
        configureEnvelope(&envelopeVoice, &_voiceConfiguration.voiceEnvelope);
    }

    if (lfoChanged(changeFlags))
    {
        Serial.printf("lfo phase: %0.3f, lfo2 freq %0.3f\n", _voiceConfiguration.decoherence, _voiceConfiguration.lfo2.frequency);

        configureLfo(&lfo1a, &_voiceConfiguration.lfo1, _frequency, 0.0f);
        configureLfo(&lfo1b, &_voiceConfiguration.lfo1, _frequency, 120.0 * _voiceConfiguration.decoherence);
        configureLfo(&lfo1c, &_voiceConfiguration.lfo1, _frequency, 240.0 * _voiceConfiguration.decoherence);
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

        Serial.printf("Pair %d: freq: %5.3f / %5.3f\n",
                      i + 1,
                      lf,
                      rf);

        oscillators[l].frequency(lf);
        oscillators[r].frequency(rf);
    }
}

void Voice::configureLfo(AudioSynthWaveform *lfo, OscillatorConfiguration *config, float frequency, float phase)
{
    lfo->amplitude(config->amplitude);
    lfo->frequency(config->frequency == 0.0f ? frequency / 16 : config->frequency);
    lfo->pulseWidth(config->pulseWidth);
    lfo->phase(phase);
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
    if (_voiceConfiguration.delay == 0.0f)
    {
        delay.delay(0, 0.0f);

        _currentDelay = 0.0f;

        delayMixer.gain(0, 1.0f);
        delayMixer.gain(1, 0.0f);
    }
    else
    {
        delayMixer.gain(0, 0.8f);
        delayMixer.gain(1, 0.5f);
    }
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
    if (_voiceConfiguration.filterCutoff == 1.0f)
    {
        filter.frequency(20000.0f);
        filter.octaveControl(0.0f);
        filter.resonance(0.0f);

        filterMixer.gain(0, 0.0f);
        filterMixer.gain(1, 0.0f);
        filterMixer.gain(2, 1.0f);
    }
    else
    {
        float minFrequency = 30.0f;
        float maxFrequency = 8000.0f;
        float maxCutoff = 12000.0f;

        float shaped = powf(_voiceConfiguration.filterCutoff, 1.5f);
        float cutoffFrequency = minFrequency * powf(maxFrequency / minFrequency, shaped);

        if (_voiceConfiguration.keyTracking > 0.0f)
        {
            float noteFrequency = (_frequency * (_voiceConfiguration.pitchBend + 1.0f));
            float refFrequency = 440.0f;

            cutoffFrequency = cutoffFrequency * powf(noteFrequency / refFrequency, _voiceConfiguration.keyTracking);
        }

        cutoffFrequency = min(cutoffFrequency, maxCutoff);

        float maxOctaves = log2f(maxCutoff / cutoffFrequency);
        float octaves = max(0, min(maxOctaves, _voiceConfiguration.octaveControl));

        Serial.printf("%d: cutoff %0.1f, omax %0.3f, oct %0.3f\n",
                      _voiceConfiguration.lowPass,
                      cutoffFrequency,
                      maxOctaves,
                      octaves);

        filter.frequency(cutoffFrequency);
        filter.octaveControl(octaves);
        filter.resonance(_voiceConfiguration.resonance);

        if (_voiceConfiguration.lowPass)
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
    }
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
    configuraOscillator(&lfo1a, &_voiceConfiguration.lfo1);
    configuraOscillator(&lfo1b, &_voiceConfiguration.lfo1);
    configuraOscillator(&lfo1c, &_voiceConfiguration.lfo1);
    configuraOscillator(&lfo2, &_voiceConfiguration.lfo2);

    uint8_t wf = _voiceConfiguration.audioWaveform(0);

    configuraOscillator(&oscillators[0], &_voiceConfiguration.oscillators[0]);

    for (int i = 0; i < 3; i++)
    {
        int l = 1 + (i * 2);
        int r = l + 1;

        configuraOscillator(&oscillators[l], &_voiceConfiguration.oscillators[i + 1]);
        configuraOscillator(&oscillators[r], &_voiceConfiguration.oscillators[i + 1]);
    }
}

void Voice::configuraOscillator(AudioSynthWaveformModulated *wf, OscillatorConfiguration *config)
{
    if (config->waveform >= CUSTOM_WAVEFORM_OFFSET)
    {
        uint8_t wfId = config->waveform - CUSTOM_WAVEFORM_OFFSET;
        int16_t *wfData = _waveformStore->waveformData(wfId);

        wf->arbitraryWaveform(wfData, 0.0f);
    }

    wf->begin(VoiceConfiguration::WaveFormMap[config->waveform]);
}

void Voice::configuraOscillator(AudioSynthWaveform *wf, OscillatorConfiguration *config)
{
    if (config->waveform >= CUSTOM_WAVEFORM_OFFSET)
    {
        uint8_t wfId = config->waveform - CUSTOM_WAVEFORM_OFFSET;
        int16_t *wfData = _waveformStore->waveformData(wfId);

        wf->arbitraryWaveform(wfData, 0.0f);
    }

    wf->begin(VoiceConfiguration::WaveFormMap[config->waveform]);
}
