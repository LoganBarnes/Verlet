#ifndef GAMEWORLD_H
#define GAMEWORLD_H

#include "world.h"
#include "obj.h"

class GameWorld : public World
{
public:
    GameWorld();
    virtual ~GameWorld();

    Triangle *intersectWorld(glm::vec3 p, glm::vec3 d, float *t);
    void onDraw(Graphics *g, OBJ* level);
    void drawShapes(Graphics* g, int pass, GLuint shader);
    void onKeyPressed(QKeyEvent *e);

    QList<Light*> m_tempLights;

private:
    int mode;

};

#endif // GAMEWORLD_H
