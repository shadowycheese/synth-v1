#ifndef VOICECONTROLLER_H
#define VOICECONTROLLER_H

#include <Audio.h>
#include "Voice.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"
#include "../Constants.h"

class VoiceController : public SynthConfigurationListener
{
public:
    VoiceController();

    AudioStream &getOutput() { return masterMix; }

    void noteOn(byte note, byte velocity);

    void noteOff(byte note, byte velocity);

    void begin();

    void task();

    void onSynthConfigurationChanged(SynthConfiguration *configuration, int changeFlags);

private:
    void updateVoices();
    void updateVoiceFilters();

    Voice voicePool[MAX_VOICES];

    AudioMixer4 mixer1;
    AudioMixer4 mixer2;
    AudioMixer4 masterMix;

    AudioConnection patch0, patch1, patch2, patch3;
    AudioConnection patch4, patch5, patch6, patch7;

    AudioConnection patchM1, patchM2, patchMaster;

    byte notesVoiceMap[256];

    int findOldestVoice(byte note);

    // Filter updating
    int nextFilterToUpdate;
    uint32_t nextFilterUpdateTime;

    // Voice updating
    SynthConfiguration voiceConfiguration;
    int voiceConfigurationVersion;
    uint32_t nextVoiceUpdateTime;
    int nextVoiceToUpdate;
    int voiceVersions[MAX_VOICES];
    int pendingChanges[MAX_VOICES];
};

#endif