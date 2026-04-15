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

                 // patchFilter(voiceMix, ladderFilter),
                 patchEnv(voiceMix, 0, envelope1, 0)
{
    for (int i = 0; i < 4; i++)
    {
        mixer1.gain(i, 1.0f);
    }

    for (int i = 0; i < 4; i++)
    {
        mixer2.gain(i, 1.0f);
    }

    ladderFilter.octaveControl(1);

    voiceMix.gain(0, 1.0f);
    voiceMix.gain(1, 1.0f);

    envelope1.attack(10);
    envelope1.decay(50);
    envelope1.sustain(0.7);
    envelope1.release(300);
}

void Voice::noteOn(byte note, float frequency, float amplitude)
{
    float divider = _voiceConfiguration.oscillators <= 0 ? 1.0 : (float)_voiceConfiguration.oscillators;

    _frequency = frequency;
    _amplitude = amplitude / divider;

    configure(frequency, _amplitude, true);

    envelope1.noteOn();

    _note = note;
    _timestamp = millis();
}

void Voice::noteOff()
{
    envelope1.noteOff();
}

void Voice::onSynthConfigurationChanged(SynthConfiguration *configuration, int changeFlags)
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
        float voiceGain = configuration->voiceGain;

        for (int i = 0; i < 4; i++)
        {
            mixer1.gain(i, voiceGain);
        }

        for (int i = 0; i < 4; i++)
        {
            mixer2.gain(i, voiceGain);
        }
    }

    if (voiceConfigurationChanged(changeFlags))
    {
        _voiceConfiguration.detune = configuration->detune;
        _voiceConfiguration.oscillators = configuration->oscillators;
        _voiceConfiguration.resonance = configuration->resonance;

        ladderFilter.resonance(_voiceConfiguration.resonance);
    }

    bool reset = false;

    if (waveFormChanged(changeFlags))
    {
        _voiceConfiguration.mainWaveForm = configuration->mainWaveForm;
        _voiceConfiguration.detuneWaveForm = configuration->mainWaveForm;

        Serial.printf("Waveform: %d, Oscilators: %d\n", _voiceConfiguration.mainWaveForm, _voiceConfiguration.oscillators);

        reset = true;
    }

    if (envelope1.isActive() && (waveFormChanged(changeFlags) || voiceConfigurationChanged(changeFlags)))
    {
        configure(_frequency, _amplitude, reset);
    }
}

bool Voice::isPlaying()
{
    return envelope1.isActive();
}

void Voice::configure(float frequency, float amplitude, bool restart)
{
    Serial.printf("configure freq: %0.3f, volume: %0.3f, oscillators: %d, detune %0.5f, resonance %0.3f\n",
                  frequency,
                  amplitude,
                  _voiceConfiguration.oscillators,
                  _voiceConfiguration.detune,
                  _voiceConfiguration.resonance);

    ladderFilter.frequency(frequency);

    if (_voiceConfiguration.oscillators < 0)
    {
        for (int i = 0; i < 7; i++)
        {
            waveforms[i].amplitude(0.0);
        }

        noise.amplitude(amplitude);

        return;
    }

    noise.amplitude(0);
    waveforms[0].frequency(frequency);

    if (restart)
    {
        waveforms[0].amplitude(amplitude);
        waveforms[0].begin(_voiceConfiguration.mainWaveForm);
    }

    float lf = frequency;
    float rf = frequency;

    int range = (_voiceConfiguration.oscillators - 1) / 2;
    int phase = 0;

    for (int i = 0; i < 3; i++)
    {
        int l = 1 + (i * 2);
        int r = l + 1;

        lf /= _voiceConfiguration.detune;
        rf *= _voiceConfiguration.detune;

        amplitude *= 0.7;

        phase += 6;

        waveforms[l].frequency(lf);
        waveforms[r].frequency(rf);

        if (i < range)
        {
            Serial.printf("left freq: %0.3f, right freq: %0.3f amplitude: %0.3f\n", lf, rf, amplitude);

            if (restart)
            {
                waveforms[l].phase(-phase);
                waveforms[r].phase(+phase);
                waveforms[l].amplitude(amplitude);
                waveforms[r].amplitude(amplitude);
                waveforms[l].begin(_voiceConfiguration.detuneWaveForm);
                waveforms[r].begin(_voiceConfiguration.detuneWaveForm);
            }
        }
        else
        {
            waveforms[l].amplitude(0);
            waveforms[r].amplitude(0);
        }
    }
}
