#include "particleemitter.h"
#include <cstdlib>
#include <iostream>


#define GLM_FORCE_RADIANS
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


ParticleEmitter::ParticleEmitter(glm::vec3 color, glm::vec3 velocity,
                                 glm::vec3 force, float scale, float fuzziness, float speed,
                                 unsigned maxParticles) :
                    m_maxParticles(maxParticles), m_speed(speed),
                    m_fuzziness(fuzziness), m_scale(scale), m_color(color), m_velocity(velocity),
                    m_force(force)
{
    m_particles = new Particle[maxParticles];
    resetParticles();
}

ParticleEmitter::~ParticleEmitter()
{
    if (m_particles)
    {
        delete[] m_particles;
        m_particles = 0;
    }
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
}

/**
  * You need to fill this in.
  *
  * Resets the particle at the given index to its initial state. Should reset the
  * position, direction, force, color, life, and decay of the particle.
  */
void ParticleEmitter::resetParticle(unsigned i)
{
    Particle &particle = m_particles[i];

    particle.pos = glm::vec3(0.0f);
    particle.life = 1.f;
    particle.decay = (rand() / (RAND_MAX * 1.f)) * .1475f + .0025f;
    particle.color = m_color;
    particle.force.x = m_force.x + (rand() / (RAND_MAX * 1.f)) * m_fuzziness *.06f - m_fuzziness *.03f;
    particle.force.y = m_force.y + (rand() / (RAND_MAX * 1.f)) * m_fuzziness *.06f - m_fuzziness *.03f;
    particle.force.z = m_force.z + (rand() / (RAND_MAX * 1.f)) * m_fuzziness *.06f - m_fuzziness *.03f;
    particle.dir.x = m_velocity.x + (rand() / (RAND_MAX * 1.f)) * 10.f - 5.f;
    particle.dir.y = m_velocity.y + (rand() / (RAND_MAX * 1.f)) * 10.f - 5.f;
    particle.dir.z = m_velocity.z + (rand() / (RAND_MAX * 1.f)) * 10.f - 5.f;
}

/**
  * Resets all particles in this emitter to their initial states
  */
void ParticleEmitter::resetParticles()
{
    for (int i = 0; i < m_maxParticles; i++)
        resetParticle(i);
}

/**
  * You need to fill this in.
  *
  * Performs one step of the particle simulation. Should perform all physics
  * calculations and maintain the life property of each particle.
  */
void ParticleEmitter::updateParticles()
{
    for(int i = 0; i < m_maxParticles; ++i)
    {
        Particle &particle = m_particles[i];

        particle.pos = particle.dir * m_speed;
        particle.dir += particle.force;
        particle.life -= particle.decay;
        if (particle.life < 0.f)
            resetParticle(i);
    }
}


/**
 * You need to fill this in!
 *
 * Draws each of the particles by making a call to glDrawArrays()
 * for each particle with a different ModelViewProjection matrix.
 */
void ParticleEmitter::drawParticlesVAO(GLuint shader, glm::vec3 source){
    if (!m_isInitialized){
        std::cout << "You must call initGL() before you can draw!" << std::endl;
    } else{
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glEnable(GL_BLEND);

        // Bind the VAO
        glBindVertexArray(m_vao);

        // @TODO: Render each particle
        glUniform1i(glGetUniformLocation(shader, "useTexture"), 0);

        glm::mat4 trans = glm::mat4();
        for (int i = 0; i < m_maxParticles; i++)
        {
            Particle &particle = m_particles[i];
            trans[3] = glm::vec4(particle.pos + source, 1.f);
            glUniform4f(glGetUniformLocation(shader, "color"), particle.color.r, particle.color.g, particle.color.b, sqrt(particle.life) * 2.f);
            glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(trans));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
        }

        // Unbind the VAO
        glBindVertexArray(0);

        glDisable(GL_BLEND);
    }
}

/**
 * You need to fill this in!
 *
 * Sets up a single square that will be drawn (number of Particles) times in different positions
 */
void ParticleEmitter::initGL(int vertexLocation){
    // Generate and bind VAO and VBO.
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    // @TODO: Fill in the buffer with data for the vertex positions and texture coordinates.
    float radius = .02f;
    float points[] = {
        -radius ,  radius,  radius, // 4
         radius ,  radius,  radius, // 3
        -radius , -radius,  radius, // 7
         radius , -radius,  radius, // 8

         radius , -radius, -radius, // 5
         radius ,  radius,  radius, // 3
         radius ,  radius, -radius, // 1

        -radius ,  radius,  radius, // 4
        -radius ,  radius, -radius, // 2
        -radius , -radius,  radius, // 7

        -radius , -radius, -radius, // 6
         radius , -radius, -radius, // 5
        -radius ,  radius, -radius, // 2
         radius ,  radius, -radius  // 1
    };

    glBufferData (GL_ARRAY_BUFFER, 3 * 14 * sizeof (float), &points, GL_STATIC_DRAW);

    glEnableVertexAttribArray (vertexLocation);
    glVertexAttribPointer (
                vertexLocation,
                3,
                GL_FLOAT,
                GL_FALSE,
                sizeof(GLfloat) * 3,
                (void*) 0
                );


    // Clean up -- unbind things.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m_isInitialized = true;
}
