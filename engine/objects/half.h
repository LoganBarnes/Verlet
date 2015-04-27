#ifndef HALF_H
#define HALF_H

#include <vector>
#include <glm.hpp>
#include <qlist>

class Triangle;
class Half
{
public:
    Half();
    //Half(std::vector<Triangle> tri, bool _top);
    //@param tri: all triangles contained within an obj file
    Half(QList<Triangle *> tris, bool _top);
    ~Half();

    std::vector<Triangle*> _triangles;
    std::vector<Triangle> flatTri;
    glm::vec2 center;
    float radius;
    glm::vec2 yLimits;
    bool top;

    void createFlat();
    bool pointOnSurface(glm::vec3& surfacePt);
    int getFlatTriangle(float x, float z);
    void readValues(const glm::vec3& min, const glm::vec3& max);

    float normalThreshold = .3;
};

#endif // HALF_H
