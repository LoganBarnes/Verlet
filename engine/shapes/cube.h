#ifndef CUBE_H
#define CUBE_H

#include "shape.h"

class Cube : public Shape
{
public:
    Cube(int complexity);
    ~Cube();

    virtual void calcVerts();

private:
    void makeSide(int *index, glm::vec3 norm, double spacing, bool first, bool last);
};

#endif // CUBE_H
