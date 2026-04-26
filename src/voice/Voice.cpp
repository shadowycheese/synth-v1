#include "../Constants.h"
#include "Voice.h"

void Voice::noteOn(byte note, float frequency, float amplitude)
{
    _amplitudeScale = 0.25f;
    _frequency = frequency;

    configureVoice(false);

    envelope1.noteOn();

    _note = note;
    _timestamp = millis();
}

void Voice::noteOff()
{
    envelope1.noteOff();
}

void Voice::updateFilter()
{
    if (_voiceConfiguration.lfoPulseWidth == 0)
    {
        if (analyze.available())
        {
            lfo.pulseWidth(analyze.read());
        }
    }

    if (_voiceConfiguration.lfoFrequency == 0)
    {
        float freq = _frequency * _voiceConfiguration.pitchBend * (1.0f + _voiceConfiguration.detune);

        lfo.frequency(freq);
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

    if (envelope1.isActive() && (restartOscillators || updateVoice))
    {
        configureVoice(restartOscillators);
    }
}

bool Voice::isPlaying()
{
    return envelope1.isActive();
}

void Voice::configureVoice(bool restartOscillators)
{
    float frequency = _frequency * (1.0 + _voiceConfiguration.pitchBend);

    Serial.printf("supersaw freq: %0.3f (%0.3f), volume: %0.3f, detune %0.5f, resonance %0.3f\n",
                  frequency,
                  _frequency,
                  _amplitudeScale,
                  _voiceConfiguration.detune,
                  _voiceConfiguration.resonance);

    noise.amplitude(_amplitudeScale * _voiceConfiguration.noiseAmplitude);

    oscillators[0].frequency(frequency);
    oscillators[0].amplitude(_amplitudeScale * _voiceConfiguration.amplitudes[0]);

    float phase = 0;
    float width = _voiceConfiguration.detune;

    for (int i = 0; i < 3; i++)
    {
        int l = 1 + (i * 2);
        int r = l + 1;

        float centsR = VoiceConfiguration::CENTS[i + 1] * width * (DETUNE_MAX_SPREAD / 7.0f);
        float centsL = -centsR;

        float lf = frequency * powf(2.0f, centsL / 1200.0f);
        float rf = frequency * powf(2.0f, centsR / 1200.0f);

        phase += (width * 90);

        float amplitude = _amplitudeScale * _voiceConfiguration.amplitudes[i + 1];

        uint8_t wf = _voiceConfiguration.audioWaveform(i + 1);

        Serial.printf("left freq: %0.3f, right freq: %0.3f amplitude: %0.3f, waveform:%d (%s)\n",
                      lf,
                      rf,
                      amplitude,
                      wf,
                      restartOscillators ? "(restart)" : "");

        oscillators[l].frequency(lf);
        oscillators[l].amplitude(amplitude);
        oscillators[r].frequency(rf);
        oscillators[r].amplitude(amplitude);
    }
}

void Voice::configureEffects()
{
    if (_voiceConfiguration.reverbEnabled)
    {
        reverb.damping(0.5f);
        reverb.roomsize(_voiceConfiguration.reverb);

        reverbMixer.gain(0, 1.0f);
        reverbMixer.gain(1, 0.0f);
    }
    else
    {
        reverbMixer.gain(0, 0.0f);
        reverbMixer.gain(1, 1.0f);
    }

    if (_voiceConfiguration.chorusEnabled)
    {
        chorusMixer.gain(0.5, 1.0f);
        chorusMixer.gain(0.5, 0.0f);
    }
    else
    {
        chorusMixer.gain(0, 1.0f);
        chorusMixer.gain(1, 0.0f);
    }
}

void Voice::configureFilter()
{
    if (_voiceConfiguration.autoCutoff)
    {
        float trackingAmount = 1.0f; // Configurable
        float co = min(15000.0f, (_voiceConfiguration.filterCutoff) + (_frequency * trackingAmount * _voiceConfiguration.pitchBend));

        filter.frequency(co);
        filter.octaveControl(log2f(12000.0f / co));
    }
    else
    {
        float co = 20000.0f;

        co *= _voiceConfiguration.filterCutoff;

        filter.frequency(co);
        filter.octaveControl(log2f(12000.0f / co));
    }

    if (_voiceConfiguration.lfoPulseWidth > 0)
    {
        lfo.pulseWidth(_voiceConfiguration.lfoPulseWidth);
    }

    if (_voiceConfiguration.lfoFrequency > 0)
    {
        lfo.frequency(max(0.5, _voiceConfiguration.lfoFrequency));
    }

    lfo.amplitude(_voiceConfiguration.lfoAmplitude);

    pitchLevel.amplitude(_voiceConfiguration.pitchLevel);
    filterLevel.amplitude(_voiceConfiguration.filterLevel);

    updateFilter();
}

inline void Voice::configureEnvelope()
{
    envelope1.attack(_voiceConfiguration.attack);
    envelope1.decay(_voiceConfiguration.decay);
    envelope1.sustain(_voiceConfiguration.sustain);
    envelope1.release(_voiceConfiguration.release);
}

void Voice::configureOscilators()
{
    lfo.begin(_voiceConfiguration.audioWaveformLfo());
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
