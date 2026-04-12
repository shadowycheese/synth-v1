#ifndef VOICE_H
#define VOICE_H

#include <Audio.h>
#include "../Constants.h"
#include "wave/WaveSetter.h"

class Voice {
public:
    Voice();
    void noteOn(byte note, float frequency, float velocity);
    void noteOff();
    void setEnvelope(float a, float d, float s, float r);
    void setWave(WaveSetter *waveSetter);
    // We need a way to "patch" this voice to a mixer outside the class
    AudioStream& getOutput() { return envelope1; }

    float volume();
    bool isPlaying();
    uint32_t timestamp() { return _timestamp; };
    byte noteLastPlayed() { return _note; };

private:
    AudioMixer4 mixer1;
    AudioMixer4 mixer2;
    AudioMixer4 voiceMix;

    AudioSynthWaveform   waveforms[7];

    AudioEffectEnvelope  envelope1;
    
    AudioConnection      patch1, patch2, patch3, patch4; 
    AudioConnection      patch5, patch6, patch7;         
    AudioConnection      patchM1, patchM2;               
    AudioConnection      patchEnv;
    
    WaveSetter* waveSetter = 0;

    uint32_t _timestamp;
    byte _note;
};

#endif