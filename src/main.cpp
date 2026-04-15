#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include "voice/VoiceController.h"
#include "USBHost_t36.h"
#include "Constants.h"
#include "io/ControllerIo.h"
#include "io/SynthConfigurationMapper.h"

AudioOutputI2S i2s1;
AudioControlSGTL5000 sgtl5000;
AudioSynthWaveform waveForm;

VoiceController voiceController;

SynthConfiguration synthConfiguration;
SynthConfigurationMapper configurationMapper(&synthConfiguration, &voiceController);

ControllerIo controllerIo(&configurationMapper);

USBHost myusb;
USBHub hunb(myusb);
MIDIDevice midi1(myusb);

AudioConnection mainPatch(voiceController.getOutput(), 0, i2s1, 0);

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

    sgtl5000.muteLineout();
    sgtl5000.volume(0.8);
    sgtl5000.lineOutLevel(31);
    sgtl5000.enable();

    delay(600);

    sgtl5000.unmuteLineout();

    for (int level = 31; level >= 21; level--)
    {
        sgtl5000.lineOutLevel(level);

        delay(25);
    }

    myusb.begin();

    usbMIDI.setHandleNoteOn(myNoteOn);
    usbMIDI.setHandleNoteOff(myNoteOff);

    controllerIo.begin();
}

int loops = 0;
void loop()
{
    if ((loops++ % 500000) == 0)
    {
        Serial.printf("CPU Usage: %02.02f%% (Max %02.02f%%)\n", AudioProcessorUsage(), AudioProcessorUsageMax());
    }
    // myusb.Task();
    usbMIDI.read();
    controllerIo.task();
    voiceController.task();
}