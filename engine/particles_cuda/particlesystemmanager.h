#ifndef PARTICLESYSTEMMANAGER_H
#define PARTICLESYSTEMMANAGER_H

#include "manager.h"

class ParticleSystem;
class ParticleRenderer;

class ParticleSystemManager : public Manager
{
public:
    ParticleSystemManager();
    ParticleSystemManager(GraphicsMode gm, GLuint shader);
    virtual ~ParticleSystemManager();

    virtual void manage(World *world, float onTickSecs, float mouseX, float mouseY);
    virtual void onDraw(Graphics *g);

    void setFluidEmitter(bool emitterOn) { m_fluidEmitterOn = emitterOn; }

private:
    ParticleSystem *m_particleSystem;
    ParticleRenderer *m_renderer;

    bool m_fluidEmitterOn;
    float m_timer;

};

#endif // PARTICLESYSTEMMANAGER_H
