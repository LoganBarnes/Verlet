#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <glm.hpp>

// optimization for later
//struct Tri
//{

//    Tri(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);
//};

class Triangle
{
public:
    Triangle();
    // Vertices must be in CCW order
    Triangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3);

    Triangle scale(glm::vec3 basis);

    float intersectPlane(glm::vec3 p, glm::vec3 d, glm::vec3 *colPoint = NULL, glm::vec3 *colNorm = NULL);
    float intersectEdges(glm::vec3 p, glm::vec3 d, glm::vec3 *colPoint = NULL, glm::vec3 *colNorm = NULL);

    bool containsXZ(glm::vec3 point);
    float getHeight(glm::vec3 point);

    glm::vec3 vertices[3];
    glm::vec3 normal;

    inline bool EQ(float a, float b) { return fabs(a - b) < 0.00001f; }

private:
    float intersectEdge(glm::vec3 p, glm::vec3 d, int v1i, int v2i, glm::vec3 *colPoint, glm::vec3 *colNorm);

    float intersectInfiniteCylinder(glm::vec3 p, glm::vec3 d, glm::vec3 v1, glm::vec3 v2, glm::vec3 *point, glm::vec3 *colNorm);
    int findTCylinder(glm::vec3 p, glm::vec3 d, glm::vec3 v1, glm::vec3 v2, float *t1, float *t2);

};

#endif // TRIANGLE_H
