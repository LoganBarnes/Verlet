#ifndef ELLIPSOID_H
#define ELLIPSOID_H

#include <glm.hpp>
#include "collisionshape.h"
#include "geometriccollisionmanager.h"

class Ellipsoid: public CollisionShape
{
public:
    Ellipsoid(glm::vec3 center, glm::vec3 radius, QString id);
    virtual ~Ellipsoid();

    virtual Collision *collides(CollisionShape *shape);
    virtual Collision *collidesCylinder(CollisionCylinder *cc);
    virtual Collision *collidesSphere(CollisionSphere *cs);
    virtual void collidesTriangle(Triangle tri, glm::vec3 d, TriCollision *col);

    void setPosition(glm::vec3 pos);

    float intersectRayWorldSpace(glm::vec3 p, glm::vec3 d);
    float intersectRay(glm::vec3 p, glm::vec3 d);

    void handleCollision(TriCollision col);

    inline bool EQ(float a, float b) { return fabs(a - b) < 0.00001f; }

private:
    int findT(glm::vec3 p, glm::vec3 d, float *t1, float *t2);
};

#endif // ELLIPSOID_H
