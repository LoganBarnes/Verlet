#ifndef MESH_H
#define MESH_H

#include "GL/glew.h"
#include <glm.hpp>

class Mesh
{
public:
    Mesh();
    virtual ~Mesh();

    void init(GLuint shader, GLuint w, GLuint h, const float *data);

    void setVerts(const float *data);
    void createBuffers(GLuint shader, GLuint size);

private:
    void fillBuffer(const float *data, int start, int count);

    GLuint m_w, m_h;
    GLuint m_numVerts;

    GLuint *m_vertMappings;

    GLuint m_vao;
    GLuint m_vbo;

};

#endif // MESH_H
