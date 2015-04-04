#include "player.h"

#define GLM_FORCE_RADIANS
#include <glm/gtx/norm.hpp>

Player::Player(ActionCamera *cam, glm::vec3 pos)
    : MovableEntity(pos)
{
    m_camera = cam;
    m_offset = 0.f;
    m_camera->setOffset(0.f);
    m_wsad = 0;
    m_jump = false;
    m_canJump = false;

    m_eyeHeight = .75f;

    setMass(1.f);
}


Player::~Player()
{
}


void Player::setEyeHeight(float height)
{
    m_eyeHeight = height;
}

void Player::onTick(float secs)
{
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
    if (m_jump && m_canJump)
        force.y += 15.f;

    glm::vec4 look = m_camera->getLook();

    glm::vec3 thrust = glm::normalize(glm::vec3(look.x, 0.f, look.z)) * force.z;
    thrust += glm::normalize(glm::vec3(-look.z, 0.f, look.x)) * force.x;
    if (glm::length2(thrust) > 0.00001)
        thrust = glm::normalize(thrust) * forceAmt;
    thrust.y = force.y;

    glm::vec3 vel = (thrust - m_vel);
    vel.y = thrust.y;
    applyImpulse(vel);
    MovableEntity::onTick(secs);

    m_canJump = false;
}


void Player::setCameraPos()
{
    m_camera->setCenter(getPosition() + glm::vec3(0, m_eyeHeight, 0));
}


glm::vec3 Player::getEyePos()
{
    return (getPosition() + glm::vec3(0, m_eyeHeight, 0));
}


void Player::onMouseMoved(QMouseEvent *, float deltaX, float deltaY)
{
    m_camera->yaw(deltaX / 10.f);
    m_camera->pitch(deltaY / 10.f);
}

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
        if (m_offset > 15.f)
            m_offset = 15.f;
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
        m_offset = 15.f;
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
    case Qt::Key_Space:
        m_jump = false;
        break;
    default:
        break;
    }
}


void Player::handleCollision(Collision *col)
{
    if (glm::dot(col->impulse, glm::vec3(0, 1, 0)) > .5)
        m_canJump = true;

}


