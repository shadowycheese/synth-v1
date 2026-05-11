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
        oscillators[i].frequencyModulation(2);
        // oscillators[i].phaseModulation(180.0f);
    }

    oscilatorMixerMain.gain(0, 1.0f);
    oscilatorMixerMain.gain(1, 1.0f);

    reverbMixer.gain(0, 1.0f);
    reverbMixer.gain(1, 0.0f);

    filterLfo.amplitude(0.1f);
    filterLfo.offset(0.9f);
    filterLfo.frequency(0.2f);
    filterLfo.begin(WAVEFORM_TRIANGLE);

    pitchLfo.amplitude(1.0f);

    pitchLevel.amplitude(0.5f);

    envelopeVoice.attack(10);
    envelopeVoice.decay(50);
    envelopeVoice.sustain(0.7);
    envelopeVoice.release(300);

    envelopeFilter.attack(20);
    envelopeFilter.decay(500);
    envelopeFilter.sustain(0.5);
    envelopeFilter.release(1000);
}

void Voice::noteOn(byte note, float frequency, float amplitude)
{
    _amplitudeScale = 0.15f;
    _frequency = frequency;

    configureFilter();
    configureVoice(false);

    envelopeFilter.noteOn();
    envelopeVoice.noteOn();

    _note = note;
    _timestamp = millis();
}

void Voice::noteOff()
{
    envelopeVoice.noteOff();
    envelopeFilter.noteOff();
}

void Voice::updateFilter()
{
    if (_voiceConfiguration.pitchLfo.pulseWidth == 0.0f)
    {
        if (analyze.available())
        {
            pitchLfo.pulseWidth(analyze.read());
        }
    }

    if (_voiceConfiguration.pitchLfo.frequency == 0.0f)
    {
        float freq = _frequency * _voiceConfiguration.pitchBend * (1.0f + _voiceConfiguration.maxDetune);

        Serial.printf("Setting pitch freq from _frequency: %0.2f\n", freq);

        pitchLfo.frequency(freq);
    }
}

void Voice::onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags)
{
    if (envelopeChanged(changeFlags))
    {
        _voiceConfiguration.copyEnvelopeConfiguration(configuration);

        configureEnvelope();
    }

    if (effectChanged(changeFlags))
    {
        _voiceConfiguration.copyEffectConfiguration(configuration);

        configureEffects();
    }

    bool updateFilter = filterChanged(changeFlags);
    bool restartOscillators = waveformChanged(changeFlags);
    bool updateVoice = voiceChanged(changeFlags);

    if (updateFilter)
    {
        _voiceConfiguration.copyFilterConfiguration(configuration);

        configureFilter();
    }

    if (updateVoice)
    {
        _voiceConfiguration.copyVoiceConfiguration(configuration);
    }

    if (restartOscillators)
    {
        _voiceConfiguration.copyWaveformConfiguration(configuration);

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

    Serial.printf("main freq: %0.3f (%0.3f), volume: %0.3f, max detune: %0.5f, resonance: %0.3f, halfsaw: %d\n",
                  frequency,
                  _frequency,
                  _amplitudeScale,
                  _voiceConfiguration.maxDetune,
                  _voiceConfiguration.resonance,
                  _voiceConfiguration.halfSaw);

    noise.amplitude(_amplitudeScale * _voiceConfiguration.noiseAmplitude);

    pitchLevel.amplitude(_voiceConfiguration.pitchLfo.level);

    oscillators[0].frequency(frequency);
    oscillators[0].amplitude(_amplitudeScale * _voiceConfiguration.waveforms[0].amplitude);
    pulseWidths[0].amplitude(_voiceConfiguration.waveforms[0].pulseWidth);

    if (_voiceConfiguration.pitchLfo.frequency > 0.0f)
    {
        Serial.printf("Setting pitch freq from config: %0.2f\n", _voiceConfiguration.pitchLfo.frequency);

        pitchLfo.frequency(_voiceConfiguration.pitchLfo.frequency);
    }

    for (int i = 0; i < 3; i++)
    {
        int l = 1 + (i * 2);
        int r = l + 1;

        pulseWidths[i + 1].amplitude(_voiceConfiguration.waveforms[i + 1].pulseWidth);

        float centsR = VoiceConfiguration::CENTS[i + 1] * _voiceConfiguration.waveforms[i + 1].detune * (DETUNE_MAX_SPREAD / 7.0f);
        float centsL = -centsR;

        float lf = frequency * powf(2.0f, centsL / 1200.0f);
        float rf = frequency * powf(2.0f, centsR / 1200.0f);

        float amplitude = _amplitudeScale * _voiceConfiguration.waveforms[i + 1].amplitude;

        float rightAmplitude = _voiceConfiguration.halfSaw ? 0.0f : amplitude;

        uint8_t wf = _voiceConfiguration.audioWaveform(i + 1);

        Serial.printf("freq: %0.3f / freq: %0.3f amplitude: %0.3f / %0.3f, waveform:%d (%s)\n",
                      lf,
                      rf,
                      amplitude,
                      rightAmplitude,
                      wf,
                      restartOscillators ? "(restart)" : "");

        oscillators[l].frequency(lf);
        oscillators[l].amplitude(amplitude);
        oscillators[r].frequency(rf);
        oscillators[r].amplitude(_voiceConfiguration.halfSaw ? 0.0f : amplitude);
    }
}

void Voice::configureEffects()
{
    if (_voiceConfiguration.reverbEnabled)
    {
        reverbMixer.gain(0, 0.0f);
        reverbMixer.gain(1, 1.0f);
    }
    else
    {
        reverbMixer.gain(0, 1.0f);
        reverbMixer.gain(1, 0.0f);
    }
}

void Voice::configureFilter()
{
    if (_voiceConfiguration.autoCutoff)
    {
        float frequency = (_frequency * (_voiceConfiguration.pitchBend + 1.0f));

        float centsR = VoiceConfiguration::CENTS[3] * _voiceConfiguration.maxDetune * (DETUNE_MAX_SPREAD / 7.0f);
        float centsL = -centsR;

        float minFrequency = frequency * powf(2.0f, centsL / 1200.0f);
        float maxFrequency = frequency * powf(2.0f, centsR / 1200.0f);

        float offset = _voiceConfiguration.filterCutoff * (maxFrequency - minFrequency) + minFrequency;

        filter.frequency(offset);

        float maxOctaves = offset == 0 ? _voiceConfiguration.octaveControl : log2f(12000 / offset);

        float octaves = min(maxOctaves, _voiceConfiguration.octaveControl);

        // Serial.printf("Filter (A): min %0.1f, max %0.1f, used %0.1f, omax %0.3f, oct %0.3f\n", minFrequency, maxFrequency, offset, maxOctaves, octaves);

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

    filter.resonance(_voiceConfiguration.resonance);

    filterLfo.pulseWidth(_voiceConfiguration.filterLfo.pulseWidth);

    if (_voiceConfiguration.filterLfo.frequency > 0)
    {
        filterLfo.frequency(max(0.5, _voiceConfiguration.filterLfo.frequency));
    }

    filterLfo.amplitude(1.0f);

    filterLevel.amplitude(_voiceConfiguration.filterLfo.level);

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
}

void Voice::configureOscilators()
{
    filterLfo.begin(_voiceConfiguration.audioWaveformFilterLfo());
    pitchLfo.begin(_voiceConfiguration.audioWaveformPitchLfo());

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
