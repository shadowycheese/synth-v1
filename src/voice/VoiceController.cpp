#include "VoiceController.h"

VoiceController::VoiceController(Indicators *inidicators) : voiceUpdates("Voice Updates"),
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

    _indicators = inidicators;

    nextVoiceUpdateTime = millis();
}

void VoiceController::onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags)
{
    _voiceConfiguration.copy(configuration);

    if (volumeChanged(changeFlags))
    {
        leftAmp.gain(_voiceConfiguration.ampGain * 5.0f);
        rightAmp.gain(_voiceConfiguration.ampGain * 5.0f);
    }

    if (effectChanged(changeFlags))
    {
        if (_voiceConfiguration.reverb > 0.0f)
        {
            left.gain(0, 0.8f);
            left.gain(1, 0.5f);
            right.gain(0, 0.8f);
            right.gain(1, 0.5f);

            reverb.reverbTime(_voiceConfiguration.reverb);
        }
        else
        {
            left.gain(0, 1.0f);
            left.gain(1, 0.0f);
            right.gain(0, 0.0f);
            right.gain(1, 0.0f);

            reverb.reverbTime(0.0f);
        }
    }

    _voiceConfigurationVersion++;

    for (int i = 0; i < MAX_VOICES; i++)
    {
        _pendingChanges[i] |= changeFlags;
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
        _notesVoiceMap[note] = voice;

        float amplitude = pow(10.0f, (velocity - 127.0f) / 63.5f);

        voicePool[voice].noteOn(note, midiNoteHz(note), amplitude);
    }
}

void VoiceController::noteOff(byte note, byte velocity)
{
    voicePool[_notesVoiceMap[note]].noteOff();
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

void VoiceController::updateVoiceFilters(uint32_t microSeconds)
{
    // Handle millis wrapping
    if (microSeconds >= _nextFilterUpdateTime)
    {
        int voice = _nextFilterToUpdate++;

        _nextFilterToUpdate &= 7;

        _nextFilterUpdateTime = microSeconds + 200;

        voicePool[voice].updateFilter();

        if (_nextFilterToUpdate == 0)
        {
            filterUpdates.inc(microSeconds);
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

    int voice = _nextVoiceToUpdate++;

    _nextVoiceToUpdate &= 7;

    if (_nextVoiceToUpdate == 0)
    {
        voiceUpdates.inc(microSeconds);
    }

    nextVoiceUpdateTime = microSeconds + 1500;

    if (_voiceVersions[voice] != _voiceConfigurationVersion)
    {
        voicePool[voice].onSynthConfigurationChanged(&_voiceConfiguration, _pendingChanges[voice]);

        _voiceVersions[voice] = _voiceConfigurationVersion;

        _pendingChanges[voice] = 0;
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

    if (microSeconds > _nextIndicatorUpdateTime)
    {
        _indicators->level(_lastPeak);

        if (_wasOverdrive)
        {
            _indicators->overdrive(microSeconds);

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

        _indicators->voices(voiceCount);

        _nextIndicatorUpdateTime = microSeconds + 50000;
    }
}

void VoiceController::task(uint32_t microSeconds)
{
    updateVoiceFilters(microSeconds);
    updateVoices(microSeconds);
    updateIndicators(microSeconds);
}
