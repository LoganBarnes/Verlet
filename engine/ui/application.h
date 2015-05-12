#ifndef APPLICATION_H
#define APPLICATION_H

#include "screen.h"

#ifdef LEAP
#include "Leap.h"
#endif

#include "graphics.h"
#include <QHash>

class Audio;
typedef unsigned int ALuint;

enum LeapGesture
{
    NO_CLICK, PINCH, GRAB, NUM_LEAP_GESTURES
};

class Application
{
public:
    Application();
    ~Application();

    void init();

    // set the current screen
    void addScreen(ScreenH *s);
    void popScreens(int num, ScreenH *s = NULL);

    // leap motion stuff for personal mac
    void useLeapMotion(bool useLeap);
    bool isUsingLeapMotion();
    void leapEnableKeyTapGesture();
    void setLeapLeftClick(LeapGesture lg);
    void setLeapRightClick(LeapGesture lg);

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
    int getWidth() { return m_width; }
    int getHeight() { return m_height; }

    // prepare graphics object for lighting
    void resetFBOs(int width, int height);

    Audio *getAudioObject();
    void playMusic(QString file);


private:

#ifdef LEAP
    void handleLeapMouseEvents();
    void checkLeapClick(Leap::Frame &frame);

    Leap::Controller *m_leapController;
    Leap::Frame m_previousLeapFrame;
#endif
    LeapGesture m_leapLeftClick;
    LeapGesture m_leapRightClick;

    QList<ScreenH *> m_screens;
    ScreenH *m_currentScreen;

    int m_decoupleKey;
    bool m_decoupleMouse;
    bool m_permanentDecouple;

    bool m_mouseDown;
    bool m_currClick;

    glm::vec3 m_mousePos; // z is 1 if decoupled, 0 otherwise
    glm::vec3 m_prevPos;

    Graphics *m_g;
    Audio *m_a;

    int m_width, m_height;

    QString m_currMusic;
    QHash<QString,ALuint> m_songs;

};

#endif // APPLICATION_H
