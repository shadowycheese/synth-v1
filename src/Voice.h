#ifndef VOICE_H
#define VOICE_H

#include <Audio.h>

class Voice {
public:
    Voice();
    void noteOn(float frequency);
    void noteOff();
    void setEnvelope(float a, float d, float s, float r);
    
    // We need a way to "patch" this voice to a mixer outside the class
    AudioStream& getOutput() { return envelope1; }

private:
    AudioSynthWaveform   waveform1;
    AudioEffectEnvelope  envelope1;
    
    // Connections within the voice
    AudioConnection      patch1; 

    float _amplitude = 0.5;
};

#endif