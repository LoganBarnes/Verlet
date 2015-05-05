#ifndef GAMEWORLD_H
#define GAMEWORLD_H

#include "world.h"
#include "obj.h"
#include "verletmanager.h"
#include "button.h"

class GameWorld : public World
{
public:
    GameWorld();
    virtual ~GameWorld();

    Triangle *intersectWorld(glm::vec3 p, glm::vec3 d, float *t);
    virtual void onDraw(Graphics *g);
    void drawShapes(Graphics* g, int pass, GLuint shader);
    void onKeyPressed(QKeyEvent *e);
    void setLights(QList<Light*> l);

    //virtual QList<OBJ* > getTerrain(){return m_islands;}
    //QList<OBJ*> m_islands;
private:
    int mode;
    bool usingFog;

};

#endif // GAMEWORLD_H
