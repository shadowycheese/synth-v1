#ifndef SYNTHCONFIGURATIONLISTENER_H
#define SYNTHCONFIGURATIONLISTENER_H

class SynthConfigurationListener {
public:
    SynthConfigurationListener() {};
    virtual ~SynthConfigurationListener() {};

    virtual void onSynthConfigurationChanged(
        bool waveFormChanged, 
        bool waveFormParamsChanged, 
        bool envelopeChanged, 
        bool volumeChanged) = 0;
};

#endif