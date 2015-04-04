#include "staticentity.h"
#include "movableentity.h"
#include "collisionshape.h"

StaticEntity::StaticEntity(glm::vec3 pos)
    : Entity(pos)
{
}

StaticEntity::~StaticEntity()
{
}


void StaticEntity::handleCollision(Collision *col)
{
    if (!col->c2->isReactable())
        return;
    MovableEntity *other = dynamic_cast<MovableEntity* >(col->e2);
    other->bump(col->mtv * -.5f);
    glm::vec3 momentum = other->getVelocity() * col->c2->getMass();
    float mag2 = glm::dot(col->mtv, col->mtv);
    glm::vec3 imp;
    if (mag2 < 0.001f)
        imp = glm::vec3(0.f);
    else
        imp = (glm::dot(momentum, col->mtv) / mag2) * col->mtv;
    other->applyImpulse(-col->impulse);
}

