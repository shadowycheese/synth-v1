#ifndef VOICECONTROLLER_H
#define VOICECONTROLLER_H

#include <Audio.h>
#include "Voice.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"
#include "../Constants.h"
#include "../utils/CallCounter.h"
#include "../utils/ClipDetector.h"

class VoiceController : public SynthConfigurationListener
{
public:
    VoiceController();

    AudioStream &getLeft() { return left; }
    AudioStream &getRight() { return right; }

    void noteOn(byte note, byte velocity);

    void noteOff(byte note, byte velocity);

    void begin();

    void task(uint32_t microSeconds);

    void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags);

private:
    void updateVoices(uint32_t microSeconds);
    void updateVoiceFilters(uint32_t microSeconds);

    Voice voicePool[MAX_VOICES];

    AudioMixer4 mixer1;
    AudioMixer4 mixer2;
    AudioMixer4 masterMix;
    AudioMixer4 left;
    AudioMixer4 right;

    AudioConnection patch0, patch1, patch2, patch3;
    AudioConnection patch4, patch5, patch6, patch7;

    AudioConnection patchM1, patchM2, patchMaster;

    AudioConnection patchLeft, patchRight;

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

    CallCounter voiceUpdates;
    CallCounter filterUpdates;
};

#endif