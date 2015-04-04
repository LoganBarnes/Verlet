#include "line.h"

Line::Line(int complexity)
    : Shape(complexity)
{
}

Line::~Line()
{
}

void Line::calcVerts()
{
    m_numVerts = 2;
    int size = m_numVerts * 8;
    m_vertexData = new GLfloat[size];

    glm::vec3 norm = glm::vec3(0, 1, 0);

    int index = 0;
    addVertex(&index, glm::vec3(), norm);
    addVertex(&index, glm::vec3(1, 0, 0), norm);
}
