#ifndef GAMESCREEN_H
#define GAMESCREEN_H

#include "screen.h"
#include "objecthandler.h"
#include "gameworld.h"
#include "ellipsoid.h"

class GamePlayer;

class GameScreen : public Screen
{
public:
    GameScreen(Application *parent, int laps);
    virtual ~GameScreen();

    // update and render
    virtual void onTick(float secs);
    virtual void onRender(Graphics *g);

    // mouse events
    virtual void onMousePressed(QMouseEvent *e);
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY);
    virtual void onMouseReleased(QMouseEvent *);

    virtual void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY);
    virtual void onMouseWheel(QWheelEvent *e);

    // key events
    virtual void onKeyPressed(QKeyEvent *e);
    virtual void onKeyReleased(QKeyEvent *e);

private:
    void render2D(Graphics *g);

    ObjectHandler *m_oh;
    GameWorld *m_world;

};

#endif // GAMESCREEN_H
