#ifndef COLLISIONSHAPE_H
#define COLLISIONSHAPE_H

#include "entity.h"
#include <QString>
#include <QList>

class CollisionCylinder;
class CollisionSphere;
class Triangle;
struct TriCollision;

class CollisionShape
{
public:
    CollisionShape(glm::vec3 pos, glm::vec3 dim, QString id)
    {
        m_posRel = pos;
        m_dim = dim;
        m_id = id;
        m_mass = 1.0;
        m_reactable = true;
    }
    virtual ~CollisionShape() {}

    void updatePos(glm::vec3 ePos) { m_pos = ePos + m_posRel; }
    void setMass(float mass) { m_mass = mass; }
    void setReactable(bool reactable) { m_reactable = reactable; }

    glm::vec3 getPos() { return m_pos; }
    glm::vec3 getDim() { return m_dim; }
    QString getID() { return m_id; }
    float getMass() { return m_mass; }
    bool isReactable() { return m_reactable; }

    virtual Collision *collides(CollisionShape *shape) = 0;
    virtual Collision *collidesCylinder(CollisionCylinder *cc) = 0;
    virtual Collision *collidesSphere(CollisionSphere *cs) = 0;
    virtual void collidesTriangle(Triangle tri, glm::vec3 d, TriCollision *col) = 0;

protected:
    glm::vec3 m_posRel, m_pos, m_dim;
    QString m_id;
    float m_mass;
    bool m_reactable;
    Entity *e;

};

#endif // COLLISIONSHAPE_H
