#ifndef MESH_H
#define MESH_H

#include "GL/glew.h"
#include <glm.hpp>

class Mesh
{
public:
    Mesh();
    virtual ~Mesh();

    void init(GLuint shader, GLuint w, GLuint h, const glm::vec3 *verts);

    void setVerts(const glm::vec3 *verts);
    void createBuffers(GLuint shader, GLuint size);

    void onDraw();

private:
    void setMappings();
    void fillBuffer(const float *data, int start, int count);
    void addVertex(int *i, glm::vec3 v, float* data);

    GLuint m_w, m_h;

    GLuint *m_vertMappings;
    GLuint m_mappingSize;

    GLuint m_vao;
    GLuint m_vbo;

};

#endif // MESH_H
