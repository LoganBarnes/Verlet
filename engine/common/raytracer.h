#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <glm/glm.hpp>
#include <vector>

class Triangle;
class Ellipsoid;
class Verlet;

struct HitTest
{
    bool hit;
    float t;
    glm::vec3 normal;
    int id;  //for verlet collisions: point hit
    Verlet* v;  //for verlet collisions
    Triangle* tri; //for triangle + mesh collisions

    HitTest() : hit(false), t(INFINITY), tri(NULL), v(NULL) {}
    HitTest(bool h, float f, Triangle* tr){hit = h; t=f; tri=tr;}
};

class RayTracer
{
public:
    glm::vec3 source;
    glm::vec3 direction;

    //Cursor ray: to track what's under the cursor
    //@params: modelview from Camera, mouseX + mouseY from World
    RayTracer(double modelview[16], int mouseX, int mouseY);
    //Call each time camera moves, usually onTick in game-side world
    void update(double modelview[16], int mouseX, int mouseY);

    //Normal ray- cannot call getRayForPixel
    RayTracer(const glm::vec3& s, const glm::vec3& d);
    virtual ~RayTracer();

    glm::vec3 getPoint(float t) {return source+direction*t;}
    float hitPlane(const glm::vec3& point, const glm::vec3& normal);

    //If true is returned, 'result' is filled in with information about the collision
    bool hitAABB(const glm::vec3 &min, const glm::vec3 &max, HitTest &result);
    bool hitEllipsoid(Ellipsoid* e, HitTest &result);
    bool hitEllipsoid(const glm::vec3& pos, const glm::vec3& dim, int id, HitTest &result);
    bool hitMesh(std::vector<Triangle*> triangles, HitTest &result);
    bool hitTriangle(Triangle* t, HitTest &result);
    bool hitVerlet(Verlet* t, HitTest &result);
private:
    //For cursor ray: returns direction for ray directly to screen coordinate
    glm::vec3 getRayForPixel(int x, int y) const;

    // Stores the viewport at the time of the constructor
    int viewport[4];
    // Stores a ray along each frustum corner at the time of the constructor
    glm::vec3 ray00, ray10, ray01, ray11;

    float solveQuadratic(float a, float b, float c);
};

#endif // RAYTRACER_H
