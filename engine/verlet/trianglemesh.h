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
    void replaceIndex(int index, int index2){
        if(a==index)
            a=index2;
        else if(b==index)
            b=index2;
        else
            c=index2;
    }
};

struct Duplicate
{
    Link* shear1;
    Link* shear2;
    Link* seg1; Link* seg2; Link* seg3; Link* seg4;

    Duplicate(Link* s1, Link* s2, Link* l1, Link* l2, Link* l3, Link* l4){
        shear1 = s1; shear2 = s2;
        seg1 = l1; seg2= l2; seg3 = l3; seg4 = l4;
    }
};

class TriangleMesh: public Verlet
{
public:
    TriangleMesh(const glm::vec2& dimension, float width,
                 const glm::vec3& start, VerletManager* vm, GLuint shader);
    virtual ~TriangleMesh();

    void onTick(float seconds);
    virtual void onDraw(Graphics *g);
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
    Link* createShear(int a, int b, int c, Link* seg1 = NULL, Link* seg2 = NULL);
    void removeShear(Link* l);

    //Tear
    //Call this to tear a link- duplicates it, then checks surrounding points
    //for tears to merge
    void tearLink(Link* l);
    //Helper: checks whether index is connected to any (other) broken links
    bool checkTorn(int index);
    //Called from tearLink if there are tears to merge- inserts a point so they
    //can be separated
    void insertPoint(int index, Tri* t1, Link* l1,
                     Tri* t2, Link* l2, std::vector<Duplicate> duplicate);
    //Helper: copies all data from index into a new point at returned index
    int duplicatePoint(int index);
    //Helper: finds all links + triangles adjoining l1 and attached to index
    void findConnecting(int index, Tri*& t1, Link*& l1,
                        QList<Tri*>& triangles, QList<Link*>& links);

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
