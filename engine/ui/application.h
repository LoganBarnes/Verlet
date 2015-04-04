#ifndef APPLICATION_H
#define APPLICATION_H

#include "screen.h"
//#include "Leap.h"

class Application
{
public:
    Application();
    ~Application();

    void init(Screen *initScreen);

    // set the current screen
    void addScreen(Screen *s);
    void popScreens(int num);

    // leap motion stuff for personal mac
//    void useLeapMotion(bool useLeap);
//    bool isUsingLeapMotion();
//    Leap::Frame getLeapFrame();

    // update and render
    void onTick(float secs);
    void onRender();

    // mouse events
    void onMousePressed(QMouseEvent *e);
    void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY);
    void onMouseReleased(QMouseEvent *e);

    void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY);
    void onMouseWheel(QWheelEvent *e);

    // key events
    void onKeyPressed(QKeyEvent *e);
    void onKeyReleased(QKeyEvent *e);

    // resize
    void onResize(int w, int h);

    void setUseCubeMap(bool use);
    GLuint getShader(GraphicsMode gm);

private:
    QList<Screen *> m_screens;
    Screen *m_currentScreen;

    Graphics *m_g;
    bool m_rayMode;

    int m_width, m_height;
//    Leap::Controller *m_leapController;

};

#endif // APPLICATION_H
