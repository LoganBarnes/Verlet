#ifndef MOVABLEENTITY_H
#define MOVABLEENTITY_H

#include "entity.h"

class MovableEntity : public Entity
{
public:
    MovableEntity(glm::vec3 pos = glm::vec3(0.f));
    virtual ~MovableEntity();

    virtual void addCollisionShape(CollisionShape *cs);

    virtual void onTick(float secs);

    virtual void handleCollision(Collision *col, bool resetVel = false);

    virtual float getMass();
    virtual glm::vec3 getVelocity();

    void setMass(float mass);
    void setVelocity(glm::vec3 vel);
    void applyImpulse(glm::vec3 impulse);
    void applyForce(glm::vec3 force);

    glm::vec3 getDestination();
    void setDestination(glm::vec3 destination);

    void setTempSolid() { m_tempSolid = true; }

    void setUpdatePositionOnTick(bool updatePos) { m_updatePosOnTick = updatePos; }

protected:
    float m_mass;
    glm::vec3 m_vel, m_force, m_impulse;
    glm::vec3 m_destination;

    bool m_updatePosOnTick;

    bool m_tempSolid; // temporarily set to true if the object is colliding with a geometric object
};

#endif // MOVABLEENTITY_H
