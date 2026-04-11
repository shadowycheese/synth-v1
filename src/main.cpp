#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include "Voice.h"
#include "USBHost_t36.h"

#define MAX_VOICES 1
Voice voicePool[MAX_VOICES];

// You'll need mixers to sum these. 
// A Mixer4 takes 4 inputs, so 2 mixers feed into a 3rd master mixer.
AudioMixer4 mixerLeft;
AudioMixer4 mixerRight;
AudioMixer4 masterMix;

// Connections (Statically defined in the global scope)
AudioConnection patch0(voicePool[0].getOutput(), 0, mixerLeft, 0);
/*AudioConnection patch1(voicePool[1].getOutput(), 0, mixerLeft, 1);
AudioConnection patch2(voicePool[2].getOutput(), 0, mixerLeft, 2);
AudioConnection patch3(voicePool[3].getOutput(), 0, mixerLeft, 3);
AudioConnection patch4(voicePool[4].getOutput(), 0, mixerRight, 0);
AudioConnection patch5(voicePool[5].getOutput(), 0, mixerRight, 1);
AudioConnection patch6(voicePool[6].getOutput(), 0, mixerRight, 2);
AudioConnection patch7(voicePool[7].getOutput(), 0, mixerRight, 3);*/

/*AudioConnection patch8(mixerLeft, 0, masterMix, 0);
AudioConnection patch9(mixerRight, 0, masterMix, 1);*/

AudioOutputI2S           i2s1;
AudioControlSGTL5000     sgtl5000_1;

AudioConnection          patchCord1(voicePool[0].getOutput(), 0, i2s1, 0);

USBHost myusb;
USBHub hunb(myusb);
MIDIDevice midi1(myusb);

void myNoteOn(byte channel, byte note, byte velocity) {
  Serial.printf("On: Channel %d, note %d, velocity %d\n", channel, note, velocity); 
  voicePool[0].noteOn(440 + note);
}

void myNoteOff(byte channel, byte note, byte velocity) {
  Serial.printf("Off: Channel %d, note %d, velocity %d\n", channel, note, velocity); 
  voicePool[0].noteOff();
}

void setup() {
  // Allocate memory for the audio engine
  AudioMemory(20);

  myusb.begin();

  // Initialize USB
  usbMIDI.setHandleNoteOn(myNoteOn);
  usbMIDI.setHandleNoteOff(myNoteOff);
  
  // Initialize the audio shield hardware
  //sgtl5000_1.enable();
  //sgtl5000_1.volume(0.5);

}

int i = 0;
void loop() {
  myusb.Task();

  if (++i % 1000000 == 0) 
  {
    Serial.print(".");
  }
}