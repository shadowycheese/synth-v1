#ifndef CONTROLLERIO_LISTENER_H
#define CONTROLLERIO_LISTENER_H

#define INPUT_GROUP_MIDI 0
#define INPUT_GROUP_MUX1 1
#define INPUT_GROUP_MUX2 2
#define INPUT_GROUP_MUX3 3

class ControllerIoListener
{
public:
    ControllerIoListener() {};
    virtual ~ControllerIoListener() {};

    virtual void onControllerIoChanged(int group, int input, int level);

    virtual void commit();
};

#endif