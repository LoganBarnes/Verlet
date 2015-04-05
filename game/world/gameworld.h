#ifndef GAMEWORLD_H
#define GAMEWORLD_H

#include "world.h"

class GameWorld : public World
{
public:
    GameWorld();
    virtual ~GameWorld();

    Triangle *intersectWorld(glm::vec3 p, glm::vec3 d, float *t);

};

#endif // GAMEWORLD_H
