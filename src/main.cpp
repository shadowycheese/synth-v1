#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include "voice/VoiceController.h"
#include "voice/wave/Waves.h"
#include "USBHost_t36.h"
#include "Constants.h"
#include "io/ControllerIo.h"

AudioOutputI2S        i2s1;
AudioControlSGTL5000  sgtl5000;
AudioSynthWaveform waveForm;

SynthConfiguration synthConfiguration;

VoiceController voiceController(&synthConfiguration);

AudioConnection testPatch(voiceController.getOutput(), 0, i2s1, 0);

SimpleWaveSetter simpleWaveSetter;
SuperWaveSetter superWaveSetter;
ControllerIo controllerIo(&synthConfiguration, &voiceController);


USBHost myusb;
USBHub hunb(myusb);
MIDIDevice midi1(myusb);


void myNoteOn(byte channel, byte note, byte velocity) 
{
    Serial.printf("On: Channel %d, note %d, velocity %d\n", channel, note, velocity); 

    voiceController.noteOn(note, velocity);
}

void myNoteOff(byte channel, byte note, byte velocity) 
{
    Serial.printf("Off: Channel %d, note %d, velocity %d\n", channel, note, velocity); 
    voiceController.noteOff(note, velocity);
} 

void setup() 
{
    // Allocate memory for the audio engine
    AudioMemory(200);

    sgtl5000.enable(); 
    sgtl5000.volume(0.8);
    sgtl5000.lineOutLevel(21);
    sgtl5000.unmuteLineout();

    myusb.begin();

    usbMIDI.setHandleNoteOn(myNoteOn);
    usbMIDI.setHandleNoteOff(myNoteOff);
    
    WaveSetter* waveSetters[] = { &simpleWaveSetter, &superWaveSetter };

    voiceController.setWaveSetters(waveSetters);

    controllerIo.begin();
}

void loop() 
{
    //myusb.Task();
    usbMIDI.read();
    controllerIo.task();
}