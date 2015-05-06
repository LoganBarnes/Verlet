#ifndef VERLET_H
#define VERLET_H

#include <glm.hpp>
#include <vector>
#include "link.h"
#include <QHash>
#include "graphics.h"
class Bend;
class VerletManager;
class Graphics;
class MovableEntity;
class OBJ;

struct Tri
{
    int a, b, c;
    glm::vec3 vertices[3];
    Link* edges[3];
    glm::vec3 normal;
    //float windForce; //value between 0 + 1 representing wind influence
    int random; //to give noise to wind

    Tri(){}
    Tri(int _a, int _b, int _c){
        a = _a; b = _b; c= _c;
        random = rand() %10;
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

struct LinkMap{
    QList<Link*> links;
    QList<Bend*> crosses;
    QList<Bend*> shears;

    LinkMap(){}
};

class Verlet
{
public:
    Verlet(VerletManager* m);
    virtual ~Verlet();

    //Maximum points, for declaring arrays
    static const int NUM = 10000;

    VerletManager* _manager;
    //between 0 and 1: how much cloth is influenced by collisions
    float sphereInfluence = .9f;
    float rayTraceSize = .6f;
    float windNoise =.06; // how much random noise to give verlet's wind response
    bool controlWind = false;
    glm::vec3 windDirection = glm::vec3(0);

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
    virtual glm::vec3 collide(MovableEntity *e);
    virtual void collideSurface(OBJ* obj);

    //QHash<int, QList<Link*> > link_map;
    LinkMap _linkMap[NUM];
    virtual void tearLink(Link* l);
    Link* findLink(int a, int b);
    void removeLink(Link* l);

    glm::vec3 *getPosArray() { return _pos; }
    glm::vec3 *getNormArray() { return _normal; }

    //Called per tick to update triangle vertices + normal based
    //on movement of verlet's points
    void calculate(Tri* t);
    //@param wind: normalized vector representing wind direction
    virtual void applyWind(Tri* t);
    void setWindDirection(float radian);
protected:
    //Creates new point (at index numPoints) w/ given position
    void createPoint(const glm::vec3& pos);
    //Specify indices of two pre-existing points
    Link* createLink(int a, int b,bool sort = true);

    //Actual number of points
    int numPoints = 0;
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


};

#endif // VERLET_H
