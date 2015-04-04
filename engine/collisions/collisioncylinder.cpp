#include "collisioncylinder.h"
#include "geometriccollisionmanager.h"
#include "triangle.h"
#include <glm/common.hpp>

CollisionCylinder::CollisionCylinder(glm::vec3 pos, glm::vec3 dim, QString id)
    : CollisionShape(pos, dim, id)
{
}

CollisionCylinder::~CollisionCylinder()
{
}

Collision *CollisionCylinder::collides(CollisionShape *cs)
{
    return cs->collidesCylinder(this);
}

Collision *CollisionCylinder::collidesCylinder(CollisionCylinder *cc)
{
    glm::vec3 pos = cc->getPos();
    glm::vec3 dim = cc->getDim();

    glm::vec4 mtv = glm::vec4(0.f, 0.f, 0.f, std::numeric_limits<float>::infinity());

    // check vertical
    float otherBelow = (pos.y + dim.y / 2.f) - (m_pos.y - m_dim.y / 2.f);
    float otherAbove = (m_pos.y + m_dim.y / 2.f) - (pos.y - dim.y / 2.f);

    if (otherBelow < 0.f || otherAbove < 0.f)
        return NULL;
    else if (otherBelow < otherAbove)
    {
        mtv.w = otherBelow;
        mtv.y = -otherBelow;
    }
    else
    {
        mtv.w = otherAbove;
        mtv.y = otherAbove;
    }

    // check xz plane
    glm::vec3 one2Two = glm::vec3(pos.x, 0.f, pos.z) - glm::vec3(m_pos.x, 0.f, m_pos.z);
    float dist2 = glm::dot(one2Two, one2Two);
    glm::vec2 vec = glm::abs(glm::vec2(one2Two));
    float angle = glm::atan(vec.y, vec.x);

    float sine = glm::sin(angle);
    float cosine = glm::cos(angle);

    glm::vec2 dim1 = glm::vec2(dim.x, dim.z) * .5f;
    glm::vec2 dim2 = glm::vec2(m_dim.x, m_dim.z) * .5f;

    float r1 = (dim1.x * dim1.y) / glm::sqrt(dim1.x*dim1.x*sine*sine + dim1.y*dim1.y*cosine*cosine);
    float r2 = (dim2.x * dim2.y) / glm::sqrt(dim2.x*dim2.x*sine*sine + dim2.y*dim2.y*cosine*cosine);

    if (dist2 < (r1+r2)*(r1+r2))
    {
        float dist = (r1+r2) - glm::sqrt(dist2);
        if (dist2 > 0.001f && dist < mtv.w)
            mtv = glm::vec4(glm::normalize(one2Two) * dist, dist);
    }
    else
        return NULL;

    Collision *col;
    col = new Collision();
    col->c1 = cc;
    col->c2 = this;
    col->mtv = glm::vec3(mtv);

    return col;
}


Collision *CollisionCylinder::collidesSphere(CollisionSphere *) { return NULL; }
void CollisionCylinder::collidesTriangle(Triangle, glm::vec3, TriCollision *) {}

