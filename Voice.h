#ifndef VOICE_H
#define VOICE_H

#include <Audio.h>

class Voice
{
public:
  // --- Audio Objects ---
  AudioSynthWaveformDc lfoShaper;
  AudioSynthWaveformDc pitchLevel;
  AudioSynthWaveformDc filterLevel;
  AudioSynthWaveformModulated lfo;
  AudioSynthWaveform waveform1;
  AudioEffectMultiply filterMultiply;
  AudioEffectMultiply pitchMultiply;
  AudioSynthWaveformDc oscillatorShaper;
  AudioSynthWaveformModulated waveformMod1;
  AudioSynthNoiseWhite noise1;
  AudioFilterStateVariable filter1;
  AudioEffectFreeverbStereo freeverbs1;
  AudioMixer4 voiceMixer1;
  AudioMixer4 voiceMixer2;
  AudioEffectEnvelope envelopeVoice;
  AudioMixer4 voceMixerMain;
  AudioEffectChorus chorus1;
  AudioOutputI2S i2s1;

  // --- Patch Cords ---
  AudioConnection patchCord1;
  AudioConnection patchCord2;
  AudioConnection patchCord3;
  AudioConnection patchCord4;
  AudioConnection patchCord5;
  AudioConnection patchCord6;
  AudioConnection patchCord20;
  AudioConnection patchCord28;
  AudioConnection patchCord29;
  AudioConnection patchCord30;
  AudioConnection patchCord31;
  AudioConnection patchCord32;
  AudioConnection patchCord33;
  AudioConnection patchCord34;

  Voice() : patchCord1(lfoShaper, 0, lfo, 1),
            patchCord2(pitchLevel, 0, pitchMultiply, 0),
            patchCord3(filterLevel, 0, filterMultiply, 1),
            patchCord4(lfo, 0, filterMultiply, 0),
            patchCord5(lfo, 0, pitchMultiply, 1),
            patchCord6(filterMultiply, 0, filter1, 1),
            patchCord20(noise1, 0, voiceMixer2, 3),
            patchCord28(filter1, 0, envelopeVoice, 0),
            patchCord29(voiceMixer1, 0, voceMixerMain, 0),
            patchCord30(voiceMixer2, 0, voceMixerMain, 1),
            patchCord31(envelopeVoice, chorus1),
            patchCord32(envelopeVoice, 0, lfo, 0),
            patchCord33(voceMixerMain, 0, filter1, 0),
            patchCord34(chorus1, 0, i2s1, 0)
  {
  } // Empty constructor body
};

#endif