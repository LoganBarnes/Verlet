#include "collisionsphere.h"
#include "geometriccollisionmanager.h"
#include "triangle.h"
#include <glm/common.hpp>

CollisionSphere::CollisionSphere(glm::vec3 pos, glm::vec3 dim, QString id)
    : CollisionShape(pos, dim, id)
{
}

CollisionSphere::~CollisionSphere()
{
}

Collision *CollisionSphere::collides(CollisionShape *cs)
{
    return cs->collidesSphere(this);
}

Collision *CollisionSphere::collidesSphere(CollisionSphere *) { return NULL; }
Collision *CollisionSphere::collidesCylinder(CollisionCylinder *) { return NULL; }
void CollisionSphere::collidesTriangle(Triangle, glm::vec3, TriCollision *) {}

