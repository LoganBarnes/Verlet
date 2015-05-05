#include "particlerenderer.h"
#include <GL/glew.h>

//#include"debugprinting.h"

ParticleRenderer::ParticleRenderer()
    : m_shader(0),
      m_vbo(0),
      m_vao(0),
      m_particleRadius(0)
{
}


ParticleRenderer::~ParticleRenderer()
{
    if (m_vbo)
        glDeleteBuffers(1, &m_vbo);
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);
}


void ParticleRenderer::createVAO(GLuint shader, GLuint vbo, float radius)
{
    if (m_vbo && m_vbo != vbo)
        glDeleteBuffers(1, &m_vbo);
    if (m_vao)
        glDeleteVertexArrays(1, &m_vao);

    m_shader = shader;
    m_vbo = vbo;
    m_particleRadius = radius;

    // Initialize the vertex array object.
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // bind vertex buffer object.
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    GLuint position = glGetAttribLocation(m_shader, "position");

    glEnableVertexAttribArray(position);
    glVertexAttribPointer(
        position,
        4,                   // Num coordinates per position
        GL_FLOAT,            // Type
        GL_FALSE,            // Normalized
        sizeof(GLfloat) * 4, // Stride
        (void*) 0            // Array buffer offset
    );

    // Unbind buffers.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}


void ParticleRenderer::render(uint numParticles)
{
//    glEnable(GL_BLEND); //Enable blending.
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLuint radiusLoc = glGetUniformLocation(m_shader, "particleRadius");

    glUniform1f(radiusLoc, m_particleRadius);

    glEnable(GL_PROGRAM_POINT_SIZE);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, numParticles);
    glBindVertexArray(0);

    glDisable(GL_PROGRAM_POINT_SIZE);

    glUniform1f(radiusLoc, -1.f);

//    glDisable(GL_BLEND); //Disable blending.
}

