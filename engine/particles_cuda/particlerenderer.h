#ifndef PARTICLERENDERER_H
#define PARTICLERENDERER_H

typedef unsigned int GLuint;
typedef unsigned int uint;

class ParticleRenderer
{
public:
    ParticleRenderer();
    ~ParticleRenderer();

    void createVAO(GLuint shader, GLuint vbo, float radius);

    void setVBO(GLuint vbo, uint numParticles);
    void render(uint numParticles);

private:
    GLuint m_shader;
    GLuint m_vbo;
    GLuint m_vao;

    float m_particleRadius;

};

#endif // PARTICLERENDERER_H
