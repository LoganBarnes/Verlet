#include "gameplayer.h"
#include "ellipsoid.h"

#define GLM_FORCE_RADIANS
#include <gtx/constants.hpp>
#include <gtx/vector_angle.hpp>

GamePlayer::GamePlayer(ActionCamera *camera, glm::vec3 pos)
    : Player(camera, pos)
{
    m_offset = 15.f;
    setMaxOffset(m_offset);
    m_camera->setOffset(m_offset);
    m_camera->setLook(glm::vec4(-5, -1, 0, 0));

    Ellipsoid *e = new Ellipsoid(glm::vec3(0, 0, 0), glm::vec3(.98f), "player");
    e->updatePos(pos);
    this->addCollisionShape(e);

    RenderShape *rs;
    rs = new RenderShape();
    rs->type = SPHERE;
    rs->color = glm::vec3(.8f);
    rs->shininess = 32.f;
    rs->transparency = 1.f;
    rs->trans = glm::scale(glm::mat4(), glm::vec3(1.9f));
    rs->inv = glm::inverse(rs->trans);
    rs->texture = "";
    rs->repeatU = 1.f;
    rs->repeatV = 1.f;
    this->addRenderShape(rs);
}


GamePlayer::~GamePlayer()
{
}

void GamePlayer::onTick(float secs)
{
    Player::onTick(secs);
}

void GamePlayer::onMouseMoved(QMouseEvent *, float deltaX, float deltaY)
{

    m_camera->yaw(deltaX / 10.f);
    m_camera->pitch(deltaY / 10.f);

}

void GamePlayer::handleCollision(Collision *col, bool resetVel)
{
    if (glm::length(col->mtv) > 0.0001f)
        col->impulse = glm::normalize(col->mtv);

    Player::handleCollision(col, resetVel);
    if (!m_tempSolid)
    {
        if (col->t > 0.0001f)
        {
            setPosition(getPosition() + col->mtv);
//            setVelocity(glm::vec3(0));
            setVelocity(getVelocity() + (col->mtv / col->t) * .05f);
//            applyForce((col->mtv / (col->t * col->t)) * getMass() * .05f);
        }
    }
}
