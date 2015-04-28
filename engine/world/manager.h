#ifndef MANAGER_H
#define MANAGER_H

#include "graphics.h"
#include <QMouseEvent>
#include <QKeyEvent>

class World;
class Graphics;

class Manager
{
public:
    Manager() { m_drawable = false; }
    Manager(GraphicsMode gm) { m_gm = gm; m_drawable = true; }
    virtual ~Manager() {}

    bool isDrawable() { return m_drawable; }
    GraphicsMode getGraphicsMode() { return m_gm; }

    virtual void manage(World *world, float onTickSecs, float mouseX, float mouseY) = 0;
    virtual void onDraw(Graphics *) {}

    // mouse events
    virtual void onMousePressed(QMouseEvent *) {}
    virtual void onMouseMoved(QMouseEvent *, float, float) {}
    virtual void onMouseReleased(QMouseEvent *) {}

    virtual void onMouseDragged(QMouseEvent *, float, float) {}
    virtual void onMouseWheel(QWheelEvent *) {}

    // key events
    virtual void onKeyPressed(QKeyEvent *) {}
    virtual void onKeyReleased(QKeyEvent *) {}

private:
    GraphicsMode m_gm;
    bool m_drawable;

};

#endif // MANAGER_H
