#ifndef VERLETCUBE_H
#define VERLETCUBE_H

#include "verlet.h"

class VerletCube: public Verlet
{
public:
    //@param: opposite corners of the cube
    VerletCube(const glm::vec3& min, const glm::vec3& max);
    ~VerletCube();
};

#endif // VERLETCUBE_H
