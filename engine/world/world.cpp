#include "world.h"
#include "manager.h"

//#include <iostream>
//using namespace std;

World::World()
{
    m_staticEntities.clear();
    m_movableEntities.clear();
    m_collisions.clear();
    m_me2Delete.clear();
    m_managers.clear();

    m_player = NULL;
    m_gravity = glm::vec3();
}

World::~World()
{
    foreach(Entity *e, m_staticEntities)
        delete e;
    foreach(Entity *e, m_movableEntities)
        delete e;
    foreach(Collision *c, m_collisions)
        delete c;
    foreach(Manager *m, m_managers)
        delete m;
    foreach(Triangle * t, m_mesh)
        delete t;
}

void World::addToMesh(QList<Triangle *> tris)
{
    m_mesh.append(tris);
}

void World::addMovableEntity(MovableEntity *me)
{
    m_movableEntities.append(me);
}

void World::addStaticEntity(StaticEntity *se)
{
    m_staticEntities.append(se);
}

bool World::removeMovableEntity(MovableEntity *me, bool clearMem)
{
    bool result = m_movableEntities.removeOne(me);
    if (clearMem)
        delete me;
    return result;
}

void World::setToDeleteMovable(MovableEntity *me)
{
    m_me2Delete.append(me);
}

bool World::removeStaticEntity(StaticEntity *se, bool clearMem)
{
    bool result = m_staticEntities.removeOne(se);
    if (clearMem)
        delete se;
    return result;
}

QList<MovableEntity *> World::getMovableEntities()
{
    return m_movableEntities;
}

QList<StaticEntity *> World::getStaticEntities()
{
    return m_staticEntities;
}

QList<Triangle *> World::getMesh()
{
    return m_mesh;
}

void World::onTick(float secs)
{
    foreach(MovableEntity *me, m_me2Delete)
    {
        removeMovableEntity(me, true);
    }
    m_me2Delete.clear();

    foreach(MovableEntity *me, m_movableEntities)
    {
        me->applyForce(m_gravity * me->getMass());
//        cout << me << endl;
    }

    // update (tick) movableEntities
    foreach(Entity *e, m_movableEntities)
    {
        e->onTick(secs);
    }

    foreach (Manager *m, m_managers) {
        m->manage(this, secs);
    }

    m_player->setCameraPos();
}

ObjectsInfo *World::getObjectInfo()
{
    ObjectsInfo *info;
    info = new ObjectsInfo();

    glm::mat4 inv;
    glm::mat4 posMat;
    QList<RenderShape*> rses;
    foreach(MovableEntity *me, m_movableEntities)
    {
        posMat = glm::translate(glm::mat4(), me->getPosition()) * me->getRotation();
        rses.clear();
        rses.append(me->getRenderShapes());
        foreach(RenderShape *rs, rses)
        {
            inv = glm::inverse( posMat * rs->trans );
            info->invs.append(inv);
            info->colors.append(glm::vec4(rs->color, rs->transparency));
            info->shapeType.append(rs->type);
        }
    }

    return info;
}

void World::onDraw(Graphics *g)
{
    foreach(Manager *m, m_managers)
        if (m->isDrawable())
        {
            g->setGraphicsMode(m->getGraphicsMode());
            m->onDraw(g);
        }

    g->setGraphicsMode(DEFAULT);

    foreach(Entity *e, m_staticEntities)
        e->onDrawOpaque(g);

    foreach(Entity *e, m_movableEntities)
        e->onDrawOpaque(g);

    foreach(Entity *e, m_staticEntities)
        e->onDrawTransparent(g);

    foreach(Entity *e, m_movableEntities)
        e->onDrawTransparent(g);
}

void World::addManager(Manager *m)
{
    m_managers.append(m);
}

Player *World::getPlayer()
{
    return m_player;
}

void World::setPlayer(Player *player)
{
    if (m_player)
        delete m_player;
    m_player = player;
    addMovableEntity(m_player);
}

void World::setGravity(glm::vec3 gravity)
{
    m_gravity = gravity;
}

// mouse events
void World::onMouseMoved(QMouseEvent *e, float deltaX, float deltaY)
{
    m_player->onMouseMoved(e, deltaX, deltaY);
}

// key events
void World::onKeyPressed(QKeyEvent *e)
{
    m_player->onKeyPressed(e);
}

void World::onKeyReleased(QKeyEvent *e)
{
    m_player->onKeyReleased(e);
}

