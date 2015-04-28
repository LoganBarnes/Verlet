#ifndef HALF_H
#define HALF_H

#include <vector>
#include <glm.hpp>
#include <QList>

class Triangle;

//Creates flat triangle representation of a mesh to allow for the triangle a
//point is within (on the x-z plane) to be found quickly.
class Half
{
public:
    Half();
    //@param tri: all triangles contained within an obj file
    Half(QList<Triangle *> tris, bool _top);
    ~Half();

    //Adjusts given point if it's below/ above the mesh- takes hitbox into account
    bool placeOnSurface(glm::vec3& surfacePt);
    //Returns y corresponding w/ given x+z- doesn't consider hitbox
    bool findY(const glm::vec2 &coord, float& y);

private:
    std::vector<Triangle*> _triangles;
    std::vector<Triangle> flatTri;

    //Deviation allowed for a face, when dividing a mesh into top + bot
    float normalThreshold = .3;
    //Allowance - distance (epsilon) point is placed away from surface
    float allowance = .05;

    //Hitbox cylindrical representation
    glm::vec2 center;
    float radius;
    glm::vec2 yLimits;
    bool top; //Whether this represents the top or bottom half of a mesh

    //Helpers
    void createFlat();
    //Finds flat triangle a point on the x-z plane is within. Returns -1 if none is found
    int getFlatTriangle(float x, float z);
    //Creates cylindrical representation Half uses from an AABB's min and max
    void calcHitbox(const glm::vec3& min, const glm::vec3& max);


};

#endif // HALF_H
