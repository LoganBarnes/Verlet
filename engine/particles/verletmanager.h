#ifndef VERLETMANAGER_H
#define VERLETMANAGER_H

#include "manager.h"

class VerletManager : public Manager
{
public:
    VerletManager();
    virtual ~VerletManager();

    virtual void manage(World *world, float onTickSecs);
};

#endif // VERLETMANAGER_H
