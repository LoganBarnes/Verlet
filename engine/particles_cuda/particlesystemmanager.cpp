#include "particlesystemmanager.h"
#include <cuda_runtime.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include "particlesystem.h"
#include "particlerenderer.h"
#include "helper_math.h"
#include "world.h"
#include "shared_variables.cuh"

#define MAX_PARTICLES 20000 // (vbo size)
#define PARTICLE_RADIUS 0.05f
#define GRID_SIZE make_uint3(64, 64, 64) // 3D

#include <random>

//#include "debugprinting.h"

inline float frand()
{
    return rand() / (float) RAND_MAX;
}

ParticleSystemManager::ParticleSystemManager()
    : Manager(),
      m_particleSystem(NULL),
      m_renderer(NULL),
      m_fluidEmitterOn(false),
      m_timer(-1.f)
{
//    m_particleSystem = new ParticleSystem(PARTICLE_RADIUS, GRID_SIZE, MAX_PARTICLES, make_int3(-50), make_int3(50), 5);
//    m_particleSystem->addParticleGrid(make_int3(-1,50,-1), make_int3(1,55,1), 1.f, true);
}


ParticleSystemManager::ParticleSystemManager(GraphicsMode gm, GLuint shader)
    : Manager(gm),
      m_particleSystem(NULL),
      m_renderer(NULL),
      m_fluidEmitterOn(false),
      m_timer(-1.f)
{
    m_particleSystem = new ParticleSystem(PARTICLE_RADIUS, GRID_SIZE, MAX_PARTICLES, make_int3(-50, 0, -50), make_int3(50, 200, 50), 5);
//    m_particleSystem->addParticleGridSparse(make_int3(-0,51,-0), make_int3(1,52,1), 1.f, true);
    float4 vel = make_float4(0.f);
    float4 pos;

    for (int i = 0; i < 100; i++)
    {
        pos = make_float4(
                    frand() * 30 - 15,
                    frand() * 20 + 40,
                    frand() * 30 - 15,
                    1.f);
        m_particleSystem->addParticle(pos, vel, 1.f, 1.5f, SOLID);
    }
    m_fluidEmitterOn = true;

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

//inline float frand()
//{
//    return rand() / (float) RAND_MAX;
//}

SimParams *ParticleSystemManager::getParams()
{
    return m_particleSystem->getParams();
}

void ParticleSystemManager::manage(World *world, float onTicSecs, float , float )
{
    glm::vec3 pos = world->getPlayer()->getPosition();

    if (m_fluidEmitterOn && m_timer <= 0.f)
    {
//        m_particleSystem->addFluid(make_int3(-1,0,-1), make_int3(1,1,1), 1.f, 1.f);
        m_particleSystem->addParticle(
                    make_float4(pos.x + 40 * frand() - 20,
                                pos.y + 20 * frand() + 10,
                                pos.z + 40 * frand() - 20, 1),
                    make_float4(0), 1.f, 1.5f, SOLID);
        if (m_particleSystem->getNumParticles() > MAX_PARTICLES - 500)
            m_fluidEmitterOn = false;
        m_timer = .2f;
    }
    m_timer -= onTicSecs;

    m_particleSystem->update(onTicSecs, make_float3(pos.x, pos.y, pos.z), 1.001);
}

void ParticleSystemManager::onDraw(Graphics *g)
{
    g->setColor(.5f, .5f, .5f, 1.f, 0.f);
    m_renderer->render(m_particleSystem->getNumParticles());
}


void ParticleSystemManager::addTriangles(QList<Triangle *> *tris, glm::vec3 center, float radius)
{
    uint start = m_particleSystem->getNumTris();

    glm::vec3 a, b, c, n;
    foreach (Triangle *tri, *tris)
    {
        n = tri->normal;
        a = tri->vertices[0];
        b = tri->vertices[1];
        c = tri->vertices[2];
        m_particleSystem->addCollisionTriangle(
                    make_float3(a.x, a.y, a.z),
                    make_float3(b.x, b.y, b.z),
                    make_float3(c.x, c.y, c.z),
                    make_float3(n.x, n.y, n.z));
    }
    m_particleSystem->addCollisionTriangleGroup(start, m_particleSystem->getNumTris(), make_float3(center.x, center.y, center.z), radius);
}

void ParticleSystemManager::onKeyPressed(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_F:
        m_particleSystem->toggleFreeze();
        break;
    default:
        break;
    }
}


