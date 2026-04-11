#include "Voice.h"

// Constructor: This is where the internal "patching" happens
Voice::Voice() : patch1(waveform1, 0, envelope1, 0) {
    waveform1.begin(WAVEFORM_SAWTOOTH);
    waveform1.amplitude(_amplitude);
    
    // Default envelope settings
    envelope1.attack(10);
    envelope1.decay(50);
    envelope1.sustain(0.7);
    envelope1.release(300);
}

void Voice::noteOn(float frequency) {
    waveform1.frequency(frequency);
    envelope1.noteOn();
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