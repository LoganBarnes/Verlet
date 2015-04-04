#include "meshbuffer.h"

MeshBuffer::MeshBuffer()
    : m_bindingPoint(1)
{
    m_ubo = 0;
}

MeshBuffer::~MeshBuffer()
{
    if (m_ubo)
        glDeleteBuffers(1, &m_ubo);
}


void MeshBuffer::setBuffer(GLuint shader, QList<Triangle*> tris)
{
    if (m_ubo)
        glDeleteBuffers(1, &m_ubo);

//    const int TRI_SIZE = 4092;

    int numVerts = tris.size() * 3;
    int size = numVerts * 4;

    GLfloat *data = new GLfloat[size];

    int i = 0;
    foreach (Triangle *tri, tris)
    {
        data[i++] = tri->vertices[0].x;
        data[i++] = tri->vertices[0].y;
        data[i++] = tri->vertices[0].z;
        data[i++] = tri->normal.x;
        data[i++] = tri->vertices[1].x;
        data[i++] = tri->vertices[1].y;
        data[i++] = tri->vertices[1].z;
        data[i++] = tri->normal.y;
        data[i++] = tri->vertices[2].x;
        data[i++] = tri->vertices[2].y;
        data[i++] = tri->vertices[2].z;
        data[i++] = tri->normal.z;
    }

    glGenBuffers (1, &m_ubo);

    // Allocate storage for the UBO
    glBindBuffer (GL_UNIFORM_BUFFER, m_ubo);
//    glBufferData (GL_UNIFORM_BUFFER, sizeof (GLfloat) * TRI_SIZE, data, GL_DYNAMIC_DRAW);

    GLuint blockIndex = glGetUniformBlockIndex(shader, "triBlock");
    glUniformBlockBinding(shader, blockIndex, m_bindingPoint);

    glBufferData(GL_UNIFORM_BUFFER, size * sizeof(GLfloat), data, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, m_bindingPoint, m_ubo);



    // Unbind buffers.
//    glBindBuffer(GL_ARRAY_BUFFER, 0);

    delete[] data;
}

GLuint MeshBuffer::getUBO()
{
    return m_ubo;
}


