#ifndef SYNTHCONFIGURATIONLISTENER_H
#define SYNTHCONFIGURATIONLISTENER_H

#define ENVELOPE_CHANGED 0x0001
#define WAVEFORM_CHANGED 0x0002
#define VOLUME_CHANGED 0x0004
#define VOICE_CONFIGURAITON_CHANGED 0x0008

#define envelopeChanged(flags) (flags & ENVELOPE_CHANGED)
#define waveFormChanged(flags) (flags & WAVEFORM_CHANGED)
#define volumeChanged(flags) (flags & VOLUME_CHANGED)
#define voiceConfigurationChanged(flags) (flags & VOICE_CONFIGURAITON_CHANGED)

class SynthConfigurationListener
{
public:
    SynthConfigurationListener() {};
    virtual ~SynthConfigurationListener() {};

    virtual void onSynthConfigurationChanged(SynthConfiguration *configuration, int changeFlags);
};

#endif