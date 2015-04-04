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

//    void decreaseHealth(int amount);
//    int getHealth();

    void setEyeHeight(float height);

    // mouse events
    virtual void onMouseMoved(QMouseEvent *e, float deltaX, float deltaY);

    // key events
    virtual void onKeyPressed(QKeyEvent *e);
    virtual void onKeyReleased(QKeyEvent *e);

    virtual void handleCollision(Collision *col);

protected:
    ActionCamera *m_camera;
    float m_offset;

    int m_wsad;
    bool m_canJump, m_jump;

private:
    float m_eyeHeight;
};

#endif // PLAYER_H
