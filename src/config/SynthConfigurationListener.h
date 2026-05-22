#ifndef SYNTH_CONFIGURATION_LISTENER_H
#define SYNTH_CONFIGURATION_LISTENER_H

class SynthConfigurationListener
{
public:
    SynthConfigurationListener() {};
    virtual ~SynthConfigurationListener() {};

    virtual void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags);
};

#endif