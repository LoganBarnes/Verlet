#include "gameplayer.h"
#include "ellipsoid.h"

GamePlayer::GamePlayer(ActionCamera *camera, glm::vec3 pos)
    : Player(camera, pos)
{
    m_offset = 10.f;
    m_camera->setOffset(m_offset);

    Ellipsoid *e = new Ellipsoid(glm::vec3(0, 0, 0), glm::vec3(.49f, .98f, .49f), "player");
    e->updatePos(pos);
    this->addCollisionShape(e);

    RenderShape *rs;
    rs = new RenderShape();
    rs->type = SPHERE;
    rs->color = glm::vec3(1.f, .5f, 0);
    rs->shininess = 32.f;
    rs->transparency = 1.f;
    rs->trans = glm::scale(glm::mat4(), glm::vec3(.99f, 1.98f, .99f));
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
