#include "movableentity.h"
#include "collisionshape.h"

//#include <iostream>
//using namespace std;
//#include <glm/ext.hpp>

#include <iostream>
using namespace std;

MovableEntity::MovableEntity(glm::vec3 pos)
    : Entity(pos),
      m_updatePosOnTick(true),
      m_tempSolid(false)
{
    m_mass = 0.001f;
    m_vel = glm::vec3();

    m_force = glm::vec3();
    m_impulse = glm::vec3();

    m_destination = pos;
}

MovableEntity::~MovableEntity() {}

float MovableEntity::getMass()
{
    return m_mass;
}

void MovableEntity::addCollisionShape(CollisionShape *cs)
{
    Entity::addCollisionShape(cs);
    m_mass += cs->getMass();
}

glm::vec3 MovableEntity::getVelocity()
{
    return m_vel;
}

void MovableEntity::setMass(float mass)
{
    m_mass = mass;
}

void MovableEntity::setVelocity(glm::vec3 vel)
{
    m_vel = vel;
}

void MovableEntity::onTick(float secs)
{
    m_tempSolid = false;

    m_vel += (m_force * secs / m_mass) + m_impulse * 1.f / m_mass;

    if (m_updatePosOnTick)
        setPosition(getPosition() + m_vel * secs);
    m_destination = getPosition() + m_vel * secs;

    m_force = glm::vec3(0.f);
    m_impulse = glm::vec3(0.f);

    Entity::onTick(secs);
//    if (m_audio && m_soundID > -1)
//        m_audio->setSource(m_soundID, m_soundFile, getPosition(), getVelocity(), m_loopAudio);
}

void MovableEntity::handleCollision(Collision *col)
{
    if (!col->c1->isReactable() || !col->c2->isReactable())
        return;
    bump(col->mtv * .5f);
    applyImpulse(col->impulse);
}

void MovableEntity::applyImpulse(glm::vec3 impulse)
{
    m_impulse += impulse;
}

void MovableEntity::applyForce(glm::vec3 force)
{
    m_force += force;
}


glm::vec3 MovableEntity::getDestination()
{
    return m_destination;
}

void MovableEntity::setDestination(glm::vec3 destination)
{
    m_destination = destination;
}

