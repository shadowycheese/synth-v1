#include "../Constants.h"
#include "Voice.h"
#include "wave/WaveSetter.h"

// Constructor: This is where the internal "patching" happens
Voice::Voice() : 
    patch1(waveforms[0], 0, mixer1, 0),
    patch2(waveforms[1], 0, mixer1, 1),
    patch3(waveforms[2], 0, mixer1, 2),
    patch4(waveforms[3], 0, mixer1, 3),

    patch5(waveforms[4], 0, mixer2, 0),
    patch6(waveforms[5], 0, mixer2, 1),
    patch7(waveforms[6], 0, mixer2, 2),

    patchM1(mixer1, 0, voiceMix, 0),
    patchM2(mixer2, 0, voiceMix, 1),

    patchEnv(waveforms[0], 0, envelope1, 0) 
{
    for (int i = 0; i < 4; i++) {
        mixer1.gain(i, 1.0f);
    }

    for (int i = 0; i < 3; i++) {
        mixer2.gain(i, 1.0f);
    }

    voiceMix.gain(0, 1.0f); 
    voiceMix.gain(1, 1.0f); 

    envelope1.attack(10);
    envelope1.decay(50);
    envelope1.sustain(0.7);
    envelope1.release(300);
}

void Voice::setWave(WaveSetter *setter) {
    waveSetter = setter;
}

void Voice::noteOn(byte note, float frequency, float velocity) {
    if (waveSetter) {
        waveSetter->configure(frequency, velocity, waveforms);
    }

    envelope1.noteOn();

    _note = note;
    _timestamp = millis();
}

void Voice::noteOff() {
    envelope1.noteOff();
}

void Voice::setEnvelope(float a, float d, float s, float r) {
    envelope1.attack(a);
    envelope1.decay(d);
    envelope1.sustain(s);
    envelope1.release(r);
}

bool Voice::isPlaying() 
{
    return envelope1.isActive();
}
