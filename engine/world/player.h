#ifndef PLAYER_H
#define PLAYER_H

#include "movableentity.h"
#include "actioncamera.h"
#include <QMouseEvent>

class Player : public MovableEntity
{
public:
    Player(ActionCamera *camera, glm::vec3 pos);
    virtual ~Player();

    virtual void onTick(float secs);
//    virtual void onDrawOpaque(Graphics *g);
//    virtual void onDrawTransparent(Graphics *g);

    void setCameraPos();
    glm::vec3 getEyePos();

    Camera *getCamera() { return m_camera; }

//    void decreaseHealth(int amount);
//    int getHealth();

    void setEyeHeight(float height);
    void setMaxOffset(float maxOffset) { m_maxOffset = maxOffset; }

    // mouse events
    virtual void onMousePressed(QMouseEvent *e);
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY);
    virtual void onMouseReleased(QMouseEvent *e);

    virtual void onMouseDragged(QMouseEvent *e, float deltaX, float deltaY);
    virtual void onMouseWheel(QWheelEvent *e);

    // key events
    virtual void onKeyPressed(QKeyEvent *e);
    virtual void onKeyReleased(QKeyEvent *e);

    virtual void handleCollision(Collision *col);

    void useSound(Audio *audio);

protected:
    ActionCamera *m_camera;
    float m_offset;
    float m_maxOffset;

    int m_wsad;
    bool m_canJump, m_jump;

private:
    float m_eyeHeight;

};

#endif // PLAYER_H
