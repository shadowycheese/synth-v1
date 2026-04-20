#ifndef SYNTHCONFIGURATIONLISTENER_H
#define SYNTHCONFIGURATIONLISTENER_H

class SynthConfigurationListener
{
public:
    SynthConfigurationListener() {};
    virtual ~SynthConfigurationListener() {};

    virtual void onSynthConfigurationChanged(SynthConfiguration *configuration, uint16_t changeFlags);
};

#endif