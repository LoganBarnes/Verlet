#ifndef COLLISIONMANAGER_H
#define COLLISIONMANAGER_H

#include "manager.h"
#include "entity.h"
#include <QList>

class CollisionManager : public Manager
{
public:
    CollisionManager();
    virtual ~CollisionManager();

    virtual void manage(World *world, float onTickSecs);

    virtual QList<Collision *> detectCollisions(World *world, float secs) = 0;
    virtual void handleCollisions(QList<Collision* > cols) = 0;
};

#endif // COLLISIONMANAGER_H
