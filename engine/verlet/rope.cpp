#include "rope.h"
#include <iostream>

Rope::Rope(int units, const glm::vec3 &start, const glm::vec3 &interval, VerletManager* vm): Verlet(vm)
{
    for(int i = 0; i<units; i++)
        createPoint(start+(float)i*interval);

    for(int i = 0; i<units-1; i++)
        createLink(i,i+1);

}

Rope::~Rope()
{

}

