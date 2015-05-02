#ifndef WORLD_H
#define WORLD_H

#include "movableentity.h"
#include "staticentity.h"
#include "player.h"
#include "triangle.h"
#include <QList>

class Manager;
class OBJ;
class VerletManager;

class World
{
public:
    World();
    virtual ~World();

    void addToMesh(QList<Triangle *> tris);
    void addObject(OBJ *obj);
    void addMovableEntity(MovableEntity *me);
    void addStaticEntity(StaticEntity *se);
    bool removeMovableEntity(MovableEntity *me, bool clearMem);
    bool removeStaticEntity(StaticEntity *se, bool clearMem);

    QList<MovableEntity *> getMovableEntities();
    QList<OBJ* > getObjs();
    QList<StaticEntity *> getStaticEntities();
    QList<Triangle *> getMesh();

    void setToDeleteMovable(MovableEntity *me);

    virtual void onTick(float secs, float mouseX = 0.f, float mouseY = 0.f);

    virtual void onDraw(Graphics *g);
//    virtual void onDraw(Graphics *g);

    void addManager(Manager *m);
    void setGravity(glm::vec3 gravity);
    void setPlayer(Player *player);
    void setLights(QList<Light*> l);
    Player *getPlayer();

    ObjectsInfo *getObjectInfo();

    // mouse events
    virtual void onMousePressed(QMouseEvent *e);
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY);
    virtual void onMouseReleased(QMouseEvent *e);

    virtual void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY);
    virtual void onMouseWheel(QWheelEvent *e);

    // key events
    virtual void onKeyPressed(QKeyEvent *e);
    virtual void onKeyReleased(QKeyEvent *e);

protected:
    Player *m_player;

    QList<StaticEntity *> m_staticEntities;
    QList<MovableEntity *> m_movableEntities;
    QList<Triangle *> m_mesh;
    QList<OBJ *> m_objs;

    QList<Collision *> m_collisions;
    QList<MovableEntity *> m_me2Delete;

    QList<Manager *> m_managers;
    glm::vec3 m_gravity;
    bool useDeferredLighting;

    QList<Light*> m_lights;

    float m_timeElapsed;

};

#endif // WORLD_H
