#include "../Constants.h"
#include "Voice.h"

// Constructor: This is where the internal "patching" happens
Voice::Voice() : patch1(waveforms[0], 0, mixer1, 0),
                 patch2(waveforms[1], 0, mixer1, 1),
                 patch3(waveforms[2], 0, mixer1, 2),
                 patch4(waveforms[3], 0, mixer1, 3),

                 patch5(waveforms[4], 0, mixer2, 0),
                 patch6(waveforms[5], 0, mixer2, 1),
                 patch7(waveforms[6], 0, mixer2, 2),
                 patch8(noise, 0, mixer2, 3),

                 patchM1(mixer1, 0, voiceMix, 0),
                 patchM2(mixer2, 0, voiceMix, 1),

                 patchFilter(voiceMix, ladderFilter),
                 patchEnv(ladderFilter, 0, envelope1, 0),
                 patchAmp(envelope1, 0, amp, 0),
                 // patchFilter(voiceMix, envelope1),
                 patchAnalyze(envelope1, 0, analyze, 0)
{
    for (int i = 0; i < 4; i++)
    {
        mixer1.gain(i, 1.0f);
    }

    for (int i = 0; i < 4; i++)
    {
        mixer2.gain(i, 1.0f);
    }

    ladderFilter.octaveControl(3);

    voiceMix.gain(0, 1.0f);
    voiceMix.gain(1, 1.0f);

    envelope1.attack(10);
    envelope1.decay(50);
    envelope1.sustain(0.7);
    envelope1.release(300);

    amp.gain(3.5f);
}

void Voice::noteOn(byte note, float frequency, float amplitude)
{
    // int activeOscilators = _voiceConfiguration.activeOscilators();

    // float divider = activeOscilators <= 0 ? 1.0 : (float)activeOscilators;

    _amplitudeScale = 0.15f;
    _frequency = frequency;

    configure(true);

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
    if (analyze.available())
    {
        float peak = analyze.read();
        int angle = _iteration++ % 360;
        float radians = (angle / 360) * 6.28318;

        float dynamicCutoff = _frequency * 1.75f + ((750 + 750 * sin(radians)) * peak);
        // ladderFilter.frequency(dynamicCutoff);
        // ladderFilter.resonance(_voiceConfiguration.resonance);
        ladderFilter.frequency(30000);
        ladderFilter.resonance(0);
    }
}

void Voice::onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags)
{
    if (envelopeChanged(changeFlags))
    {
        envelope1.attack(configuration->attack);
        envelope1.decay(configuration->decay);
        envelope1.sustain(configuration->sustain);
        envelope1.release(configuration->release);
    }

    if (volumeChanged(changeFlags))
    {
        // amp.gain(1.5 + 1.5 * configuration->voiceGain);
        amp.gain(2.5 + configuration->voiceGain);
    }

    if (filterChanged(changeFlags))
    {
        _voiceConfiguration.copyFilterConfiguration(configuration);

        ladderFilter.resonance(_voiceConfiguration.resonance);
    }

    bool restartOscillators = oscillatorChanged(changeFlags);
    bool updateVoice = voiceChanged(changeFlags);

    if (updateVoice)
    {
        _voiceConfiguration.copyVoiceConfiguration(configuration);
    }

    if (restartOscillators)
    {
        _voiceConfiguration.copyOscillatorConfiguration(configuration);

        Serial.printf("Waveform: %d, Oscilators: %d\n", _voiceConfiguration.audioWaveform(0), _voiceConfiguration.activeOscilators());
    }

    if (envelope1.isActive() && (restartOscillators || updateVoice))
    {
        configure(restartOscillators);
    }
}

bool Voice::isPlaying()
{
    return envelope1.isActive();
}

void Voice::configure(bool restart)
{
    if (_voiceConfiguration.superSawMode)
    {
        configureSuperSaw(restart);
    }
    else
    {
        configureStandard(restart);
    }
}

void Voice::configureStandard(bool restart)
{
    float frequency = _frequency * (1.0 + _voiceConfiguration.pitch);

    Serial.printf("standard freq: %0.3f (%0.3f), volume: %0.3f, oscillators: %d, detune %0.5f, resonance %0.3f\n",
                  frequency,
                  _frequency,
                  _amplitudeScale,
                  _voiceConfiguration.activeOscilators(),
                  _voiceConfiguration.detune,
                  _voiceConfiguration.resonance);

    noise.amplitude(_amplitudeScale * _voiceConfiguration.noiseAmplitude);

    for (int i = 0; i < 4; i++)
    {
        float amplitude = _amplitudeScale * _voiceConfiguration.amplitudes[i];

        Serial.printf("Osc %d freq: %0.3f, amplitude: %0.3f, waveform:%d\n",
                      i,
                      frequency,
                      amplitude,
                      _voiceConfiguration.audioWaveform(i));

        configureOscilator(i, frequency, amplitude, 0);

        if (restart)
        {
            waveforms[i].begin(_voiceConfiguration.audioWaveform(i));
        }
    }

    for (int i = 4; i < 7; i++)
    {
        waveforms[i].amplitude(0.0f);
    }
}

void Voice::configureSuperSaw(bool restart)
{
    float frequency = _frequency * (1.0 + _voiceConfiguration.pitch);

    Serial.printf("supersaw freq: %0.3f (%0.3f), volume: %0.3f, oscillators: %d, detune %0.5f, resonance %0.3f\n",
                  frequency,
                  _frequency,
                  _amplitudeScale,
                  _voiceConfiguration.activeOscilators(),
                  _voiceConfiguration.detune,
                  _voiceConfiguration.resonance);
    waveforms[0].frequency(frequency);
    waveforms[0].pulseWidth(_voiceConfiguration.pulseWidth);

    noise.amplitude(_amplitudeScale * _voiceConfiguration.noiseAmplitude);
    waveforms[0].amplitude(_amplitudeScale * _voiceConfiguration.amplitudes[0]);

    if (restart)
    {
        waveforms[0].begin(_voiceConfiguration.audioWaveform(0));
    }

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

        Serial.printf("left freq: %0.3f, right freq: %0.3f amplitude: %0.3f, waveform:%d\n",
                      lf,
                      rf, amplitude,
                      _voiceConfiguration.audioWaveform(i + 1));

        configureOscilator(l, lf, amplitude, -phase);
        configureOscilator(r, rf, amplitude, phase);

        if (restart)
        {
            waveforms[l].begin(_voiceConfiguration.audioWaveform(i + 1));
            waveforms[r].begin(_voiceConfiguration.audioWaveform(i + 1));
        }
    }
}

inline void Voice::configureOscilator(int id, float frequency, float amplitude, float phase)
{
    waveforms[id].frequency(frequency);
    waveforms[id].amplitude(amplitude);
    waveforms[id].phase(phase);
    waveforms[id].pulseWidth(_voiceConfiguration.pulseWidth);
}
