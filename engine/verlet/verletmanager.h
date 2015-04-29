#ifndef VERLETMANAGER_H
#define VERLETMANAGER_H

#include "manager.h"
#include <glm.hpp>

class MovableEntity;
class Verlet;
class OBJ;
class Ellipsoid;
class Ray;
struct Link;

class VerletManager: public Manager
{
public:
    VerletManager(Camera *cam, GLuint shader);
    ~VerletManager();

    void addVerlet(Verlet* v);
    Verlet* getVerlet(int i){return verlets[i];}

    void enableSolve(){solve = !solve;}
    void setWind(const glm::vec3& w){wind = w;}

    virtual void manage(World *world, float onTickSecs, float mouseX, float mouseY);
    void onDraw(Graphics *g);
    void rayTrace(float x, float y);

    glm::vec3 collideTerrain(MovableEntity* e);
    void collideSurface(OBJ* obj);

    virtual void onMousePressed(QMouseEvent *e);
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY);
    virtual void onMouseReleased(QMouseEvent *e);
    virtual void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY);

    virtual void onKeyPressed(QKeyEvent *e);
    virtual void onKeyReleased(QKeyEvent *e);

    //Settings
    glm::vec3 gravity = glm::vec3(0,-3,0);
    glm::vec3 wind = glm::vec3(0,0,0.5);
    float windPow = 3;

    //Whether constraints are solved
    bool solve = true;

private:
    //Verlet objects manager maintains
    std::vector<Verlet*> verlets;

    //for box constraint: keeps all points within here
    glm::vec3 _boxMin = glm::vec3(-15,15,-15);
    glm::vec3 _boxMax = glm::vec3(15,50,15);
    //How many times constraints are solved
    int _numSolves = 2;

    //Helpers for onTick, to cycle through each step
    //Updates positions of all points w/ velocity + acc
    void verlet(float seconds);
    //Adjusts positions to satisfy listed constraints
    //*note: box currently disabled
    void constraints();
    // updates any vbos if necessary
    void updateBuffer();

    //testing dragging
    bool m_dragMode; //true if player selects point + holds LMB

    //Selected attributes- don't change once dragMode is enabled
    int m_draggedPoint;
    Verlet* m_draggedVerlet;

    //For moving the selected point
    //World-space pt: where cursor's ray intersects w/ draggedPoint's plane
    glm::vec3 m_draggedMouse;

    //from draggedPoint to draggedMouse
    glm::vec3 m_interpolate;

    //testing wind
    glm::vec3 m_windDirection;

    //testing tearing
    bool m_tearMode;
    //World-space pt: where cursor's ray intersects w/ tear's plane
    int m_tear_ptA;
    int m_tear_ptB;
    Verlet* m_tearVerlet;
    Link* m_tearLink;
    int rayTrace(float x, float y, std::vector<int> points, Verlet* v);


    int m_curV;
    int m_curI;
    Ray *m_ray;
};

#endif // VERLETMANAGER_H
