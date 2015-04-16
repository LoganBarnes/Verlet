#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "verlet.h"

class Mesh;
class VerletManager;
typedef unsigned int GLuint;

struct Tri
{
    int a, b, c;
    glm::vec3 vertices[3];
    glm::vec3 normal;
    float windForce; //value between 0 + 1 representing wind influence
    Tri(int _a, int _b, int _c){
        a = _a; b = _b; c= _c;
    }
    /*
    void drawNormal(Graphic* g){
        Vector3 center = (vertices[0]+vertices[1]+vertices[2])/3.0;
        g->drawLine(center, center+normal);
    }
    */
};

class TriangleMesh: public Verlet
{
public:
    TriangleMesh(const glm::vec2& dimension, float width,
                 const glm::vec3& start, VerletManager* vm, GLuint shader);
    ~TriangleMesh();

    void onTick(float seconds);
    virtual void onDraw(Graphics *g);
    virtual void updateBuffer();

    //Called per tick to update triangle vertices + normal based
    //on movement of verlet's points
    void calculate(Tri& t);
    //@param wind: normalized vector representing wind direction
    void applyWind(Tri& t);
private:
    float _scalar[NUM]; //for triangleMesh: 1/numTriangles, for averaging

    Mesh *m_mesh;
    GLuint m_shader;
    //Called in constructor to add triangles
    void triangulate(int a, int b, int c);
    std::vector<Tri> _triangles;
    QHash<int, int> numTri; //how many triangles each index is part of
};

#endif // TRIANGLEMESH_H
