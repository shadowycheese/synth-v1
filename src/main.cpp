#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include "USBHost_t36.h"
#include "voice/VoiceController.h"
#include "io/InputController.h"
#include "io/SynthConfigurationMapper.h"
#include "store/WaveformStore.h"
#include "store/PresetStore.h"

#define LOG_AUDIO_LIB_STATS 0
#define MAX_VOLUME 19

AudioOutputI2S i2s1;
AudioControlSGTL5000 sgtl5000;

Indicators indicators;

WaveformStore waveformStore(&indicators);

VoiceController voiceController(&indicators, &waveformStore);

PresetStore presetStore(&voiceController);

SynthConfiguration synthConfiguration;
SynthConfigurationMapper configurationMapper(&synthConfiguration, &presetStore, &indicators);

InputController inputController(&configurationMapper);

USBHost myusb;
USBHub hub1(myusb);
MIDIDevice usbMidi1(myusb);

AudioConnection leftPatch(voiceController.getLeft(), 0, i2s1, 0);
AudioConnection rightPatch(voiceController.getRight(), 0, i2s1, 1);

void midiNoteOn(byte channel, byte note, byte velocity)
{
    presetStore.checkStorePreset(note, true);

    voiceController.noteOn(note, velocity);
}

void midiNoteOff(byte channel, byte note, byte velocity)
{
    presetStore.checkStorePreset(note, false);

    voiceController.noteOff(note, velocity);
}

void midiPitchChange(byte channel, int pitch)
{
    inputController.midiHandler()->midiPitch(pitch);
}

void midiControlChange(byte channel, byte control, byte value)
{
    inputController.midiHandler()->midiControl(control, value);
}

void midiHandleSystemExclusive(byte *array, unsigned int size)
{
    waveformStore.midiHandleSystemExclusive(array, size);
}

void setup()
{
    Serial.begin(115200);

    AudioMemory(1200);

    indicators.voices(8);

    sgtl5000.lineOutLevel(31);
    sgtl5000.enable();
    sgtl5000.unmuteLineout();
    sgtl5000.volume(0.3);
    sgtl5000.unmuteHeadphone();

    presetStore.init();
    waveformStore.init();

    uint8_t flash = 0;

    indicators.voices(flash);

    for (int level = 31; level >= 23; level--)
    {
        sgtl5000.lineOutLevel(level);

        delay(25);

        flash = flash == 0 ? 8 : 0;

        indicators.voices(flash);
    }

    myusb.begin();

    // Allow MIDI input on both USB's.
    usbMidi1.setHandleNoteOn(midiNoteOn);
    usbMidi1.setHandleNoteOff(midiNoteOff);
    usbMidi1.setHandlePitchChange(midiPitchChange);
    usbMidi1.setHandleControlChange(midiControlChange);
    usbMidi1.setHandleSystemExclusive(midiHandleSystemExclusive);

    usbMIDI.setHandleNoteOn(midiNoteOn);
    usbMIDI.setHandleNoteOff(midiNoteOff);
    usbMIDI.setHandlePitchChange(midiPitchChange);
    usbMIDI.setHandleControlChange(midiControlChange);
    usbMIDI.setHandleSystemExclusive(midiHandleSystemExclusive);

    inputController.begin();

    indicators.keyboardConnected(0, false);
}

inline void readUsb()
{
    while (usbMidi1.read())
        ;
    while (usbMIDI.read())
        ;
}

bool isKeyboardConnected()
{
    static bool connected = false;
    static bool disconnected = false;

    if (usbMidi1)
    {
        if (!connected)
        {
            Serial.println("Keyboard Connected!");

            indicators.keyboardConnected(0, true);

            connected = true;
            disconnected = false;
        }

        return true;
    }
    else
    {
        if (!disconnected)
        {
            Serial.println("Keyboard NOT detected...");

            indicators.keyboardConnected(0, false);

            connected = false;
            disconnected = true;
        }

        readUsb();

        return false;
    }
}

#if LOG_AUDIO_LIB_STATS
inline void logAudioCPU()
{
    static int loops = 0;

    if ((loops++ % 1000000) == 0)
    {
        Serial.printf("CPU Usage: %02.02f%% (Max %02.02f%%) Memory Usage: %d% (Max %d)\n",
                      AudioProcessorUsage(),
                      AudioProcessorUsageMax(),
                      AudioMemoryUsage(),
                      AudioMemoryUsageMax());
    }
}
#endif

void loop()
{
    uint32_t microSeconds = micros();

    inputController.task(microSeconds);

#if LOG_AUDIO_LIB_STATS
    logAudioCPU();
#endif

    myusb.Task();
    indicators.task(microSeconds);

    if (!isKeyboardConnected())
    {
        return;
    }

    voiceController.task(microSeconds);

    readUsb();
}