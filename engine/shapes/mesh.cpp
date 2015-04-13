#include "mesh.h"

Mesh::Mesh()
    : m_w(0),
      m_h(0)
{
}

Mesh::~Mesh()
{
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
}

void Mesh::init(GLuint shader, GLuint w, GLuint h, const float *data)
{
    m_w = w;
    m_h = h;
    m_numVerts = m_w * m_h;
    GLuint memsize = m_numVerts * 8 * sizeof(float);
    createBuffers(shader, memsize);
    setVerts(data);
}

void Mesh::setVerts(const float *data)
{

}

void Mesh::fillBuffer(const float *data, int start, int count)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, start * 8 * sizeof(float), count*8*sizeof(float), data);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
}

void Mesh::createBuffers(GLuint shader, GLuint size)
{
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);

    // Initialize the vertex array object.
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Initialize the vertex buffer object.
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);

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
