#include "../Constants.h"
#include "Voice.h"

// Constructor: This is where the internal "patching" happens
Voice::Voice() : pitchPatch(pitch, 0, pitchMultiply, 0),
                 lfoPatch1(lfo, 0, pitchMultiply, 1),

                 filterLevelPatch(filterLevel, 0, filterMultiply, 0),
                 lfoPatch2(lfo, 0, filterMultiply, 1),

                 patchOscIn0(pitchMultiply, 0, oscillators[0], 0),
                 patchOscIn1(pitchMultiply, 0, oscillators[1], 0),
                 patchOscIn2(pitchMultiply, 0, oscillators[2], 0),
                 patchOscIn3(pitchMultiply, 0, oscillators[3], 0),
                 patchOscIn4(pitchMultiply, 0, oscillators[4], 0),
                 patchOscIn5(pitchMultiply, 0, oscillators[5], 0),
                 patchOscIn6(pitchMultiply, 0, oscillators[6], 0),

                 patchOscOut0(oscillators[0], 0, oscilatorMixer1, 0),
                 patchOscOut1(oscillators[1], 0, oscilatorMixer1, 1),
                 patchOscOut2(oscillators[2], 0, oscilatorMixer1, 2),
                 patchOscOut3(oscillators[3], 0, oscilatorMixer1, 3),
                 patchOscOut4(oscillators[4], 0, oscilatorMixer2, 0),
                 patchOscOut5(oscillators[5], 0, oscilatorMixer2, 1),
                 patchOscOut6(oscillators[6], 0, oscilatorMixer2, 1),
                 patchOscOut7(noise, 0, oscilatorMixer2, 3),

                 patchOscMain1(oscilatorMixer1, 0, oscilatorMixerMain, 0),
                 patchOscMain2(oscilatorMixer2, 0, oscilatorMixerMain, 1),

                 patchFilter1(oscilatorMixerMain, 0, filter, 0),
                 patchFilter2(filterMultiply, 0, filter, 1),
                 patchEnv(filter, 0, envelope1, 0),
                 patchReverb1(envelope1, 0, reverbMixer, 0),
                 patchReverb2(envelope1, 0, reverb, 0),
                 patchReverb3(reverb, 0, reverbMixer, 1),
                 patchChorus1(reverbMixer, 0, chorusMixer, 0),
                 patchChorus2(reverbMixer, 0, chorus, 0),
                 patchChorus3(chorus, 0, chorusMixer, 1)
{
    for (int i = 0; i < 4; i++)
    {
        oscilatorMixer1.gain(i, 1.0f);
    }

    for (int i = 0; i < 4; i++)
    {
        oscilatorMixer2.gain(i, 1.0f);
    }

    for (int i = 0; i < 7; i++)
    {
        oscillators[i].frequencyModulation(2);
    }

    oscilatorMixerMain.gain(0, 1.0f);
    oscilatorMixerMain.gain(1, 1.0f);

    reverbMixer.gain(0, 1.0f);
    reverbMixer.gain(1, 0.0f);

    chorusMixer.gain(0, 0.5f);
    chorusMixer.gain(1, 0.5f);

    lfo.amplitude(1.0f);
    pitch.amplitude(0.5f);

    envelope1.attack(10);
    envelope1.decay(50);
    envelope1.sustain(0.7);
    envelope1.release(300);

    chorus.begin(delayBuffer, CHORUS_DELAY_LEN, 4);
}

void Voice::noteOn(byte note, float frequency, float amplitude)
{
    _amplitudeScale = 0.15f;
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

    if (envelope1.isActive())
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

    lfo.pulseWidth(_voiceConfiguration.lfoPulseWidth);
    lfo.frequency(max(0.5, _voiceConfiguration.lfoFrequency));
    lfo.amplitude(_voiceConfiguration.lfoAmplitude);

    pitch.amplitude(_voiceConfiguration.pitchLevel);

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
        float co = 50 * exp(5.481 * _voiceConfiguration.manualCutoff);
        filter.frequency(co);
        filter.octaveControl(log2f(12000.0 / co));
    }
    else
    {
        float co = _voiceConfiguration.manualCutoff * _frequency * (TWELTH_ROOT_OF_TWO * TWELTH_ROOT_OF_TWO);
        filter.frequency(co);
        filter.octaveControl(log2f(12000.0 / co));
    }

    // filter.resonance(_voiceConfiguration.resonance);

    filterLevel.amplitude(_voiceConfiguration.filterLevel);
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

    Serial.printf("Restart oscilators: %d\n", wf);

    for (int i = 0; i < 3; i++)
    {
        wf = _voiceConfiguration.audioWaveform(i + 1);

        int l = 1 + (i * 2);
        int r = l + 1;
        oscillators[l].begin(wf);
        oscillators[r].begin(wf);
    }
}
