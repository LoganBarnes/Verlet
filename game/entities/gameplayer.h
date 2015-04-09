#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

#include "player.h"

class GamePlayer : public Player
{
public:
    GamePlayer(ActionCamera *camera, glm::vec3 pos);
    virtual ~GamePlayer();

    virtual void onTick(float secs);
    virtual void onMouseMoved(QMouseEvent *, float deltaX, float deltaY);

    bool m_usingLeap; // temp hack

};

#endif // GAMEPLAYER_H
