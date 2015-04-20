#ifndef SOUNDTESTER_H
#define SOUNDTESTER_H

#include "movableentity.h"

class SoundTester : public MovableEntity
{
public:
    SoundTester(Audio *audio, glm::vec3 pos);
    virtual ~SoundTester();

    virtual void onTick(float secs);
    virtual void handleCollision(Collision *col);
};

#endif // SOUNDTESTER_H
