#ifndef VOICE_H
#define VOICE_H

#include <Audio.h>
#include "../Constants.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"
#include "VoiceConfiguration.h"
#include "../utils/ClipDetector.h"

#define CHORUS_DELAY_LEN (44100 * 40 / 1000) // 35ms

class Voice : public SynthConfigurationListener
{
public:
    Voice() : pitchPatch(pitchLevel, 0, pitchMultiply, 0),
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
            // oscillators[i].frequencyModulation(2);
            oscillators[i].phaseModulation(180.0f);
        }

        oscilatorMixerMain.gain(0, 1.0f);
        oscilatorMixerMain.gain(1, 1.0f);

        reverbMixer.gain(0, 1.0f);
        reverbMixer.gain(1, 0.0f);

        chorusMixer.gain(0, 0.5f);
        chorusMixer.gain(1, 0.5f);

        lfo.amplitude(1.0f);
        pitchLevel.amplitude(0.5f);

        envelope1.attack(10);
        envelope1.decay(50);
        envelope1.sustain(0.7);
        envelope1.release(300);

        chorus.begin(delayBuffer, CHORUS_DELAY_LEN, 4);
    };

    void noteOn(byte note, float frequency, float velocity);
    void noteOff();

    AudioStream &getOutput() { return chorusMixer; }

    float volume();
    bool isPlaying();
    uint32_t timestamp() { return _timestamp; };
    byte noteLastPlayed() { return _note; };

    void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags);
    void updateFilter();

private:
    AudioMixer4 oscilatorMixer1;
    AudioMixer4 oscilatorMixer2;
    AudioMixer4 oscilatorMixerMain;

    AudioSynthWaveformDc pitchLevel;
    AudioSynthWaveformDc filterLevel;
    AudioSynthWaveformModulated oscillators[7];
    AudioSynthWaveform lfo;
    AudioEffectMultiply pitchMultiply;
    AudioEffectMultiply filterMultiply;
    AudioSynthNoiseWhite noise;
    AudioFilterStateVariable filter;
    AudioEffectEnvelope envelope1;
    AudioEffectFreeverb reverb;
    AudioEffectChorus chorus;
    AudioAnalyzePeak analyze;
    AudioMixer4 reverbMixer;
    AudioMixer4 chorusMixer;

    AudioConnection pitchPatch, lfoPatch1, filterLevelPatch, lfoPatch2;
    AudioConnection pitchMultiplyPatch, filterMultiplyPatch;

    AudioConnection patchOscIn0, patchOscIn1, patchOscIn2, patchOscIn3;
    AudioConnection patchOscIn4, patchOscIn5, patchOscIn6;

    AudioConnection patchOscOut0, patchOscOut1, patchOscOut2, patchOscOut3;
    AudioConnection patchOscOut4, patchOscOut5, patchOscOut6, patchOscOut7;

    AudioConnection patchOscMain1, patchOscMain2;

    AudioConnection patchFilter1, patchFilter2;

    AudioConnection patchEnv, patchAnalyze;
    AudioConnection patchReverb1, patchReverb2, patchReverb3;
    AudioConnection patchChorus1, patchChorus2, patchChorus3;

    VoiceConfiguration _voiceConfiguration;

    short delayBuffer[CHORUS_DELAY_LEN];

    uint32_t _timestamp;
    byte _note;
    float _frequency;
    float _amplitudeScale;
    uint32_t _iteration;

    void configureVoice(bool restart);
    void configureFilter();
    void configureEffects();
    void configureOscilators();
    inline void configureEnvelope();
};

#endif