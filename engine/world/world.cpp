#include "world.h"
#include "manager.h"
#include "obj.h"

//#include "debugprinting.h"


World::World()
{
    m_staticEntities.clear();
    m_movableEntities.clear();
    m_collisions.clear();
    m_me2Delete.clear();
    m_managers.clear();
    m_objs.clear();

    m_player = NULL;
    m_gravity = glm::vec3();
    useDeferredLighting = true;
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

void World::addObject(OBJ *obj)
{
    m_objs.append(obj);
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

QList<OBJ* > World::getObjs(){
    return m_objs;
}


QList<Triangle *> World::getMesh()
{
    return m_mesh;
}

void World::onTick(float secs, float mouseX, float mouseY)
{
    foreach(MovableEntity *me, m_me2Delete)
    {
        removeMovableEntity(me, true);
    }
    m_me2Delete.clear();

    foreach(MovableEntity *me, m_movableEntities)
    {
        me->applyForce(m_gravity * me->getMass());
    }

    // update (tick) movableEntities
    foreach(Entity *e, m_movableEntities)
    {
        e->onTick(secs);
    }

//    cout << "/t STARTING TICK" << endl;
    foreach (Manager *m, m_managers)
    {
        m->manage(this, secs, mouseX, mouseY);
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
//void World::onDraw(Graphics *g)
{
    foreach(Manager *m, m_managers)
        if (m->isDrawable())
        {
//            g->setGraphicsMode(m->getGraphicsMode());
            m->onDraw(g);
        }

//    g->setGraphicsMode(DEFAULT);

    foreach(Entity *e, m_staticEntities)
        e->onDrawOpaque(g);

    foreach(Entity *e, m_movableEntities)
        e->onDrawOpaque(g);

    foreach(OBJ *obj, m_objs)
        g->drawObject(obj, glm::mat4());

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

void World::setLights(QList<Light*> l){
    m_lights = l;
}

// mouse events
void World::onMousePressed(QMouseEvent *e)
{
    foreach (Manager *manager, m_managers)
        manager->onMousePressed(e);
    m_player->onMousePressed(e);
}

void World::onMouseMoved(QMouseEvent *e, float deltaX, float deltaY)
{
    foreach (Manager *manager, m_managers)
        manager->onMouseMoved(e, deltaX, deltaY);
    m_player->onMouseMoved(e, deltaX, deltaY);
}

void World::onMouseReleased(QMouseEvent *e)
{
    foreach (Manager *manager, m_managers)
        manager->onMouseReleased(e);
    m_player->onMouseReleased(e);
}

void World::onMouseDragged(QMouseEvent *e, float deltaX, float deltaY)
{
    foreach (Manager *manager, m_managers)
        manager->onMouseDragged(e, deltaX, deltaY);
    m_player->onMouseDragged(e, deltaX, deltaY);
}

void World::onMouseWheel(QWheelEvent *e)
{
    foreach (Manager *manager, m_managers)
        manager->onMouseWheel(e);
    m_player->onMouseWheel(e);
}

// key events
void World::onKeyPressed(QKeyEvent *e)
{
    foreach (Manager *manager, m_managers)
        manager->onKeyPressed(e);
    m_player->onKeyPressed(e);

    // toggle deferred lighting
    if(e->key() == Qt::Key_L){
        if(useDeferredLighting)
            useDeferredLighting = false;
        else
            useDeferredLighting = true;
    }
}

void World::onKeyReleased(QKeyEvent *e)
{
    foreach (Manager *manager, m_managers)
        manager->onKeyReleased(e);
    m_player->onKeyReleased(e);
}

