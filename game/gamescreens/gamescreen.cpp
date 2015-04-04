#include "gamescreen.h"
#include "application.h"
#include "actioncamera.h"
#include "player.h"
#include "geometriccollisionmanager.h"
#include "triangle.h"

GameScreen::GameScreen(Application *parent, int laps)
    : Screen(parent)
{
    m_parentApp->setUseCubeMap(false);

    m_oh = new ObjectHandler();

    ActionCamera *cam = new ActionCamera();
    glm::vec3 playerPos = glm::vec3(0, 2, 0);
    cam->setCenter(playerPos);

    GamePlayer player = new GamePlayer(cam, playerPos);

    m_world = new GameWorld();
    m_world->addManager(gcm);
    m_world->setPlayer(player);
//    m_world->setGravity(glm::vec3(0, -10, 0));

    setCamera(cam);
}

GameScreen::~GameScreen()
{
    delete m_oh;
    delete m_world;
}

// update and render
void GameScreen::onTick(float secs  )
{

    if (m_startTimer > 0.f)
    {
        m_startTimer -= secs;
        m_player->setCameraPos();
        return;
    }

    m_world->onTick(secs);

    m_racer1->buildPath(m_nmh);
    m_racer2->buildPath(m_nmh);

    if (m_player->checkFinishedLap())
        m_laps[0]++;
    if (m_racer1->checkFinishedLap())
        m_laps[1]++;
    if (m_racer2->checkFinishedLap())
        m_laps[2]++;

    if (m_laps[0] == m_maxLaps)
    {
        QList<glm::vec3> waypoints;
        waypoints.append(glm::vec3(  60, 1, -50));
        m_player->setWaypoints(waypoints);
    }
    if (m_laps[1] == m_maxLaps)
    {
        QList<glm::vec3> waypoints;
        waypoints.append(glm::vec3(  81, 1, -55));
        m_racer1->setWaypoints(waypoints);
    }
    if (m_laps[2] == m_maxLaps)
    {
        QList<glm::vec3> waypoints;
        waypoints.append(glm::vec3(  81, 1, -55));
        m_racer2->setWaypoints(waypoints);
    }
    if (m_outcome == 0 && (m_laps[0] > m_maxLaps ||
                           (m_laps[2] > m_maxLaps && m_laps[1] > m_maxLaps)))
    {
        if (m_laps[1] > m_laps[0] && m_laps[2] > m_laps[0])
            m_outcome = 3;
        else if (m_laps[1] > m_laps[0] || m_laps[2] > m_laps[0])
            m_outcome = 2;
        else
            m_outcome = 1;
    }
    if (m_laps[1] > m_maxLaps)
        m_racer1->setWaypoints(QList<glm::vec3>());
    if (m_laps[2] > m_maxLaps)
        m_racer2->setWaypoints(QList<glm::vec3>());
}

void GameScreen::onRender(Graphics *g)
{
    if (!m_graphicsCardDestructionMode)
    {
        g->setWorldColor(.1, .1, .1);
        g->setColor(1, 1, 1, 1, 0);

        Light light1;
        light1.type = DIRECTIONAL;
        light1.color = glm::vec3(.5f, 1, .5f);
        light1.posDir = glm::normalize(glm::vec3(-0.89f, -0.41f, -0.2f));
        light1.id = 1;

        g->addLight(light1);

        g->setTexture("");
        g->setColor(.25f, .75f, 1, 1, 0);
        glm::mat4 trans;

        if (m_drawNavMesh && m_nmh->hasObject())
        {
            trans = glm::translate(glm::mat4(), glm::vec3(0, .3f, 0));

            g->setTransparentMode(true);
            trans[3][1] += 0.01f;
            m_nmh->drawPath(trans, g, glm::vec4(1, 1, 0, .3f));
            g->setColor(1, 0, 1, .3f, 0);
            trans[3][1] += 0.01f;
            m_nmh->drawEnd(trans);
            g->setColor(0, 0, 1, .3f, 0);
            m_nmh->drawStart(trans);
            g->setColor(0, 1, 1, .3f, 0);
            m_nmh->draw(trans);

            g->setColor(0, 0, 0, 1, 0);
            g->setTransparentMode(false);
            m_nmh->drawLines(trans);

            g->setTransparentMode(true);
            g->setTexture(m_levelTexture);
            g->setColor(1, 1, 1, .7f, 0);
            m_level->draw(glm::mat4());
            g->setTransparentMode(false);
        } else
        {
            g->setAllWhite(true);
            g->setTexture(m_levelTexture, 50.f, 50.f);
            m_level->draw(glm::mat4());
            g->setAllWhite(false);
        }
        m_world->onDraw(g);

        if (m_startTimer > 0.f)
        {
            glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(0,.4f,0)) *
                    glm::scale(glm::mat4(), glm::vec3(.35f));
            g->setGraphicsMode(DRAW2D);
            g->setAllWhite(true);
            if (m_startTimer > 2.f)
                g->setTexture("three.jpg");
            else if (m_startTimer > 1.f)
                g->setTexture("two.jpg");
            else
                g->setTexture("one.jpg");
            g->drawQuad(trans);
            g->setAllWhite(false);
        }
        if (m_outcome > 0)
        {
            glm::mat4 trans = glm::scale(glm::mat4(), glm::vec3(.7f, .75f, .1f));
            g->setGraphicsMode(DRAW2D);
            g->setAllWhite(true);
            if (m_outcome == 3)
                g->setTexture("third.png");
            else if (m_outcome == 2)
                g->setTexture("second.png");
            else
                g->setTexture("first.png");
            g->drawQuad(trans);
            g->setAllWhite(false);
        }
        else
            this->render2D(g);
    }
    else
    {
        // ray cast attempt
        g->setGraphicsMode(RAY);
        g->rayAddObjects(m_world->getObjectInfo());
        g->rayAddTransparents(m_player->getWaypointInfo());
        g->rayDrawQuad();
    }
}

