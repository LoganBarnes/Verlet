#include "geometriccollisionmanager.h"
#include "ellipsoid.h"
#include "triangle.h"
#include "movableentity.h"
#include "world.h"

#define GLM_FORCE_RADIANS
#include <glm/gtx/norm.hpp>

//#include <iostream>
//using namespace std;
//#include <glm/ext.hpp>

GeometricCollisionManager::GeometricCollisionManager()
{
}


GeometricCollisionManager::~GeometricCollisionManager()
{
}


void GeometricCollisionManager::manage(World *world, float)
{
    int iterations = 4;

    QList<MovableEntity*> mes;
    QList<Triangle*> triangles;

    mes.append(world->getMovableEntities());
    triangles = world->getMesh();



    for (int i = 0; i < iterations; i++)
    {
        QList<Collision *> cols = detectEllipsoidCollisions(mes);
        handleEllipsoidCollisions(cols);

        QList<TriCollision *> colsT = detectTriangleCollisions(mes, triangles);
        handleTriangleCollisions(colsT);

        // delete collisions
        foreach(Collision *c, cols)
            delete c;
        foreach (TriCollision *ct, colsT)
            delete ct;
    }
}


QList<Collision *> GeometricCollisionManager::detectEllipsoidCollisions(QList<MovableEntity *> mes)
{
    QList<Collision *> cols;

    int size = mes.size();
    for (int i = 0; i < size-1; i++)
    {
        for (int j = i+1; j < size; j++)
        {

        }
    }

    return cols;
}


void GeometricCollisionManager::handleEllipsoidCollisions(QList<Collision *> )
{

}


QList<TriCollision* > GeometricCollisionManager::detectTriangleCollisions(
        QList<MovableEntity *> mes, QList<Triangle *> tris)
{
    float origT;
    QList<TriCollision *> cols;

    foreach (MovableEntity *me, mes)
    {
        TriCollision col, best;
        best.me = me;
        QList<CollisionShape*> shapes = me->getCollisionShapes();

        foreach (CollisionShape *cs, shapes)
        {
            glm::vec3 d = (me->getDestination() - cs->getPos());
            origT = glm::length(d);
            if (origT < 0.00001f)
                continue; // Ellipsoid hasn't moved

            best.t = origT;
            d = glm::normalize(d);
            best.dir = d;

            // put Ellipsoid and ray into sphere space
            glm::vec3 r = cs->getDim();
            d = d / r;

            glm::vec3 basis = glm::vec3(1.f / r.x, 1.f / r.y, 1.f / r.z);
            // check for collisions with each triangle in the scene
            foreach (Triangle *tri, tris)
            {
                cs->collidesTriangle(tri->scale(basis), d, &col);

                if (col.t < best.t)
                {
                    best.t = col.t;
                    best.tMinus = origT - col.t;
                    best.type = col.type;
                    best.colPoint = col.colPoint * r;   // back to world space
                    best.colNorm = col.colNorm * basis;     // back to world space
                    assert(glm::length2(best.colNorm) > 0.00001f);
                    best.colNorm = glm::normalize(best.colNorm);
                }
            }
            // collision occurred
            cols.append(new TriCollision(best));
        }
    }

    return cols;
}


void GeometricCollisionManager::handleTriangleCollisions(QList<TriCollision *> cols)
{
    float eps = 0.00001f;
    glm::vec3 up = glm::vec3(0, 1, 0);

    // handle collisions
    foreach (TriCollision *col, cols)
    {
        MovableEntity *me = col->me;

        glm::vec3 &n = col->colNorm;
        glm::vec3 hit = me->getPosition() + col->dir * col->t + n * eps;

        glm::vec3 rem = col->dir * col->tMinus;
        glm::vec3 para;

        if (glm::length2(col->colNorm) > eps)
        {
            if (col->type == PLANE && glm::dot(n, up) > 0.0001f) // do ramp hack
            {
                rem = rem - (glm::dot(n, rem) / glm::dot(n, up) * up);
                float tMinus = glm::length(rem - glm::dot(n, rem) * n);
                if (tMinus > 0.000001f)
                    rem = glm::normalize(rem) * tMinus;
                else
                    rem = glm::vec3();
            }

            glm::vec3 perp = n * (glm::dot(n, rem));
            para = rem - perp;

            glm::vec3 vel = me->getVelocity();
            if (glm::length2(vel) > 0.000001f)
            {
                glm::vec3 newVel = vel - vel * glm::abs(n);
                vel.x = (n.x*vel.x > 0.0 ? vel.x: newVel.x);
                vel.y = (n.y*vel.y > 0.0 ? vel.y: newVel.y);
                vel.z = (n.z*vel.z > 0.0 ? vel.z: newVel.z);
            }
            else
                vel = glm::vec3();
            me->setVelocity(vel);
        }
        else
            para = glm::vec3();

        me->setDestination(hit + para);
        me->setPosition(hit);

        Collision c;
        c.impulse = n;
        me->handleCollision(&c);
    }
}


