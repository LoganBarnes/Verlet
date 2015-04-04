#include "collisionmanager.h"

CollisionManager::CollisionManager()
{
}


CollisionManager::~CollisionManager()
{
}


void CollisionManager::manage(World *world, float onTickSecs)
{
    // collisions
    QList<Collision *> cols = detectCollisions(world, onTickSecs);
    handleCollisions(cols);

        foreach (Collision * col, cols)
            delete col;
}

//void World::detectCollisions(float secs)
//{
//    foreach(Collision *c, m_collisions)
//        delete c;
//    m_collisions.clear();

//    QList<Collision *> collisions;
//    Entity *e1, *e2;

//    int moveSize = m_movableEntities.size();
//    for (int i = 0; i < moveSize; i++)
//    {
//        e1 = m_movableEntities.value(i);

//        // check static entities
//        foreach(Entity *es, m_staticEntities)
//        {
//            collisions = es->collides(e1, secs);
//            if (collisions.size() > 0)
//                m_collisions.append(collisions);
//        }

//        // check other movable entities
//        for (int j = i + 1; j < moveSize; j++)
//        {
//            e2 = m_movableEntities.value(j);
//            collisions = e1->collides(e2, secs);
//            if (collisions.size() > 0)
//                m_collisions.append(collisions);
//        }
//    }
//}

//void World::handleCollisions()
//{
//    foreach(Collision *col, m_collisions)
//    {
//        col->e1->handleCollision(col);

//        // swap
//        Entity *tempE = col->e1;
//        col->e1 = col->e2;
//        col->e2 = tempE;
//        CollisionShape *tempS = col->c1;
//        col->c1 = col->c2;
//        col->c2 = tempS;
//        col->mtv *= -1.f;
//        col->impulse *= -1.f;

//        col->e1->handleCollision(col);
//    }

//}


