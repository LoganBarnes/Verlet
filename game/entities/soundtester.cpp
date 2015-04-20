#include "soundtester.h"

SoundTester::SoundTester(glm::vec3 pos)
    : MovableEntity(pos)
{
}

SoundTester::~SoundTester()
{
}


void SoundTester::onTick(float secs)
{
    Entity::onTick(secs);
}

void SoundTester::handleCollision(Collision *)
{

}
