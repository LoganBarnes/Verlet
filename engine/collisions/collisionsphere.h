#ifndef COLLISIONSPHERE_H
#define COLLISIONSPHERE_H

#include "collisionshape.h"
#include "entity.h"

class CollisionSphere : public CollisionShape
{
public:
    CollisionSphere(glm::vec3 pos, glm::vec3 dim, QString id);
    virtual ~CollisionSphere();

    virtual Collision *collides(CollisionShape *cs);
    virtual Collision *collidesCylinder(CollisionCylinder *cc);
    virtual Collision *collidesSphere(CollisionSphere *cs);
    virtual void collidesTriangle(Triangle tri, glm::vec3 dest, TriCollision *col);
};

#endif // COLLISIONSPHERE_H
