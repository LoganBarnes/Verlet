#ifndef CYLINDER_H
#define CYLINDER_H

#include "shape.h"

class Cylinder : public Shape
{
public:
    Cylinder(int complexity);
    virtual ~Cylinder();

    virtual void calcVerts();

protected:
    virtual void make3DSlice(int *index, glm::vec2 left, glm::vec2 right, float leftU, float rightU);
    virtual void makeTopSlice(int *index, glm::vec3 left, glm::vec3 right);
    virtual void makeBottomSlice(int *index, glm::vec3 left, glm::vec3 right);
};

#endif // CYLINDER_H
