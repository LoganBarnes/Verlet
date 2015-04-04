#include "shape.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
using namespace std;

Shape::Shape(int complexity)
{
//    m_numVerts = 4;
    int c = max(complexity, 3);
    m_p1 = c;
    m_p2 = c;
    m_vertexData = NULL;
    m_vaoID = 0;
    m_vboID = 0;

    m_halfWidth = 0.5f;

    // hack for raytracing
    if (complexity == -1)
    {
        m_p1 = 1;
        m_p2 = 1;
        m_halfWidth = 1.f;
    }
}

Shape::~Shape()
{
    cleanUp();
    if (m_vaoID)
        glDeleteVertexArrays(1, &m_vaoID);
    if (m_vboID)
        glDeleteBuffers(1, &m_vboID);
}


void Shape::init(GLuint shader)
{
    calcVerts();
    updateGL(shader);
    cleanUp();
}


void Shape::calcVerts()
{
    int w = m_p1 + 1;
    m_numVerts = ((2 * w + 2) * m_p1) - 2;
    int size = m_numVerts * 8;
    m_vertexData = new GLfloat[size];

    double spacing = m_halfWidth * 2.0 / m_p1;

    int index = 0;
    glm::vec3 norm = glm::vec3(0, 0, 1);

    glm::vec2 omin = glm::vec2(0, 0);
    glm::vec2 omax = glm::vec2(m_p1, m_p1);

    glm::vec2 nmin = glm::vec2(1.f, 1.f);
    glm::vec2 nmax = glm::vec2(0.f, 0.f);

    glm::vec3 v1, v2;
    glm::vec2 t1, t2; // texture coords

    // set z
    v1.z = 0;

    for (int i = 0; i < m_p1; i++) {

        v1.x = m_halfWidth;
        v1.y = (i * spacing - m_halfWidth);

        // double the first point if it isn't the start of the cube
        if (i != 0)
            addVertex(&index, v1, norm);

        for (int j = 0; j <= m_p1; j++) {
            v1.x = (j * spacing - m_halfWidth) * -1.f;
            v2 = v1;
            v2.y = v1.y + spacing;

            t1 = mapPoints(glm::vec2(j, i), omin, omax, nmin, nmax);
            t2 = mapPoints(glm::vec2(j, i+1), omin, omax, nmin, nmax);

            addVertexT(&index, v1, norm, t1);
            addVertexT(&index, v2, norm, t2);
        }
        // double the last point if it isn't the end of the cube
        if (i != m_p1 - 1)
            addVertex(&index, v2, norm);
    }
}


void Shape::updateGL(GLuint shader)
{
    if (m_vaoID)
        glDeleteVertexArrays(1, &m_vaoID);
    if (m_vboID)
        glDeleteBuffers(1, &m_vboID);

    // Initialize the vertex array object.
    glGenVertexArrays(1, &m_vaoID);
    glBindVertexArray(m_vaoID);

    // Initialize the vertex buffer object.
    glGenBuffers(1, &m_vboID);
    glBindBuffer(GL_ARRAY_BUFFER, m_vboID);

    glBufferData(GL_ARRAY_BUFFER, 8 * m_numVerts * sizeof(GLfloat), m_vertexData, GL_STATIC_DRAW);

    GLuint position = glGetAttribLocation(shader, "position");
    GLuint normal = glGetAttribLocation(shader, "normal");
    GLuint texCoord = glGetAttribLocation(shader, "texCoord");

    glEnableVertexAttribArray(position);
    glVertexAttribPointer(
        position,
        3,                   // Num coordinates per position
        GL_FLOAT,            // Type
        GL_FALSE,            // Normalized
        sizeof(GLfloat) * 8, // Stride
        (void*) 0            // Array buffer offset
    );
    glEnableVertexAttribArray(normal);
    glVertexAttribPointer(
        normal,
        3,                              // Num coordinates per normal
        GL_FLOAT,                       // Type
        GL_TRUE,                        // Normalized
        sizeof(GLfloat) * 8,            // Stride
        (void*) (sizeof(GLfloat) * 3)   // Array buffer offset
    );
    glEnableVertexAttribArray(texCoord);
    glVertexAttribPointer(
        texCoord,
        2,                              // Num coordinates per position
        GL_FLOAT,                       // Type
        GL_TRUE,                        // Normalized
        sizeof(GLfloat) * 8,            // Stride
        (void*) (sizeof(GLfloat) * 6)   // Array buffer offset
    );

    // Unbind buffers.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Shape::cleanUp()
{
    if (m_vertexData)
    {
        delete[] m_vertexData;
        m_vertexData = NULL;
    }
}


void Shape::render()
{
    glBindVertexArray(m_vaoID);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, m_numVerts);
    glBindVertexArray(0);
}


void Shape::transformAndRender(GLuint shader, glm::mat4 trans, GLenum mode)
{
    glBindVertexArray(m_vaoID);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(trans));
    glDrawArrays(mode, 0, m_numVerts);
    glBindVertexArray(0);

//    glUniform3f(glGetUniformLocation(shader, "allBlack"), 0, 0, 0);
//    glBindVertexArray(m_vaoID);
//    glDrawArrays(GL_LINE_STRIP, 0, m_numVerts);
//    glBindVertexArray(0);
//    glUniform3f(glGetUniformLocation(shader, "allBlack"), 1, 1, 1);
}

void Shape::addVertex(int *i, glm::vec3 v, glm::vec3 norm)
{
    m_vertexData[(*i)++] = v.x;
    m_vertexData[(*i)++] = v.y;
    m_vertexData[(*i)++] = v.z;
    m_vertexData[(*i)++] = norm.x;
    m_vertexData[(*i)++] = norm.y;
    m_vertexData[(*i)++] = norm.z;
    m_vertexData[(*i)++] = 0;
    m_vertexData[(*i)++] = 0;
}


void Shape::addVertexT(int *i, glm::vec3 v, glm::vec3 norm, glm::vec2 tex)
{
    m_vertexData[(*i)++] = v.x;
    m_vertexData[(*i)++] = v.y;
    m_vertexData[(*i)++] = v.z;
    m_vertexData[(*i)++] = norm.x;
    m_vertexData[(*i)++] = norm.y;
    m_vertexData[(*i)++] = norm.z;
    m_vertexData[(*i)++] = tex.x;
    m_vertexData[(*i)++] = tex.y;
}

bool Shape::animate()
{
    return false;
}


glm::vec2 Shape::mapPoints(const glm::vec2 val, const glm::vec2 oldMin, const glm::vec2 oldMax,
                       const glm::vec2 newMin, const glm::vec2 newMax)
{
    return (val - oldMin) / (oldMax - oldMin) * (newMax - newMin) + newMin;
}


float Shape::map(const float val, const float oldMin, const float oldMax, const float newMin, const float newMax)
{
    return (val - oldMin) / (oldMax - oldMin) * (newMax - newMin) + newMin;
}


