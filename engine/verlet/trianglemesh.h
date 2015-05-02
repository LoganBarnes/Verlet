#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "verlet.h"

class Mesh;
class VerletManager;
typedef unsigned int GLuint;

class TriangleMesh: public Verlet
{
public:
    TriangleMesh(const glm::vec2& dimension, float width, const glm::vec3& start,
                 VerletManager* vm, GLuint shader, int axis = 0, float flat = false);
    virtual ~TriangleMesh();

    void onTick(float seconds);
    virtual void onDraw(Graphics *g);
    virtual void updateBuffer();
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
    void insertPoint(int index, Tri* t1, Link* l1,Tri* t2, Link* l2);
    //Helper: copies all data from index into a new point at returned index
    int duplicatePoint(int index);
    //Helper: finds all links + triangles adjoining l1 and attached to index
    void findConnecting(int index, Tri*& t1, Link*& l1,QList<Tri*>& triangles,
                         QList<Link*>& links, QList<int>& points);
    bool checkShearValid(Link* s);

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
