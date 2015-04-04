#ifndef STATICENTITY_H
#define STATICENTITY_H

#include "entity.h"

class StaticEntity : public Entity
{
public:
    StaticEntity(glm::vec3 pos = glm::vec3(0.f));
    virtual ~StaticEntity();

    virtual void handleCollision(Collision *col);
};

#endif // STATICENTITY_H
