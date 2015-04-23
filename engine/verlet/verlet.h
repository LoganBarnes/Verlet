#ifndef VERLET_H
#define VERLET_H

#include <glm.hpp>
#include <vector>
#include "link.h"
#include <QHash>

class VerletManager;
class Graphics;
class MovableEntity;

class Verlet
{
public:
    Verlet(VerletManager* m);
    virtual ~Verlet();

    VerletManager* _manager;
    //between 0 and 1: how much cloth is influenced by collisions
    float sphereInfluence = 1;
    float rayTraceSize = .13f;

    int getSize(){return numPoints;}
    glm::vec3 getPoint(const int& id){return _pos[id];}

    //Fix point at specified index to its _pos
    void createPin(int index);
    //Sets point at specified index to given pos
    void setPos(int index, const glm::vec3& pos);

    //Per update, called in VerletManager's onTick:
    //Update pos + prevPos
    void verlet(float seconds);
    //Solve individual constraints
    void boxConstraint(const glm::vec3& _boxMin, const glm::vec3& _boxMax);
    void linkConstraint();
    void pinConstraint();

    virtual void onTick(float seconds);
    virtual void onDraw(Graphics *g);
    virtual void updateBuffer() {}
    glm::vec3 collide(MovableEntity *e);

    virtual Link* closestLink(int id, const glm::vec3& point);
    virtual void tearLink(Link* l);
    QHash<int, QList<Link*> > link_map;

    glm::vec3 *getPosArray() { return _pos; }
    glm::vec3 *getNormArray() { return _normal; }
protected:
    //Creates new point (at index numPoints) w/ given position
    void createPoint(const glm::vec3& pos);
    //Specify indices of two pre-existing points
    Link* createLink(int a, int b);

    //Actual number of points
    int numPoints = 0;
    //Maximum points, for declaring arrays
    static const int NUM = 300000;
    //Points
    glm::vec3 _pos[NUM];
    glm::vec3 _prevPos[NUM];
    glm::vec3 _acc[NUM];
    glm::vec3 _normal[NUM];

    //Constraints
    typedef struct{
        int index;
        glm::vec3 pinPos;
    }Pin;
    std::vector<Pin> pins;
    std::vector<Link*> links;

    //Utility for editing links (tearing)
    void removeLink(Link* l);
    Link* findLink(int a, int b);
    void replaceLink(Link* key, Link* oldLink, Link* newLink,
                     QHash<Link*, QList<Link*> >& hash);
};

#endif // VERLET_H
