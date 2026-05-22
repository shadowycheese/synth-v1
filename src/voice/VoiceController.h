#ifndef VOICE_CONTROLLER_H
#define VOICE_CONTROLLER_H

#include <Audio.h>
#include "Voice.h"
#include "../config/SynthConfiguration.h"
#include "../utils/CallCounter.h"
#include "../io/Indicators.h"
#include "../store/WaveformStore.h"
#include "../audio/AudioAnalyzeOverdrive.h"

#define MAX_VOICES 8

class VoiceController : public SynthConfigurationListener
{
public:
    VoiceController(Indicators *indicators, WaveformStore *waveformStore);

    AudioStream &getLeft() { return leftAmp; }
    AudioStream &getRight() { return rightAmp; }

    void noteOn(byte note, byte velocity);

    void noteOff(byte note, byte velocity);

    void begin();

    void task(uint32_t microSeconds);

    void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags);

private:
    void updateVoices(uint32_t microSeconds);
    void updateVoiceFiltersAndEffects(uint32_t microSeconds);
    void updateIndicators(uint32_t microSeconds);

    inline static float midiNoteHz(int8_t note) { return 440.0f * pow(2.0f, (note - 69.0f) / 12.0f); };

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
    AudioAnalyzeOverdrive overdrive[4];

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

            AudioConnection(masterMix, 0, left, 0),
            AudioConnection(masterMix, 0, right, 0),

            AudioConnection(reverb, 0, left, 1),
            AudioConnection(reverb, 0, right, 1),

            AudioConnection(left, 0, leftAmp, 0),
            AudioConnection(right, 0, rightAmp, 0),

            AudioConnection(leftAmp, 0, peak, 0),

            AudioConnection(mixer1, 0, overdrive[0], 0),
            AudioConnection(mixer2, 0, overdrive[1], 0),
            AudioConnection(masterMix, 0, overdrive[2], 0),
            AudioConnection(leftAmp, 0, overdrive[3], 0),
        };

    byte _notesVoiceMap[256];

    int findOldestVoice(byte note);

    // Filter updating
    int _nextFilterToUpdate;
    uint32_t _nextFilterUpdateTime;

    // Voice updating
    SynthConfiguration _synthConfiguration;
    int _voiceConfigurationVersion;
    uint32_t nextVoiceUpdateTime;
    int _nextVoiceToUpdate;
    int _voiceVersions[MAX_VOICES];
    int _pendingChanges[MAX_VOICES];

    CallCounter _voiceUpdates;
    CallCounter _filterUpdates;

    // Indicators
    uint32_t _nextIndicatorUpdateTime;
    Indicators *_indicators;
    float _lastPeak;
    bool _wasOverdrive;
};

#endif