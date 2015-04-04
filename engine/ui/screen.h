#ifndef SCREEN_H
#define SCREEN_H

#include <QKeyEvent>
#include "graphics.h"
#include "camera.h"

#include <iostream>

using namespace std;

class Application;

class Screen
{
public:

    Screen(Application *parent)
    {
        m_parentApp = parent;
        m_camera = new Camera();
    }

    virtual ~Screen()
    {
        delete m_camera;
    }

    // update and render
    virtual void onTick(float secs) = 0;
    virtual void onRender(Graphics *g) = 0;

    // mouse events
    virtual void onMousePressed(QMouseEvent *e) = 0;
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY) = 0;
    virtual void onMouseReleased(QMouseEvent *e) = 0;

    virtual void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY) = 0;
    virtual void onMouseWheel(QWheelEvent *e) = 0;

    // key events
    virtual void onKeyPressed(QKeyEvent *e) = 0;
    virtual void onKeyReleased(QKeyEvent *e) = 0;

    // return screen camera
    Camera *getCamera() const
    {
        return m_camera;
    }

    void setCamera(Camera *camera)
    {
        delete m_camera;
        m_camera = camera;
    }

    // resize
    virtual void onResize(int w, int h)
    {
        m_camera->setAspectRatio(w * 1.f / h);
        m_center.setX(w / 2);
        m_center.setY(h / 2);
    }

protected:
    Camera *m_camera;
    Application *m_parentApp;

    QPoint m_center;
};

#endif // SCREEN_H
