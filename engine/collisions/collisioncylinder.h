#ifndef COLLISIONCYLINDER_H
#define COLLISIONCYLINDER_H

#include "collisionshape.h"
#include "entity.h"

class CollisionCylinder : public CollisionShape
{
public:
    CollisionCylinder(glm::vec3 pos, glm::vec3 dim, QString id);
    virtual ~CollisionCylinder();

    virtual Collision *collides(CollisionShape *cs);
    virtual Collision *collidesCylinder(CollisionCylinder *cc);
    virtual Collision *collidesSphere(CollisionSphere *cs);
    virtual void collidesTriangle(Triangle tri, glm::vec3 dest, TriCollision *col);
};

#endif // COLLISIONCYLINDER_H
