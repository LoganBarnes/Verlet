#include "gamescreen.h"
#include "application.h"
#include "actioncamera.h"
#include "gameplayer.h"
#include "geometriccollisionmanager.h"
#include "triangle.h"

#include <glm/gtc/constants.hpp>

#include <debugprinting.h>

GameScreen::GameScreen(Application *parent)
    : Screen(parent)
{
    m_parentApp->setUseCubeMap(false);

    m_oh = new ObjectHandler();

    ActionCamera *cam = new ActionCamera();
    glm::vec3 playerPos = glm::vec3(0, 0, 0);
    cam->setCenter(playerPos);

    GamePlayer *player = new GamePlayer(cam, playerPos);

    GeometricCollisionManager *gcm = new GeometricCollisionManager();

    m_world = new GameWorld();
    m_world->setPlayer(player);
    m_world->addManager(gcm);
//    m_world->setGravity(glm::vec3(0, -10, 0));

    setCamera(cam);



    // how lights are created (10 is the maximum in the shader for now)
    Light *light;
    for (int i = 0; i < 10; i++)
    {
        float angle = i*.2f*glm::pi<float>();

        light = new Light();
        light->id = i;
        light->type = POINT;
        light->color = glm::vec3(.5);
        light->posDir = glm::vec3(cos(angle) * 5, 10, sin(angle) * 5 - 15); // position or direction depending on light type

        cout << glm::to_string(light->posDir) << endl;
        m_tempLights.append(light);
    }
}

GameScreen::~GameScreen()
{
    delete m_oh;
    delete m_world;

    foreach (Light *l, m_tempLights)
        delete l;
}

// update and render
void GameScreen::onTick(float secs  )
{
    m_world->onTick(secs);
}

void GameScreen::onRender(Graphics *g)
{
    /////////////////graphics class basics///////////////////

    // ambient
    g->setWorldColor(.2f, .2f, .2f);

    glm::mat4 trans = glm::scale(glm::mat4(), glm::vec3(.1f));
    g->setAllWhite(true);
    foreach(Light *light, m_tempLights)
    {
        g->addLight(*light);
        g->drawSphere(glm::translate(glm::mat4(), light->posDir) * trans);
    }
    g->setAllWhite(false);

    // sphere
    trans = glm::translate(glm::mat4(), glm::vec3(-5, 0, -15));
    g->setColor(1, 0, 0, 1, 0);
    g->drawSphere(trans);

    // cone
    trans = glm::translate(glm::mat4(), glm::vec3(-3, 0, -15));
    g->setColor(1, .5, 0, 1, 0);
    g->drawCone(trans);

    // cube
    trans = glm::translate(glm::mat4(), glm::vec3(-1, 0, -15));
    g->setColor(.5, 1, 0, 1, 0);
    g->drawCube(trans);

    // cylinder
    trans = glm::translate(glm::mat4(), glm::vec3(1, 0, -15));
    g->setColor(0, 1, .3, 1, 0);
    g->drawCylinder(trans);

    // quad
    trans = glm::translate(glm::mat4(), glm::vec3(3, 0, -15));
    g->setColor(0, 0, 1, 1, 0);
    g->drawQuad(trans);

    /////////////////////////////////////////////////////////

    // draw world
    m_world->onDraw(g);
}

void GameScreen::render2D(Graphics *)
{
    // draw UI stuff;
}


void GameScreen::onMouseMoved(QMouseEvent *e, float deltaX, float deltaY)
{
    m_world->onMouseMoved(e, deltaX*.5f, deltaY*.5);
}

void GameScreen::onMouseDragged(QMouseEvent *e, float deltaX, float deltaY)
{
    m_world->onMouseMoved(e, deltaX, deltaY);
}

void GameScreen::onKeyPressed(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Backspace)
    {
        m_parentApp->popScreens(1);
        return;
    }
    m_world->onKeyPressed(e);
}

void GameScreen::onKeyReleased(QKeyEvent *e )
{
    m_world->onKeyReleased(e);
}


// unused in game
void GameScreen::onMousePressed(QMouseEvent *) {}
void GameScreen::onMouseReleased(QMouseEvent *) {}
void GameScreen::onMouseWheel(QWheelEvent *) {}
