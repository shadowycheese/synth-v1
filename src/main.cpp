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
USBHub hub1(myusb);
MIDIDevice usbMidi1(myusb);

AudioConnection mainPatch(voiceController.getOutput(), 0, i2s1, 0);

void midiNoteOn(byte channel, byte note, byte velocity)
{
    Serial.printf("On: Channel %d, note %d, velocity %d\n", channel, note, velocity);

    voiceController.noteOn(note, velocity);
}

void midiNoteOff(byte channel, byte note, byte velocity)
{
    Serial.printf("Off: Channel %d, note %d, velocity %d\n", channel, note, velocity);

    voiceController.noteOff(note, velocity);
}

void midiPitchChange(byte channel, int pitch)
{
    Serial.printf("Pitch: Channel %d, pitch %d\n", channel, pitch);

    controllerIo.midiControl(MIDI_INPUT_PITCH, pitch);
}

void midiControlChange(byte channel, byte control, byte value)
{
    Serial.printf("Control: Channel %d, control %d, value %d\n", channel, control, value);

    if (control == 7)
    {
        controllerIo.midiControl(MIDI_INPUT_RESONANCE, 8 * (int)value);
    }
    else if (control == 74)
    {
        controllerIo.midiControl(MIDI_INPUT_DETUNE, 8 * (int)value);
    }
    else if (control == 71)
    {
        controllerIo.midiControl(MIDI_INPUT_PW, 8 * (int)value);
    }
    else if (control == 72)
    {
        controllerIo.midiControl(MIDI_INPUT_WAVEFORM, 8 * (int)value);
    }
}

void setup()
{
    Serial.begin(115200);

    // Allocate memory for the audio engine
    AudioMemory(200);

    sgtl5000.muteLineout();
    sgtl5000.volume(0.8);
    sgtl5000.lineOutLevel(31);
    sgtl5000.enable();

    sgtl5000.unmuteLineout();

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

    controllerIo.begin();
}

void loop()
{
    static int loops = 0;
    static bool connected = false;
    static bool disconnected = false;

    if ((loops++ % 3000000) == 0)
    {
        Serial.printf("CPU Usage: %02.02f%% (Max %02.02f%%)\n", AudioProcessorUsage(), AudioProcessorUsageMax());
    }

    myusb.Task();

    if (usbMidi1)
    {
        // This will print ONLY when the keyboard is physically recognized
        if (!connected)
        {
            Serial.println("Keyboard Connected!");
            connected = true;
        }
    }
    else
    {
        if (!disconnected)
        {
            Serial.println("Keyboard NOT detected. Check cable/hub.");
            disconnected = true;
        }

        return;
    }

    while (usbMidi1.read())
        ;

    controllerIo.task();
    voiceController.task();
}