#ifndef CONE_H
#define CONE_H

#include "shape.h"

class Cone : public Shape
{
public:
    Cone(int complexity);
    virtual ~Cone();

    virtual void calcVerts();

protected:
    virtual void make3DSlice(int *index, glm::vec2 left, glm::vec2 right, float normSlope, float leftU, float rightU);
    void makeBottomSlice(int *index, glm::vec3 left, glm::vec3 right);
};

#endif // CONE_H
