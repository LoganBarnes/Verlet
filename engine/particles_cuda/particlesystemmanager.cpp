#include "particlesystemmanager.h"
#include <cuda_runtime.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include "particlesystem.h"
#include "particlerenderer.h"
#include "helper_math.h"

#define MAX_PARTICLES 20000 // (vbo size)
#define PARTICLE_RADIUS 0.25f
#define GRID_SIZE make_uint3(64, 64, 64) // 3D

#include <random>

//#include "debugprinting.h"

ParticleSystemManager::ParticleSystemManager()
    : Manager(),
      m_particleSystem(NULL),
      m_renderer(NULL),
      m_fluidEmitterOn(false),
      m_timer(-1.f)
{
    m_particleSystem = new ParticleSystem(PARTICLE_RADIUS, GRID_SIZE, MAX_PARTICLES, make_int3(-50, 0, -50), make_int3(50, 200, 50), 5);
    m_particleSystem->addParticleGrid(make_int3(49.f), make_int3(51.f), 1.f, true);
}


ParticleSystemManager::ParticleSystemManager(GraphicsMode gm, GLuint shader)
    : Manager(gm),
      m_particleSystem(NULL),
      m_renderer(NULL),
      m_fluidEmitterOn(false),
      m_timer(-1.f)
{
    m_particleSystem = new ParticleSystem(PARTICLE_RADIUS, GRID_SIZE, MAX_PARTICLES, make_int3(-50, 0, -50), make_int3(50, 200, 50), 5);
    m_particleSystem->addParticleGrid(make_int3(-2,45,-49), make_int3(2,49,-45), 1.f, true);
    m_renderer = new ParticleRenderer();
    m_renderer->createVAO(shader,
                          m_particleSystem->getCurrentReadBuffer(),
                          m_particleSystem->getParticleRadius());
}


ParticleSystemManager::~ParticleSystemManager()
{
    if (m_particleSystem)
        delete m_particleSystem;
    if (m_renderer)
        delete m_renderer;
}

inline float frand()
{
    return rand() / (float) RAND_MAX;
}


void ParticleSystemManager::manage(World *, float onTicSecs, float , float )
{
    if (m_fluidEmitterOn && m_timer <= 0.f)
    {
        m_particleSystem->addFluid(make_int3(-1,0,-1), make_int3(1,1,1), 1.f, 1.f, make_float3(1,0,0));
        m_timer = 0.1f;
    }
    m_timer -= onTicSecs;

    m_particleSystem->update(onTicSecs);
}

void ParticleSystemManager::onDraw(Graphics *)
{
    m_renderer->render(m_particleSystem->getNumParticles());
}


