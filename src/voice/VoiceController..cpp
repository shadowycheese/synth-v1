#include "VoiceController.h"

VoiceController::VoiceController() : 
    patch0(voicePool[0].getOutput(), 0, mixer1, 0),
    patch1(voicePool[1].getOutput(), 0, mixer1, 1),
    patch2(voicePool[2].getOutput(), 0, mixer1, 2),
    patch3(voicePool[3].getOutput(), 0, mixer1, 3),

    patch4(voicePool[4].getOutput(), 0, mixer2, 0),
    patch5(voicePool[5].getOutput(), 0, mixer2, 1),
    patch6(voicePool[6].getOutput(), 0, mixer2, 2),
    patch7(voicePool[7].getOutput(), 0, mixer2, 3),

    patchM1(mixer1, 0, masterMix, 0),
    patchM2(mixer2, 0, masterMix, 1)
{
    for (int i = 0; i < 4; i++) {
        mixer1.gain(i, 1.0f);
    }

    for (int i = 0; i < 4; i++) {
        mixer2.gain(i, 1.0f);
    }

    masterMix.gain(0, 1.0f); 
    masterMix.gain(1, 1.0f); 
}

void VoiceController::setWaveSetters(WaveSetter** setters) 
{
    Serial.println("setWaveSetters");
    waveSetters = setters;
    currentSetterIndex = 0;

    for(int i = 0; i < MAX_VOICES; i++) {
        voicePool[i].setWave(waveSetters[currentSetterIndex]);
    }
}

void VoiceController::noteOn(byte note, byte velocity) 
{
    int voice = findOldestVoice(note);

    if (voice >= 0) 
    {
        notesVoiceMap[note] = voice;

        voicePool[voice].noteOn(note, MIDI_FREQ(note), ((float)velocity) / 128);
    }
}

void VoiceController::noteOff(byte note, byte velocity) 
{
    voicePool[notesVoiceMap[note]].noteOff();
}

int VoiceController::findOldestVoice(byte note) 
{
    byte oldest = -1;
    uint32_t oldestTimestamp = -1;

    for(int i = 0; i < MAX_VOICES; i++) 
    {
        if (!voicePool[i].isPlaying()) 
        {
            return i;
        }

        if (voicePool[i].noteLastPlayed() == note) 
        {
            return i;
        }

        uint32_t timestamp = voicePool[i].timestamp();

        if (oldest < 0) 
        {
            oldest = i;
            oldestTimestamp = timestamp;
        } 
        else if (timestamp < oldestTimestamp) 
        {
            oldest = i;
            oldestTimestamp = timestamp;
        }
    }

    return oldest;
}
