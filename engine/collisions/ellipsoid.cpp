#include "ellipsoid.h"
#include "triangle.h"

//#include <iostream>
//using namespace std;
//#include <glm/ext.hpp>

Ellipsoid::Ellipsoid(glm::vec3 center, glm::vec3 radius, QString id)
    : CollisionShape(center, radius, id)
{
}


Ellipsoid::~Ellipsoid()
{
}


float Ellipsoid::intersectRayWorldSpace(glm::vec3 p, glm::vec3 d)
{
    p = (p - getPos()) * glm::vec3(1 / m_dim.x, 1 / m_dim.y, 1 / m_dim.z);
    d = glm::vec3(d.x / m_dim.x, d.y / m_dim.y, d.z / m_dim.z);
//    cout << "P: " << glm::to_string(p) << endl;
//    cout << "D: " << glm::to_string(d) << endl;
    return intersectRay(p, d);
}


float Ellipsoid::intersectRay(glm::vec3 p, glm::vec3 d)
{
    float bestT = INFINITY;

    float t1 = INFINITY;
    float t2 = INFINITY;

    // tees will only be 0, 1, or 2 (number of intersections)
    int tees = findT(p, d, &t1, &t2);

    if (tees)
    {
        if (t1 >= 0.f && t1 < bestT)
            bestT = t1;

        if (tees == 2)
        {
            if (t2 >= 0.f && t2 < bestT)
                bestT = t2;
        }
    }
    return bestT;
}


int Ellipsoid::findT(glm::vec3 p, glm::vec3 d, float *t1, float *t2)
{
    double a = d.x * d.x + d.y * d.y + d.z * d.z;
    double b = 2 * p.x * d.x + 2 * p.y * d.y + 2 * p.z * d.z;
    double c = p.x * p.x + p.y * p.y + p.z * p.z - 1; // radius 1

    // a == 0
    // one solution
    if (EQ(a, 0.0)) {
        *t1 = -c / b;
        return 1;
    }

    double disc = b * b - 4 * a * c;

    // one solution
    if (EQ(disc, 0.0)) {
        *t1 = -b / (2 * a);
        return 1;
    }
    // no solution
    if (disc < 0)
        return 0;

    // two solutions
    *t1 = (-b + sqrt(disc)) / (2 * a);
    *t2 = (-b - sqrt(disc)) / (2 * a);
    return 2;
}

/**
 *
 * Assumes Ellipsoid is already scaled to be sphere with radius 1
 *
 * @brief Ellipsoid::intersectTriangle
 * @param triOrig
 * @param p
 * @param d
 * @return
 */
void Ellipsoid::collidesTriangle(Triangle tri, glm::vec3 d, TriCollision *col)
{
    glm::vec3 p = getPos() * glm::vec3(1 / m_dim.x, 1 / m_dim.y, 1 / m_dim.z);
    glm::vec3 point, norm;

    // plane test
    col->t = tri.intersectPlane(p - tri.normal, d, &point, &norm);

    // if the plane test fails check the edges and vertices
    if (col->t == INFINITY)
    {
        float t;

        // edges
        t = tri.intersectEdges(p, d, &point, &norm);
        if (t < col->t)
        {
            col->t = t;
            col->colPoint = point;
            col->colNorm = norm;
            col->type = EDGE;
            assert(glm::length(norm) > 0.00001f);
        }

        // vertices
        for (int i = 0; i < 3; i++)
        {
            t = intersectRay(tri.vertices[i] - p, -d);
            if (t < col->t)
            {
                col->t = t;
                col->colPoint = tri.vertices[i];
                col->colNorm = (p + d * col->t) - col->colPoint;
                col->type = VERTEX;
                assert(glm::length(col->colNorm) > 0.00001f);
            }
        }
    }
    else
    {
        col->colPoint = point;
        col->colNorm = norm;
        col->type = PLANE;
        assert(glm::length(norm) > 0.00001f);
    }
}

void Ellipsoid::setPosition(glm::vec3 pos)
{
    m_pos = pos;
}


void Ellipsoid::handleCollision(TriCollision) {}
Collision *Ellipsoid::collides(CollisionShape *) { return NULL; }
Collision *Ellipsoid::collidesSphere(CollisionSphere *) { return NULL; }
Collision *Ellipsoid::collidesCylinder(CollisionCylinder *) { return NULL; }
