#ifndef VERLETMANAGER_H
#define VERLETMANAGER_H

#include "engine/world/manager.h"
#include <glm/glm.hpp>
#include "link.h"
#include "verlet.h"
#include "rope.h"
#include "engine/common/raytracer.h"

class Ellipsoid;
class VerletManager: public Manager
{
public:
    VerletManager(GLuint shader);
    ~VerletManager();

    void addVerlet(Verlet* v);
    Verlet* getVerlet(int i){return verlets[i];}

    void enableSolve(){solve = !solve;}
    void setWind(const glm::vec3& w){wind = w;}

    void manage(World *world, float onTickSecs);
    void onDraw(Graphics *g);
    glm::vec3 collideTerrain(Entity* e);
    bool rayTrace(RayTracer* ray, HitTest &result);

    //Settings
    glm::vec3 gravity = glm::vec3(0,-3,0);
    glm::vec3 wind = glm::vec3(0,0,0.1);
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
