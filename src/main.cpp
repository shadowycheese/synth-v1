#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include "voice/VoiceController.h"
#include "USBHost_t36.h"
#include "Constants.h"
#include "io/SynthConfigurationOrchestrator.h"
#include "io/SynthConfigurationMapper.h"
#include "utils/ClipDetector.h"

AudioOutputI2S i2s1;
AudioControlSGTL5000 sgtl5000;
AudioSynthWaveform waveForm;

VoiceController voiceController;

SynthConfiguration synthConfiguration;
SynthConfigurationMapper configurationMapper(&synthConfiguration, &voiceController);

SynthConfigurationOrchestrator configurationOrchestrator(&configurationMapper);

USBHost myusb;
USBHub hub1(myusb);
MIDIDevice usbMidi1(myusb);

AudioConnection leftPatch(voiceController.getLeft(), 0, i2s1, 0);
AudioConnection rightPatch(voiceController.getRight(), 0, i2s1, 1);

ClipDetectorTask clipDetector;

void midiNoteOn(byte channel, byte note, byte velocity)
{
    voiceController.noteOn(note, velocity);
}

void midiNoteOff(byte channel, byte note, byte velocity)
{
    voiceController.noteOff(note, velocity);
}

void midiPitchChange(byte channel, int pitch)
{
    configurationOrchestrator.midiHandler()->midiPitch(pitch);
}

void midiControlChange(byte channel, byte control, byte value)
{
    configurationOrchestrator.midiHandler()->midiControl(control, value);
}

void setup()
{
    Serial.begin(115200);

    // Allocate memory for the audio engine
    AudioMemory(1200);

    sgtl5000.muteLineout();
    sgtl5000.lineOutLevel(31);
    sgtl5000.enable();

    sgtl5000.unmuteLineout();
    sgtl5000.volume(0.3);
    sgtl5000.unmuteHeadphone();

    for (int level = 31; level >= 21; level--)
    {
        sgtl5000.lineOutLevel(level);

        delay(25);
    }

    myusb.begin();

    usbMidi1.setHandleNoteOn(midiNoteOn);
    usbMidi1.setHandleNoteOff(midiNoteOff);
    usbMidi1.setHandlePitchChange(midiPitchChange);
    usbMidi1.setHandleControlChange(midiControlChange);
    usbMidi1.setHandleControlChange(midiControlChange);

    configurationOrchestrator.begin();
}

bool isKeyboardConnected()
{
    static bool connected = false;
    static bool disconnected = false;

    if (usbMidi1)
    {
        // This will print ONLY when the keyboard is physically recognized
        if (!connected)
        {
            Serial.println("Keyboard Connected!");
            connected = true;
        }

        return true;
    }
    else
    {
        if (!disconnected)
        {
            Serial.println("Keyboard NOT detected...");
            disconnected = true;
        }

        return false;
    }
}

inline void logAudioCPU()
{
    static int loops = 0;

    if ((loops++ % 3000000) == 0)
    {
        Serial.printf("CPU Usage: %02.02f%% (Max %02.02f%%) Memory Usage: %d (Max %d)\n",
                      AudioProcessorUsage(),
                      AudioProcessorUsageMax(),
                      AudioMemoryUsage(),
                      AudioMemoryUsageMax());
    }
}

void loop()
{
    uint32_t microSeconds = micros();

    configurationOrchestrator.task(microSeconds);

    logAudioCPU();

    myusb.Task();

    if (!isKeyboardConnected())
    {
        return;
    }

    voiceController.task(microSeconds);

    while (usbMidi1.read())
        ;
}