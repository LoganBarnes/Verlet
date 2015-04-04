#ifndef SPHERE_H
#define SPHERE_H

#include "shape.h"

class Sphere : public Shape
{
public:
    Sphere(int complexity);
    virtual ~Sphere();

    virtual void calcVerts();

protected:
    virtual void make3Dslice(int *index, float thetaR, float thetaL);
    virtual void calcSliceSeg(int *index, float thetaR, float thetaL, float phi);
};

#endif // SPHERE_H
