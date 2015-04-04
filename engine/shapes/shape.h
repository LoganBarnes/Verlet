#ifndef SHAPE_H
#define SHAPE_H

#include "GL/glew.h"
#include <glm.hpp>

class Shape
{
public:
    Shape(int complexity);
    virtual ~Shape();

    void init(GLuint shader);

    virtual void calcVerts();
    virtual void updateGL(GLuint shader);
    void cleanUp();

    virtual bool animate();

    void render();
    virtual void transformAndRender(GLuint shader, glm::mat4 trans, GLenum mode);

    float map(const float val, const float oldMin, const float oldMax, const float newMin, const float newMax);
    glm::vec2 mapPoints(const glm::vec2 val, const glm::vec2 oldMin, const glm::vec2 oldMax,
                        const glm::vec2 newMin, const glm::vec2 newMax);

protected:

    void addVertex(int *i, glm::vec3 v, glm::vec3 norm);
    void addVertexT(int *i, glm::vec3 v, glm::vec3 norm, glm::vec2 tex);

    GLuint m_vaoID; // The vertex array object ID, which is passed to glBindVertexArray.
    GLuint m_vboID;
    GLfloat *m_vertexData;

    int m_p1;
    int m_p2;
    float m_p3;

    int m_numVerts;
    float m_halfWidth;
};

#endif // SHAPE_H
