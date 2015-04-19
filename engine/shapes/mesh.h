#ifndef MESH_H
#define MESH_H

#include "GL/glew.h"
#include <glm.hpp>
#include <vector>

struct Tri;

class Mesh
{
public:
    Mesh();
    virtual ~Mesh();

    //for testing rendering w/ pointers
    void initTriangles(GLuint shader, std::vector<Tri*> tris, const glm::vec3 *verts);
    void setTriangles(std::vector<Tri*> tris, const glm::vec3 *verts);

    void initStrip(GLuint shader, GLuint w, GLuint h, const glm::vec3 *verts, const glm::vec3 *norms);
    void initTriangles(GLuint shader, std::vector<Tri> tris, const glm::vec3 *verts);

    void setVerts(const glm::vec3 *verts, const glm::vec3 *norms);
    void setTriangles(std::vector<Tri> tris, const glm::vec3 *verts);
    void createBuffers(GLuint shader, GLuint size);

    void onDraw(GLenum mode);

private:
    void setMappings();
    void fillBuffer(const float *data, int start, int count);
    void addVertex(int *i, glm::vec3 v, glm::vec3 norm, float* data);

    GLuint m_w, m_h;

    GLuint *m_vertMappings;
    GLuint m_mappingSize;

    GLuint m_vao;
    GLuint m_vbo;

};

#endif // MESH_H
