#ifndef VOICECONTROLLER_H
#define VOICECONTROLLER_H

#include <Audio.h>
#include "Voice.h"
#include "../SynthConfiguration.h"
#include "../SynthConfigurationListener.h"
#include "../Constants.h"
#include "../io/Indicators.h"
#include "../utils/CallCounter.h"

class VoiceController : public SynthConfigurationListener
{
public:
    VoiceController();

    AudioStream &getLeft() { return leftAmp; }
    AudioStream &getRight() { return rightAmp; }

    void noteOn(byte note, byte velocity);

    void noteOff(byte note, byte velocity);

    void begin();

    void task(uint32_t microSeconds);

    void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags);

private:
    void updateVoices(uint32_t microSeconds);
    void updateVoiceFilters(uint32_t microSeconds);
    void updateIndicators(uint32_t microSeconds);

    Voice voicePool[MAX_VOICES];

    AudioMixer4 mixer1;
    AudioMixer4 mixer2;
    AudioMixer4 masterMix;

    AudioMixer4 left;
    AudioMixer4 right;
    AudioAmplifier leftAmp;
    AudioAmplifier rightAmp;
    AudioEffectReverb reverb;
    AudioAnalyzePeak peak;
    AudioAnalyzePeak peak1;
    AudioAnalyzePeak peak2;
    AudioAnalyzePeak peak3;
    AudioAnalyzePeak peak4;
    AudioAnalyzePeak peak5;

    AudioConnection patches[40] =
        {
            AudioConnection(voicePool[0].getOutput(), 0, mixer1, 0),
            AudioConnection(voicePool[1].getOutput(), 0, mixer1, 1),
            AudioConnection(voicePool[2].getOutput(), 0, mixer1, 2),
            AudioConnection(voicePool[3].getOutput(), 0, mixer1, 3),

            AudioConnection(voicePool[4].getOutput(), 0, mixer2, 0),
            AudioConnection(voicePool[5].getOutput(), 0, mixer2, 1),
            AudioConnection(voicePool[6].getOutput(), 0, mixer2, 2),
            AudioConnection(voicePool[7].getOutput(), 0, mixer2, 3),

            AudioConnection(mixer1, 0, masterMix, 0),
            AudioConnection(mixer2, 0, masterMix, 1),

            AudioConnection(masterMix, 0, reverb, 0),

            AudioConnection(mixer1, 0, peak1, 0),
            AudioConnection(mixer2, 0, peak2, 0),

            AudioConnection(masterMix, 0, peak3, 0),
            AudioConnection(reverb, 0, peak4, 0),

            AudioConnection(masterMix, 0, left, 0),
            AudioConnection(masterMix, 0, right, 0),

            AudioConnection(reverb, 0, left, 1),
            AudioConnection(reverb, 0, right, 1),

            AudioConnection(left, 0, leftAmp, 0),
            AudioConnection(right, 0, rightAmp, 0), //
            AudioConnection(left, 0, peak5, 0),

            AudioConnection(leftAmp, 0, peak, 0)};

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
    float peak1f, peak2f, peak3f, peak4f, peak5f;

    CallCounter voiceUpdates;
    CallCounter filterUpdates;

    // Indicators
    uint32_t nextIndicatorUpdateTime;
    Indicators indicators;
    float _lastPeak;
    bool _wasOverdrive;
};

#endif