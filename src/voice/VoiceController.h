#ifndef VOICECONTROLLER_H
#define VOICECONTROLLER_H

#include <Audio.h>
#include "Voice.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"
#include "../Constants.h"
#include "wave/WaveSetter.h"

class VoiceController : public SynthConfigurationListener
{
public:
    VoiceController();

    AudioStream &getOutput() { return masterMix; }

    void setWaveSetters(WaveSetter *setters[]);

    void noteOn(byte note, byte velocity);

    void noteOff(byte note, byte velocity);

    void begin();

    void setWaveSetter(int setter);

    void onSynthConfigurationChanged(SynthConfiguration *configuration, int changeFlags);

private:
    Voice voicePool[MAX_VOICES];

    AudioMixer4 mixer1;
    AudioMixer4 mixer2;
    AudioMixer4 masterMix;

    AudioConnection patch0, patch1, patch2, patch3;
    AudioConnection patch4, patch5, patch6, patch7;

    AudioConnection patchM1, patchM2, patchMaster;

    byte notesVoiceMap[256];

    int findOldestVoice(byte note);
};

#endif