#ifndef NET_H
#define NET_H

#include "verlet.h"

class Mesh;
typedef unsigned int GLuint;

class Net: public Verlet
{
public:
    //@param dimension: how many points in the net
    //@param start: where first point of net is created
    //@param interval1 + 2: spacing from one point to the next, 'horizontally' and 'vertically'
    Net(glm::vec2 dimension, const glm::vec3& start,
        const glm::vec3& interval1, const glm::vec3& interval2, GLuint shader);
    ~Net();

    virtual void onDraw(Graphics *g);
    virtual void updateBuffer();

private:
    Mesh *m_mesh;
    GLuint m_shader;
};

#endif // NET_H
