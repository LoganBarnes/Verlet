#ifndef GAMEMENU_H
#define GAMEMENU_H

#include "screen.h"
#include "button.h"

class GameMenu : public ScreenH
{
public:
    GameMenu(Application *parent);
    virtual ~GameMenu();

    // update and render
    virtual void onTick(float secs);
    virtual void onRender(Graphics *g);

    // mouse events
    virtual void onMousePressed(QMouseEvent *e);
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos);
    virtual void onMouseReleased(QMouseEvent *);

    virtual void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY, glm::vec3 pos);
    virtual void onMouseWheel(QWheelEvent *e);

    // key events
    virtual void onKeyPressed(QKeyEvent *e);
    virtual void onKeyReleased(QKeyEvent *e);

    virtual void onLeapKeyTap(glm::vec3 pos);

    virtual void onResize(int w, int h);

private:
    Button *m_startButton;
    // Button m_settingsButton;
    // Button m_controlsButton;

    glm::mat4 m_cursor;
};

#endif // GAMEMENU_H
