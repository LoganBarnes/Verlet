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
    Link* edges[3];
    glm::vec3 normal;
    float windForce; //value between 0 + 1 representing wind influence

    Tri(){}
    Tri(int _a, int _b, int _c){
        a = _a; b = _b; c= _c;
    }

    bool operator == (const Tri &t)
        const {return (a==t.a&&b==t.b&&c==t.c);}

    void replaceLink(Link* orig, Link*& l){
        if(edges[0]==orig)
            edges[0]=l;
        if(edges[1]==orig)
            edges[1]=l;
        if(edges[2]==orig)
            edges[2]=l;
    }
};

class TriangleMesh: public Verlet
{
public:
    TriangleMesh(const glm::vec2& dimension, float width,
                 const glm::vec3& start, VerletManager* vm, GLuint shader);
    virtual ~TriangleMesh();

    void onTick(float seconds);
    virtual void onDraw(Graphics *g, GLuint shader, int pass);
    virtual void updateBuffer();

    //Called per tick to update triangle vertices + normal based
    //on movement of verlet's points
    void calculate(Tri* t);
    //@param wind: normalized vector representing wind direction
    void applyWind(Tri* t);
private:
    float _scalar[NUM]; //for triangleMesh: 1/numTriangles, for averaging

    Mesh *m_mesh;
    GLuint m_shader;
    //Called in constructor to add triangles
    void triangulate(int a, int b, int c);
    std::vector<Tri*> _triangles;
    QHash<int, int> numTri; //how many triangles each index is part of

    //Shear
    Link* createShearLink(int a, int b, int c, Link* seg1 = NULL, Link* seg2 = NULL);
    void removeShear(Link* l);

    //For tearing
    //which shear constraints end at index: reassign if point is duplicated
    QHash<int, QList<Link*> > shear_map;
    //which shear constraints cross over index: destroy all if point is duplicated
    QHash<int, QList<Link*> > crossover_map;
    //which links a shear is mapping over
    QHash<Link*, QList<Link*> > shear_to_link;
    //which shears a edge is between
    QHash<Link*, QList<Link*> > link_to_shear;
    //edge to triangles it connects to
    QHash<Link*, QList<Tri*> > link_to_tri;
};

#endif // TRIANGLEMESH_H
