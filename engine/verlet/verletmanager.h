#ifndef VERLETMANAGER_H
#define VERLETMANAGER_H

#include "manager.h"
#include <glm/glm.hpp>
#include "link.h"
#include "verlet.h"
#include "rope.h"

class Ellipsoid;
class Ray;

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
    glm::vec3 collideTerrain(MovableEntity* e);
    bool rayTrace(float x, float y);

    //Settings
    glm::vec3 gravity = glm::vec3(0,-3,0);
    glm::vec3 wind = glm::vec3(0,0,0.1);
    float windPow = 3;

    //Whether constraints are solved
    bool solve = true;

    int m_curV;
    int m_curI;
    Ray *m_ray;
private:
    //Verlet objects manager maintains
    std::vector<Verlet*> verlets;

    //for box constraint: keeps all points within here
    glm::vec3 _boxMin = glm::vec3(-15,15,-15);
    glm::vec3 _boxMax = glm::vec3(15,50,15);
    //How many times constraints are solved
    int _numSolves = 3;

    //Helpers for onTick, to cycle through each step
    //Store acceleration for all points
    void accumulateForces();
    void resetForces();
    //Updates positions of all points w/ velocity + acc
    void verlet(float seconds);
    //Adjusts positions to satisfy listed constraints
    //*note: box currently disabled
    void constraints();
    // updates any vbos if necessary
    void updateBuffer();
};

#endif // VERLETMANAGER_H
