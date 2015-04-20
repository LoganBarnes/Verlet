#include "soundtester.h"

SoundTester::SoundTester(Audio *audio, glm::vec3 pos)
    : MovableEntity(pos)
{
//    this->setSound(audio, "kickDrumMod.wav", true);
//    this->playSound();
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
