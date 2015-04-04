#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

#include "player.h"

class GamePlayer : public Player
{
public:
    GamePlayer(ActionCamera *camera, glm::vec3 pos);
    virtual ~GamePlayer();

    virtual void onTick(float secs);

};

#endif // GAMEPLAYER_H
