#ifndef MESH_H
#define MESH_H

#include "GL/glew.h"
#include <glm.hpp>

class Mesh
{
public:
    Mesh();
    virtual ~Mesh();

    void init(GLuint shader);

    virtual void calcVerts();
    virtual void updateGL(GLuint shader);
    void cleanUp();

};

#endif // MESH_H