void GameScreen::render2D(Graphics *g)
{
    glm::mat4 trans;
    glm::vec4 look = m_camera->getLook();
    glm::vec3 pos = m_player->getPosition();
    glm::vec3 wp = m_player->getCurrentWaypoint();

    g->setGraphicsMode(DRAW2D);
    g->setTexture("");

    // no waypoints set
    if (glm::dot(wp, wp) < 0.0001f)
        return;

    glm::vec4 p = m_camera->getProjectionMatrix() *
            m_camera->getViewMatrix() * glm::vec4(wp, 1);

    p /= p.w;
    // check if waypoint is within frustum
    if (p.z >= 0 && p.z <= 1 && p.x >= -1 && p.x <= 1 && p.y >= -1 && p.y <= 1)
        return;


    glm::vec2 vec = glm::vec2(wp.x - pos.x, wp.z - pos.z);
    vec = glm::normalize(vec);
    glm::vec2 up = glm::normalize(glm::vec2(look.x, look.z));

    float dot = glm::dot(up, vec);
    float det = up.x*vec.y - up.y*vec.x;
    float angle = std::atan2(det, dot);
    trans = glm::rotate(glm::mat4(), -angle, glm::vec3(0, 0, 1));
    trans *= glm::translate(glm::mat4(), glm::vec3(0, .9f, 0));
    trans *= glm::scale(glm::mat4(), glm::vec3(.05f));

    g->drawCone(trans);
}


void GameScreen::onMouseMoved(QMouseEvent *e, float deltaX, float deltaY)
{
    if (m_startTimer < 0.f && m_outcome == 0)
        m_world->onMouseMoved(e, deltaX, deltaY);
}
void GameScreen::onKeyReleased(QKeyEvent *e )
{
    if (e->key() == Qt::Key_N)
        m_drawNavMesh = !m_drawNavMesh;
    else if (e->key() == Qt::Key_G)
        m_graphicsCardDestructionMode = !m_graphicsCardDestructionMode;

    m_world->onKeyReleased(e);
}
void GameScreen::onMouseDragged(QMouseEvent *e, float deltaX, float deltaY)
{
    if (m_startTimer < 0.f && m_outcome == 0)
        m_world->onMouseMoved(e, deltaX, deltaY);
}

void GameScreen::onKeyPressed(QKeyEvent *e)
{
    if (m_outcome > 0)
    {
        switch (e->key())
        {
        case Qt::Key_W:
        case Qt::Key_A:
        case Qt::Key_S:
        case Qt::Key_D:
        case Qt::Key_G:
        case Qt::Key_N:
            break;
        default:
            m_parentApp->popScreens(1);
            break;
        }
        return;
    }

    switch (e->key())
    {
    case Qt::Key_Back:
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
        m_parentApp->popScreens(1);
        break;
    default:
        if (m_startTimer < 0.f && m_outcome == 0)
            m_world->onKeyPressed(e);
        break;
    }
}


// unused in game
void GameScreen::onMousePressed(QMouseEvent *) {}
void GameScreen::onMouseReleased(QMouseEvent *) {}
void GameScreen::onMouseWheel(QWheelEvent *) {}
