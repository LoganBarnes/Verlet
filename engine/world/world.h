#ifndef WORLD_H
#define WORLD_H

#include "movableentity.h"
#include "staticentity.h"
#include "player.h"
#include "triangle.h"
#include <QList>

class Manager;

class World
{
public:
    World();
    virtual ~World();

    void addToMesh(QList<Triangle *> tris);
    void addMovableEntity(MovableEntity *me);
    void addStaticEntity(StaticEntity *se);
    bool removeMovableEntity(MovableEntity *me, bool clearMem);
    bool removeStaticEntity(StaticEntity *se, bool clearMem);

    QList<MovableEntity *> getMovableEntities();
    QList<StaticEntity *> getStaticEntities();
    QList<Triangle *> getMesh();

    void setToDeleteMovable(MovableEntity *me);

    virtual void onTick(float secs);
    virtual void onDraw(Graphics *g);

    void addManager(Manager *m);
    void setGravity(glm::vec3 gravity);
    void setPlayer(Player *player);
    Player *getPlayer();

    ObjectsInfo *getObjectInfo();

    // mouse events
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY);

    // key events
    virtual void onKeyPressed(QKeyEvent *e);
    virtual void onKeyReleased(QKeyEvent *e);

protected:
    Player *m_player;

    QList<StaticEntity *> m_staticEntities;
    QList<MovableEntity *> m_movableEntities;
    QList<Triangle *> m_mesh;

    QList<Collision *> m_collisions;
    QList<MovableEntity *> m_me2Delete;

    QList<Manager *> m_managers;
    glm::vec3 m_gravity;

};

#endif // WORLD_H
