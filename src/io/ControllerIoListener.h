#ifndef CONTROLLERIOLISTENER_H
#define CONTROLLERIOLISTENER_H

#include "../Constants.h"

#define MUX0 0
#define MUX1 1

class ControllerIoListener
{
public:
    ControllerIoListener() {};
    virtual ~ControllerIoListener() {};

    virtual void onControllerIoChanged(int group, int input, int level);

    virtual void commit();
};

#endif