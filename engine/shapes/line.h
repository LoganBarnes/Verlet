#ifndef LINE_H
#define LINE_H

#include "shape.h"

class Line : public Shape
{
public:
    Line(int complexity);
    virtual ~Line();

    virtual void calcVerts();
};

#endif // LINE_H
