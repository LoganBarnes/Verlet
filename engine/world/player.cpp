#include "player.h"
#include "audio.h"

#define GLM_FORCE_RADIANS
#include <gtx/norm.hpp>
#include <iostream>
using namespace std;

Player::Player(ActionCamera *cam, glm::vec3 pos)
    : MovableEntity(pos),
      m_camera(cam),
      m_offset(0.f),
      m_maxOffset(15.f),
      m_offsetFactor(.1),
      m_wsad(0),
      m_canJump(false),
      m_jump(false),
      m_jumping(false),
      m_eyeHeight(.75f),
      m_yaw(0.f),
      m_pitch(0.f)
{
    m_camera->setOffset(m_offset);
    setUpdatePositionOnTick(false);

    setMass(1.f);
}


Player::~Player()
{
}

glm::vec4 Player::getCamEye(){
    return m_camera->getEye();
}


void Player::setEyeHeight(float height)
{
    m_eyeHeight = height;
}

void Player::onTick(float secs)
{
    glm::vec3 v = getVelocity();

    float forceAmt = 8.f;
    glm::vec3 force = glm::vec3();
    if (m_wsad & 0b1000)
        force.z += 1;
    if (m_wsad & 0b0100)
        force.z -= 1;
    if (m_wsad & 0b0010)
        force.x -= 1;
    if (m_wsad & 0b0001)
        force.x += 1;
    if (m_jump && m_canJump){
        m_canJump = false;
        m_jumping = true;
        v.y = 11.5f;
        m_jumpVel = v;
    }

    //else if (m_jump&&!m_canJump)
    //    m_jump = false;

    glm::vec4 look = m_camera->getLook();

    glm::vec3 thrust = glm::normalize(glm::vec3(look.x, 0.f, look.z)) * force.z;
    thrust += glm::normalize(glm::vec3(-look.z, 0.f, look.x)) * force.x;
    if (glm::length2(thrust) > 0.00001)
        thrust = glm::normalize(thrust) * forceAmt;
//    thrust.y = force.y;

//    glm::vec3 vel = (thrust - m_vel);
//    vel.y = 0;
    //    applyImpulse(vel);

    if(!m_jumping){
        v.x = thrust.x;
        v.z = thrust.z;
    }
    if(m_jumping){ //less horizontal displacement allows more controlled landing
        //v.x = .7*thrust.x;
        //v.z = .7*thrust.z;

        v.x=m_jumpVel.x*.96+thrust.x*.04;
        v.z=m_jumpVel.z*.96+thrust.z*.04;
        m_jumpVel = v;
    }
    setVelocity(v);
//    if (m_canJump)
//        applyForce(glm::vec3(0, 10, 0) * getMass()); // no gravity on cloth hack
    MovableEntity::onTick(secs);

    //m_canJump = false; //jump latency issue due to this being set to false before space is activated?
    m_jump = false;
}


void Player::setCameraPos()
{

    m_camera->yaw(m_yaw);
    m_camera->pitch(m_pitch);
    m_yaw = 0.f;
    m_pitch = 0.f;
    m_camera->setCenter(getPosition() + glm::vec3(0, m_eyeHeight, 0));

    if (m_audio)
        m_audio->setListener(m_camera->getEye(), getVelocity(), m_camera->getLook(), m_camera->getUp());
}


glm::vec3 Player::getEyePos()
{
    return (getPosition() + glm::vec3(0, m_eyeHeight, 0));
}

// mouse event
void Player::onMouseMoved(QMouseEvent *, float deltaX, float deltaY)
{

    cout<<"here"<<endl;
    m_yaw += deltaX / 10.f;
    m_pitch += deltaY / 10.f;
    m_camera->yaw(deltaX / 10.f);
    m_camera->pitch(deltaY / 10.f);
}

// key events
void Player::onKeyPressed(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_W:
        m_wsad |= 0b1000;
        break;
    case Qt::Key_S:
        m_wsad |= 0b0100;
        break;
    case Qt::Key_A:
        m_wsad |= 0b0010;
        break;
    case Qt::Key_D:
        m_wsad |= 0b0001;
        break;
    case Qt::Key_Space:
        m_jump = true;
        break;
    case Qt::Key_Minus:
    case Qt::Key_Underscore:
        m_offset += 1.f;
        if (m_offset > m_maxOffset)
            m_offset = m_maxOffset;
        m_camera->setOffset(m_offset);
        break;
    case Qt::Key_Plus:
    case Qt::Key_Equal:
        m_offset -= 1.f;
        if (m_offset < 0.f)
            m_offset = 0.f;
        m_camera->setOffset(m_offset);
        break;
    case Qt::Key_ParenRight:
    case Qt::Key_0:
        m_offset = 0.f;
        m_camera->setOffset(m_offset);
        break;
    case Qt::Key_ParenLeft:
    case Qt::Key_9:
        m_offset = m_maxOffset;
        m_camera->setOffset(m_offset);
        break;

    default:
        break;
    }
}

void Player::onKeyReleased(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_W:
        m_wsad &= 0b0111;
        break;
    case Qt::Key_S:
        m_wsad &= 0b1011;
        break;
    case Qt::Key_A:
        m_wsad &= 0b1101;
        break;
    case Qt::Key_D:
        m_wsad &= 0b1110;
        break;
    default:
        break;
    }
}


void Player::handleCollision(Collision *col, bool resetVel)
{
    //if (glm::dot(col->impulse, glm::vec3(0, 1, 0)) > 1)
    if(col->impulse.y>0)
    {
        m_jumping = false;
        m_canJump = true;
        if (resetVel)
        {
            glm::vec3 v = getVelocity();
            v.y = 0;
            setVelocity(v);
        }
    }

}

// unused
void Player::onMousePressed(QMouseEvent *) {}
void Player::onMouseReleased(QMouseEvent *) {}

void Player::onMouseDragged(QMouseEvent *, float, float) {}
void Player::onMouseWheel(QWheelEvent *e) {
    int scroll = e->delta();
    if((scroll>0)&&(m_offset>0))
        m_offset*=(1-m_offsetFactor);
    else if((scroll<0)&&(m_offset<m_maxOffset))
        m_offset*=(1+m_offsetFactor);
    m_camera->setOffset(m_offset);
}

void Player::useSound(Audio *audio)
{
    m_audio = audio;
    m_audio->setListener(m_camera->getEye(), getVelocity(), m_camera->getLook(), m_camera->getUp());
}


