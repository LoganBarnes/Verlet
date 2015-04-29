#ifndef ENTITY_H
#define ENTITY_H

#include "graphics.h"

#define GLM_FORCE_RADIANS
#include <gtx/transform.hpp>

class Entity;
class CollisionShape;
class Audio;

struct Collision
{
    Entity *e1;
    Entity *e2;
    CollisionShape *c1;
    CollisionShape *c2;
    glm::vec3 mtv;
    glm::vec3 impulse;
    float t;
};

class Entity
{
public:
    Entity(glm::vec3 pos);
    virtual ~Entity();

    void setPosition(glm::vec3 pos);
    glm::vec3 getPosition();
    glm::mat4 getRotation();
    QList<CollisionShape *> getCollisionShapes();
    QList<RenderShape *> getRenderShapes();

    void addRenderShape(RenderShape *rs);
    virtual void addCollisionShape(CollisionShape *cs);

    virtual void onTick(float secs);
    virtual void onDrawOpaque(Graphics *g);
    virtual void onDrawTransparent(Graphics *g);
    void drawFromList(Graphics *g, QList<RenderShape *> shapes, glm::mat4 posMat);

    virtual glm::vec3 getVelocity();
    virtual float getMass();

    virtual QList<Collision *> collides(Entity *e, float secs);
    virtual void handleCollision(Collision *col, bool resetVel) = 0;

    void bump(glm::vec3 amount);

    void setSound(Audio *audio, QString filename, bool loop);
    void playSound();

protected:
    glm::mat4 m_rotation;
    Audio* m_audio;

private:
    glm::vec3 m_pos;

    QList<RenderShape *> m_renderShapes;
    QList<RenderShape *> m_transparentShapes;
    QList<CollisionShape *> m_collisionShapes;

    int m_soundID;
    QString m_soundFile;
    bool m_loopAudio;

};

#endif // ENTITY_H
