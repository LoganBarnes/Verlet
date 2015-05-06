#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "verlet.h"

class Mesh;
class Bend;
class VerletManager;
typedef unsigned int GLuint;

enum PinMode
{
    NONE, TOP_CORNERS, ALL_CORNERS, TOP_EDGE, HORIZONTAL_EDGE, ALL_EDGE, ONE_CORNER
};
enum Axis
{
    X, Y, Z
};
struct Connected{
     QList<Tri*> connectedTri;
     QList<Link*> connectedLink;
     QList<int> points;
     Connected(){
         connectedTri = QList<Tri*>();
         connectedLink = QList<Link*>();
         points = QList<int>();
     }
};

class TriangleMesh: public Verlet
{
public:
    TriangleMesh(const glm::vec2& dimension, float width, const glm::vec3& start,
                 VerletManager* vm, GLuint shader, Axis axis = X, bool flat = false,
                 PinMode p = TOP_CORNERS, int angle = 0);
    virtual ~TriangleMesh();

    void pin(PinMode p, int r, int c);
    void onTick(float seconds);
    virtual void onDraw(Graphics *g);
    virtual void updateBuffer();
    int m_row, m_col;

    //Duplicates l and any shears over it, then calls insertPoint if necessary
    void tearLink(Link* l);
private:
    float _scalar[NUM]; //for triangleMesh: 1/numTriangles, for averaging

    Mesh *m_mesh;
    GLuint m_shader;
    //Called in constructor to add triangles
    void triangulate(int a, int b, int c);
    glm::vec2 rotatePoint(const glm::vec2& p, int angle);
    QPair<glm::vec2,glm::vec2> rotateTriangle(float w, int angle);

    std::vector<Tri*> _triangles;
    QHash<int, int> numTri; //how many triangles each index is part of

    //Shear
    Bend* createShear(int a, int b, int c, Link* seg1 = NULL, Link* seg2 = NULL);
    void removeShear(Bend* l);

    //Tear
    //returns list of torn edges a is connected to, not those equivalent to l
    bool checkTorn(int a);
    //Duplicates index so edges can be torn apart
    int insertPoint(int index, Connected c1, Connected c2);
    //copies all data from index into a new point at returned index
    int duplicatePoint(int index_a);
    //Finds all links + triangles adjoining l1 and attached to index
    Connected findConnecting(int index_a, Tri*& t1, Link*& l1);
    void handleShears(bool tear, QList<Bend*> duplicates, Link* l,
                          QList<Link*> l1_links, QList<Link*> l2_links);
    bool assignLink(QList<Link*> connected, Bend* b, Link *l);

    //For tearing
    QHash<Link*, QList<Bend*> > link_to_shear;
    QHash<Link*, QList<Tri*> > link_to_tri;
};

#endif // TRIANGLEMESH_H
