#include "entity.h"
#include "collisionshape.h"

#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>

Entity::Entity(glm::vec3 pos)
{
    m_pos = pos;
    m_rotation = glm::mat4();

    m_renderShapes.clear();
    m_transparentShapes.clear();
    m_collisionShapes.clear();
}

Entity::~Entity()
{
    foreach(RenderShape *rs, m_renderShapes)
        delete rs;
    foreach(RenderShape *rs, m_transparentShapes)
        delete rs;
    foreach(CollisionShape *cs, m_collisionShapes)
        delete cs;
}

void Entity::setPosition(glm::vec3 pos)
{
    m_pos = pos;
    foreach(CollisionShape *cs, m_collisionShapes)
        cs->updatePos(m_pos);
}

glm::vec3 Entity::getPosition()
{
    return m_pos;
}

glm::vec3 Entity::getVelocity()
{
    return glm::vec3();
}

float Entity::getMass()
{
    return 0.f;
}

QList<CollisionShape *> Entity::getCollisionShapes()
{
    return m_collisionShapes;
}

QList<RenderShape *> Entity::getRenderShapes()
{
    return m_renderShapes;
}

void Entity::addRenderShape(RenderShape *rs)
{
    if (rs->transparency < 1.f)
        m_transparentShapes.append(rs);
    else
        m_renderShapes.append(rs);
}

void Entity::addCollisionShape(CollisionShape *cs)
{
    cs->updatePos(getPosition());
    m_collisionShapes.append(cs);
}

void Entity::onTick(float)
{
}

glm::mat4 Entity::getRotation()
{
    return m_rotation;
}

void Entity::onDrawOpaque(Graphics *g)
{
    glm::mat4 posMat = glm::translate(glm::mat4(), m_pos) * m_rotation;

    drawFromList(g, m_renderShapes, posMat);
}

void Entity::onDrawTransparent(Graphics *g)
{
    glm::mat4 posMat = glm::translate(glm::mat4(), m_pos);

    if (m_transparentShapes.size() > 0)
    {
        g->setTransparentMode(true);
        drawFromList(g, m_transparentShapes, posMat);
        g->setTransparentMode(false);
    }
}

void Entity::drawFromList(Graphics *g, QList<RenderShape *> shapes, glm::mat4 posMat)
{
    foreach(RenderShape *rs, shapes)
    {
        g->setTexture(rs->texture, rs->repeatU, rs->repeatV);
        g->setColor(rs->color.r, rs->color.g, rs->color.b, rs->transparency, rs->shininess);
        glm::mat4 trans = posMat * rs->trans;

        switch (rs->type)
        {
        case LINE:
            g->drawLine(trans);
            break;
        case QUAD:
            g->drawQuad(trans);
            break;
        case CONE:
            g->drawCone(trans);
            break;
        case CUBE:
            g->drawCube(trans);
            break;
        case CYLINDER:
            g->drawCyl(trans);
            break;
        case SPHERE:
            g->drawSphere(trans);
            break;
        }
    }
}

void Entity::bump(glm::vec3 amount)
{
    setPosition(getPosition() + amount);
}

QList<Collision *> Entity::collides(Entity *e, float)
{
    QList<CollisionShape *> cshapes = e->getCollisionShapes();
    QList<Collision *> cols;
    Collision *col;

    foreach(CollisionShape *cs1, m_collisionShapes)
    {
        foreach(CollisionShape *cs2, cshapes)
        {
            col = cs1->collides(cs2);
            if (col)
            {
               col->e1 = this;
               col->e2 = e;
               float m1 = cs1->getMass();
               float m2 = cs2->getMass();
               glm::vec3 v1 = this->getVelocity();
               glm::vec3 v2 = e->getVelocity();
               float mag2 = glm::dot(col->mtv, col->mtv);
               if (mag2 < 0.0001)
                   col->impulse = glm::vec3(0.f);
               else
               {
                   col->impulse = (v2 - v1) * ((m1 * m2 /**(1+cor)*/) / (m1 + m2));
                   col->impulse = (glm::dot(col->impulse, col->mtv) / mag2) * col->mtv;
               }
               cols.append(col);
            }
        }
    }

    return cols;
}

