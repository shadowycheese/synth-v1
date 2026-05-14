#include "../Constants.h"
#include "Voice.h"

void Voice::init()
{
    for (int i = 0; i < 4; i++)
    {
        oscilatorMixer1.gain(i, 0.25f);
    }

    for (int i = 0; i < 4; i++)
    {
        oscilatorMixer2.gain(i, 0.25f);
    }

    for (int i = 0; i < 7; i++)
    {
        oscillators[i].phaseModulation(180);
        oscillators[i].amplitude(1.0f);
    }

    noise.amplitude(1.0f);
    oscilatorMixerMain.gain(0, 0.25f);
    oscilatorMixerMain.gain(1, 0.25f);

    filterLfo.amplitude(0.1f);
    filterLfo.offset(0.9f);

    modLfo.amplitude(1.0f);
    modLfo.offset(0.0f);
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
    envelopeLfo.noteOn();

    _note = note;
    _timestamp = millis();
}

void Voice::noteOff()
{
    envelopeVoice.noteOff();
    envelopeFilter.noteOff();
    envelopeLfo.noteOff();
}

void Voice::updateFilter()
{
    if (_voiceConfiguration.pitchLfo.pulseWidth == 0.0f)
    {
        // if (analyze.available())
        //{
        //   pitchLfo.pulseWidth(analyze.read());
        //}
    }
}

void Voice::onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags)
{
    _voiceConfiguration.copy(configuration);

    if (envelopeChanged(changeFlags))
    {
        configureEnvelope();
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

    modLevel.amplitude(_voiceConfiguration.pitchLfo.gain);

    modLfo.frequency(_voiceConfiguration.pitchLfo.frequency == 0.0f
                         ? frequency / 2
                         : _voiceConfiguration.pitchLfo.frequency);
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

    Serial.printf("Osc 0 gain: %0.3f\n", osc0Gain);

    oscilatorMixer1.gain(0, osc0Gain);
    oscilatorMixer1.gain(1, osc1Gain);
    oscilatorMixer1.gain(2, osc1Gain);
    oscilatorMixer1.gain(3, osc2Gain);

    oscilatorMixer2.gain(0, osc2Gain);
    oscilatorMixer2.gain(1, osc3Gain);
    oscilatorMixer2.gain(2, osc3Gain);
    oscilatorMixer2.gain(3, noiseGain);
}

void Voice::configureFilter()
{
    if (_voiceConfiguration.autoCutoff)
    {
        float frequency = (_frequency * (_voiceConfiguration.pitchBend + 1.0f)) * _voiceConfiguration.keyTracking;

        float minFrequency = frequency / (_voiceConfiguration.lowPass ? 2 : 4);
        float maxFrequency = frequency * (_voiceConfiguration.lowPass ? 4 : 2);

        float offset = _voiceConfiguration.filterCutoff * (maxFrequency - minFrequency) + minFrequency;

        filter.frequency(offset);

        float maxOctaves = offset == 0 ? _voiceConfiguration.octaveControl : log2f(12000 / offset);

        float octaves = min(maxOctaves, _voiceConfiguration.octaveControl);

        Serial.printf("Filter (A): base %0.1f (%0.1f) min %0.1f, max %0.1f, used %0.1f, omax %0.3f, oct %0.3f\n", frequency, _frequency, minFrequency, maxFrequency, offset, maxOctaves, octaves);

        filter.octaveControl(octaves);
    }
    else
    {
        float offset = powf(2.0f, 5.0f + (_voiceConfiguration.filterCutoff * 6.0f));

        filter.frequency(offset);

        float maxOctaves = log2f(12000 / offset);

        float octaves = min(maxOctaves, _voiceConfiguration.octaveControl);

        // Serial.printf("Filter (M): used %0.1f, omax %0.3f, oct %0.3f\n", offset, maxOctaves, octaves);

        filter.octaveControl(octaves);
    }

    if (_voiceConfiguration.filterCutoff == 0.0)
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

    filter.resonance(_voiceConfiguration.resonance);

    filterLevel.amplitude(_voiceConfiguration.filterLfo.gain);

    filterLfo.pulseWidth(_voiceConfiguration.filterLfo.pulseWidth);

    // if (_voiceConfiguration.filterLfo.frequency > 0)
    //{
    filterLfo.frequency(max(0.5, _voiceConfiguration.filterLfo.frequency));
    //}

    updateFilter();
}

inline void Voice::configureEnvelope()
{
    envelopeVoice.attack(_voiceConfiguration.voiceEnvelope.attack);
    envelopeVoice.decay(_voiceConfiguration.voiceEnvelope.decay);
    envelopeVoice.sustain(_voiceConfiguration.voiceEnvelope.sustain);
    envelopeVoice.release(_voiceConfiguration.voiceEnvelope.release);

    envelopeFilter.attack(_voiceConfiguration.filterEnvelope.attack);
    envelopeFilter.decay(_voiceConfiguration.filterEnvelope.decay);
    envelopeFilter.sustain(_voiceConfiguration.filterEnvelope.sustain);
    envelopeFilter.release(_voiceConfiguration.filterEnvelope.release);

    Serial.printf("LFO Env: %0.3f %0.3f %0.3f %0.3f\n", _voiceConfiguration.lfoEnvelope.attack, _voiceConfiguration.lfoEnvelope.decay, _voiceConfiguration.lfoEnvelope.sustain, _voiceConfiguration.lfoEnvelope.release);

    envelopeLfo.attack(_voiceConfiguration.lfoEnvelope.attack);
    envelopeLfo.decay(_voiceConfiguration.lfoEnvelope.decay);
    envelopeLfo.sustain(_voiceConfiguration.lfoEnvelope.sustain);
    envelopeLfo.release(_voiceConfiguration.lfoEnvelope.release);
}

void Voice::configureOscilators()
{
    filterLfo.begin(_voiceConfiguration.audioWaveformFilterLfo());
    modLfo.begin(_voiceConfiguration.audioWaveformPitchLfo());

    uint8_t wf = _voiceConfiguration.audioWaveform(0);

    oscillators[0].begin(wf);

    // Serial.printf("Restart oscilators: %d\n", wf);

    for (int i = 0; i < 3; i++)
    {
        wf = _voiceConfiguration.audioWaveform(i + 1);

        int l = 1 + (i * 2);
        int r = l + 1;
        oscillators[l].begin(wf);
        oscillators[r].begin(wf);
    }
}
