#ifndef APPLICATION_H
#define APPLICATION_H

#include "screen.h"
#if defined(__APPLE__) || defined(__MACOSX)
#define LEAP
#include "Leap.h"
#endif

#include "graphics.h"

class Audio;

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
    void useLeapMotion(bool useLeap);
    bool isUsingLeapMotion();
    void leapEnableKeyTapGesture();

    // update and render
    void onTick(float secs);
    void onRender();

    // mouse events
    void onMousePressed(QMouseEvent *e);
    void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY);
    void onMouseReleased(QMouseEvent *e);

    void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos);
    void onMouseWheel(QWheelEvent *e);

    // key events
    void onKeyPressed(QKeyEvent *e);
    void onKeyReleased(QKeyEvent *e);

    // resize
    void onResize(int w, int h);

    void setMouseDecoupleKey(int key);
    int getMouseDecoupleKey();
    void setMouseDecoupled(bool decouple);
    bool isMouseDecoupled();

    void setUseCubeMap(bool use);
    GLuint getShader(GraphicsMode gm);

    Audio *getAudioObject();

private:

#ifdef LEAP
    void handleLeapMouseEvents();

    Leap::Controller *m_leapController;
    Leap::Frame m_previousLeapFrame;
#endif

    QList<Screen *> m_screens;
    Screen *m_currentScreen;

    int m_decoupleKey;
    bool m_decoupleMouse;
    bool m_permanentDecouple;

    bool m_mouseDown;

    glm::vec3 m_mousePos; // z is 1 if decoupled, 0 otherwise
    glm::vec3 m_prevPos;

    Graphics *m_g;
    Audio *m_a;

    int m_width, m_height;

};

#endif // APPLICATION_H
