#include "triangle.h"
#include <stdio.h>
#include <algorithm>

#define GLM_FORCE_RADIANS
#include <glm/gtx/norm.hpp>

Triangle::Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3){
    vertices[0] = v1;
    vertices[1] = v2;
    vertices[2] = v3;
    normal = glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
}

Triangle Triangle::scale(glm::vec3 basis){
    return Triangle(vertices[0]*basis, vertices[1]*basis, vertices[2]*basis);
}


float Triangle::intersectPlane(glm::vec3 p, glm::vec3 d, glm::vec3 *colPoint, glm::vec3 *colNorm)
{
    // intersect infinite plane
    float t = glm::dot(-normal, p - vertices[0]) / glm::dot(normal, d);
    glm::vec3 point = p + d * t;

    // check if collision point is within triangle
    glm::vec3 pab = glm::cross(vertices[0] - point, vertices[1] - point);
    glm::vec3 pbc = glm::cross(vertices[1] - point, vertices[2] - point);
    glm::vec3 pca = glm::cross(vertices[2] - point, vertices[0] - point);

    if (t < 0.f || glm::dot(pab, pbc) < 0.00001f || glm::dot(pbc, pca) < 0.00001f)
        t = INFINITY;
    else
    {
        if (colPoint)
            *colPoint = point;
        if (colNorm)
            *colNorm = normal;
    }

    return t;
}


float Triangle::intersectEdges(glm::vec3 p, glm::vec3 d, glm::vec3 *colPoint, glm::vec3 *colNorm)
{
    // intersect each edge and return the smallest t value
    float bestT = INFINITY;
    glm::vec3 point, norm;
    float t = intersectEdge(p, d, 0, 1, &point, &norm);
    if (t > 0.f && t < bestT)
    {
        bestT = t;
        if (colPoint)
            *colPoint = point;
        if (colNorm)
            *colNorm = norm;
    }

    t = intersectEdge(p, d, 1, 2, &point, &norm);
    if (t > 0.f && t < bestT)
    {
        bestT = t;
        if (colPoint)
            *colPoint = point;
        if (colNorm)
            *colNorm = norm;
    }

    t = intersectEdge(p, d, 2, 0, &point, &norm);
    if (t > 0.f && t < bestT)
    {
        bestT = t;
        if (colPoint)
            *colPoint = point;
        if (colNorm)
            *colNorm = norm;
    }

    return bestT;
}


float Triangle::intersectEdge(glm::vec3 p, glm::vec3 d, int v1i, int v2i, glm::vec3 *colPoint, glm::vec3 *colNorm)
{
    return intersectInfiniteCylinder(p, d, vertices[v1i], vertices[v2i], colPoint, colNorm);
}


float Triangle::intersectInfiniteCylinder(glm::vec3 p, glm::vec3 d, glm::vec3 v1, glm::vec3 v2, glm::vec3 *colPoint, glm::vec3 *colNorm)
{
    float bestT = INFINITY;
    float t1 = INFINITY;
    float t2 = INFINITY;

    int tees = findTCylinder(p, d, v1, v2, &t1, &t2);

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

    // if intersection with infinite cylinder
    if (bestT < INFINITY)
    {
        // check if projection is between the end points
        glm::vec3 point = p + d * bestT;

        glm::vec3 a = v2 - v1;
        glm::vec3 b = point - v1;

        float comp = glm::dot(b, a); // component
        if (comp < 0.f || comp > glm::length2(a))
            bestT = INFINITY;
        else
        {
            *colPoint = a;
            float mag2 = glm::dot(*colPoint, *colPoint);
            *colPoint *= (mag2 > 0.000001f ? (comp / mag2) : 0.f); // a * (b.a / a.a)
            *colPoint += v1;

            *colNorm = point - *colPoint;
        }
    }

    return bestT;
}

int Triangle::findTCylinder(glm::vec3 p, glm::vec3 d, glm::vec3 v1, glm::vec3 v2, float *t1, float *t2)
{
    glm::vec3 axis = v2 - v1;
    glm::vec3 v2p = p - v1;

    glm::vec3 cross1 = glm::cross(d, axis);
    glm::vec3 cross2 = glm::cross(v2p, axis);

    double a = glm::length2(cross1);
    double b = 2.f * glm::dot(cross1, cross2);
    double c = glm::length2(cross2) - glm::length2(axis);

    // a == 0
    // one solution
    if (EQ(a, 0.f)) {
        *t1 = -c / b;
        return 1;
    }

    double disc = b * b - 4 * a * c;

    // one solution
    if (EQ(disc, 0.f)) {
        *t1 = -b / (2.f * a);
        return 1;
    }
    // no solution
    if (disc < 0)
        return 0;

    // two solutions
    *t1 = (-b + sqrt(disc)) / (2.f * a);
    *t2 = (-b - sqrt(disc)) / (2.f * a);
    return 2;
}


inline float signXZ(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
    return ((p1.x - p3.x) * (p2.z - p3.z) - (p2.x - p3.x) * (p1.z - p3.z));
}

bool Triangle::containsXZ(glm::vec3 point)
{
        bool b1, b2, b3;

        b1 = signXZ(point, vertices[0], vertices[1]) < 0.0f;
        b2 = signXZ(point, vertices[1], vertices[2]) < 0.0f;
        b3 = signXZ(point, vertices[2], vertices[0]) < 0.0f;

        return ((b1 == b2) && (b2 == b3));
}

float Triangle::getHeight(glm::vec3)
{
    return glm::min(vertices[0].y, glm::min(vertices[1].y, vertices[2].y));
}
