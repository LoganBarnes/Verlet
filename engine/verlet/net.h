#ifndef NET_H
#define NET_H

#include "verlet.h"

class Mesh;

class Net: public Verlet
{
public:
    //@param dimension: how many points in the net
    //@param start: where first point of net is created
    //@param interval1 + 2: spacing from one point to the next, 'horizontally' and 'vertically'
    Net(glm::vec2 dimension, const glm::vec3& start,
        const glm::vec3& interval1, const glm::vec3& interval2);
    ~Net();

//    virtual void onDraw(Graphics *g);

private:
    Mesh *m_mesh;
};

#endif // NET_H
