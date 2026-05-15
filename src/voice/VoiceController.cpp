#include "VoiceController.h"

VoiceController::VoiceController() : voiceUpdates("Voice Updates"),
                                     filterUpdates("Filter Updates")
{
    for (int i = 0; i < 4; i++)
    {
        mixer1.gain(i, 0.35f);
        mixer2.gain(i, 0.35f);
    }

    masterMix.gain(0, 0.70f);
    masterMix.gain(1, 0.70f);

    left.gain(0, 1.0f);
    right.gain(0, 1.0f);

    nextVoiceUpdateTime = millis();
}

void VoiceController::onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags)
{
    voiceConfiguration.copy(configuration);

    if (volumeChanged(changeFlags))
    {
        leftAmp.gain(voiceConfiguration.ampGain * 5.0f);
        rightAmp.gain(voiceConfiguration.ampGain * 5.0f);
    }

    if (effectChanged(changeFlags))
    {
        if (voiceConfiguration.reverbEnabled)
        {
            left.gain(0, 0.8f);
            left.gain(1, 0.5f);
            right.gain(0, 0.8f);
            right.gain(1, 0.5f);

            reverb.reverbTime(voiceConfiguration.reverb);
        }
        else
        {
            left.gain(0, 1.0f);
            left.gain(1, 0.0f);
            right.gain(0, 0.0f);
            right.gain(1, 0.0f);
        }
    }

    voiceConfigurationVersion++;

    for (int i = 0; i < MAX_VOICES; i++)
    {
        pendingChanges[i] |= changeFlags;
    }
}

void VoiceController::noteOn(byte note, byte velocity)
{
    if (velocity == 0)
    {
        noteOff(note, velocity);

        return;
    }

    int voice = findOldestVoice(note);

    if (voice >= 0)
    {
        notesVoiceMap[note] = voice;

        float amplitude = pow(10.0f, (velocity - 127.0f) / 63.5f);

        voicePool[voice].noteOn(note, midiNoteHz(note), amplitude);
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

    for (int i = 0; i < MAX_VOICES; i++)
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

int every = 0;
void VoiceController::updateVoiceFilters(uint32_t microSeconds)
{
    if (peak1.available())
    {
        peak1f = max(peak1f, peak1.read());
    }
    if (peak2.available())
    {
        peak2f = max(peak2f, peak2.read());
    }
    if (peak3.available())
    {
        peak3f = max(peak3f, peak3.read());
    }
    if (peak4.available())
    {
        peak4f = max(peak4f, peak4.read());
    }
    if (peak5.available())
    {
        peak5f = max(peak5f, peak5.read());
    }
    // Handle millis wrapping
    if (microSeconds >= nextFilterUpdateTime)
    {
        int voice = nextFilterToUpdate++;

        nextFilterToUpdate &= 7;

        nextFilterUpdateTime = microSeconds + 200;

        voicePool[voice].updateFilter();

        if (nextFilterToUpdate == 0)
        {
            filterUpdates.inc(microSeconds);
        }
        if (every++ == 1000)
        {
            Serial.printf("T: %0.2f %0.2f %0.2f %0.2f %0.2f", peak1f, peak2f, peak3f, peak4f, peak5f);

            for (int i = 0; i < 7; i++)
            {
                if (voicePool[i].isPlaying())
                {
                    Serial.printf(" %d:", i);
                    voicePool[i].task(true);
                }
            }

            peak1f = peak2f = peak3f = peak4f = peak5f = 0;

            every = 0;
            Serial.println();
        }
    }
    else
    {
        for (int i = 0; i < 7; i++)
        {
            voicePool[i].task(false);
        }
    }
}

void VoiceController::updateVoices(uint32_t microSeconds)
{
    // Handle millis wrapping
    if ((nextVoiceUpdateTime - microSeconds) > 1000000)
    {
        nextVoiceUpdateTime = microSeconds + 2;
        return;
    }

    if (microSeconds < nextVoiceUpdateTime)
    {
        return;
    }

    int voice = nextVoiceToUpdate++;

    nextVoiceToUpdate &= 7;

    if (nextVoiceToUpdate == 0)
    {
        voiceUpdates.inc(microSeconds);
    }

    nextVoiceUpdateTime = microSeconds + 1500;

    if (voiceVersions[voice] != voiceConfigurationVersion)
    {
        voicePool[voice].onSynthConfigurationChanged(&voiceConfiguration, pendingChanges[voice]);

        voiceVersions[voice] = voiceConfigurationVersion;

        pendingChanges[voice] = 0;
    }
}

void VoiceController::updateIndicators(uint32_t microSeconds)
{
    if (peak.available())
    {
        _lastPeak = peak.read();

        if (_lastPeak >= 1.0f)
        {
            _wasOverdrive = true;
        }
    }

    if (microSeconds > nextIndicatorUpdateTime)
    {
        indicators.level(_lastPeak);

        if (_wasOverdrive)
        {
            indicators.overdrive(microSeconds);

            _wasOverdrive = false;
        }

        uint8_t voiceCount = 0;

        for (int i = 0; i < MAX_VOICES; i++)
        {
            if (voicePool[i].isPlaying())
            {
                voiceCount++;
            }
        }

        indicators.voices(voiceCount);

        nextIndicatorUpdateTime = microSeconds + 50000;
    }

    indicators.task(microSeconds);
}

void VoiceController::task(uint32_t microSeconds)
{
    updateVoiceFilters(microSeconds);
    updateVoices(microSeconds);
    updateIndicators(microSeconds);
}
